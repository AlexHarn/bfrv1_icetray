/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file HypoFrame.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 *
 * Provides functions to create and destroy HypoFrames as unsplit hypothesis of SplitFrames
 */

#include <vector>
#include <string>
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3String.h"

#include "boost/make_shared.hpp"

#include "CoincSuite/Modules/FrameCombiner.h"
#include "icetray/I3ConditionalModule.h"

/// A I3Module that creates HypoFrames by forcefully combining all 2-permutations of SplitFrames,
/// or if a the Parameter 'Flagname' is defined, only frames of which one is holding the flag will be combined
class HypoFrameCreator : public FrameCombiner {
  SET_LOGGER("HypoFrameCreator");
private: //parameters
  /// PARAM: Name of the RecoPulseSeriesMap(Mask) to recombine
  std::string recoMapName_;
  /// PARAM: maximum time separation between any two events that can be recombined
  double maxTimeSeparation_;
  /// PARAM: name of the flag that should be recombined (a useful thing to have)
  std::string flagName_;

public:
  /// Constructor
  HypoFrameCreator(const I3Context& context);
  /// Configure
  void Configure();
  /** @brief Action: create the HypoFrames
   * loop over every combination of two SplitFrames and combine their RecoPulseSeries; name this new frame HypoFrame
   */
  void FramePacket(std::vector<I3FramePtr> &packet);
};

I3_MODULE(HypoFrameCreator);


// For NULLSPLIT or IN_ICE: Just Assume that it can be hypothesis of a double split event
/// A Module that forces recombination keys onto frames which could act like hypothesis of a DOUBLE split (e.g. 'in_ice')
class HypoFrameFaker : public FrameCombiner {
  SET_LOGGER("HypoFrameFaker");
public:
  /// Constructor
  HypoFrameFaker(const I3Context& context);
  /// Action: if there are exactly two splits found and exactly one such hypothesis frame, place the keys
  void FramePacket(std::vector<I3FramePtr> &packet);
};

I3_MODULE(HypoFrameFaker);


// For NULLSPLIT or IN_ICE: Just Assume that it can be hypothesis of a double-split event
/// A Module that wipes away CoincSuite-keys of FAKE HypoFrames and reverts them to clean (plain) P-frames
class FakeHypoFrameReverter : public I3ConditionalModule {
  SET_LOGGER("FakeHypoFrameReverter");
private: //perameters  
  // PARAM : name of the hypoframes to reverts
  std::string hypoName_;
public:
  ///constructor
  FakeHypoFrameReverter(const I3Context& context);
  ///configure
  void Configure();
  ///Action: if 2 splits are found and there is only 1 such hypothesis frame, place the keys
  void Physics(I3FramePtr frame);
};

I3_MODULE(FakeHypoFrameReverter);


//===========================IMPLEMENTATIONS====================================

using namespace CoincSuite;

//============ CLASS HypoFrameCreator ===========
//______________________________________________________________________________
HypoFrameCreator::HypoFrameCreator(const I3Context& context): FrameCombiner(context),
  recoMapName_(),
  maxTimeSeparation_(NAN),
  flagName_()
{
  AddParameter("RecoMapName","Name of the RecoPulseSeriesMap(Mask) to recombine", recoMapName_);
  AddParameter("MaxTimeSeparation", "Maximum time separation between any two events that can be recombined", maxTimeSeparation_);
  AddParameter("FlagName", "If specified only frames which hold this Flag will combined into HypoFrames", flagName_);
};

//______________________________________________________________________________
void HypoFrameCreator::Configure() {
  FrameCombiner::Configure();
  GetParameter("recoMapName", recoMapName_);
  GetParameter("MaxTimeSeparation", maxTimeSeparation_);
  GetParameter("FlagName", flagName_);

  if (recoMapName_=="")
    log_fatal("Configure Parameter 'RecoMapName'");
  if (maxTimeSeparation_<0)
    log_fatal("Parameter 'MaxTimeSeparation' has to have a positive value");
};

