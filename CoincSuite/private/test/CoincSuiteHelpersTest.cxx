/**
 * $Id: CoincSuiteHelpersTest.cxx 104975 2013-06-03 18:46:34Z mzoll $
 * $Author: mzoll <marcel.zoll@fysdik.su.se> $
 * $Date: 2013-06-03 20:46:34 +0200 (Mon, 03 Jun 2013) $
 * $Revision: 104975 $
 *
 * Test all the small helper functions in the CoincSuite namespace
 */

#include <I3Test.h>

#include <icetray/I3Frame.h>
#include <dataclasses/physics/I3EventHeader.h>
#include "dataclasses/physics/I3RecoPulse.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

#include "CoincSuite/lib/CoincSuiteHelpers.h"


//=================== NAMESPACE CoincSuite =================
TEST_GROUP(CoincSuite_namespace);

using namespace std;
using namespace boost;
using namespace CoincSuite;

TEST (TimeSeparation) {
  
  /* @brief Give the time-separation of two time windows; preferred either I3Time or double-expressions of whatever
  * @note the '<' operator has to be defined for this; aka the times have to obey an ordering principle
  * @param startA start of eventA
  * @param endA stop of eventA
  * @param startB start of eventB
  * @param endB stop of eventB
  * @return NAN: full inclusion,
  *         negative value: partial inclusion by so many ns,
  *         positive value: separated by that many ns
  */
  // create test scenarios like this: // -0-1--2-3--4--5---6-7--8-9-
                                      // -o-o-[A-A-[AB-AB]-B-B]-o-o-
  ENSURE( TimeSeparation<double>(2, 5, 4, 7) == -1 ); //-o-o-[A-A-[AB-AB]-B-B]-o-o-
  
  ENSURE( std::isnan(TimeSeparation<double>(2, 8, 4, 7)) ); //-o-o-[A-A-[AB-AB-AB]-A-A]-o-o-
  
  ENSURE( TimeSeparation<double>(2, 4, 6, 8) == 2 ); //-o-o-[A-A-A]-o-[B-B-B]-o-o-
  
  ENSURE( TimeSeparation<double>(4, 7, 2, 5) == -1 ); //-o-o-[B-B-[AB-AB]-A-A]-o-o-
  
  ENSURE( std::isnan(TimeSeparation<double>(4, 7, 2, 8)) ); //-o-o-[B-B-[AB-AB-AB]-B-B]-o-o-
  
  ENSURE( TimeSeparation<double>(6, 8, 2, 4) == 2 ); //-o-o-[B-B-B]-o-[A-A-A]-o-o-
}

TEST (GetMaskAncestry) {
  I3FramePtr frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  
  I3RecoPulseSeriesMap map;
  I3RecoPulseSeries series;
  I3RecoPulse reco;
  OMKey key;
  
  series.push_back(reco);
  map[key] = series;
  
  frame->Put("Pulses0", boost::make_shared<I3RecoPulseSeriesMap>(map)); //I3RecoMap containing 1 pulse
  frame->Put("Pulses1", boost::make_shared<I3RecoPulseSeriesMapMask>(*frame, "Pulses0", map)); //Mask containingh one pulse
  frame->Put("Pulses2", boost::make_shared<I3RecoPulseSeriesMapMask>(*frame, "Pulses1", I3RecoPulseSeriesMap())); //Mask containing no pulses
  frame->Put("Pulses3", boost::make_shared<I3RecoPulseSeriesMapMask>(*frame, "Pulses2", I3RecoPulseSeriesMap())); //Mask containing no pulses
  
  /* Get a list of all deriations of this mask; children up front, grandparents in the back
   * @param frame The frame which is searched for the the mask and its ancestors
   * @param key The name of a I3RecoPulseSeriesMap or I3RecoPulseSeriesMapMask
   * @return the ancestry of this map/mask, parents in the back
   */
  
  std::vector<std::string> ancestry = GetMaskAncestry (frame, "Pulses3");
  
  ENSURE( ancestry.size()==4);
  ENSURE( ancestry[0] == "Pulses3");
  ENSURE( ancestry[1] == "Pulses2");
  ENSURE( ancestry[2] == "Pulses1");
  ENSURE( ancestry[3] == "Pulses0");
}

