/**
 * $Id: SplittingTest.cxx 104975 2013-06-03 18:46:34Z mzoll $
 * $Author: mzoll <marcel.zoll@fysdik.su.se> $
 * $Date: 2013-06-03 20:46:34 +0200 (Mon, 03 Jun 2013) $
 * $Revision: 104975 $
 *
 * A Unit test which generates some artificial test cases and let the Splitter gnaw on it;
 *
 * Test these testcases:
 * TestCase1: COMPLETENESS horizontal particle along x-axis; Test completenes of hits
 * TestCase2: TIMESPEARATION Two timeseparated horizontal particles along same track; Should be separated
 * TestCase3: H_SPACESEPARATION Two spaceseparated horizontally travelling particles; should be separated
 * TestCase4: CLEAN Hit on detector center and horizontal separated ring3; central hit should not manifest, as shoudl not ring3 because of Multiplicity argument
 * TestCase5: DC_SEPARATION Hit on boundary of DCfidutial volume horizontaly separated; should not manifest in any event
 * TestCase6: BRIDGE Previous Isolated Clusters get connected by a single mutually connected hit;
 */

//need to define operational parameters to define the correct tests
#define MULTIPLICITY 4
#define TIMEWINDOW 2000
#define TIMECONEMINUS 1000
#define TIMECONEPLUS 1000
#define DOMSPACINGOPT false
//#define SINGLEDENSERINGLIMITS 
//#define DOUBLEDENSERINGLIMITS
//#define TRIPPLEDENSERINGLIMITS
#define MOPEOPT 1

#define ICINTERSTRINGSPACING 125.
#define DCINTERSTRINGSPACING 72.17

#include <I3Test.h>

#include "HiveSplitter/Hive-lib.h"
#include "HiveSplitter/HiveSplitter.h"


#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/geometry/I3Geometry.h"
#include <icetray/I3Frame.h>
#include <dataclasses/physics/I3EventHeader.h>
#include <dataclasses/I3Constants.h>
#include <icetray/I3Units.h>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

TEST_GROUP(HiveSplitter);

//helpers
inline bool IsICstring (uint s) {if (s<=79) return true; else return false;};

inline bool IsDCstring (uint s) {return !IsICstring(s);};

/** @brief a helper class that can compares to sets and evaluates the identity by evaluating omkey, time and charge of every element
  * @return true, if sets are identical in every element basepropperty; false if not
  */
template <class ordered_set>
bool HitsIdentical(const ordered_set &lhs, const ordered_set &rhs) {
  if (lhs.size() != rhs.size())
    return false;

  typename ordered_set::const_iterator lhs_iter = lhs.begin();
  typename ordered_set::const_iterator rhs_iter = rhs.begin();
  while (lhs_iter!=lhs.end()) {
    log_trace_stream("Lhs: "<<lhs_iter->domIndex<<" "<<lhs_iter->pulseIndex<<" "<<lhs_iter->time<<" "<<lhs_iter->charge<<std::endl);
    log_trace_stream("Rhs: "<<rhs_iter->domIndex<<" "<<rhs_iter->pulseIndex<<" "<<rhs_iter->time<<" "<<rhs_iter->charge<<std::endl);
    if (!(lhs_iter->domIndex==rhs_iter->domIndex)
      || !(std::abs(lhs_iter->charge-rhs_iter->charge)<0.1)
      || !(std::abs(lhs_iter->time-rhs_iter->time)<0.1)) //compare OMkey, charge and time; allow a small difference in the last two
      return false;
    lhs_iter++;
    rhs_iter++;
  }
  return true;
};

