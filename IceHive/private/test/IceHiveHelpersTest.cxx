/**
 * \file IceHiveHelpersTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * Unit test to test the robustness of IceHiveHelpers Hashing as it is implemented in IceHive/IceHiveHelpers.h
 */

#include "IceHive/IceHiveHelpers.h"

#include <I3Test.h>

#include "dataclasses/physics/I3RecoPulse.h"

/// cheat function because I3RecoPulse does not have a constructor
/// @NOTE a similar constructor should be added to dataclasses/physics/I3RecoPulse.h
I3RecoPulse CreateRecoPulse (const double t, const double c=1., const double w=1., const uint8_t flags=0) {
  I3RecoPulse p;
  p.SetTime(t);
  p.SetCharge(c);
  p.SetWidth(w);
  p.SetFlags(flags);
  return p;
}

TEST_GROUP(IceHiveHelpers);

using namespace IceHiveHelpers;

/// sensible value test for hardcoded values
TEST(GetPulsesInTimeRange) {
  /* @brief Give me all pulses from this ResponseMap which fall within this time-window
   *  A simple selection function
   *  @param pulses from this pulses
   *  @param tw_start starting at this time
   *  @param tw_stop ending at this time
   *  @return the subset of pulses
   */
  I3RecoPulseSeriesMap reco_map;
  reco_map[OMKey(1,1)].push_back(CreateRecoPulse(0.));
  reco_map[OMKey(1,1)].push_back(CreateRecoPulse(1.));
  reco_map[OMKey(2,2)].push_back(CreateRecoPulse(1.));
  reco_map[OMKey(2,2)].push_back(CreateRecoPulse(2.));
  //testcase done
  
  I3RecoPulseSeriesMap clip_map;
  I3RecoPulseSeriesMap::const_iterator c_iter;
  
  //just take the first part 
  clip_map = GetPulsesInTimeRange<I3RecoPulse>(reco_map, 0., 0.);
  ENSURE_EQUAL(clip_map.size(), (size_t)1, "Size is of output is as expected");
  c_iter= clip_map.begin();
  ENSURE(c_iter->first==OMKey(1,1) && c_iter->second.size()==(size_t)1, "Right OMKey(1,1) has right amount of pulses");
  
  //just take the first part 
  clip_map = GetPulsesInTimeRange<I3RecoPulse>(reco_map, 0., 1.);
  ENSURE_EQUAL(clip_map.size(), (size_t)2, "Size is of output is as expected");
  c_iter= clip_map.begin();
  ENSURE(c_iter->first==OMKey(1,1) && c_iter->second.size()==(size_t)2, "Right OMKey(1,1) has right amount of pulses");
  c_iter++;
  ENSURE(c_iter->first==OMKey(2,2) && c_iter->second.size()==(size_t)1, "Right OMKey(2,2) has right amount of pulses");
  
  //just take the middle part wher upper==lower
  clip_map = GetPulsesInTimeRange<I3RecoPulse>(reco_map, 1., 1.);
  ENSURE_EQUAL(clip_map.size(), (size_t)2, "Size is of output is as expected");
  c_iter= clip_map.begin();
  ENSURE(c_iter->first==OMKey(1,1) && c_iter->second.size()==(size_t)1, "Right OMKey(1,1) has right amount of pulses");
  c_iter++;
  ENSURE(c_iter->first==OMKey(2,2) && c_iter->second.size()==(size_t)1, "Right OMKey(2,2) has right amount of pulses");
};