TEST (GetCommonMaskAncestry) {
  I3FramePtr frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  
  I3RecoPulseSeriesMap map;
  I3RecoPulseSeries series;
  I3RecoPulse reco;
  OMKey key;
  
  series.push_back(reco);
  map[key] = series;
  
  //the common hierarchy
  frame->Put("Pulses0", boost::make_shared<I3RecoPulseSeriesMap>(map)); //I3RecoMap containing 1 pulse
  frame->Put("Pulses1", boost::make_shared<I3RecoPulseSeriesMapMask>(*frame, "Pulses0", map)); //Mask containingh one pulse
  //the one mask
  frame->Put("Fork1", boost::make_shared<I3RecoPulseSeriesMapMask>(*frame, "Pulses1", map)); //Mask containing one pulse
  //the other mask
  frame->Put("Fork2", boost::make_shared<I3RecoPulseSeriesMapMask>(*frame, "Pulses1", I3RecoPulseSeriesMap())); //Mask containing no pulses
  
  /*Get me the oldest common ancestors for masks
   * @param frame The frame which is searched for the the mask and its ancestors
   * @param key1 the one I3RecoPulseSeriesMapMask
   * @param key2 the other I3RecoPulseSeriesMapMask
   * @return key of the first common ancestor
   */
  std::vector<std::string> common_ancestry = GetCommonMaskAncestry (frame, "Fork1", "Fork2");
  
  ENSURE( common_ancestry.size()==2);
  ENSURE( common_ancestry[0] == "Pulses1");
  ENSURE( common_ancestry[1] == "Pulses0");
}  

TEST(UniteRecoMaps) {
  I3FramePtr frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  
  I3RecoPulseSeriesMap map1, map2, map3;
  I3RecoPulseSeries series1, series2;
  I3RecoPulse reco1; reco1.SetTime(1.); reco1.SetCharge(1.);
  I3RecoPulse reco2; reco2.SetTime(2.); reco2.SetCharge(2.);
  
  OMKey key1(1,1);
  OMKey key2(2,2);
  
  series1.push_back(reco1);
  series2.push_back(reco2);
  
  map1[key1] = series1; //reco1 at omkey1
  map2[key1] = series2; //reco2 at omkey1
  map3[key2] = series2; //reco2 at omkey2

  /** @brief A convenience function to unite two RecoMaps into a single one
   * @param mapA the one map
   * @param mapB the other map
   * @return the united map
   */
  I3RecoPulseSeriesMap unite12 = UniteRecoMaps (map1, map2);
  
  ENSURE(unite12.size()==1);
  I3RecoPulseSeriesMap::const_iterator u12_iter = unite12.begin();
  ENSURE(u12_iter->first == OMKey(1,1));
  ENSURE(u12_iter->second.size() == 2);
  ENSURE(u12_iter->second[0] == reco1);
  ENSURE(u12_iter->second[1] == reco2);
  
  I3RecoPulseSeriesMap unite13 = UniteRecoMaps (map1, map3);
  ENSURE(unite13.size()==2);
  I3RecoPulseSeriesMap::const_iterator u13_iter = unite13.begin();
  ENSURE(u13_iter->first == OMKey(1,1));
  ENSURE(u13_iter->second.size() == 1);
  ENSURE(u13_iter->second[0] == reco1);
  u13_iter++;
  ENSURE(u13_iter->first == OMKey(2,2));
  ENSURE(u13_iter->second.size() == 1);
  ENSURE(u13_iter->second[0] == reco2);
}

TEST(UniteTriggerHierarchies) {
  //make a scenario where triggers are order [t[i]t]-[t[i]t]
  
  I3TriggerHierarchy trigHierU;
  I3TriggerHierarchy trigHier1;
  I3TriggerHierarchy::iterator eachTrig;
  
  I3Trigger hsTrigger;
  hsTrigger.GetTriggerKey() = TriggerKey(TriggerKey::IN_ICE, TriggerKey::FRAGMENT_MULTIPLICITY, 1);
  hsTrigger.SetTriggerFired(true);
  
  I3Trigger tpTrigger;
  tpTrigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
  tpTrigger.SetTriggerFired(true);
  
  hsTrigger.SetTriggerTime(1);
  hsTrigger.SetTriggerLength(1);
  tpTrigger.SetTriggerTime(0);
  tpTrigger.SetTriggerLength(3);

  eachTrig = trigHier1.append_child(trigHier1.end(), tpTrigger);
  trigHier1.append_child(eachTrig, hsTrigger);
  eachTrig = trigHierU.append_child(trigHierU.end(), tpTrigger);
  trigHierU.append_child(eachTrig, hsTrigger);
  

  I3TriggerHierarchy trigHier2;
  
  hsTrigger.SetTriggerTime(5);
  hsTrigger.SetTriggerLength(1);

  tpTrigger.SetTriggerTime(4);
  tpTrigger.SetTriggerLength(3);
  
  eachTrig = trigHier2.append_child(trigHier2.end(), tpTrigger);
  trigHier2.append_child(eachTrig, hsTrigger);
  eachTrig = trigHierU.append_child(trigHierU.end(), tpTrigger);
  trigHierU.append_child(eachTrig, hsTrigger);
  //scenario ready
  
  //FIXME
  //I3TriggerHierarchy unite12 = UniteTriggerHierarchies (trigHier1, trigHier2);
  // make the test if everything did work out;
  //ENSURE(trigHierU==unite12);
}