//create a geometry
I3Geometry GenerateTestGeometry () {
  //construct an artificial geometry that is:
  //   exactly centered in string 36
  //contains 3 rings of DOMS
  // 40 DOMs on the positive z axis in regular IceCube strings
  // another 20 DOMs on the negative z axis plus 50 DC DOMs
  // IC distance 17m , DC distance 7m
  I3Geometry geometry;

  geometry.startTime = I3Time(0,0);
  geometry.endTime = I3Time(0,10*10*I3Units::ms);

  I3OMGeoMap &omgeo = geometry.omgeo;

  std::map<uint, I3Position> string_position_map;
  
  string_position_map[9]=I3Position(-187.5, -324.75, 0.);
  string_position_map[10]=I3Position(-62.5, -324.75, 0.);
  string_position_map[11]=I3Position(62.5, -324.75, 0.);
  string_position_map[12]=I3Position(187.5, -324.75, 0.);
  string_position_map[16]=I3Position(-250., -216.5, 0.);
  string_position_map[17]=I3Position(-125., -216.5, 0.);
  string_position_map[18]=I3Position(0., -216.5, 0.);
  string_position_map[19]=I3Position(125., -216.5, 0.);
  string_position_map[20]=I3Position(250., -216.5, 0.);
  string_position_map[24]=I3Position(-312.4, -308.25, 0.);
  string_position_map[25]=I3Position(-187.5, -308.25, 0.);
  string_position_map[26]=I3Position(-62.5, -308.25, 0.);
  string_position_map[27]=I3Position(62.5, -308.25, 0.);
  string_position_map[28]=I3Position(187.5, -308.25, 0.);
  string_position_map[29]=I3Position(312.5, -308.25, 0.);
  string_position_map[33]=I3Position(-375., 0., 0.);
  string_position_map[34]=I3Position(-250., 0., 0.);
  string_position_map[35]=I3Position(-125., 0., 0.);
  string_position_map[36]=I3Position(0., 0., 0.); //origin per definition
  string_position_map[37]=I3Position(125., 0., 0.);
  string_position_map[38]=I3Position(250., 0., 0.);
  string_position_map[39]=I3Position(375., 0., 0.);
  string_position_map[43]=I3Position(-312.5, 0., 0.);
  string_position_map[44]=I3Position(-187.5, 0., 0.);
  string_position_map[45]=I3Position(-62.5, 0., 0.);
  string_position_map[46]=I3Position(62.5, 108.5, 0.);
  string_position_map[47]=I3Position(187.5, 108.5, 0.);
  string_position_map[48]=I3Position(312.5, 108.5, 0.);
  string_position_map[53]=I3Position(-250., 216.5, 0.);
  string_position_map[54]=I3Position(-125, 216.5, 0.);
  string_position_map[55]=I3Position(0., 216.5, 0.);
  string_position_map[56]=I3Position(125., 216.5, 0.);
  string_position_map[57]=I3Position(250., 216.5, 0.);
  string_position_map[62]=I3Position(-187.5, 324.75, 0.);
  string_position_map[63]=I3Position(-62.5, 324.75, 0.);
  string_position_map[64]=I3Position(62.5, 324.75, 0.);
  string_position_map[65]=I3Position(187.5, 324.75, 0.);
  string_position_map[81]=I3Position(0., 72.14, 0.);
  string_position_map[82]=I3Position(62.5, 36.09, 0.);
  string_position_map[83]=I3Position(-62.5, -36.09, 0.);
  string_position_map[84]=I3Position(0., -72.14, 0.);
  string_position_map[85]=I3Position(-62.5, -36.09, 0.);
  string_position_map[86]=I3Position(-62.5, 36.09, 0.);

  // BOOST_FOREACH(const std::map<uint, I3Position>::value_type &str_pos_pair, string_position_map) {
  for (std::map<uint, I3Position>::const_iterator str_pos_pair=string_position_map.begin(); str_pos_pair != string_position_map.end(); ++str_pos_pair) {
    const uint& str = str_pos_pair->first;
    const I3Position& pos = str_pos_pair->second;
    if (IsICstring(str)) {
      for (int om=1; om<61; om++) {
        const OMKey omkey(str, om);
        const double z_pos = (39.-om)*17.;
        const I3Position dom_pos(pos.GetX(), pos.GetY(), z_pos);

        omgeo[omkey].position = I3Position(dom_pos);
      }
    }
    else {
      for (int om=11; om<61; om++) {
        const OMKey omkey(str, om);
        const double z_pos = (-om)*10.;
        const I3Position dom_pos(pos.GetX(), pos.GetY(), z_pos);

        omgeo[omkey].position = I3Position(dom_pos);
      }
    }
  }
  
  return geometry;
};

//Make my most private instance; this is globally available and has to be prepared only once
HiveSplitter testFacility;
//bind a method that is only called once
int Init () {
  //testFacility.Configure(HiveSplitter_ParameterSet());
  I3GeometryConstPtr geo(new I3Geometry(GenerateTestGeometry()));
  testFacility.BuildDistanceMap(geo);
  return 0;
};

int InitTestFacility(){
  static int bla = Init(); //static, so this is done only once
  return bla;
}

/*
 * TestCase1: COMPLETENESS horizontal particle along x-axis; Test completenes of hits
 * TestCase2: TIMESPEARATION Two timeseparated horizontal particles along same track; Should be separated
 * TestCase3: H_SPACESEPARATION Two spaceseparated horizontally travelling particles; should be separated
 * TestCase4: CLEAN Hit on detector center and horizontal separated ring3; central hit should not manifest
 * TestCase5: DC_SEPARATION Hit on boundary of DCfidutial volume horizontaly separated; should not manifest in any event
*/
//run the tests
using namespace HitSorting;

