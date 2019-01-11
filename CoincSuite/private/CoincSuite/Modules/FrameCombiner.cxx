/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file FrameCombiner.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#include "CoincSuite/Modules/FrameCombiner.h"

#include <algorithm>

#include <boost/make_shared.hpp>

using namespace std;
using namespace CoincSuite;

FrameCombiner::FrameCombiner(const I3Context& context):
  I3PacketModule(context),
  splitName_(""),
  hypoName_("hypoframe")
{
  AddParameter("SplitName", "Name of the Topological Splitter (subevent names)", splitName_);
  AddParameter("HypoName", "Name of the HypoFrame to compare", hypoName_);
  AddOutBox("OutBox");
};

void FrameCombiner::Configure() {
  I3PacketModule::Configure();
  GetParameter("SplitName", splitName_);
  GetParameter("HypoName", hypoName_);

  if (splitName_=="")
    log_fatal("Configure 'SplitName'");
};

void FrameCombiner::Finish() {
  I3PacketModule::Finish();
};

void FrameCombiner::FramePacket(std::vector<I3FramePtr> &packet) {
  BuildFrameRegister(packet);
  PushFrameRegister();
};

void FrameCombiner::BuildFrameRegister(std::vector<I3FramePtr> &packet) {
  FrameRegister_.BuildFrameRegister(packet, splitName_, hypoName_);
};

void FrameCombiner::PushPacket(std::vector<I3FramePtr> &packet) {
  BOOST_FOREACH(const I3FramePtr &frame, packet)
    PushFrame(frame);
};

void FrameCombiner::PushFrameRegister() {
  FramePack packet = FrameRegister_.RetrieveAllContent();
  PushPacket(packet);
};

//_________________________________________________________
std::pair<int , I3FramePtr > FrameCombiner::FindCorrIndices(const std::string& splitName, const uint subEventID) const {
  //log_debug("Entering FindCorrIndices(): %s %d", splitName.c_str(), subEventID);
  return FrameRegister_.FindCorrIndices(splitName,subEventID);
};

void FrameCombiner::ReduceEffSplitCount(const uint reduce) {
  log_debug("Entering ReduceEffSplitCount()");
  // place the corrected splitcount into the Q frame
  std::ostringstream reducedCountName;
  reducedCountName << splitName_ << "ReducedCount";
  uint ReducedValue = 0;
  if (FrameRegister_.QFrames_[0].second->Has(reducedCountName.str())) {
    ReducedValue=(FrameRegister_.QFrames_[0].second->Get<I3IntConstPtr>(reducedCountName.str()))->value;
    FrameRegister_.QFrames_[0].second->Delete(reducedCountName.str());
  }
  FrameRegister_.QFrames_[0].second->Put(reducedCountName.str(), I3IntConstPtr(new I3Int(ReducedValue+reduce)));
  return;
}

// global namespace for all helpers
namespace CoincSuite {
  //======BEGIN===== CLASS FRAMEREGISTER
  FrameRegister::FrameRegister():
    SplitCount_(0),
    ReducedCount_(0),
    SplitName_(),
    HypoName_()
    {
      AllFrames_.clear();
      QFrames_.clear();
      SplitFrames_.clear();
      HypoFrames_.clear();
      OtherFrames_.clear();
    }

  void FrameRegister::Clear() {
    AllFrames_.clear();
    QFrames_.clear();
    SplitFrames_.clear();
    HypoFrames_.clear();
    OtherFrames_.clear();
    SplitCount_=0;
    ReducedCount_=0;
    SplitName_="";
    HypoName_="";
  }

  FrameRegister::FrameRegister(std::vector<I3FramePtr> &packet, const std::string& splitName, const std::string& hypoName) {
    BuildFrameRegister(packet, splitName, hypoName);
  }

