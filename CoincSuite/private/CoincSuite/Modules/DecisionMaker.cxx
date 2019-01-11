/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file DecisionMaker.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#ifndef DECISIONMAKER_H
#define DECISIONMAKER_H

#include <limits>
#include <vector>
#include <string>
#include <boost/make_shared.hpp>

#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3String.h"
#include "icetray/I3Bool.h"
#include "dataclasses/I3TimeWindow.h"

#include "CoincSuite/Modules/FrameCombiner.h"


/// A class to unify all decisions of the tester modules and make a final decision, if these events should be recombined or not
class DecisionMaker : public FrameCombiner {
  SET_LOGGER("DecisionMaker");
private: //parameters
  /// PARAM: List of all decision that liked to be recombined if any of them is true
  std::vector< std::string > likeNameList_;
  /// PARAM: List of all decision that veto to be recombined if any of them is false
  std::vector< std::string > vetoNameList_;
  /// PARAM: List of all decision that are not affected by veto list
  std::vector< std::string > trueNameList_;
  /// PARAM: Name of the trigger hierarchy to recombine
  std::string trigHierName_;
  /// PARAM: Name of the RecoPulseSeriesMap to recombine
  std::string recoMapName_;
  /// PARAM: A List of RecoPulseSeriesMap which also should be recombined alongside
  std::vector<std::string> recoMapList_;
  
  /// PARAM: discard recombined frames directly
  bool discard_;
  /// PARAM: hidden parameter: unite triggerHierarchies; //FIXME needs to be implemented properly
  bool unite_th;
  
private: // bookkeeping
  ///count how many hypoframes have been recombined
  uint64_t n_YES_;
  //count how many hypoframes have NOT been recombined
  uint64_t n_NO_;
  //count how many splitframes have been recombined
  uint64_t n_RECOMBINED_;

private: //properties
  /** A most ingenious structure, whose index of recluster_ directly corresponds to the SplitFrame.SubEventID
   * and whose value is the cluster the frames belong to. Entries equal zero(==0) are not assorted to any clusters.
   * e.g.: recluster_(0,0,1,2,0,1,2) does mean that subevents 3 and 6 are forming one cluster, so do subevents 4 and 7
   */
  class ClustersRegister{
  public: //properties
    /// index is SplitFrame.SubEventID; value is cluster_index (starting at '1', '0' is no cluster)
    std::vector<uint> recluster_;
    ///current max cluster index
    uint n_max_cluster_;
    
  public: //methods
    ///constructor
    ClustersRegister(const uint a);
    ///add a new cluster of these 2 frames: add them to a previous cluster if A or B already belong to one,
    ///otherwise open a new cluster, raise the n_max_cluster_ index
    void AddFrameCluster(const uint subEID_A, const uint subEID_B);
    ///considering the formed clusters, how many frames would be removed
    uint ComputeReducedCount();
  };
  /// maybe stuff is clustering, this object handles this
  ClustersRegister clusterRegister_;

public:
  /// Constructor
  DecisionMaker(const I3Context& context);
  /// Configure function
  void Configure();
  /// Finish and report
  void Finish();
  ///ACTION: work through the HypoFrames and their RecombinationSuccess tables to derive a final decision, if we should recombine
  void FramePacket(std::vector<I3FramePtr> &packet);
  
private: //methods
  /// put all necessary keys in the split- and hypoframe
  void RecombinationMarking (I3FramePtr hypoframe,
                             I3FramePtr frameA,
                             I3FramePtr frameB);
  /// recombine frames and put them into the FrameRegister
  void RecombineFrames();
};

I3_MODULE(DecisionMaker);

#endif // DECISIONMAKER_H


//============================IMPLEMENTATIONs===================================

using namespace std;
using namespace CoincSuite;

//=========== CLASS ClusterRegister =============
DecisionMaker::ClustersRegister::ClustersRegister(const uint a):
  recluster_(a,0),
  n_max_cluster_(0)
{};