const double c125= ICINTERSTRINGSPACING*I3Units::m/I3Constants::c; // time to traverse 125m with timespeed ~(416.7ns)
const double tc= 4*TIMECONEPLUS*I3Units::ns; // assume this as the timewindow-parameter and therefore the time spread between two particles so that they still can be seperated

TEST (TestCase1) {
  InitTestFacility();
  log_info("TestCase1 Building");
  
  HitSorting::RetrievalOrderedHitSet hits_p1;

  hits_p1.insert(Hit(OMKey(33, 20), 0, 0., 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(24, 20), 0, c125, 1.));
  hits_p1.insert(Hit(OMKey(34, 20), 0, c125, 1.));
  hits_p1.insert(Hit(OMKey(43, 20), 0, c125, 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(25, 20), 0, 2*c125, 1.));
  hits_p1.insert(Hit(OMKey(35, 20), 0, 2*c125, 1.));
  hits_p1.insert(Hit(OMKey(44, 20), 0, 2*c125, 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(26, 20), 0, 3*c125, 1.));
  hits_p1.insert(Hit(OMKey(36, 20), 0, 3*c125, 1.));
  hits_p1.insert(Hit(OMKey(45, 20), 0, 3*c125, 1.));

  hits_p1.insert(Hit(OMKey(27, 20), 0, 4*c125, 1.));
  hits_p1.insert(Hit(OMKey(37, 20), 0, 4*c125, 1.));
  hits_p1.insert(Hit(OMKey(46, 20), 0, 4*c125, 1.));

  hits_p1.insert(Hit(OMKey(28, 20), 0, 5*c125, 1.));
  hits_p1.insert(Hit(OMKey(38, 20), 0, 5*c125, 1.));
  hits_p1.insert(Hit(OMKey(47, 20), 0, 5*c125, 1.));

  hits_p1.insert(Hit(OMKey(29, 20), 0, 6*c125, 1.));
  hits_p1.insert(Hit(OMKey(39, 20), 0, 6*c125, 1.));
  hits_p1.insert(Hit(OMKey(48, 20), 0, 6*c125, 1.));

  I3RecoPulseSeriesMap recomap_p1 = ConvertHits2RecoPulseSeriesMap(hits_p1);
  log_info("Built TestCase1");
  std::vector<SubEventStartStop> sESSs = testFacility.HiveSplitting(recomap_p1);

  HitSorting::RetrievalOrderedHitSet s1 = HitSorting::ExtractHits(sESSs[0].subevent_);

  ENSURE(sESSs.size()==1, "Expected number of splits");
  ENSURE(SetsIdentical(hits_p1, s1), "particle1_pulses == SplitPulses1");
};