  bool FrameRegister::BuildFrameRegister(std::vector<I3FramePtr> &packet, const std::string& splitName, const std::string& hypoName) {
    log_debug("Entering BuildFrameRegister()");
    
    if (splitName.empty() || hypoName.empty()) {
      log_fatal("Fields 'splitName' and 'hypoName' cannot be empty!");
    }

    Clear();
    SplitName_ = splitName;
    HypoName_ = hypoName;
  
    for (uint frame_index = 0; frame_index < packet.size(); frame_index++) {
      I3FramePtr frame = packet[frame_index];
      AllFrames_.push_back(std::make_pair(frame_index, frame)); //add any frame to the 'All'register regardlessly

      //its the Q-Frame: try to locate special objects [splitName]SplitCount and [splitName]ReducedCount
      if (frame->GetStop() == I3Frame::DAQ) {
        QFrames_.push_back(std::make_pair(frame_index, frame));

        I3IntConstPtr sc = frame->Get<I3IntConstPtr>(splitName+"SplitCount");
        if (sc)
          SplitCount_ = sc->value;
        else {
          log_warn("Could not locate the SplitCount <I3Int>('%s'); this could lead to problems!", (splitName+"SplitCount").c_str());
          SplitCount_ = -1;
        }

        I3IntConstPtr rc = frame->Get<I3IntConstPtr>(splitName+"ReducedCount");
        if (rc) 
          ReducedCount_ = rc->value;
        else {
          log_warn("Could not locate the ReducedCount <I3Int>('%s'); this could lead to problems!", (splitName+"ReducedCount").c_str());
          ReducedCount_ = -1;
        }
      }
      // if the frame is Physics: see if its a SplitFrame or HypoFrame
      else if (frame->GetStop() == I3Frame::Physics) { 
        if (frame->Has("I3EventHeader")) {
          I3EventHeaderConstPtr eventHeader = packet[frame_index]->Get<I3EventHeaderConstPtr>("I3EventHeader");
          if (eventHeader->GetSubEventStream() == splitName) { // its either a Splitframe
            SplitFrames_.push_back(std::make_pair(frame_index, frame));
          }
          else if (eventHeader->GetSubEventStream() == hypoName) { // or a HypoFrame
            HypoFrames_.push_back(std::make_pair(frame_index, frame));
          }
          else {//or something else
            OtherFrames_.push_back(std::make_pair(frame_index, frame));
          }
        }
        else
          log_warn("Physics Frames are expected to have an 'I3EventHeader', this one doesn't: %s", CoincSuite::FrameIDstring(frame).c_str());
      }
      else { // or something else entirely
        OtherFrames_.push_back(std::make_pair(frame_index, frame));
      }
    }

    //try to account for some possible errors by the user
    if (SplitCount_ == -1) {
      log_warn_stream("Trying to recover missing SplitCount <I3Int>('"<<splitName<<"SplitCount') by guessing from number of SplitFrames!"<<endl
                    <<"Silence this warning by either creating a <I3Int>('"<<splitName<<"SplitCount')=[#SplitFrames] in the Q-frame in a previous process or let the Splitter write this variable;"<<endl
                    <<"Will create it NOW");
      SplitCount_ = GetNumberSplitFrames();
      QFrames_[0].second->Put(splitName+"SplitCount", boost::make_shared<I3Int>(SplitCount_));
    }
    if (ReducedCount_ == -1) {
      log_warn_stream("Trying to recover missing ReducedCount <I3Int>('"<<splitName<<"ReducedCount') by guessing from number of SplitFrames!"<<endl
                    <<"Silence this warning by creating a <I3Int>('"<<splitName<<"ReducedCount')=0  in the Q-frame in a previous process;"<<endl
                    <<"Will create it NOW");
      if (GetNumberSplitFrames()==0)
        ReducedCount_=0;
      else{
        int max_splitSubEventID = -1;
        for (vector<PosFrame>::const_iterator pf_iter=SplitFrames_.begin(); pf_iter!=SplitFrames_.begin(); pf_iter++) {
          I3FrameConstPtr frame = pf_iter->second;
          I3EventHeaderConstPtr eventHeader = frame->Get<I3EventHeaderConstPtr>("I3EventHeader");
          max_splitSubEventID = std::max(max_splitSubEventID, (int)eventHeader->GetSubEventID());
        }
        ReducedCount_ = max_splitSubEventID+1 - GetNumberSplitFrames(); //remeber counting of subevents starts with 0
      }
      QFrames_[0].second->Put(splitName+"ReducedCount", boost::make_shared<I3Int>(ReducedCount_));
    }
    log_debug("FrameRegister built");
    return true;
  }
  
  //____________________________________________________________________________
  bool FrameRegister::InsertFrame(I3FramePtr frame) {
    log_debug("Entering InsertFrame");
    if (frame->GetStop() == I3Frame::DAQ) { // if its the q frame
      log_error("Can not insert such DAQ-frame");
      return false;
    }
    const uint frame_index = GetNumberAllFrames();
    AllFrames_.push_back(std::make_pair(frame_index, frame)); //add any frame to the 'All'register regardlessly
    if (frame->GetStop() == I3Frame::Physics) { // if the frame is Physics
      if (frame->Has("I3EventHeader")) {
        I3EventHeaderConstPtr eventHeader = frame->Get<I3EventHeaderConstPtr>("I3EventHeader");
        if (eventHeader->GetSubEventStream() == SplitName_) { // its either a Splitframe
          SplitFrames_.push_back(std::make_pair(frame_index, frame));
        }
        else if (eventHeader->GetSubEventStream() == HypoName_) { // or a HypoFrame
          HypoFrames_.push_back(std::make_pair(frame_index, frame));
        }
      }
    }
    else { // or something else entirely
      OtherFrames_.push_back(std::make_pair(frame_index, frame));
    }
    log_debug("Frame inserted");
    return true;
  };
  
