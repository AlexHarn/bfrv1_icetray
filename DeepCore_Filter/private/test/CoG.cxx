#include <I3Test.h>

#include "DeepCore_Filter/I3DeepCoreFunctions.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "icetray/OMKey.h"

#include "TestHelpers.cxx"

TEST_GROUP(CoGFunctions);

TEST(CoG_Pulses){
    // Create the geometry
    const I3Geometry geo = DCTestHelpers::GetTestGeometry();
    
    // create a place to store the output
    I3Particle *cog = new I3Particle();
    double sigma = -1;

    // And a faux hit series
    I3RecoPulseSeriesMapPtr pulsemap(new I3RecoPulseSeriesMap());
    (*pulsemap)[OMKey(1,1)] = I3RecoPulseSeries();
    (*pulsemap)[OMKey(1,1)].push_back(DCTestHelpers::MakePulse(1));
    (*pulsemap)[OMKey(1,1)].push_back(DCTestHelpers::MakePulse(20));
    (*pulsemap)[OMKey(7,3)] = I3RecoPulseSeries();
    (*pulsemap)[OMKey(7,3)].push_back(DCTestHelpers::MakePulse(30));
    (*pulsemap)[OMKey(7,3)].push_back(DCTestHelpers::MakePulse(40));
    (*pulsemap)[OMKey(7,3)].push_back(DCTestHelpers::MakePulse(50));

    // OMKey(1,1) is at (0, -10, -10)
    // OMKey(7,3) is at (0, 10, 10)

    DeepCoreFunctions<I3RecoPulse>::GetCoG(geo,
					   pulsemap,
					   cog, 
					   false,
					   true);
    
    DeepCoreFunctions<I3RecoPulse>::GetCoGTime(geo,
					       pulsemap,
					       cog, 
					       false);

    I3Position expected(0, 2, 2);
    
    ENSURE_DISTANCE(cog->GetPos().GetX(), expected.GetX(), 0.0001, 
		    "CoG X position is not at the right place");
    ENSURE_DISTANCE(cog->GetPos().GetY(), expected.GetY(), 0.0001, 
		    "CoG Y position is not at the right place");
    ENSURE_DISTANCE(cog->GetPos().GetZ(), expected.GetZ(), 0.0001, 
		    "CoG Z position is not at the right place");
    ENSURE_DISTANCE(cog->GetTime(),
		    33.223, 0.001, "CoG time is not at the right place")
}



TEST(CoG_Launches){
    // Create the geometry
    const I3Geometry geo = DCTestHelpers::GetTestGeometry();
    
    // create a place to store the output
    I3Particle *cog = new I3Particle();
    double sigma = -1;

    // And a faux hit series
    I3DOMLaunchSeriesMapPtr launchmap(new I3DOMLaunchSeriesMap());
    (*launchmap)[OMKey(1,1)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(1,1)].push_back(DCTestHelpers::MakeLaunch(1));
    (*launchmap)[OMKey(2,2)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(2,2)].push_back(DCTestHelpers::MakeLaunch(20));
    (*launchmap)[OMKey(3,3)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(3,3)].push_back(DCTestHelpers::MakeLaunch(30));
    (*launchmap)[OMKey(6,1)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(6,1)].push_back(DCTestHelpers::MakeLaunch(40));
    (*launchmap)[OMKey(7,3)] = I3DOMLaunchSeries();
    (*launchmap)[OMKey(7,3)].push_back(DCTestHelpers::MakeLaunch(50));

    DeepCoreFunctions<I3DOMLaunch>::GetCoG(geo,
					   launchmap,
					   cog, 
					   true,
					   true);
    
    DeepCoreFunctions<I3DOMLaunch>::GetCoGTime(geo,
					       launchmap,
					       cog, 
					       true);

    I3Position expected(-2, 0, 0);
    
    ENSURE_DISTANCE(cog->GetPos().GetX(), expected.GetX(), 0.0001, 
		    "CoG X position is not at the right place");
    ENSURE_DISTANCE(cog->GetPos().GetY(), expected.GetY(), 0.0001, 
		    "CoG Y position is not at the right place");
    ENSURE_DISTANCE(cog->GetPos().GetZ(), expected.GetZ(), 0.0001, 
		    "CoG Z position is not at the right place");
    ENSURE_DISTANCE(cog->GetTime(),
		    38.0719, 0.001, "CoG time is not at the right place")
}