//______________________________________________________________________________
void HypoFrameCreator::FramePacket(std::vector<I3FramePtr> &packet) {
  log_debug("Entering FramePacket()");

  BuildFrameRegister(packet);
  
  uint NSplits = FrameRegister_.GetNumberSplitFrames();
  if (NSplits >= 2) { // only if there are at least 2 splits
    uint sub_event_index=0;
    
    for (std::vector<PosFrame>::const_iterator posframe_A=FrameRegister_.SplitFrames_.begin(); posframe_A!=FrameRegister_.SplitFrames_.end(); posframe_A++) { // loop over all frames and set common keys
      log_trace("looping splitframe A");
      const I3FramePtr& frameA = posframe_A->second;
      I3RecoPulseSeriesMapMaskConstPtr recoMapA = frameA->Get<I3RecoPulseSeriesMapMaskConstPtr>(recoMapName_);
      I3EventHeaderConstPtr eventHeaderA = frameA->Get<I3EventHeaderConstPtr>("I3EventHeader");

      for (std::vector<PosFrame>::const_iterator posframe_B=posframe_A+1; posframe_B!=FrameRegister_.SplitFrames_.end(); posframe_B++) { // loop over all frames and set common keys
        log_trace("looping splitframe B");
        const I3FramePtr& frameB = posframe_B->second;
        I3RecoPulseSeriesMapMaskConstPtr recoMapB = frameB->Get<I3RecoPulseSeriesMapMaskConstPtr>(recoMapName_);
        I3EventHeaderConstPtr eventHeaderB = frameB->Get<I3EventHeaderConstPtr>("I3EventHeader");

        if (flagName_!="") {
          if (!(frameA->Has(flagName_) || frameB->Has(flagName_))) {
            log_trace("Neither FrameA nor FrameB has a tag that recombinations should be probed");
            continue;
          }
        }

        if (!std::isnan(maxTimeSeparation_)) {
          const I3Time& start_A = eventHeaderA->GetStartTime();
          const I3Time& start_B = eventHeaderB->GetStartTime();
          const I3Time& stop_A = eventHeaderA->GetEndTime();
          const I3Time& stop_B = eventHeaderB->GetEndTime();

          const double time_separation = CoincSuite::TimeSeparation(start_A, stop_A, start_B, stop_B);

          if (time_separation>maxTimeSeparation_) {
            log_debug("Time separation (%d) between subevents too huge (>%d)", (int)(time_separation), (int)(maxTimeSeparation_));
            continue;
          }
        }
        //figure out start and end times
        const I3Time start_time=  std::min(eventHeaderA->GetStartTime(), eventHeaderB->GetStartTime());
        const I3Time end_time=  std::max(eventHeaderA->GetEndTime(), eventHeaderB->GetEndTime());

        // reserve the new Hypoframe and create tracer objects:
        // the combined RecoMap, the subevent I3EventHeader, and the CreatedFrom recombination table
        I3FramePtr hypoframeAB = boost::make_shared<I3Frame>(I3Frame::Physics);
        //NOTE the frame creation and tracking could be solved by making this an physics_services.I3Splitter,
        // just to obtain this functionality, even though this is not the intention.
        // However this will not avoid object juggling!

        I3EventHeaderPtr eventHeader = boost::make_shared<I3EventHeader>(); //get a copy
        eventHeader->SetRunID(eventHeaderA->GetRunID());
        eventHeader->SetSubRunID(eventHeaderA->GetSubRunID());
        eventHeader->SetEventID(eventHeaderA->GetEventID());
        eventHeader->SetState(I3EventHeader::State(eventHeaderA->GetState()));
        eventHeader->SetSubEventStream(hypoName_);
        eventHeader->SetSubEventID(sub_event_index);
        eventHeader->SetStartTime(start_time);
        eventHeader->SetEndTime(end_time);

        //construct the combined map, or rather the mask of this
        // this is a bigger endeavour than should be; because we have to be sure to construct the combined mask 
        // from the source of a pulse which resides in the Q-frame
        //NOTE here follows a similar implementation to what is done in CoincSuiteHelpers::CommonMaskAncestry
        I3FrameConstPtr qframe = FrameRegister_.QFrames_[0].second;
        I3RecoPulseSeriesMapMaskConstPtr recoMapMaskAB;

        std::vector<std::string> ancestryA = CoincSuite::GetMaskAncestry(frameA, recoMapName_);
        std::vector<std::string> ancestryB = CoincSuite::GetMaskAncestry(frameB, recoMapName_);
                
        if (ancestryA.size()==1 || ancestryB.size()==1)
          log_fatal("RecoPulses in the SplitFames have to be I3RecoPulseSeriesMapMasks; Cannot guess a common ancestor");
        
        std::vector<std::string> common_ancestry; //the common linage of A and B
        
        std::vector<std::string>::reverse_iterator ancestorA=ancestryA.rbegin();
        std::vector<std::string>::reverse_iterator ancestorB=ancestryB.rbegin();
        while (ancestorA != ancestryA.rend() && (*ancestorA == *ancestorB)) {
          common_ancestry.push_back(*ancestorA);
	  ancestorA++; ancestorB++;
        }
        std::reverse(common_ancestry.begin(),common_ancestry.end()); 
        
        if (common_ancestry.size() == 0)
          log_fatal("Trying to unite maps which have no common ancestor");
        
        std::string common_q_ancestor; // the first ancestor in the linage that resides in a q-frame
        
        std::vector<std::string>::reverse_iterator common_ancestor=common_ancestry.rbegin();
        while (frameA->GetStop(*common_ancestor)==I3Frame::DAQ) {
          common_q_ancestor = *common_ancestor;
	  common_ancestor++;
        }
        
        if (common_q_ancestor.empty())
          log_fatal("Trying to unite maps which have no common ancestor in the Q-frame");
        
        if (common_q_ancestor==ancestryA[1] && common_q_ancestor==ancestryB[1]) { //both maps are direct descendent from a q frame object 
          I3RecoPulseSeriesMapMaskConstPtr recoMapMaskA = frameA->Get<I3RecoPulseSeriesMapMaskConstPtr>(recoMapName_);
          I3RecoPulseSeriesMapMaskConstPtr recoMapMaskB = frameB->Get<I3RecoPulseSeriesMapMaskConstPtr>(recoMapName_);
          recoMapMaskAB = boost::make_shared<I3RecoPulseSeriesMapMask>(*recoMapMaskA | *recoMapMaskB);
        }
        else { // at least on ancestor is local to one or the other P-frame
          I3RecoPulseSeriesMapConstPtr recoMapA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
          I3RecoPulseSeriesMapConstPtr recoMapB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
          I3RecoPulseSeriesMap recoMapAB = CoincSuite::UniteRecoMaps (*recoMapA, *recoMapB);
          recoMapMaskAB = boost::make_shared<I3RecoPulseSeriesMapMask>(*qframe, common_q_ancestor, recoMapAB);
        }

        //register the subevents that created this HypoFrame
        I3MapStringVectorDoublePtr created_from = boost::make_shared<I3MapStringVectorDouble>();
        (*created_from)[splitName_] = I3VectorDouble();
        (*created_from)[splitName_].push_back(eventHeaderA->GetSubEventID());
        (*created_from)[splitName_].push_back(eventHeaderB->GetSubEventID());

        //necessary identification objects to the frame
        hypoframeAB->Put("I3EventHeader", eventHeader);
        hypoframeAB->Put(recoMapName_, recoMapMaskAB);
        hypoframeAB->Put("CS_CreatedFrom", created_from);

        //place the hypoframe into the FrameRegister_ at the end of vectors
        FrameRegister_.InsertFrame(hypoframeAB);

        sub_event_index++;
        log_debug("Spliced HypoFrame %s from SplitFrames %s + %s", FrameIDstring(hypoframeAB).c_str(), FrameIDstring(frameA).c_str(), FrameIDstring(frameB).c_str());
      }
    }
  }
  else{
    log_debug("Only one split; nothing to do here");
  }
  PushFrameRegister();
  log_debug("Leaving FramePacket()");
};