TEST (TestCase2) {
  InitTestFacility();
  log_info("TestCase2 Building");
  HitSorting::RetrievalOrderedHitSet hits_p1, hits_p2;
  HitSorting::RetrievalOrderedHitSet hits_separated;

  hits_p1.insert(Hit(OMKey(33, 20), 0, 0., 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(24, 20), 0, c125, 1.));
  hits_p1.insert(Hit(OMKey(34, 20), 0, c125, 1.));
  hits_p1.insert(Hit(OMKey(43, 20), 0, c125, 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(25, 20), 0, 2*c125, 1.));
  hits_p1.insert(Hit(OMKey(35, 20), 0, 2*c125, 1.));
  hits_p1.insert(Hit(OMKey(44, 20), 0, 2*c125, 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(26, 20), 0, 3*c125, 1.));
  hits_p1.insert(Hit(OMKey(36, 20), 0, 3*c125, 1.));
  hits_p1.insert(Hit(OMKey(45, 20), 0, 3*c125, 1.));

  hits_p1.insert(Hit(OMKey(27, 20), 0, 4*c125, 1.));
  hits_p1.insert(Hit(OMKey(37, 20), 0, 4*c125, 1.));
  hits_p1.insert(Hit(OMKey(46, 20), 0, 4*c125, 1.));

  hits_p1.insert(Hit(OMKey(28, 20), 0, 5*c125, 1.));
  hits_p1.insert(Hit(OMKey(38, 20), 0, 5*c125, 1.));
  hits_p1.insert(Hit(OMKey(47, 20), 0, 5*c125, 1.));

  hits_p1.insert(Hit(OMKey(29, 20), 0, 6*c125, 1.));
  hits_p1.insert(Hit(OMKey(39, 20), 0, 6*c125, 1.));
  hits_p1.insert(Hit(OMKey(48, 20), 0, 6*c125, 1.));
  //second particle entering detector
  hits_p2.insert(Hit(OMKey(33, 20), 1, tc, 1.));
  //timestep
  hits_p2.insert(Hit(OMKey(24, 20), 1, c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(34, 20), 1, c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(43, 20), 1, c125+tc, 1.));
  //timestep
  hits_p2.insert(Hit(OMKey(25, 20), 1, 2*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(35, 20), 1, 2*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(44, 20), 1, 2*c125+tc, 1.));
  //timestep
  hits_p2.insert(Hit(OMKey(26, 20), 1, 3*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(36, 20), 1, 3*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(45, 20), 1, 3*c125+tc, 1.));

  hits_p2.insert(Hit(OMKey(27, 20), 1, 4*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(37, 20), 1, 4*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(46, 20), 1, 4*c125+tc, 1.));

  hits_p2.insert(Hit(OMKey(28, 20), 1, 5*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(38, 20), 1, 5*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(47, 20), 1, 5*c125+tc, 1.));

  hits_p2.insert(Hit(OMKey(29, 20), 1, 6*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(39, 20), 1, 6*c125+tc, 1.));
  hits_p2.insert(Hit(OMKey(48, 20), 1, 6*c125+tc, 1.));
  
  hits_separated = UniteSets(hits_p1, hits_p2);

  I3RecoPulseSeriesMap recomap_p1 = ConvertHits2RecoPulseSeriesMap(hits_p1);
  I3RecoPulseSeriesMap recomap_p2 = ConvertHits2RecoPulseSeriesMap(hits_p2);
  I3RecoPulseSeriesMap recomap_separated = ConvertHits2RecoPulseSeriesMap(hits_separated);
  log_info("Built TestCase2 ");
  std::vector<SubEventStartStop> sESSs = testFacility.HiveSplitting(recomap_separated);
  
  ENSURE(sESSs.size()==2, "Expected number of splits");

  HitSorting::RetrievalOrderedHitSet s1 = HitSorting::ExtractHits(sESSs[0].subevent_);
  HitSorting::RetrievalOrderedHitSet s2 = HitSorting::ExtractHits(sESSs[1].subevent_);
  
  ENSURE(HitsIdentical(hits_p1, s1), "particle1_pulses == SplitPulses1");
  ENSURE(HitsIdentical(hits_p2, s2), "particle2_pulses == SplitPulses2");
};

TEST (TestCase3) {
  InitTestFacility();
  log_info("TestCase3 Building");
  HitSorting::RetrievalOrderedHitSet hits_p1, hits_p2;
  HitSorting::RetrievalOrderedHitSet hits_separated;

  hits_p1.insert(Hit(OMKey(9, 20), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(16, 20), 0, 0., 1.));
  hits_p2.insert(Hit(OMKey(53, 20), 0, 0., 1.));
  hits_p2.insert(Hit(OMKey(62, 20), 0, 0., 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(10, 20), 0, c125, 1.));
  hits_p1.insert(Hit(OMKey(17, 20), 0, c125, 1.));
  hits_p2.insert(Hit(OMKey(54, 20), 0, c125, 1.));
  hits_p2.insert(Hit(OMKey(63, 20), 0, c125, 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(11, 20), 0, 2*c125, 1.));
  hits_p1.insert(Hit(OMKey(18, 20), 0, 2*c125, 1.));
  hits_p2.insert(Hit(OMKey(55, 20), 0, 2*c125, 1.));
  hits_p2.insert(Hit(OMKey(64, 20), 0, 2*c125, 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(12, 20), 0, 3*c125, 1.));
  hits_p1.insert(Hit(OMKey(19, 20), 0, 3*c125, 1.));
  hits_p2.insert(Hit(OMKey(56, 20), 0, 3*c125, 1.));
  hits_p2.insert(Hit(OMKey(65, 20), 0, 3*c125, 1.));
  //timestep
  hits_p1.insert(Hit(OMKey(20, 20), 0, 4*c125, 1.));
  hits_p2.insert(Hit(OMKey(57, 20), 0, 4*c125, 1.));
  //united
  hits_separated = UniteSets(hits_p1, hits_p2);

  I3RecoPulseSeriesMap recomap_separated = ConvertHits2RecoPulseSeriesMap(hits_separated);
  log_info("Built TestCase3");
  std::vector<SubEventStartStop> sESSs = testFacility.HiveSplitting(recomap_separated);

  ENSURE(sESSs.size()==2, "Expected number of splits");
  HitSorting::RetrievalOrderedHitSet s1 = HitSorting::ExtractHits(sESSs[0].subevent_);
  HitSorting::RetrievalOrderedHitSet s2 = HitSorting::ExtractHits(sESSs[1].subevent_);

  ENSURE(SetsIdentical(hits_p1, s1), "particle1_pulses == SplitPulses1");
  ENSURE(SetsIdentical(hits_p2, s2), "particle2_pulses == SplitPulses2");
};

