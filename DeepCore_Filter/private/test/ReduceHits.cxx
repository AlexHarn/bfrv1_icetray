#include <I3Test.h>

#include "DeepCore_Filter/I3DeepCoreFunctions.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "icetray/OMKey.h"

#include "TestHelpers.cxx"

TEST_GROUP(ReduceHits);

TEST(ReduceHits_Pulses){
    // Create the geometry
    const I3Geometry geo = DCTestHelpers::GetTestGeometry();
    
    // create a place to store the output
    I3Particle *cog = new I3Particle();
    double sigma = -1;

    // And a faux hit series
    I3RecoPulseSeriesMapPtr pulsemap(new I3RecoPulseSeriesMap());
    (*pulsemap)[OMKey(1,1)] = I3RecoPulseSeries();
    (*pulsemap)[OMKey(1,1)].push_back(DCTestHelpers::MakePulse(1));
    (*pulsemap)[OMKey(1,1)].push_back(DCTestHelpers::MakePulse(2));
    (*pulsemap)[OMKey(3,2)] = I3RecoPulseSeries();
    (*pulsemap)[OMKey(3,2)].push_back(DCTestHelpers::MakePulse(3));
    (*pulsemap)[OMKey(3,2)].push_back(DCTestHelpers::MakePulse(4));
    (*pulsemap)[OMKey(3,2)].push_back(DCTestHelpers::MakePulse(5));

    DeepCoreFunctions<I3RecoPulse>::GetAverageTime(geo,
						   pulsemap,
						   cog,
						   sigma,
						   false,
						   "");

    ENSURE_EQUAL(cog->GetTime(), 3, "GetAverageTime didn't return the correct mean for t=[1,2,3,4,5]");
    ENSURE_DISTANCE(sigma, sqrt(5.0/2.0), 0.0001, "GetAverageTime didn't return the correct stddev for t=[1,2,3,4,5]");

    // Now do the ReduceHits fuction. We should remove the hits outside of t = 3 +- 1.26 
    I3RecoPulseSeriesMap reducedmap;
    DeepCoreFunctions<I3RecoPulse>::ReduceHits(geo,
					       pulsemap,
					       cog,
					       sigma,
					       reducedmap,
					       false);

    int nhits = 0;
    I3RecoPulseSeriesMap::iterator mapiter;
    for (mapiter=reducedmap.begin(); mapiter!=reducedmap.end(); ++mapiter){
      I3RecoPulseSeries::iterator seriesiter;
      for (seriesiter=mapiter->second.begin(); seriesiter!=mapiter->second.end(); ++seriesiter)
	++nhits;
    }
    
    ENSURE_EQUAL(nhits, 3, "DeepCoreFunctions::ReduceHits did not remove the right hits.")
}




TEST(ReduceHits_Launches){
    // Create the geometry
    const I3Geometry geo = DCTestHelpers::GetTestGeometry();
    
    // create a place to store the output
    I3Particle *cog = new I3Particle();
    double sigma = 0;

    // And a faux hit series
    I3DOMLaunchSeriesMapPtr launchmap(new I3DOMLaunchSeriesMap());
    (*launchmap)[OMKey(1,1)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(1,1)].push_back(DCTestHelpers::MakeLaunch(1));
    (*launchmap)[OMKey(2,3)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(2,3)].push_back(DCTestHelpers::MakeLaunch(2));
    (*launchmap)[OMKey(4,2)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(4,2)].push_back(DCTestHelpers::MakeLaunch(3));
    (*launchmap)[OMKey(6,2)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(6,2)].push_back(DCTestHelpers::MakeLaunch(4));
    (*launchmap)[OMKey(8,3)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(8,3)].push_back(DCTestHelpers::MakeLaunch(5));
    
    DeepCoreFunctions<I3DOMLaunch>::GetAverageTime(geo,
						   launchmap,
						   cog,
						   sigma,
						   true,
						   "");
    
    ENSURE_EQUAL(cog->GetTime(), 3, "GetAverageTime didn't return the correct mean for t=[1,2,3,4,5]");
    ENSURE_DISTANCE(sigma, sqrt(5.0/2.0), 0.0001, "GetAverageTime didn't return the correct stddev for t=[1,2,3,4,5]");
    // Now do the ReduceHits fuction. We should remove the hits outside of t = 3 +- 1.44 
    I3DOMLaunchSeriesMap reducedmap;
    DeepCoreFunctions<I3DOMLaunch>::ReduceHits(geo,
					       launchmap,
					       cog,
					       sigma,
					       reducedmap,
					       true);

    int nhits = 0;
    I3DOMLaunchSeriesMap::iterator mapiter;
    for (mapiter=reducedmap.begin(); mapiter!=reducedmap.end(); ++mapiter){
      I3DOMLaunchSeries::iterator seriesiter;
      for (seriesiter=mapiter->second.begin(); seriesiter!=mapiter->second.end(); ++seriesiter)
	++nhits;
    }
    
    ENSURE_EQUAL(nhits, 3, "DeepCoreFunctions::ReduceHits did not remove the right hits.")
}