//pose a simple scenario to clip a trigger Hierarchy
TEST(ClipTriggerHierarchy) {
  /* @brief Clip out the triggers with the given configIDs and within the specified TimeWindow
   * @param trigHier find triggers in there
   * @param configIDs the configured Trigger-IDs of thoese we gonna look for
   * @param tw_start the start time from which clipping commenses
   * @param tw_stop the stop time until which clipping commenses
   * @return a clipped trigger Hierarchy
   */
  
  //build a trigger Hierarchy which looks like this
  //   Time         : 0         |10
  // GlobalTroughput: GGGGGGGGGG
  // Throughput     :  TTTTTTTT
  // TriggerID-1    :   111111
  // Throughput     :  TTT  TTT
  // TriggerID-2    :   2    2
  
  I3TriggerHierarchy th;
  I3TriggerHierarchy::iterator subIter;
  I3TriggerHierarchy::iterator eachTrig;
  
  //a global trigger
  I3Trigger globTrigger;
  //local throughput
  I3Trigger tpTrigger_1_0, tpTrigger_2_0, tpTrigger_2_1;
  //inice-triggers
  I3Trigger aTrigger_1_0, aTrigger_2_0, aTrigger_2_1;
  
  //now fill and insert in trigger hierarchy
  globTrigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::MERGED);
  globTrigger.SetTriggerFired(true);
  globTrigger.SetTriggerTime(0.);
  globTrigger.SetTriggerLength(10.);
  
  //fill in the triggers, strat with the global trigger
  subIter = th.insert(th.end(), globTrigger);
   
  //fill a trigger, strat with its troughput
  tpTrigger_1_0.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
  tpTrigger_1_0.SetTriggerFired(true);
  tpTrigger_1_0.SetTriggerTime(0.);
  tpTrigger_1_0.SetTriggerLength(10.);
  aTrigger_1_0.GetTriggerKey() = TriggerKey(TriggerKey::IN_ICE, TriggerKey::SIMPLE_MULTIPLICITY);
  aTrigger_1_0.SetTriggerFired(true);
  aTrigger_1_0.GetTriggerKey().SetConfigID(1);
  aTrigger_1_0.SetTriggerTime(1.);
  aTrigger_1_0.SetTriggerLength(8.);
  
  eachTrig = th.append_child(subIter, tpTrigger_1_0);
  th.append_child(eachTrig, aTrigger_1_0);
  
  // fill next trigger
  tpTrigger_2_0.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
  tpTrigger_2_0.SetTriggerFired(true);
  tpTrigger_2_0.SetTriggerTime(1.);
  tpTrigger_2_0.SetTriggerLength(3.);
  aTrigger_2_0.GetTriggerKey() = TriggerKey(TriggerKey::IN_ICE, TriggerKey::SIMPLE_MULTIPLICITY);
  aTrigger_2_0.SetTriggerFired(true);
  aTrigger_2_0.GetTriggerKey().SetConfigID(2);
  aTrigger_2_0.SetTriggerTime(2.);
  aTrigger_2_0.SetTriggerLength(1.);

  eachTrig = th.append_child(subIter, tpTrigger_2_0);
  th.append_child(eachTrig, aTrigger_2_0);
  
  // fill next trigger
  tpTrigger_2_1.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
  tpTrigger_2_1.SetTriggerFired(true);
  tpTrigger_2_1.SetTriggerTime(6.);
  tpTrigger_2_1.SetTriggerLength(3.);
  aTrigger_2_1.GetTriggerKey() = TriggerKey(TriggerKey::IN_ICE, TriggerKey::SIMPLE_MULTIPLICITY);
  aTrigger_2_1.SetTriggerFired(true);
  aTrigger_2_1.GetTriggerKey().SetConfigID(2);
  aTrigger_2_1.SetTriggerTime(7.);
  aTrigger_2_1.SetTriggerLength(1.);
  
  eachTrig = th.append_child(subIter, tpTrigger_2_1);
  th.append_child(eachTrig, aTrigger_2_1);
  
  // TESTCASE CONSTRUCTED

  std::vector<int> configIDs;
  I3TriggerHierarchy clip_th;
  
  // testcase: take everything and tell me if its the same
  configIDs = std::vector<int>();
  clip_th = ClipTriggerHierarchy(th, 0., 10., configIDs);
  ENSURE_EQUAL(I3TriggerHierarchyUtils::Count(clip_th, aTrigger_1_0.GetTriggerKey()), (size_t)1);
  ENSURE_EQUAL(I3TriggerHierarchyUtils::Count(clip_th, aTrigger_2_0.GetTriggerKey()), (size_t)2);
  
  //testcase: take first half
  configIDs = std::vector<int>();
  clip_th = ClipTriggerHierarchy(th, 0., 5., configIDs);
  ENSURE_EQUAL(I3TriggerHierarchyUtils::Count(clip_th, aTrigger_1_0.GetTriggerKey()), (size_t)1);
  ENSURE_EQUAL(I3TriggerHierarchyUtils::Count(clip_th, aTrigger_2_0.GetTriggerKey()), (size_t)1);
  
  //testcase: take only triggers with ID=1
  configIDs = std::vector<int>(1,1);
  clip_th = ClipTriggerHierarchy(th, 0., 5., configIDs);
  ENSURE_EQUAL(I3TriggerHierarchyUtils::Count(clip_th, aTrigger_1_0.GetTriggerKey()), (size_t)1);
  ENSURE_EQUAL(I3TriggerHierarchyUtils::Count(clip_th, aTrigger_2_0.GetTriggerKey()), (size_t)0);
  
  //testcase: select nothing
  clip_th = ClipTriggerHierarchy(th, 0., 0., configIDs);
  ENSURE_EQUAL(I3TriggerHierarchyUtils::Count(clip_th, aTrigger_1_0.GetTriggerKey()), (size_t)0);
  ENSURE_EQUAL(I3TriggerHierarchyUtils::Count(clip_th, aTrigger_2_0.GetTriggerKey()), (size_t)0);
};

