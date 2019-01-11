/**
 * \file IceHiveHelpers.cxx
 *
 * copyright (c) 2011
 * the IceCube Collaboration
 * $Id$
 *
 * @date $Date: 2013-10-22$
 * @author mzoll <marcel.zoll@fysik.su.se>
 *
 * A modular algorithm as an rewrite from project IceHive by Naoko
 */

#include "IceHive/IceHiveHelpers.h"

#include <boost/foreach.hpp>
#include <algorithm>
#include <vector>

I3TriggerHierarchy IceHiveHelpers::ClipTriggerHierarchy(const I3TriggerHierarchy &trigHier,
                                                        const double tw_start,
                                                        const double tw_stop,
                                                        const std::vector<int>& configIDs) {
  /*  The Structure of a TriggerHierarchy is as follows
   *  object<>:
   *  -> globalTrigger (Merged Readout window including all paddings)
   *  -> individual trigger throughput (The padding arround each trigger)
   *  --> the actual trigger (time whenever the trigger condition was fullfilled)
   *  -> next trigger troughput
   *  --> next trigger window
   */
  I3TriggerHierarchy subtrigs; // new trig hierarchy

  // first make and insert global merged trigger, it'll
  // ALWAYS MAKE THIS EVEN IF THERE AREN'T TRIGS TO MERGE
  I3Trigger globTrigger;
  globTrigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::MERGED);
  globTrigger.SetTriggerFired(true);
  globTrigger.SetTriggerTime(tw_start);
  globTrigger.SetTriggerLength(tw_stop-tw_start);

  I3TriggerHierarchy::iterator subIter = subtrigs.insert(subtrigs.end(), globTrigger);
  
  if (configIDs.size()==0) { //take every trigger there is
    //BOOST_FOREACH(const I3Trigger &trigger, trigHier) {
    for (I3TriggerHierarchy::iterator trigIter = trigHier.begin(); trigIter != trigHier.end(); ++trigIter) {
      if ((trigIter->GetTriggerKey().GetSource() != TriggerKey::GLOBAL) //no merged or gloabltriggers in the concideration
        && ((tw_start <= trigIter->GetTriggerTime()) && (trigIter->GetTriggerTime() < tw_stop) )) 
      {
        //make and insert throughput
        I3Trigger tpTrigger;
        tpTrigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
        tpTrigger.SetTriggerFired(true);
        tpTrigger.SetTriggerTime(trigIter->GetTriggerTime());
        tpTrigger.SetTriggerLength(trigIter->GetTriggerLength());

        //append the troughput trigger to the global trigger
        I3TriggerHierarchy::iterator eachTrig = subtrigs.append_child(subIter, tpTrigger);
        //insert actual trigger as child of the individual throughput
        subtrigs.append_child(eachTrig, trigIter);       
      }
    }
  } 
  else { //take only triggers in the list
    for (I3TriggerHierarchy::iterator trigIter = trigHier.begin(); trigIter != trigHier.end(); ++trigIter) {
      if ( (find(configIDs.begin(), configIDs.end(), trigIter->GetTriggerKey().GetConfigIDOptional()) != configIDs.end()) 
        && ((tw_start <= trigIter->GetTriggerTime()) && (trigIter->GetTriggerTime() < tw_stop) ))
      {      
        //make and insert throughput
        I3Trigger tpTrigger;
        tpTrigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
        tpTrigger.SetTriggerFired(true);
        tpTrigger.SetTriggerTime(trigIter->GetTriggerTime());
        tpTrigger.SetTriggerLength(trigIter->GetTriggerLength());

        //append the troughput trigger to the global trigger
        I3TriggerHierarchy::iterator eachTrig = subtrigs.append_child(subIter, tpTrigger);
        //insert actual trigger as child of the individual throughput
        subtrigs.append_child(eachTrig, trigIter);
      }
    } 
  }

  return subtrigs;
}

I3TriggerHierarchy IceHiveHelpers::ClipTriggerHierarchy(const I3TriggerHierarchy &trigHier,
                                        const I3TimeWindow& twindow,
                                        const std::vector<int> &configIDs)
  {return ClipTriggerHierarchy(trigHier, twindow.GetStart(), twindow.GetStop(), configIDs);};