void DecisionMaker::ClustersRegister::AddFrameCluster(const uint subEID_A, const uint subEID_B) {
  log_debug("Entering AddFrameCluster()");
  log_debug("Handle subevent_id %d and %d of %d already formed clusters", (int)subEID_A, (int)subEID_B, (int)recluster_.size());

  if (recluster_[subEID_A] != 0) { // frameA is already in a cluster
    if (recluster_[subEID_B] != 0) { // frameB is already in a cluster // both are in a cluster
      if (recluster_[subEID_A] ==  recluster_[subEID_B]) { //already in the same clsuter
        log_debug("FrameA and FrameB are already in the same cluster: no action hast to be taken");
      }
      else { //found in different clusters
        log_debug("FrameA and FrameB pre-existed in different clusters: going to reunite");
        if (recluster_[subEID_B]>recluster_[subEID_A]) {
          uint change_cluster = recluster_[subEID_B];
          for (uint recluster_iter=0; recluster_iter<recluster_.size(); recluster_iter++){
            if (recluster_[recluster_iter]==change_cluster){
              recluster_[recluster_iter]=recluster_[subEID_A];
            }
          }
        }
        else{
          uint change_cluster = recluster_[subEID_A];
          for (uint recluster_iter=0; recluster_iter<recluster_.size(); recluster_iter++){
            if (recluster_[recluster_iter]==change_cluster){
              recluster_[recluster_iter]=recluster_[subEID_B];
            }
          }
        }
      }
    }
    else{ //only frame A is in a cluster
      log_debug("Only FrameB pre-existed in a cluster: Assign FrameA to that cluster");
      recluster_[subEID_A]=recluster_[subEID_B];
    }
  }
  else{ // FrameA is not in a cluster
    if (recluster_[subEID_B] != 0) { //frameB is already in a cluster; only B is in a cluster
      log_debug("Only FrameA pre-existed in a cluster: Assign FrameB to that cluster");
      recluster_[subEID_B]=recluster_[subEID_A];
    }
    else{ // both are not in any cluster
      log_debug("Neither FrameA nor FrameB were pre-exising in a cluster: Open a new cluster");
      n_max_cluster_+=1;
      recluster_[subEID_A] = n_max_cluster_;
      recluster_[subEID_B] = n_max_cluster_;
    }
  }
  log_debug("Leaving AddFrameCluster()");
};

uint DecisionMaker::ClustersRegister::ComputeReducedCount() {
  uint reduced=0;
  for (uint cluster_index=1; cluster_index <=n_max_cluster_; cluster_index++){ //loop over cluster indeces starting at 1 (human numbering)
    uint nframes_cluster=0; //count the frames under this cluster_index
    for (uint recluster_iter=0; recluster_iter<recluster_.size(); recluster_iter++){//loop over recluster_ entries
      if (recluster_[recluster_iter] == cluster_index) {
        nframes_cluster++;
      }
    }
    if (nframes_cluster>=2)
      reduced+=nframes_cluster-1; //2 frames found in a cluster reduce the count only by 1, 3 only by 2
  }
  return reduced;
};

