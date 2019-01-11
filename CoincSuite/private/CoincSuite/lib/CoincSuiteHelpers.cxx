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

#include "CoincSuite/lib/CoincSuiteHelpers.h"

#include <algorithm>

//============= NAMESPACE CoincSuite ============
namespace CoincSuite {

  //____________________________________________________________________________
  std::string FrameIDstring (const I3FrameConstPtr frame) {
    I3EventHeaderConstPtr eh = frame->Get<I3EventHeaderConstPtr>("I3EventHeader");
    std::ostringstream oss;
    if (frame->GetStop() == I3Frame::Geometry)
      {oss << "Geometry";}
    else if (frame->GetStop() == I3Frame::Calibration)
      {oss << "Calibration";}
    else if (frame->GetStop() == I3Frame::DetectorStatus)
      {oss << "DetectorStatus";}
    else if (frame->GetStop() == I3Frame::DAQ)
      {oss << "DAQ";}
    else if (frame->GetStop() ==I3Frame::Physics)
      {oss << "Physics";}
    else
      {oss << frame->GetStop();}
     
    if (eh) {
      oss <<"::"<<eh->GetRunID()<<"/"<<eh->GetEventID()<<"_"<<eh->GetSubEventStream()<<"/"<<eh->GetSubEventID(); 
    }

    return oss.str();
  } 

  //____________________________________________________________________________
  I3RecoPulseSeriesMap UniteRecoMaps (const I3RecoPulseSeriesMap& mapA,
                                      const I3RecoPulseSeriesMap& mapB) {
    I3RecoPulseSeriesMap new_map;
    new_map = mapA; //first make a plain copy
    BOOST_FOREACH (I3RecoPulseSeriesMap::value_type omkey_pulses, mapB) { //each new DOM
      BOOST_FOREACH (I3RecoPulseSeries::value_type pulse, omkey_pulses.second) {//each new Pulse
        new_map[omkey_pulses.first].push_back(pulse); //just insert it into the plain copy
      }
    }
    //now sort the map we have just created
    BOOST_FOREACH (I3RecoPulseSeriesMap::value_type omkey_pulses, new_map) { //each DOM
      sort(omkey_pulses.second.begin(), omkey_pulses.second.end(), RecoPulse_timeorder); //sort it
    }
    return new_map;
  };
  
  //____________________________________________________________________________
  I3RecoPulseSeriesMapMask UniteRecoMaps_To_Mask(const std::string &key,
                                                 const I3FramePtr qframe,
                                                 const std::vector<I3FramePtr> &frames) {
    std::vector<std::string> ancestry = GetMaskAncestry(frames.at(0), key);

    std::string q_ancestor;
    BOOST_FOREACH(const std::string &ancestor, ancestry) {
      if (frames.at(0)->GetStop(ancestor)==I3Frame::DAQ) {
        q_ancestor = ancestor;
        break;
      }
    }
    if (q_ancestor.empty())
      log_fatal("No Q-frame ancestor found for this RecoMap <I3RecoPulseSeriesMapMask>('%s')", key.c_str());
    
    //begin here to act on it
    if (ancestry.size()==2) { // the mask is a direct derivative of a q-frame object: masks can be united by simple binary operators
        //start to identify clusters in the ClusterRegister: unite the RecoMaps/Masks and remember which subevents were added to it
        I3RecoPulseSeriesMapMask unite = I3RecoPulseSeriesMapMask(*qframe, q_ancestor, I3RecoPulseSeriesMap()); // empty container
        
        BOOST_FOREACH(const I3FramePtr &frame, frames) {
          I3RecoPulseSeriesMapMaskConstPtr recoMapMask = frame->Get<I3RecoPulseSeriesMapMaskConstPtr>(key);
          unite = (unite | *recoMapMask);
        }
        
        return unite;
    }
    else { // the situation is more complicated: abstract the masks to pulses, unite them and make a mask of the ancestor in the q-frame 
      //start to identify clusters in the ClusterRegister: unite the RecoMaps/Masks and remember which subevents were added to it
        I3RecoPulseSeriesMap pulse_map = I3RecoPulseSeriesMap(); // empty container
        
        BOOST_FOREACH(const I3FramePtr &frame, frames) {
            I3RecoPulseSeriesMapConstPtr recoMap = frame->Get<I3RecoPulseSeriesMapConstPtr>(key);
            pulse_map = CoincSuite::UniteRecoMaps(pulse_map, *recoMap);
        }//finished working that cluster index

        return I3RecoPulseSeriesMapMask(*qframe, q_ancestor, pulse_map);
    }  
  };
  
  
  //______________________________________________________________________________
  std::vector<std::string> GetMaskAncestry (I3FrameConstPtr frame,
                                                const std::string &key)
  {
    log_trace("Searching anchestry for %s", key.c_str());
    std::vector<std::string> ancestry;
    ancestry.push_back(key);
    I3RecoPulseSeriesMapMaskConstPtr mask = frame->Get<I3RecoPulseSeriesMapMaskConstPtr>(key);
    if (!mask) {
      log_trace("Key is no Mask!");
      return ancestry;
    }
    while (mask) {
      std::string ancestor_name = mask->GetSource();
      ancestry.push_back(ancestor_name);
      mask = frame->Get<I3RecoPulseSeriesMapMaskConstPtr>(ancestor_name);
      log_trace(" -> has ancestor %s", ancestor_name.c_str());
    }
    return ancestry;
  }


