/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 *
 * A Unit test which generates some artificial test cases and let the Cleaning gnaw on them;
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

#include "IceHive/HiveCleaning.h"
#include "tools/IdealGeometry.h"
#include "TestHelpers.h"

#include "dataclasses/I3Constants.h"
#include "icetray/I3Units.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace HitSorting;
using namespace OMKeyHash;

TEST_GROUP(HiveCleaning);

//Make my most private instance and prepare it
HiveCleaning cleaningFacility;
//bind a method that is only called once
int PrepareCleaningFacility () {
  //cleaningFacility.Configure(HiveSplitter_ParameterSet());
  I3GeometryConstPtr geo = boost::make_shared<I3Geometry>(GenerateTestGeometry());
  cleaningFacility.BuildLookUpTables(*geo);
  return 0;
};

int InitCleaningFacility(){
    static int bla = PrepareCleaningFacility();
    return bla;
}

/*
 * TestCase1: CLEAN SELF-COINCIDENCE clean away all isolated DOMs even in self-conicidence
 * TestCase2: PRESERVATION keep all DOMs which have neighbors
*/
//run the tests
using namespace HitSorting;

// const double c125= ICINTERSTRINGSPACING*I3Units::m/I3Constants::c; // time to traverse 125m with timespeed ~(416.7ns)
// const double tc= 4*TIMECONEPLUS*I3Units::ns; // assume this as the timewindow-parameter and therefore the time spread between two particles so that they still can be seperated

/// Clean away any isolated pulse in time on the same DOM, even if it is hit twice
TEST (CleanSelfCoinc) {
  InitCleaningFacility();
  HitSeries hits_p1;

  std::vector<I3RecoPulse_HitObject> hitObjects_p1;  
  //theoretically test for all possible DOMs
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(36, 30), MakeRecoPulse(0., 1.)));
  hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(36, 30), MakeRecoPulse(0., 1.)));

  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p1) {
    hits_p1.push_back(ho.CreateAssociatedHit());
  }
  
  HitSeries hits_clean = cleaningFacility.Clean(hits_p1);

  ENSURE(hits_clean.size()==0, "Expected number of retained DOMs");
};


/// preserve all participating DOMs if they have neighbors
TEST (Preservation) {
  InitCleaningFacility();
  
  HitSeries hits_p1;
  std::vector<I3RecoPulse_HitObject> hitObjects_p1;  
  
  // let the whole detector blink once
  for (int s=1; s<=86; ++s) {
    for (int o=1; o<=60; ++o) {
      hitObjects_p1.push_back(HitObject<I3RecoPulse>(OMKey(s, o), MakeRecoPulse(0., 1.)));
    }
  }
  
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_p1) {
    hits_p1.push_back(ho.CreateAssociatedHit());
  }
  
  HitSeries hits_clean = cleaningFacility.Clean(hits_p1);
  
  ENSURE(hits_clean.size()==hits_p1.size(), "Expected number of retained DOMs");
};
