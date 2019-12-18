#include "polyplopia/PolyplopiaUtils.h"
#include <algorithm>
#include <iterator>

#include <dataclasses/physics/I3MCTreePhysicsLibrary.hh>
#include "dataclasses/physics/I3ParticleID.h"
#include "dataclasses/physics/I3MCTreeUtils.h"
#include <sim-services/I3CombineMCPE.h>
#include <phys-services/I3Calculator.h>

#include <boost/function.hpp>

using CompareFloatingPoint::Compare;
using I3MCTreeUtils::GetBestFilter;
using I3MCTreeUtils::GetFilter;
using I3MCTreeUtils::GetBestFilterPtr;

namespace{

   /*
   * Assume that the particle is travelling to the origin
   */
  double TimeAtDetector(const I3Particle& p){ 

	  I3Position appos,appos2;
	  I3Position origin(0.,0.,0.);
	  double apdist,apdist2;

	  I3Calculator::ClosestApproachCalc(p,origin,appos2,apdist2,appos,apdist);
	  return p.GetTime() + apdist/p.GetSpeed();
  }


  bool Earliest(const I3Particle& a, const I3Particle& b){
    
    float a_time = TimeAtDetector(a);
    float b_time = TimeAtDetector(b);

    if (std::isnan(a_time) && std::isnan(b_time))
        return true; //irrelevant which we choose
    else if (std::isnan(a_time))
        return false;
    else if (std::isnan(b_time))
        return true;
    else
        return a_time < b_time;
  }

  bool Latest(const I3Particle& a, const I3Particle& b){

    float a_time = TimeAtDetector(a);
    float b_time = TimeAtDetector(b);

    if (std::isnan(a_time) && std::isnan(b_time))
        return true; //irrelevant which we choose
    else if (std::isnan(a_time))
        return false;
    else if (std::isnan(b_time))
        return true;
    else
        return a_time > b_time;
  }
  
  bool IsInIce(const I3Particle& p){return p.GetLocationType() == I3Particle::InIce; }

}

I3MCTree::optional_value
PolyplopiaUtils::GetEarliestInIce(const I3MCTree& t){
  I3MCTree::optional_value rval = I3MCTreeUtils::GetBestFilter(t, IsInIce, Earliest);
  if(!rval)
    log_fatal_stream("No suitable particle found in tree: " << t);
  return rval;
}

double
PolyplopiaUtils::GetEarliestInIceTime(const I3MCTree& t){
  return TimeAtDetector(GetEarliestInIce(t));
}

I3MCTree::optional_value
PolyplopiaUtils::GetLatestInIce(const I3MCTree& t){
  I3MCTree::optional_value rval = I3MCTreeUtils::GetBestFilter(t, IsInIce, Latest);
  if(!rval)
    log_fatal_stream("No suitable particle found in tree: " << t);
  return rval;
}

double
PolyplopiaUtils::GetLatestInIceTime(const I3MCTree& t){
  return TimeAtDetector(GetLatestInIce(t));
}

void 
PolyplopiaUtils::MergeMMCInfo(I3MMCTrackList& dest, const I3MMCTrackList& src, double timeOffset)
{
  for(I3MMCTrack mmc_entry : src){
    I3Particle new_particle(mmc_entry.GetI3Particle());
    double t_0=new_particle.GetTime();
    new_particle.SetTime(t_0+timeOffset);
    mmc_entry.SetParticle(new_particle);
    
    mmc_entry.ti+=timeOffset;
    mmc_entry.tc+=timeOffset;
    mmc_entry.tf+=timeOffset;
    dest.push_back(mmc_entry);
  }
}

void
PolyplopiaUtils::MergeMCTrees(I3MCTree& dest, const I3MCTree& src, double timeOffset)
{
  // Make a copy of tree so that times can be adjusted
  I3MCTree ctree(src);
  PolyplopiaUtils::OffsetTime(ctree,timeOffset);
  
  // merge tree into main tree
  dest.merge(ctree);
}

double
PolyplopiaUtils::GetFirstHitTime(const I3MCPESeriesMap& hitmap)
{
  double start_time = NAN;
  for(const auto om_entry : hitmap){
    for(const I3MCPE& pe : om_entry.second){
      if(pe.time >= start_time) //this will be false if start_time is NAN
        continue;
      start_time = pe.time;
    }
  }
  return start_time;
}

void 
PolyplopiaUtils::CopyWeights(I3MapStringDouble& dest, const I3MapStringDouble src) 
{
  std::copy(src.begin(),src.end(),std::inserter(dest,dest.end()));
}

void
PolyplopiaUtils::OffsetTime(I3MCTree& ctree, double offsetTime)
{
  for(I3Particle& particle : ctree)
    particle.SetTime(particle.GetTime() + offsetTime);
}

I3Frame PolyplopiaUtils::MergeFrames(I3Frame frame1, I3Frame frame2, I3Map<std::string,std::string> names, float delta_t)
{
  MergeFrames(I3FramePtr(&frame1), I3FramePtr(&frame2), names, delta_t);
  return frame1;
}