TEST (TestCase4) {
  InitTestFacility();
  log_info("TestCase4 Building");
  HitSorting::RetrievalOrderedHitSet hits_p1, hits_p2;
  HitSorting::RetrievalOrderedHitSet hits_all;

  //boundary strings
  hits_p1.insert(Hit(OMKey(9, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(10, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(11, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(12, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(16, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(20, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(24, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(29, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(33, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(39, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(43, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(49, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(53, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(57, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(62, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(63, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(64, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(65, 50), 0, 0., 1.));
  //inner rings
  hits_p2.insert(Hit(OMKey(36, 50), 0, 0., 1.));
  
  hits_all = UniteSets(hits_p1, hits_p2);
  log_info("Built TestCase4");
  I3RecoPulseSeriesMap recomap_all = ConvertHits2RecoPulseSeriesMap(hits_all);
  
  std::vector<SubEventStartStop> sESSs = testFacility.HiveSplitting(recomap_all);

  ENSURE(sESSs.size()==0, "Expected number of splits");
};

TEST (TestCase5) {
  InitTestFacility();
  log_info("TestCase5 Building");
  HitSorting::RetrievalOrderedHitSet hits_p1, hits_p2, hits_p3;
  HitSorting::RetrievalOrderedHitSet hits_all, hits_separated;
  
  using namespace std;
  ///Definitions can be personal prference and are subject to change, once the IC86 detector should be further expanded
  
  //left DCfid boundary (35,40-60)
  hits_p1.insert(Hit(OMKey(35, 50), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(35, 51), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(35, 52), 0, 0., 1.));
  hits_p1.insert(Hit(OMKey(35, 53), 0, 0., 1.));
  //right DCfid boundary (37,40-60); distance to string35 ==250m
  hits_p2.insert(Hit(OMKey(37, 50), 0, 0., 1.));
  hits_p2.insert(Hit(OMKey(37, 51), 0, 0., 1.));
  hits_p2.insert(Hit(OMKey(37, 52), 0, 0., 1.));
  hits_p2.insert(Hit(OMKey(37, 53), 0, 0., 1.));
  //bridging (37,40-60)
  hits_p3.insert(Hit(OMKey(36, 50), 0, 0., 1.));
  
  hits_separated = UniteSets(hits_p1, hits_p2);
  hits_all = UniteSets(hits_separated, hits_p3);
  log_info_stream("Size hits: "<<hits_separated.size());

  I3RecoPulseSeriesMap recomap_p1 = ConvertHits2RecoPulseSeriesMap(hits_p1);
  I3RecoPulseSeriesMap recomap_p2 = ConvertHits2RecoPulseSeriesMap(hits_p2);
  I3RecoPulseSeriesMap recomap_p3 = ConvertHits2RecoPulseSeriesMap(hits_p3);
  I3RecoPulseSeriesMap recomap_separated = ConvertHits2RecoPulseSeriesMap(hits_separated);
  I3RecoPulseSeriesMap recomap_all = ConvertHits2RecoPulseSeriesMap(hits_all);
  
  log_info("Built TestCase5");
  std::vector<SubEventStartStop> sESSs = testFacility.HiveSplitting(recomap_separated);
  
  ENSURE(sESSs.size()==2, "Expected number of splits");
  HitSorting::RetrievalOrderedHitSet s1 = HitSorting::ExtractHits(sESSs[0].subevent_);
  HitSorting::RetrievalOrderedHitSet s2 = HitSorting::ExtractHits(sESSs[1].subevent_);
  
  ENSURE(SetsIdentical(hits_p1, s1), "particle1_pulses == SplitPulses1");
  ENSURE(SetsIdentical(hits_p2, s2), "particle2_pulses == SplitPulses2");
  //test briding
  sESSs = testFacility.HiveSplitting(recomap_all);

  ENSURE(sESSs.size()==1, "Expected number of splits");
  s1 = HitSorting::ExtractHits(sESSs[0].subevent_);
  
  ENSURE(SetsIdentical(hits_all, s1), "particle1_pulses == SplitPulses1");
};