//=========== CLASS DecisionMaker =============
DecisionMaker::DecisionMaker(const I3Context& context):
  FrameCombiner(context),
  likeNameList_(),
  vetoNameList_(),
  trueNameList_(),
  trigHierName_("I3TriggerHierarchy"),
  recoMapName_("MaskedOfflinePulses"),
  recoMapList_(),
  discard_(false),
  unite_th(false), //FIXME this should be activated as soon as UniteTriggerHierarchies is fixed
  //bookkeeping
  n_YES_(0),
  n_NO_(0),
  n_RECOMBINED_(0),
  //internals
  clusterRegister_(0)
{
  AddParameter("TriggerHierarchyName","Name of the trigger hierarchy", trigHierName_);
  AddParameter("RecoMapName","Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("LikeNameList","Names of TesterModules that like to recombine", likeNameList_);
  AddParameter("VetoNameList","Names of TesterModules that veto to recombine", vetoNameList_);
  AddParameter("TrueNameList","Names of TesterModules that will never tell wrong", trueNameList_);
  AddParameter("Discard", "Immediately discard identified AfterpulseEvents as such", discard_);
  AddParameter("RecombineRecoMaps", "A List of RecoPulseSeriesMap which also should be recombined alongside", recoMapList_);
};

void DecisionMaker::Configure() {
  FrameCombiner::Configure();
  GetParameter("TriggerHierarchyName", trigHierName_);
  GetParameter("RecoMapName", recoMapName_);
  GetParameter("LikeNameList", likeNameList_);
  GetParameter("VetoNameList", vetoNameList_);
  GetParameter("TrueNameList", trueNameList_);
  GetParameter("Discard", discard_);
  GetParameter("RecombineRecoMaps", recoMapList_);
};

void DecisionMaker::Finish() {
  FrameCombiner::Finish();
  log_notice_stream(std::endl
    <<GetName()<<" reporting: "<<std::endl
    <<"  positive Decisions   : "<<n_YES_<<std::endl
    <<"  negative Decisions   : "<<n_NO_<<std::endl
    <<"  combined SplitFrames : "<<n_RECOMBINED_<<std::endl
  );
};


void DecisionMaker::FramePacket(std::vector<I3FramePtr> &packet) {
  log_debug("Entering FramePacket()");

  BuildFrameRegister(packet);

  if (FrameRegister_.GetEffSplits()<=1) {
    log_debug("Nothing to do here; Push everything");
    PushFrameRegister();
    log_debug("Leaving FramePacket()");
    return;
  }

  // create a new empty ClusterRegister
  clusterRegister_ = ClustersRegister(GetSplitCount());
  //DANGER if the number of splitframes has unacountably increased in the meantime, this undershoots

  uint frame_index=0;
  while (frame_index < FrameRegister_.GetNumberHypoFrames()) { //loop over all hypoframes
    std::pair<uint, I3FramePtr> Event = FrameRegister_.HypoFrames_[frame_index];
    I3FramePtr hypoframe = Event.second;

    I3MapStringBoolConstPtr recombSuccessPtr=hypoframe->Get<I3MapStringBoolConstPtr>("CS_RecombSuccess");
    I3MapStringVectorDoubleConstPtr created_from = hypoframe->Get<I3MapStringVectorDoubleConstPtr>("CS_CreatedFrom");

    if (! recombSuccessPtr || ! created_from) {
      log_warn_stream("Neither "<<splitName_<<"'CS_RecombSuccess' nor 'CS_CreatedFrom' key found in HypoFrame");
    } else {
      log_debug("Found a frame that we can judge");

      log_debug("Origin of these frames: FrameA %s-%d; FrameB: %s-%d", created_from->begin()->first.c_str(), (int)(created_from->begin()->second[0]), created_from->begin()->first.c_str(), (int)(created_from->begin()->second[1]));
      I3FramePtr frameA = FindCorrIndices(splitName_, (created_from->begin()->second)[0]).second; //hypoframe is made up from this first frame
      I3FramePtr frameB = FindCorrIndices(splitName_, (created_from->begin()->second)[1]).second; //hypoframe is made up from this second frame

      bool liketorecombine = false;
      bool vetotorecombine = false;
      bool truetorecombine = false;

      log_trace("looping Like-List: all Testers which favour recombination");
      for (uint like_iter = 0; like_iter < likeNameList_.size(); like_iter++) {
        I3MapStringBool::const_iterator decisionPtr = recombSuccessPtr->find(likeNameList_[like_iter]);
        if (decisionPtr == recombSuccessPtr->end())
          log_warn("Could not find that the configured Tester-Module '%s' has been run", likeNameList_[like_iter].c_str());
        else {
          bool criteria = decisionPtr->second;
          liketorecombine = liketorecombine || criteria;
        }
      }

      log_trace("looping Veto-List: all Testers which veto recombination");
      for (uint veto_iter = 0; veto_iter < vetoNameList_.size(); veto_iter++) {
        I3MapStringBool::const_iterator decisionPtr = recombSuccessPtr->find(vetoNameList_[veto_iter]);
        if (decisionPtr == recombSuccessPtr->end())
          log_warn("Could not find that the configured Tester-Module '%s' has been run", vetoNameList_[veto_iter].c_str());
        else {
          bool criteria = decisionPtr->second;
          vetotorecombine = vetotorecombine || !criteria;
        }
      }

      log_trace("looping True-List: all Testers which force recombination regardless");
      for (uint true_iter = 0; true_iter < trueNameList_.size(); true_iter++) {
        I3MapStringBool::const_iterator decisionPtr = recombSuccessPtr->find(trueNameList_[true_iter]);
        if (decisionPtr == recombSuccessPtr->end())
          log_warn("Could not find that the configured Tester-Module '%s' has been run", trueNameList_[true_iter].c_str());
        else {
          bool criteria = decisionPtr->second;
          truetorecombine = truetorecombine || criteria;
        }
      }

      // final decision
      const bool final_decision = (liketorecombine && (not (vetotorecombine))) || truetorecombine;
      if (final_decision) {
        log_debug("Decision: Frames should be recombined");
        hypoframe->Put("CS_RECOMBINE", boost::make_shared<I3Bool>(true));
        n_YES_++;
        
        RecombinationMarking (hypoframe, frameA, frameB);

        uint subEID_A((created_from->begin()->second)[0]);
        uint subEID_B((created_from->begin()->second)[1]);

        clusterRegister_.AddFrameCluster(subEID_A, subEID_B);
      }
      else {
        log_debug("Decision: Frames should NOT be recombined");
        hypoframe->Put("CS_RECOMBINE", boost::make_shared<I3Bool>(false));
        n_NO_++;
      }
    }
    frame_index++;
  }//end of loop over frames

  //make the reduced counting
  const unsigned int reducing_n_frames = clusterRegister_.ComputeReducedCount();
  n_RECOMBINED_+= reducing_n_frames;
  
  ReduceEffSplitCount(reducing_n_frames);
  RecombineFrames(); // NOTE ACTION here the real deal is going down

  //have to postpone the discard until here
  if (discard_) {
    //search for any occurrence where an event has been recombined
    std::vector<I3FramePtr> outframes = FrameRegister_.RetrieveFrameSequence();
    BOOST_FOREACH(const I3FramePtr &outframe, outframes) {
      if (! outframe->Has(GetName())) 
        PushFrame(outframe);
    }
  }
  else
    PushFrameRegister(); //just push everything
  
  
  log_debug("Leaving FramePacket()");
};

//_________________________________________________________
void DecisionMaker::RecombinationMarking (I3FramePtr hypoframe, I3FramePtr frameA, I3FramePtr frameB) {
  log_debug("Entering RecombinationMarking()");

  I3EventHeaderConstPtr hypoEventHeader = hypoframe->Get<I3EventHeaderConstPtr>("I3EventHeader");
  if (! hypoEventHeader)
    log_fatal("Cannot find <I3EventHeader>('I3EventHeader') in hypoframe");
  
  I3MapStringVectorDoubleConstPtr created_from = hypoframe->Get<I3MapStringVectorDoubleConstPtr>("CS_CreatedFrom");
  if (! created_from)
    log_fatal("Cannot find <I3MapStringVectorDouble>('CS_CreatedFrom') in hypoframe");
  
  I3MapStringVectorDouble reducing;
  if (hypoframe->Has("CS_Reducing")) { // if there exists already a map
    I3MapStringVectorDoubleConstPtr red = hypoframe->Get<I3MapStringVectorDoubleConstPtr>("CS_Reducing");
    if (!red)
      log_fatal("The Object in the HypoFrame %s at key 'CS_Reducing' is not an <I3MapStringVectorDouble>", FrameIDstring(hypoframe).c_str());
    reducing= *red;
    hypoframe->Delete("CS_Reducing");
  }
  else //create a new one
    reducing = I3MapStringVectorDouble(); 

  reducing[splitName_].push_back((created_from->begin()->second)[0]);
  reducing[splitName_].push_back((created_from->begin()->second)[1]);

  hypoframe->Put("CS_Reducing", boost::make_shared<I3MapStringVectorDouble>(reducing));
  
  
  //put a tracer in the parent SplitFrames that they are reduced by the HypoFrame
  I3MapStringVectorDouble reduced_by_A;
  if (frameA->Has("CS_ReducedBy")) {
    I3MapStringVectorDoubleConstPtr red_by_A = frameA->Get<I3MapStringVectorDoubleConstPtr>("CS_ReducedBy");
    if (! red_by_A)
      log_fatal("The Object in the SplitFrame %s at key 'CS_ReducedBy' is not an <I3MapStringVectorDouble>", FrameIDstring(frameA).c_str());
    reduced_by_A=*red_by_A;
    frameA->Delete("CS_ReducedBy");
  }
  else
    reduced_by_A = I3MapStringVectorDouble();

  reduced_by_A[hypoName_].push_back(hypoEventHeader->GetSubEventID());

  frameA->Put("CS_ReducedBy", boost::make_shared<I3MapStringVectorDouble>(reduced_by_A));
  
  
  I3MapStringVectorDouble reduced_by_B;
  if (frameB->Has("CS_ReducedBy")) {
    I3MapStringVectorDoubleConstPtr red_by_B = frameB->Get<I3MapStringVectorDoubleConstPtr>("CS_ReducedBy");
    if (! red_by_B)
      log_fatal("The Object in the SplitFrame %s at key 'CS_ReducedBy' is not an <I3MapStringVectorDouble>", FrameIDstring(frameB).c_str());
    reduced_by_B=*red_by_B;
    frameB->Delete("CS_ReducedBy");
  }
  else
    reduced_by_B = I3MapStringVectorDouble();

  reduced_by_B[hypoName_].push_back(hypoEventHeader->GetSubEventID());

  frameB->Put("CS_ReducedBy", boost::make_shared<I3MapStringVectorDouble>(reduced_by_B));
  
  
  // put a tracer in the SplitFrames which partner they have to be reduced
  I3MapStringVectorDouble reduced_with_A;
  if (frameA->Has("CS_ReducedWith")) {
    I3MapStringVectorDoubleConstPtr red_with_A = frameA->Get<I3MapStringVectorDoubleConstPtr>("CS_ReducedWith");
    if (! red_with_A)
      log_fatal("The Object in the SplitFrame %s at key 'CS_ReducedWith' is not an <I3MapStringVectorDouble>", FrameIDstring(frameA).c_str());
    reduced_with_A=*red_with_A;
    frameA->Delete("CS_ReducedWith");
  }
  else
    reduced_with_A = I3MapStringVectorDouble();

  reduced_with_A[(created_from->begin()->first)].push_back((created_from->begin()->second)[1]);

  frameA->Put("CS_ReducedWith", boost::make_shared<I3MapStringVectorDouble>(reduced_with_A));
  
  
  I3MapStringVectorDouble reduced_with_B;
  if (frameB->Has("CS_ReducedWith")) {
    I3MapStringVectorDoubleConstPtr red_with_B = frameB->Get<I3MapStringVectorDoubleConstPtr>("CS_ReducedWith");
    if (! red_with_B)
      log_fatal("The Object in the SplitFrame %s at key 'CS_ReducedWith' is not an <I3MapStringVectorDouble>", FrameIDstring(frameB).c_str());
    reduced_with_B=*red_with_B;
    frameB->Delete("CS_ReducedWith");
  }
  else
    reduced_with_B = I3MapStringVectorDouble();

  reduced_with_B[(created_from->begin()->first)].push_back((created_from->begin()->second)[1]);

  frameB->Put("CS_ReducedWith", boost::make_shared<I3MapStringVectorDouble>(reduced_with_B));

  log_debug("Leaving RecombinationMarking()");
};

void DecisionMaker::RecombineFrames() {
  log_debug("Entering RecombineFrames()");
  if (clusterRegister_.n_max_cluster_==0) {
    log_debug("Nothing to recombine");
    return;
  }

  //BEGIN preparation phase
  //DANGER Assume that all splitframes.RecoMapMasks are created from the same Source 
  //create an empty mask, and hold the object, because we must always use bitwise operators; also hold on to a prototype I3EventHeader
  I3FramePtr qframe = FrameRegister_.QFrames_[0].second;
  I3Time qstart_time = qframe->Get<I3EventHeaderConstPtr>("I3EventHeader")->GetStartTime(); // DJB: quick hack for timewindow calculation
  I3FramePtr splitframe = FrameRegister_.SplitFrames_[0].second;
  std::vector<std::string> ancestry = CoincSuite::GetMaskAncestry(splitframe, recoMapName_); //DANGER assume all SplitFrames have a similar structure MaskAncestry

  std::string q_ancestor;
  BOOST_FOREACH(const std::string &ancestor, ancestry) {
    if (splitframe->GetStop(ancestor)==I3Frame::DAQ) {
      q_ancestor = ancestor;
      break;
    }
  }
  if (q_ancestor.empty())
    log_fatal("No Q-frame ancestor found for this RecoMap <I3RecoPulseSeriesMapMask>('%s')", recoMapName_.c_str());
  
  I3EventHeaderConstPtr splitEventHeader = splitframe->Get<I3EventHeaderConstPtr>("I3EventHeader"); //assume there is at lease one SplitFrame

  int max_SplitFrame_SubEventID = GetSplitCount()-1; //DANGER Assumes that there were no further splits added to the subevent-stream since the splitter

  I3Time start_time = I3Time(std::numeric_limits<int32_t>::max(), std::numeric_limits<int64_t>::max()); //track the event times
  I3Time end_time = I3Time(std::numeric_limits<int32_t>::min(), std::numeric_limits<int64_t>::min());
  //END preparation phase
  

  if (ancestry.size()==2) { // the mask is a direct derivative of a q-frame object: masks can be united by simple binary operators
    //start to identify clusters in the ClusterRegister: unite the RecoMaps/Masks and remember which subevents were added to it
    for (uint cluster_index=1; cluster_index <=clusterRegister_.n_max_cluster_; cluster_index++){ //loop over cluster indices, starting at 1 (human numbering)
      std::vector<I3FramePtr> frames;
      
      I3RecoPulseSeriesMapMask unite = I3RecoPulseSeriesMapMask(*qframe, q_ancestor, I3RecoPulseSeriesMap()); // empty container
      I3MapStringVectorDouble combo_from; //tracing
      I3TriggerHierarchy trighier;
      bool trigfirst=true;
      
      for (uint recluster_iter=0; recluster_iter<clusterRegister_.recluster_.size(); recluster_iter++){//loop over recluster_ entries
        if (clusterRegister_.recluster_[recluster_iter] == cluster_index) {
          I3FramePtr frame = FindCorrIndices(splitName_, recluster_iter).second;

          frames.push_back(frame);
          
          I3EventHeaderConstPtr eventH = frame->Get<I3EventHeaderConstPtr>("I3EventHeader");
          // DJB: I added this trigger hierarchy code, please check
          I3TriggerHierarchyConstPtr th = frame->Get<I3TriggerHierarchyConstPtr>(trigHierName_);
          if (unite_th) {
            if (trigfirst){
                trighier = *th; // copy
                trigfirst = false;
            } else {
                trighier = UniteTriggerHierarchies(trighier,*th);
            }
          }
          I3RecoPulseSeriesMapMaskConstPtr recoMapMask = frame->Get<I3RecoPulseSeriesMapMaskConstPtr>(recoMapName_);
          unite = (unite | *recoMapMask);
            
          start_time = std::min(start_time, eventH->GetStartTime());
          end_time = std::max(start_time, eventH->GetEndTime());

          combo_from[splitName_].push_back(recluster_iter);

          if (! frame->Has(GetName())) //mark this frame as discardable
            frame->Put(GetName(), boost::make_shared<I3Bool>(true));
        }
      }//finished working that cluster index

      if (frames.size()==0) {//FIXME quickfix; the recluster algorithm needs to be reviewed to macke this right
        log_warn("quickFix; please correct me");
        break;
      }

      //create a new recombined frame and put it at the end of the SplitFrames
      I3FramePtr comboframe = boost::make_shared<I3Frame>(I3Frame::Physics);
      max_SplitFrame_SubEventID++;
      
      I3EventHeader comboEventHeader = I3EventHeader();
      comboEventHeader.SetSubEventID(max_SplitFrame_SubEventID);
      comboEventHeader.SetStartTime(start_time);
      comboEventHeader.SetEndTime(end_time);
      comboEventHeader.SetRunID(splitEventHeader->GetRunID());
      comboEventHeader.SetEventID(splitEventHeader->GetEventID());
      comboEventHeader.SetSubRunID(splitEventHeader->GetSubRunID());
      comboEventHeader.SetSubEventStream(splitEventHeader->GetSubEventStream());
      comboEventHeader.SetState(I3EventHeader::State(splitEventHeader->GetState()));
      
  //  I3TimeWindow tw(start_time, end_time); //FIXME start_time have to be in ns since QEventHeader.start_time
      I3TimeWindow tw(start_time-qstart_time, end_time-qstart_time); // NOTE: in CoincSuite/python/coincsuite.py:createTimeWindow() a different definition is used

      if (unite_th) {
        comboframe->Put(trigHierName_, boost::make_shared<I3TriggerHierarchy>(trighier));
      }
      comboframe->Put(recoMapName_, boost::make_shared<I3RecoPulseSeriesMapMask>(unite));
      comboframe->Put(recoMapName_+"TimeRange", boost::make_shared<I3TimeWindow>(tw));
      comboframe->Put("I3EventHeader", boost::make_shared<I3EventHeader>(comboEventHeader));
      comboframe->Put("CS_ComboFrom", boost::make_shared<I3MapStringVectorDouble>(combo_from));

      BOOST_FOREACH(const std::string &key, recoMapList_) {
        comboframe->Put(key, boost::make_shared<const I3RecoPulseSeriesMapMask>(UniteRecoMaps_To_Mask(key, qframe, frames)));
      }
      
      FrameRegister_.InsertFrame(comboframe);
    }
  }
  else { // the situation is more complicated: abstract the masks to pulses, unite them and make a mask of the ancestor in the q-frame 
    //start to identify clusters in the ClusterRegister: unite the RecoMaps/Masks and remember which subevents were added to it
    for (uint cluster_index=1; cluster_index <=clusterRegister_.n_max_cluster_; cluster_index++){ //loop over cluster indices, starting at 1 (human numbering)

      std::vector<I3FramePtr> frames;
      
      I3RecoPulseSeriesMap pulse_map = I3RecoPulseSeriesMap(); // empty container
      I3MapStringVectorDouble combo_from; //tracing
      I3TriggerHierarchy trighier;
      bool trigfirst=true;
      
      for (uint recluster_iter=0; recluster_iter<clusterRegister_.recluster_.size(); recluster_iter++){//loop over recluster_ entries
        if (clusterRegister_.recluster_[recluster_iter] == cluster_index) {
          I3FramePtr frame = FindCorrIndices(splitName_, recluster_iter).second;
          
          frames.push_back(frame);
          
          I3EventHeaderConstPtr eventH = frame->Get<I3EventHeaderConstPtr>("I3EventHeader");
          // DJB: I added this trigger hierarchy code, please check (BTW: there is a lot of code copy in this file!)
          I3TriggerHierarchyConstPtr th = frame->Get<I3TriggerHierarchyConstPtr>(trigHierName_);
          if (unite_th) {
            if (trigfirst){
                trighier = *th; // copy
                trigfirst = false;
            } else {
                trighier = UniteTriggerHierarchies(trighier,*th);
            }
          }
          I3RecoPulseSeriesMapConstPtr recoMap = frame->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
          pulse_map = CoincSuite::UniteRecoMaps(pulse_map, *recoMap);

          start_time = std::min(start_time, eventH->GetStartTime());
          end_time = std::max(start_time, eventH->GetEndTime());

          combo_from[splitName_].push_back(recluster_iter);

          if (! frame->Has(GetName()))
            frame->Put(GetName(), I3BoolConstPtr(new I3Bool(true)));
        }
      }//finished working that cluster index

      if (frames.size()==0) {//FIXME quickfix; the recluster algorithm needs to be reviewed to macke this right
        log_warn("quickFix; please correct me");
        break;
      }


      //create a new recombined frame and put it at the end of the list
      I3FramePtr comboframe = boost::make_shared<I3Frame>(I3Frame::Physics);
      max_SplitFrame_SubEventID++;

      I3EventHeader comboEventHeader = I3EventHeader();
      comboEventHeader.SetSubEventID(max_SplitFrame_SubEventID);
      comboEventHeader.SetStartTime(start_time);
      comboEventHeader.SetEndTime(end_time);
      comboEventHeader.SetRunID(splitEventHeader->GetRunID());
      comboEventHeader.SetEventID(splitEventHeader->GetEventID());
      comboEventHeader.SetSubRunID(splitEventHeader->GetSubRunID());
      comboEventHeader.SetSubEventStream(splitEventHeader->GetSubEventStream());
      comboEventHeader.SetState(I3EventHeader::State(splitEventHeader->GetState()));
      
  //	I3TimeWindow tw(start_time, end_time); //FIXME start_time have to be in ns since QEventHeader.start_time
      I3TimeWindow tw(start_time-qstart_time, end_time-qstart_time); // NOTE: in CoincSuite/python/coincsuite.py:createTimeWindow() a different definition is used

      if (unite_th) {
        comboframe->Put(trigHierName_, boost::make_shared<I3TriggerHierarchy>(trighier));
      }
      comboframe->Put(recoMapName_, boost::make_shared<I3RecoPulseSeriesMapMask>(*qframe, q_ancestor, pulse_map));
      comboframe->Put(recoMapName_+"TimeRange", boost::make_shared<I3TimeWindow>(tw));
      comboframe->Put("I3EventHeader", boost::make_shared<I3EventHeader>(comboEventHeader));
      comboframe->Put("CS_ComboFrom", boost::make_shared<I3MapStringVectorDouble>(combo_from));

      BOOST_FOREACH(const std::string &key, recoMapList_) {
        comboframe->Put(key, boost::make_shared<const I3RecoPulseSeriesMapMask>(UniteRecoMaps_To_Mask(key, qframe, frames)));
      }
      
      FrameRegister_.InsertFrame(comboframe);
    }
  }
  log_debug("Leaving RecombineFrames()");
};
