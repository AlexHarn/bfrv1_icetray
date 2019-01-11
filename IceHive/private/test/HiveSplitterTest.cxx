/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 *
 * A Unit test which generates some artificial test cases and let the Splitter gnaw on them;
 *
 * Test these testcases:
 * TestCase1: COMPLETENESS horizontal particle along x-axis; Test completenes of hits
 * TestCase2: TIMESPEARATION Two timeseparated horizontal particles along same track; Should be separated
 * TestCase3: H_SPACESEPARATION Two spaceseparated horizontally travelling particles; should be separated
 * TestCase4: CLEAN Hit on detector center and horizontal separated ring3; central hit should not manifest, as should not hits on ring3 because of Multiplicity argument
 * TestCase5: DC_SEPARATION Hit on boundary of DCfidutial volume horizontaly separated; should not manifest in any event
 * TestCase6: BRIDGE Previous isolated clusters get connected by a single mutually connected hit;
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

#include "IceHive/HiveSplitter.h"

#include "dataclasses/I3Constants.h"
#include "icetray/I3Units.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

#include "tools/IdealGeometry.h"
#include "TestHelpers.h"

using namespace std;
using namespace boost;
using namespace HitSorting;
using namespace OMKeyHash;

TEST_GROUP(HiveSplitter);

//Make my most private instance and prepare it
HiveSplitter splittingFacility;
//bind a method that is only called once
int PrepareSplittingFacility () {
  //splittingFacility.Configure(HiveSplitter_ParameterSet());
  I3GeometryConstPtr geo = boost::make_shared<I3Geometry>(GenerateTestGeometry());
  splittingFacility.BuildLookUpTables(*geo);
  return 0;
};