TEST (AsymmetricIndexMatrix_Bool) {
  AsymmetricIndexMatrix_Bool b(3);
  
  ENSURE(b.Get(0,0)==false, "Inital state is correctly False");
  
  ENSURE(b.Get(2,2)==false, "the maximum index is accessible as requested");
  
  b.Set(1,1,true);
  ENSURE(b.Get(1,1)==true, "Setting/Getting works correct");
  
  b.Set(0,1,true);
  ENSURE(b.Get(0,1)==true && b.Get(1,0)==false, "Probe asymmetry");
};
 
TEST (AsymmetricIndexMatrix_Double) {
  AsymmetricIndexMatrix_Double d(3);
  
  ENSURE(std::isnan(d.Get(0,0)), "Inital state is correctly False");
  
  ENSURE(std::isnan(d.Get(2,2)), "the maximum index is accessible as requested");
  
  d.Set(1,1,1.);
  ENSURE(d.Get(1,1)==1., "Setting/Getting works correct");
  
  d.Set(0,1,1.);
  ENSURE(d.Get(0,1)==1. && std::isnan(d.Get(1,0)), "Probe asymmetry");
};

TEST (SymmetricIndexMatrix_Bool) {
  SymmetricIndexMatrix_Bool b(3);
  
  ENSURE(b.Get(0,0)==false, "Inital state is correctly False");
  
  ENSURE(b.Get(2,2)==false, "the maximum index is accessible as requested");
  
  b.Set(1,1,true);
  ENSURE(b.Get(1,1)==true, "Setting/Getting works correct");

  b.Set(0,1, true);
  ENSURE(b.Get(0,1)==true && b.Get(1,0)==true, "Probe symmetry");
};
 
TEST (SymmetricIndexMatrix_Double) {
  SymmetricIndexMatrix_Double d(3);
  
  ENSURE(std::isnan(d.Get(0,0)), "Inital state is correctly False");
  
  ENSURE(std::isnan(d.Get(2,2)), "the maximum index is accessible as requested");
  
  d.Set(1,1,1.);
  ENSURE(d.Get(1,1)==1., "Setting/Getting works correct");
  
  d.Set(0,1,1.);
  ENSURE(d.Get(0,1)==1. && d.Get(1,0)==1., "Probe symmetry");
};
