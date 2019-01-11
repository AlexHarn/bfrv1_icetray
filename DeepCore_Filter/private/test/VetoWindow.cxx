#include <I3Test.h>

#include "DeepCore_Filter/I3DeepCoreFunctions.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "icetray/OMKey.h"

#include "TestHelpers.cxx"

TEST_GROUP(VetoWindow);

TEST(VetoWindow_Pulses){
    // Create the geometry
    const I3Geometry geo = DCTestHelpers::GetTestGeometry();

    // Finally, create a place to store the output
    I3Particle *cog = new I3Particle();
    cog->SetPos(0,0,0);
    cog->SetTime(0);
    
    // And a faux hit series
    I3RecoPulseSeriesMapPtr pulsemap(new I3RecoPulseSeriesMap());
    (*pulsemap)[OMKey(4,1)] = I3RecoPulseSeries();
    for (int i=0; i>-1000; --i)
      (*pulsemap)[OMKey(4,1)].push_back(DCTestHelpers::MakePulse(i));

    int nVetoHits = 0;
    double nVetoCharge = 0;
    
    DeepCoreFunctions<I3RecoPulse>::CountHitsInWindow(geo,
						      pulsemap,
						      cog, 
						      nVetoHits,
						      nVetoCharge,
						      false);

    ENSURE_EQUAL(nVetoHits, 16, "Not enough hits found for veto calculation.");
    ENSURE_EQUAL(nVetoCharge, 16, "Not enough charge found for veto calculation.");
}



TEST(VetoWindow_Launches){
    // Create the geometry
    const I3Geometry geo = DCTestHelpers::GetTestGeometry();

    // Finally, create a place to store the output
    I3Particle *cog = new I3Particle();
    cog->SetPos(0,0,0);
    cog->SetTime(0);
    
    // And a faux hit series
    I3DOMLaunchSeriesMapPtr launchmap(new I3DOMLaunchSeriesMap());
    (*launchmap)[OMKey(4,1)] = I3DOMLaunchSeries();
    for (int i=0; i>-1000; --i)
      (*launchmap)[OMKey(4,1)].push_back(DCTestHelpers::MakeLaunch(i));

    int nVetoHits = 0;
    double nVetoCharge = 0;
    
    DeepCoreFunctions<I3DOMLaunch>::CountHitsInWindow(geo,
						      launchmap,
						      cog, 
						      nVetoHits,
						      nVetoCharge,
						      false);

    ENSURE_EQUAL(nVetoHits, 16, "Not enough hits found for veto calculation.");
    ENSURE_EQUAL(nVetoCharge, 16, "Not enough charge found for veto calculation.");
}