  //____________________________________________________________________________
  std::vector<I3FramePtr> FrameRegister::RetrieveAllContent() const {
    std::vector<I3FramePtr> frames;
    BOOST_FOREACH(const PosFrame &pframe, AllFrames_)
      frames.push_back(pframe.second);
    return frames;
  };
  
  //____________________________________________________________________________
  std::vector<I3FramePtr> FrameRegister::RetrieveFrameSequence(const bool qframes,
                                                            const bool splitframes,
                                                            const bool hypoframes,
                                                            const bool otherframes) const {
    std::vector<I3FramePtr> away;
    if (qframes) {
      BOOST_FOREACH(const PosFrame &posframe, QFrames_)
        away.push_back(posframe.second);
    }
    if (splitframes) {
      BOOST_FOREACH(const PosFrame &posframe, SplitFrames_)
        away.push_back(posframe.second);
    }
    if (hypoframes) {
      BOOST_FOREACH(const PosFrame &posframe, HypoFrames_)
        away.push_back(posframe.second);
    }
    if (otherframes) {
      BOOST_FOREACH(const PosFrame &posframe, OtherFrames_)
        away.push_back(posframe.second);
    }
    return away;
  };
  
  
  //____________________________________________________________________________
  bool FrameRegister::RemoveFrameAtPosition(const uint frameindex) {
    log_debug("Purging index from FrameRegister");
    
    //let's assume that the frame can be directly accessed
    std::vector<PosFrame>::iterator af_iter;
    bool found = false;
    
    if (GetNumberAllFrames()>frameindex) {
      if ((AllFrames_.begin()+frameindex)->first == frameindex) { //if that's not the case
        found = true;
        af_iter= (AllFrames_.begin()+frameindex);
      }
    }
    if (! found) {
      for (std::vector<PosFrame>::iterator a_iter=AllFrames_.begin(); a_iter!=AllFrames_.end(); a_iter++) {
        if (a_iter->first == frameindex) {
          found = true;
          af_iter=a_iter;
        }
      }
    }
    
    if (! found) {
      log_error("Can not locate this frame-index anywhere");
      return false; 
    }
    
    AllFrames_.erase(af_iter); //kill the frame
    std::vector<I3FramePtr> tmp = RetrieveAllContent();
    
    BuildFrameRegister(tmp, std::string(SplitName_), std::string(HypoName_)); //rebuild //NOTE need to make copy of internal objects

    return true;
  };
  
  //_________________________________________________________
  PosFrame FrameRegister::FindCorrIndices(const std::string& subEventStream,
                                          const uint subEventID) const {
    log_debug("Entering FindCorrIndices(): %s %d", subEventStream.c_str(), subEventID);

    if (subEventStream == "Q") //special
      return QFrames_[0];

    if (subEventStream == SplitName_ || subEventStream == HypoName_) {
      //split and HypoFrames will all have I3EventHeaders by construction
      const std::vector<PosFrame>* const search_here( (subEventStream==SplitName_) ? &SplitFrames_ : &HypoFrames_ );

      // try find the frame by assuming that the SplitFrame vector is still complete and well ordered
      if (subEventID < search_here->size()) {
        if (subEventID == CoincSuite::GetSubEventID(search_here->at(subEventID).second))
          return search_here->at(subEventID); //found it by direct access
      }
      // try to find the frame by stepping through the vector
      log_debug("More granular search");
      for (std::vector<PosFrame>::const_iterator iter=search_here->begin(); iter!=search_here->end(); ++iter) {
        if (subEventID == CoincSuite::GetSubEventID(iter->second))
          return *iter; //found it by iteration
      }
      log_warn("Could not find this frame at the proper place in the FrameRegister; this might point to trouble");
    }

    log_debug("Search in allFrames"); //out in the wild we have to check for I3EventHeader
    for (std::vector<PosFrame>::const_iterator iter=AllFrames_.begin(); iter!=AllFrames_.end(); ++iter) {
      I3EventHeaderConstPtr eh = iter->second->Get<I3EventHeaderConstPtr>("I3EventHeader");
      if (eh)
        if (subEventID == eh->GetSubEventID() && subEventStream == eh->GetSubEventStream())
          return *iter; //found it by frute force iteration
    }

    log_error("Could not find the requested frame  %s-%d", subEventStream.c_str(), subEventID);
    return std::make_pair(0, I3FramePtr((I3Frame*)(0)));
  };

  PosFrame FrameRegister::GetFrameAtPosition(const uint position) const {
    if (position > GetNumberAllFrames()-1)
      log_error("No frame at requested position");

    return AllFrames_[position];
  };
} //======END===== CLASS FRAMEREGISTER