int InitSplittingFacility(){
    static int bla = PrepareSplittingFacility();
    return bla; //stupid compiler
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

/// Test the completeness of the output
TEST (Completeness) {
  InitSplittingFacility();
  HitSeries hits_p1;
  
  std::vector<HitObject<I3RecoPulse> > hitObjects_p1;

  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(33, 20), MakeRecoPulse(0., 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(24, 20), MakeRecoPulse(c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(34, 20), MakeRecoPulse(c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(43, 20), MakeRecoPulse(c125, 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(25, 20), MakeRecoPulse(2*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(35, 20), MakeRecoPulse(2*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(44, 20), MakeRecoPulse(2*c125, 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(26, 20), MakeRecoPulse(3*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(36, 20), MakeRecoPulse(3*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(45, 20), MakeRecoPulse(3*c125, 1.)));  

  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(27, 20), MakeRecoPulse(4*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(37, 20), MakeRecoPulse(4*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(46, 20), MakeRecoPulse(4*c125, 1.)));

  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(28, 20), MakeRecoPulse(5*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(38, 20), MakeRecoPulse(5*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(47, 20), MakeRecoPulse(5*c125, 1.)));
  
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(29, 20), MakeRecoPulse(6*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(39, 20), MakeRecoPulse(6*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(48, 20), MakeRecoPulse(6*c125, 1.)));
  
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p1) {
    hits_p1.push_back(ho.CreateAssociatedHit());
  }
  
  HitSeriesSeries hits_split = splittingFacility.Split(hits_p1);
  
  ENSURE(hits_split.size()==1, "Expected number of splits");
  ENSURE(SetsIdentical(hits_p1, hits_split[0]), "particle1_pulses == SplitPulses1");
};


/// test separation
TEST (TimeSeparation) {
  InitSplittingFacility();
  HitSeries hits_p1, hits_p2;
  HitSeries hits_united;

  std::vector<I3RecoPulse_HitObject> hitObjects_p1, hitObjects_p2;
  
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(33, 20), MakeRecoPulse(0., 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(24, 20), MakeRecoPulse(c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(34, 20), MakeRecoPulse(c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(43, 20), MakeRecoPulse(c125, 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(25, 20), MakeRecoPulse(2*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(35, 20), MakeRecoPulse(2*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(44, 20), MakeRecoPulse(2*c125, 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(26, 20), MakeRecoPulse(3*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(36, 20), MakeRecoPulse(3*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(45, 20), MakeRecoPulse(3*c125, 1.)));  

  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(27, 20), MakeRecoPulse(4*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(37, 20), MakeRecoPulse(4*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(46, 20), MakeRecoPulse(4*c125, 1.)));

  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(28, 20), MakeRecoPulse(5*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(38, 20), MakeRecoPulse(5*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(47, 20), MakeRecoPulse(5*c125, 1.)));
  
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(29, 20), MakeRecoPulse(6*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(39, 20), MakeRecoPulse(6*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(48, 20), MakeRecoPulse(6*c125, 1.)));
  
  //second particle entering detector
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(33, 20), MakeRecoPulse(tc, 1.)));
  //timestep
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(24, 20), MakeRecoPulse(c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(34, 20), MakeRecoPulse(c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(43, 20), MakeRecoPulse(c125+tc, 1.)));
  //timestep
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(25, 20), MakeRecoPulse(2*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(35, 20), MakeRecoPulse(2*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(44, 20), MakeRecoPulse(2*c125+tc, 1.)));
  //timestep
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(26, 20), MakeRecoPulse(3*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(36, 20), MakeRecoPulse(3*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(45, 20), MakeRecoPulse(3*c125+tc, 1.)));  

  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(27, 20), MakeRecoPulse(4*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(37, 20), MakeRecoPulse(4*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(46, 20), MakeRecoPulse(4*c125+tc, 1.)));

  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(28, 20), MakeRecoPulse(5*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(38, 20), MakeRecoPulse(5*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(47, 20), MakeRecoPulse(5*c125+tc, 1.)));
  
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(29, 20), MakeRecoPulse(6*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(39, 20), MakeRecoPulse(6*c125+tc, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(48, 20), MakeRecoPulse(6*c125+tc, 1.)));

  
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p1) {
    hits_p1.push_back(ho.CreateAssociatedHit());
  }
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p2) {
    hits_p2.push_back(ho.CreateAssociatedHit());
  }
  
  hits_united.insert(hits_united.end(), hits_p1.begin(), hits_p1.end());
  hits_united.insert(hits_united.end(), hits_p2.begin(), hits_p2.end());
  sort(hits_united.begin(), hits_united.end());
  
  HitSeriesSeries hits_split = splittingFacility.Split(hits_united);
  
  ENSURE(hits_split.size()==2, "Expected number of splits");
  ENSURE(SetsIdentical(hits_p1, hits_split[0]), "particle1_pulses == SplitPulses1");
  ENSURE(SetsIdentical(hits_p2, hits_split[1]), "particle2_pulses == SplitPulses2");
};

/// test separation
TEST (SpaceSeparation) {
  InitSplittingFacility();
  HitSeries hits_p1, hits_p2;
  HitSeries hits_united;

  std::vector<I3RecoPulse_HitObject> hitObjects_p1, hitObjects_p2;
  
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(9, 20), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(16, 20), MakeRecoPulse(0., 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(53, 20), MakeRecoPulse(0., 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(62, 20), MakeRecoPulse(0., 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(10, 20), MakeRecoPulse(c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(17, 20), MakeRecoPulse(c125, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(54, 20), MakeRecoPulse(c125, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(63, 20), MakeRecoPulse(c125, 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(11, 20), MakeRecoPulse(2*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(18, 20), MakeRecoPulse(2*c125, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(55, 20), MakeRecoPulse(2*c125, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(64, 20), MakeRecoPulse(2*c125, 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(12, 20), MakeRecoPulse(3*c125, 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(19, 20), MakeRecoPulse(3*c125, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(56, 20), MakeRecoPulse(3*c125, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(65, 20), MakeRecoPulse(3*c125, 1.)));
  //timestep
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(20, 20), MakeRecoPulse(4*c125, 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(57, 20), MakeRecoPulse(4*c125, 1.)));
  
  
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p1) {
    hits_p1.push_back(ho.CreateAssociatedHit());
  }
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p2) {
    hits_p2.push_back(ho.CreateAssociatedHit());
  }
  
  //united
  hits_united.insert(hits_united.end(), hits_p1.begin(), hits_p1.end());
  hits_united.insert(hits_united.end(), hits_p2.begin(), hits_p2.end());
  sort(hits_united.begin(), hits_united.end());
  
  HitSeriesSeries hits_split = splittingFacility.Split(hits_united);
  
  ENSURE(hits_split.size()==2, "Expected number of splits");
  ENSURE(SetsIdentical(hits_p1, hits_split[0]), "particle1_pulses == SplitPulses1");
  ENSURE(SetsIdentical(hits_p2, hits_split[1]), "particle2_pulses == SplitPulses2");
};

/// test discrimination power
TEST (Discrimination) {
  InitSplittingFacility();
  HitSeries hits_p1, hits_p2;
  HitSeries hits_united;

  std::vector<I3RecoPulse_HitObject> hitObjects_p1, hitObjects_p2;
  //boundary strings
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey( 9, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(10, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(11, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(12, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(16, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(20, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(24, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(29, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(33, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(39, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(43, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(49, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(53, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(57, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(62, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(63, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(64, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(65, 50), MakeRecoPulse(0., 1.)));
  //inner rings
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(36, 50), MakeRecoPulse(0., 1.)));

  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p1) {
    hits_p1.push_back(ho.CreateAssociatedHit());
  }
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p2) {
    hits_p2.push_back(ho.CreateAssociatedHit());
  }
  
  hits_united.insert(hits_united.end(), hits_p1.begin(), hits_p1.end());
  hits_united.insert(hits_united.end(), hits_p2.begin(), hits_p2.end());
  sort(hits_united.begin(), hits_united.end());
  
  HitSeriesSeries hits_split = splittingFacility.Split(hits_united);
  
  ENSURE(hits_split.size()==0, "Expected number of splits");
};

// test Separation in DC strings
TEST (DeepCoreSeparation) {
  InitSplittingFacility();
  HitSeries hits_p1, hits_p2, hits_p3;
  HitSeries hits_united;
  
  std::vector<I3RecoPulse_HitObject> hitObjects_p1, hitObjects_p2, hitObjects_p3;
  //left DCfid boundary (35,40-60)
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(35, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(35, 51), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(35, 52), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(35, 53), MakeRecoPulse(0., 1.)));
  //right DCfid boundary (37,40-60); distance to string35 ==250m
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(37, 50), MakeRecoPulse(0., 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(37, 51), MakeRecoPulse(0., 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(37, 52), MakeRecoPulse(0., 1.)));
  hitObjects_p2.push_back(HitObject<I3RecoPulse>(OMKey(37, 53), MakeRecoPulse(0., 1.)));
  //briding (37,40-60)  
  hitObjects_p3.push_back(HitObject<I3RecoPulse>(OMKey(36, 50), MakeRecoPulse(0., 1.)));
  
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p1) {
    hits_p1.push_back(ho.CreateAssociatedHit());
  }
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p2) {
    hits_p2.push_back(ho.CreateAssociatedHit());
  }
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p3) {
    hits_p3.push_back(ho.CreateAssociatedHit());
  }
  
  
  //where things should still be separated
  hits_united.insert(hits_united.end(), hits_p1.begin(), hits_p1.end());
  hits_united.insert(hits_united.end(), hits_p2.begin(), hits_p2.end());
  sort(hits_united.begin(), hits_united.end());
  
  HitSeriesSeries hits_split = splittingFacility.Split(hits_united);
  
  ENSURE(hits_split.size()==2, "Expected number of splits");
  ENSURE(SetsIdentical(hits_p1, hits_split[0]), "particle1_pulses == SplitPulses1");
  ENSURE(SetsIdentical(hits_p2, hits_split[1]), "particle2_pulses == SplitPulses2");
  
  //insert the bridging hit
  hits_united.insert(hits_united.end(), hits_p3.begin(), hits_p3.end());
  sort(hits_united.begin(), hits_united.end());
  
  HitSeriesSeries hits_unsplit = splittingFacility.Split(hits_united);
  
  ENSURE(hits_unsplit.size()==1, "Expected number of splits");
  ENSURE(SetsIdentical(hits_united, hits_unsplit[0]), "all particle_pulses == SplitPulses");
};
