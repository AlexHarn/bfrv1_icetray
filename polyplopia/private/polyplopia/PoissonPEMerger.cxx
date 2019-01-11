#include "polyplopia/PoissonPEMerger.h"

#include <icetray/Utility.h>
#include <icetray/I3Int.h>
#include <icetray/I3Units.h>
#include <polyplopia/PolyplopiaUtils.h>
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/physics/I3MCTree.h>
#include <simclasses/I3Photon.h>
#include <simclasses/I3CompressedPhoton.h>
#include <simclasses/I3ParticleIDMap.hpp>
#include <sim-services/I3CombineMCPE.h>

I3_MODULE(PoissonPEMerger);

PoissonPEMerger::PoissonPEMerger(const I3Context& ctx) :
  I3Module(ctx),
  mcTreeName_("I3MCTree"),
  timeWindow_(40.0*I3Units::microsecond),
  backgroundRate_(NAN),
  base_is_background_(true),
  counter_(0)
{
  AddParameter("BaseIsBackground",
               "Whether the base simulation into which events will be merged is "
               "itself background (i.e. Corsika or MuonGun)",
               base_is_background_);

  AddParameter("CoincidentEventService",
               "Service to inject background CR events from");

  AddParameter("MCTreeName",
               "Name of I3MCTree objects to merge",
               mcTreeName_);

  AddParameter("TimeWindow",
               "Coincident event time window",
               timeWindow_);

  AddParameter("Rate", 
               "Event rate (if NaN polyplopia should get this from background file)",
               backgroundRate_);
  
  AddParameter("RandomService",
               "The name of the random service to use",
               "I3RandomService");
  
  AddParameter("MCPEsToMerge", "The name of the I3MCPESeriesMaps to merge","");
  AddParameter("PhotonsToMerge", "The name of the I3PhotonSeriesMaps to merge","");
  AddParameter("MMCTrackName", "The name of the MMCTrackList to merge (if any)","");

  AddOutBox("OutBox");
}

void PoissonPEMerger::Configure(){
  GetParameter("BaseIsBackground", base_is_background_);
  GetParameter("CoincidentEventService",backgroundService_);
  GetParameter("TimeWindow",timeWindow_);
  GetParameter("MCTreeName", mcTreeName_);
  GetParameter("Rate", backgroundRate_);
  GetParameter("RandomService", randomService_);
  GetParameter("MCPEsToMerge", mcpesName_);
  GetParameter("PhotonsToMerge", photonsName_);
  GetParameter("MMCTrackName", mmcTrackName_);
  
  if(!randomService_)
    log_fatal_stream("A random service must be specified");
  
  if(!backgroundService_)
    log_fatal("No CoincidentEventService found'");
  
  if(std::isnan(backgroundRate_))
    backgroundRate_ = backgroundService_->GetRate();
  
  if(mcpesName_.empty() && photonsName_.empty())
    log_fatal("Either MCPEsToMerge or PhotonsToMerge must be specified");
  else if(!mcpesName_.empty() && !photonsName_.empty())
    //We could actually treat both, but probably no one cares
    log_fatal("MCPEsToMerge or PhotonsToMerge cannot both be specified");
  
  if(!photonsName_.empty())
    mergeType_=MergeType::Photon;
  else if(!mcpesName_.empty())
    mergeType_=MergeType::PhotoElectron;
  
  log_info_stream("Rate=" << backgroundRate_/I3Units::hertz
                  << " Hz, Time window=" << timeWindow_/I3Units::microsecond
                  << " µs, Poisson mean=" << timeWindow_*backgroundRate_);
}