  //______________________________________________________________________________
  std::vector<std::string> GetCommonMaskAncestry (I3FrameConstPtr frame,
                                          const std::string &keyA,
                                          const std::string &keyB)
  {
    std::vector<std::string> common_ancestry;
    
    if (!(frame->Has(keyB) && frame->Has(keyA))) {
      log_error("Frame is missing these keys: %s and %s", keyA.c_str(), keyB.c_str());
      return common_ancestry;
    }

    std::vector<std::string> ancestryA = GetMaskAncestry(frame, keyA);
    std::vector<std::string> ancestryB = GetMaskAncestry(frame, keyB);
    
    if (ancestryA.size()==1 || ancestryB.size()==1) {
      log_error("Keys have to be I3RecoPulseSeriesMapMasks; Cannot guess a common ancestor");
      return common_ancestry;
    }
    std::vector<std::string>::reverse_iterator ancestorA=ancestryA.rbegin();
    std::vector<std::string>::reverse_iterator ancestorB=ancestryB.rbegin();
    while (ancestorA != ancestryA.rend() && (*ancestorA == *ancestorB)) {
      common_ancestry.push_back(*ancestorA);
      ancestorA++; ancestorB++;
    }
    std::reverse(common_ancestry.begin(),common_ancestry.end());
  
    return (common_ancestry);
  }

  //____________________________________________________________________________
  I3TriggerHierarchy UniteTriggerHierarchies (const I3TriggerHierarchy &trigA,
                                              const I3TriggerHierarchy &trigB) {
    log_fatal("Implement me");
    //There are two ways to approach this:
    // 1. just take the united triggerHiererarchy as the direct sum of both trigger UniteTriggerHierarchies
    // -OR-
    // 2. try to figure out matching triggers in the Hierarchies and write them only once; also throughput triggers have to be adapted accordingly
    using namespace I3TriggerHierarchyUtils;
    
    I3TriggerHierarchy uth; //the united TriggerHierarchy

    return uth; //NOTE fix warning for now, by returning a blank I3TriggerHierarchy; however this should be rectified somewhere in the future
    
    /*
    ///for now assume that there is only one global throughput per trigger    
    I3TriggerHierarchy::iterator globA = Find(trigA, TriggerKey::GLOBAL, TriggerKey::THROUGHPUT); // with iter at end i can forward iterate
    I3TriggerHierarchy::iterator globB = Find(trigB, TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
    
    const double separation = TimeSeparation(globA->GetTriggerTime(),
                                             globA->GetTriggerTime()+globA->GetTriggerLength(),
                                             globB->GetTriggerTime(),
                                             globB->GetTriggerTime()+globB->GetTriggerLength());
    
    if (std::isnan(separation)) { //inclusion
      //take the eralier trigger and insert there the includes subtriggers of
      // TODO make an effort not to double insert subtriggers 
      
      if (globA->GetTriggerTime()<globB->GetTriggerTime()) { // A before B
        uth = trigA;
        // append triggers from B into th except the global one
      }
      else { // B before A
        uth = trigB;
        // append triggers from A to th except the global one
      }
    }
    else if (separation<=0) { //partial inclusion
      // append triggers from A and B to th; create a new global readout
    }
    else if (separation>0) { //separted
      // just glue together A and B
    }
    //usefull function in this context
    //const std::vector<Type> GetDaughters(const I3Tree<Type>& t, const Type& p)
    //void AppendChild(I3Tree<Type>& t, const Type& parent, const Type& child)
    */
  }
}