// ========== CLASS HypoFrameFaker ============
//______________________________________________________________________________
HypoFrameFaker::HypoFrameFaker(const I3Context& context):
  FrameCombiner(context) {};

void HypoFrameFaker::FramePacket(std::vector<I3FramePtr> &packet) {
  BuildFrameRegister(packet);
  
  //Fake HypoFrames can only be created if there are exactly 2 splitframes 
  if (FrameRegister_.GetNumberSplitFrames() ==2 && FrameRegister_.GetNumberHypoFrames() ==1) {
    I3FramePtr frame = FrameRegister_.HypoFrames_[0].second;

    I3MapStringVectorDoublePtr created_from(new I3MapStringVectorDouble());
    (*created_from)[splitName_] = I3VectorDouble();
    (*created_from)[splitName_].push_back(0);
    (*created_from)[splitName_].push_back(1);

    frame->Put("CS_CreatedFrom", created_from);
  }
  
  PushFrameRegister();
};


// =============== CLASS FakeHypoFrameReverter ===============
//______________________________________________________________________________
FakeHypoFrameReverter::FakeHypoFrameReverter(const I3Context& context):
  I3ConditionalModule(context),
  hypoName_()
{
  AddParameter("HypoName", "Name of the HypoFrame to compare", hypoName_);
};

void FakeHypoFrameReverter::Configure() {
  GetParameter("HypoName", hypoName_);
  if (hypoName_.empty())
    log_fatal("Configure pararmeter 'HypoName'");
}
  
void FakeHypoFrameReverter::Physics(I3FramePtr frame) {
  I3EventHeaderConstPtr eh = frame->Get<I3EventHeaderConstPtr>("I3EventHeader");
  if (eh->GetSubEventStream()==hypoName_) {
    frame->Delete("CS_CreatedFrom");
    frame->Delete("CS_RecombSuccess");
    frame->Delete("CS_Reducing");
  }
  PushFrame(frame)  ;
};