void PolyplopiaUtils::MergeFrames(I3FramePtr frame1, const I3FramePtr frame2, const I3Map<std::string,std::string> names, float delta_t)
{
  // Cache frame into vector for event merging

  double time1      = NAN;
  double time2      = NAN;

  // Iterate over cached frames and merge objects
  double offsetTime1,offsetTime2; 
  std::vector<std::string>::iterator name_iter;
  std::string mcTreeName    = names.at("MCTreeName");
  std::string mmcTrackName  = names.at("MMCTrackName");

  I3MCTreeConstPtr tree1 = I3MCTreeUtils::Get(*frame1,mcTreeName);
  I3MCTreeConstPtr tree2 = I3MCTreeUtils::Get(*frame2,mcTreeName);
  if (!(tree1 && tree2)) 
       log_fatal("Could not find I3MCTree '%s'", mcTreeName.c_str());

  // get earliest time for each event 
  time1 = PolyplopiaUtils::GetEarliestInIce(*tree1)->GetTime();
  time2 = PolyplopiaUtils::GetEarliestInIce(*tree2)->GetTime();
  if ( std::isnan(time1) || std::isnan(time2) ) {
      log_fatal("Unable to find a valid time in given events");
  }

  // this gets the internal time in event 2 to zero (-time2) and adds the delta_t 
  // for merging (no matter if its negative or positive)
  offsetTime1= -time1;
  offsetTime2= -time2+delta_t;

  // Initialize I3MCTree 
  I3MCTreePtr  mergedMCTree = I3MCTreePtr(new I3MCTree());

  // Initialize MMCInfoTree and create the first MMCList as an empty list
  I3MMCTrackListPtr mergedMMCTrack = I3MMCTrackListPtr(new I3MMCTrackList());

  // Merge trees from each of the cached frames
  log_trace("Merging '%s' I3MCTree ", mcTreeName.c_str());

  log_trace("src_id = %u, dest_id =%u", 
                    I3MCTreeUtils::GetPrimaries(*tree1)[0].GetMinorID(), 
                    I3MCTreeUtils::GetPrimaries(*tree2)[0].GetMinorID()); 

  // Make a copy of tree set new time of I3Particle 
  I3MCTreePtr ctree1(new I3MCTree(*tree1));
  I3MCTreePtr ctree2(new I3MCTree(*tree2));

  // offset time of main tree 
  OffsetTime(*ctree1,offsetTime1);

  // merge into main tree 
  MergeMCTrees(*ctree1,*ctree2, offsetTime2);

  // Merge I3MMCTrackLists 
  I3MMCTrackListConstPtr mmcList1 = frame1->Get<I3MMCTrackListConstPtr>(mmcTrackName);
  I3MMCTrackListConstPtr mmcList2 = frame2->Get<I3MMCTrackListConstPtr>(mmcTrackName);
  
  // only merge mmcList1 if it contains an actual mmclist
  // this is not the case for nue e simulation
  if(mmcList1)
  {
    MergeMMCInfo(*mergedMMCTrack, *mmcList1, offsetTime1);
  }
 
  if(mmcList2)
  {
     MergeMMCInfo(*mergedMMCTrack, *mmcList2, offsetTime2);
     if(frame1->Has(mmcTrackName)) { 
	     frame1->Delete(mmcTrackName); 
     } 
     frame1->Put(mmcTrackName,mergedMMCTrack); 
  } else {
    log_debug("Could not find I3MMCTrackList '%s'", mmcTrackName.c_str());
  }

  // merge the I3MCPESeriesMaps if present
  if (names.count("I3MCPESeriesName")) {
     std::string hitSeriesName = names.at("I3MCPESeriesName");
     I3MCPESeriesMapConstPtr hitmap1 = frame1->Get<I3MCPESeriesMapConstPtr>(hitSeriesName); 
     I3MCPESeriesMapConstPtr hitmap2 = frame2->Get<I3MCPESeriesMapConstPtr>(hitSeriesName); 

     if ( ( hitmap1 && hitmap2 ) ) { 
         log_trace("merging hiseries maps"); 
         I3MCPESeriesMapPtr  mergedHits  = I3MCPESeriesMapPtr(new I3MCPESeriesMap());
         I3ParticleIDMapPtr null_;
         MergeMCPEs(mergedHits,hitmap1,offsetTime1,null_,null_);
         MergeMCPEs(mergedHits,hitmap2,offsetTime2,null_,null_);
         frame1->Delete(hitSeriesName); 
         frame1->Put(hitSeriesName,mergedHits);
     }
  }


  // merge weights
  I3MapStringDoubleConstPtr weights2 = frame2->Get<I3MapStringDoubleConstPtr>(names.at("Weights2"));
  I3MapStringDouble mergedWeights;
 
  if (frame1->Has(names.at("Weights2")) ) { // check if frame1 already has this weightmap
     I3MapStringDoubleConstPtr weights1 = frame1->Get<I3MapStringDoubleConstPtr>(names.at("Weights2"));
     CopyWeights(mergedWeights,*weights1);
     if (weights2 && weights2->count("Weight") && mergedWeights.count("Weights"))
        mergedWeights["Weight"] *= weights2->at("Weight");
  } else if (weights2) {
     frame1->Put(names.at("Weights2"),weights2);
  }


  // Now replace object in frame
  frame1->Delete(mcTreeName);
  frame1->Put(mcTreeName,ctree1);
 
  return;
}