void PoissonPEMerger::DAQ(I3FramePtr frame){
  // determine how many CR muons in static window
  double mean = timeWindow_*backgroundRate_;
  int events_to_merge = -1;
  do{
    events_to_merge = randomService_->Poisson(mean);
    //avoid overcounting if primary and background are both cosmic rays
    if(base_is_background_)
      events_to_merge -= 1;
  } while(events_to_merge < 0);
  
  log_debug_stream("merging " << events_to_merge << " events");
  coincidenceHistogram_[events_to_merge]++;
  
  // == Extract information from the base event ==
  
  double base_time=NAN;
  
  const I3MCTree& mctree = frame->Get<I3MCTree>(mcTreeName_);
  //Need to copy the immutable, original tree, in case we need to make changes
  boost::shared_ptr<I3MCTree> main_tree(new I3MCTree(mctree));
  
  auto primaries = I3MCTreeUtils::GetPrimaries(mctree);
  if(!primaries.empty())
    frame->Put("PolyplopiaPrimary",I3ParticlePtr(new I3Particle(primaries.front())));
  else{
    log_debug("no primary found in base MC tree");
    frame->Put("PolyplopiaPrimary",I3ParticlePtr(new I3Particle()));
  }
  
  boost::shared_ptr<I3MCPESeriesMap> main_PEs;
  boost::shared_ptr<I3ParticleIDMap> main_PE_parents;
  boost::shared_ptr<I3PhotonSeriesMap> main_photons;
  boost::shared_ptr<I3CompressedPhotonSeriesMap> main_cphotons;
  bool compressed_photons;
  
  if(mergeType_==MergeType::PhotoElectron){
    const I3MCPESeriesMap& PEs=frame->Get<I3MCPESeriesMap>(mcpesName_);
    if(PEs.empty())
      log_error("Base event has no hit DOMs");
    base_time = PolyplopiaUtils::GetFirstHitTime(PEs);
    main_PEs.reset(new I3MCPESeriesMap(PEs));
    //check for a side table in case of compressed data
    if(frame->Has(mcpesName_+"ParticleIDMap"))
      main_PE_parents.reset(new I3ParticleIDMap(frame->Get<I3ParticleIDMap>(mcpesName_+"ParticleIDMap")));
  }
  else if(mergeType_==MergeType::Photon){
    //first try for compressed photons
    boost::shared_ptr<const I3CompressedPhotonSeriesMap> cphotons=
    frame->Get<boost::shared_ptr<const I3CompressedPhotonSeriesMap>>(photonsName_);
    if(cphotons){
      compressed_photons = true;
      if(cphotons->empty())
        log_error("Base event has no hit DOMs");
      base_time = PolyplopiaUtils::GetFirstPhotonTime<I3CompressedPhoton>(*cphotons);
      main_cphotons.reset(new I3CompressedPhotonSeriesMap(*cphotons));
    }
    else{ //try again for uncompressed photons
      compressed_photons = false;
      const I3PhotonSeriesMap& photons=frame->Get<I3PhotonSeriesMap>(photonsName_);
      if(photons.empty())
        log_error("Base event has no hit DOMs");
      base_time = PolyplopiaUtils::GetFirstPhotonTime<I3Photon>(photons);
      main_photons.reset(new I3PhotonSeriesMap(photons));
    }
  }
  else
    log_fatal("Unexpected merge type");
  log_debug_stream("Original event time: " << base_time);
  if(std::isnan(base_time)){
    log_warn("Ignoring empty event");
    return;
  }
  
  boost::shared_ptr<I3MMCTrackList> main_mmctracks;
  if(!mmcTrackName_.empty())
    main_mmctracks.reset(new I3MMCTrackList(frame->Get<I3MMCTrackList>(mmcTrackName_)));
  
  // == Merge in background events ==
  
  for(int event_i=0; event_i<events_to_merge; event_i++){
    // Get a time centered around start of the base event
    double ti = randomService_->Uniform(base_time-0.5*timeWindow_,
                                        base_time+0.5*timeWindow_);
    log_debug_stream(" Target coincident event time: " << ti);
    
    boost::shared_ptr<I3Frame> bframe = backgroundService_->GetNextFrame();
    if(!bframe)
      log_fatal("Ran out of background events");
    
    double background_time=NAN;
    
    boost::shared_ptr<const I3MCPESeriesMap> background_PEs;
    boost::shared_ptr<const I3ParticleIDMap> background_PE_parents;
    boost::shared_ptr<const I3PhotonSeriesMap> background_photons;
    boost::shared_ptr<const I3CompressedPhotonSeriesMap> background_cphotons;
    
    if(mergeType_==MergeType::PhotoElectron){
      background_PEs=bframe->Get<boost::shared_ptr<const I3MCPESeriesMap>>(mcpesName_);
      if(!background_PEs)
        log_fatal_stream("MCPEs named \"" << mcpesName_ << "\" not found in background event");
      if(background_PEs->empty())
        log_error("Background event has no hit DOMs");
      background_time = PolyplopiaUtils::GetFirstHitTime(*background_PEs);
      //check for a side table in case of compressed data
      if(bframe->Has(mcpesName_+"ParticleIDMap"))
        background_PE_parents=bframe->Get<boost::shared_ptr<const I3ParticleIDMap>>(mcpesName_+"ParticleIDMap");
    }
    else if(mergeType_==MergeType::Photon){
      if(compressed_photons){
        background_cphotons=bframe->Get<boost::shared_ptr<const I3CompressedPhotonSeriesMap>>(photonsName_);
        if(!background_cphotons)
          log_fatal_stream("Compressed photons named \"" << photonsName_ << "\" not found in background event");
        if(background_cphotons->empty())
          log_error("Background event has no hit DOMs");
        background_time = PolyplopiaUtils::GetFirstPhotonTime<I3CompressedPhoton>(*background_cphotons);
      }
      else{
        background_photons=bframe->Get<boost::shared_ptr<const I3PhotonSeriesMap>>(photonsName_);
        if(!background_photons)
          log_fatal_stream("Photons named \"" << photonsName_ << "\" not found in background event");
        if(background_photons->empty())
          log_error("Background event has no hit DOMs");
        background_time = PolyplopiaUtils::GetFirstPhotonTime<I3Photon>(*background_photons);
      }
    }
    else
      log_fatal("Unexpected merge type");
    log_debug_stream(" Original coincident event time: " << background_time);
    if(std::isnan(background_time)){
      log_warn("Skipping invisible background event");
      event_i--;
      continue;
    }
    
    //start merging, making sure to include the time shift
    if(bframe->Has(mcTreeName_)){ //MCTree
      const I3MCTree& background_tree = bframe->Get<I3MCTree>(mcTreeName_);
      PolyplopiaUtils::MergeMCTrees(*main_tree,background_tree,ti-background_time);
    }
    //MMC info
    if(!mmcTrackName_.empty() && bframe->Has(mmcTrackName_)){
      const I3MMCTrackList& background_mmctracks=bframe->Get<I3MMCTrackList>(mmcTrackName_);
      PolyplopiaUtils::MergeMMCInfo(*main_mmctracks, background_mmctracks, ti-background_time);
    }
    //PE/Photons
    if(mergeType_==MergeType::PhotoElectron)
      main_PE_parents = MergeMCPEs(main_PEs,background_PEs,ti-background_time,main_PE_parents,background_PE_parents);
    else if(mergeType_==MergeType::Photon){
      if(compressed_photons)
        PolyplopiaUtils::MergePhotons<I3CompressedPhoton>(*main_cphotons,*background_cphotons,ti-background_time);
      else
        PolyplopiaUtils::MergePhotons<I3Photon>(*main_photons,*background_photons,ti-background_time);
    }
  }
  
  // == Dump everything back in the frame ==
  
  //frame->Replace(mcTreeName_,main_tree);
  frame->Delete(mcTreeName_);
  frame->Put(mcTreeName_,main_tree);
  if(!mmcTrackName_.empty()){
    //frame->Replace(mmcTrackName_,main_mmctracks);
    frame->Delete(mmcTrackName_);
    frame->Put(mmcTrackName_,main_mmctracks);
  }
  if(mergeType_==MergeType::PhotoElectron){
    //frame->Replace(mcpesName_,main_PEs);
    frame->Delete(mcpesName_);
    frame->Put(mcpesName_,main_PEs);
    if(main_PE_parents){
      //frame->Replace(mcpesName_+"ParticleIDMap",main_PE_parents);
      frame->Delete(mcpesName_+"ParticleIDMap");
      frame->Put(mcpesName_+"ParticleIDMap",main_PE_parents);
    }
  }
  else if(mergeType_==MergeType::Photon){
    if(compressed_photons){
      //frame->Replace(photonsName_,main_cphotons);
      frame->Delete(photonsName_);
      frame->Put(photonsName_,main_cphotons);
    }
    else{
      //frame->Replace(photonsName_,main_photons);
      frame->Delete(photonsName_);
      frame->Put(photonsName_,main_photons);
    }
  }
  frame->Put("PolyplopiaCount",boost::make_shared<I3Int>(events_to_merge));
  //TODO: should we add "DiplopliaWeight"->1 to CorsikaWeightMap if it exists?
  
  PushFrame(frame);
  counter_++;
}

void PoissonPEMerger::Finish(){
  log_info("Coincidence multiplicities:");
  for(const auto& freq : coincidenceHistogram_)
    log_info_stream(freq.first << " coincidences: " << freq.second);
  log_info_stream("Processed " << counter_ << " events");
}
