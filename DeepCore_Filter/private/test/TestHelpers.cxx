#include "DeepCore_Filter/I3DeepCoreFunctions.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/I3Map.h"
#include "icetray/I3Units.h"
#include "icetray/OMKey.h"

#include <vector>
#include <string>

#ifndef DCFILTER_TESTHELPERS_
#define DCFILTER_TESTHELPERS_

namespace DCTestHelpers
{
  static const I3Geometry& GetTestGeometry();
  static I3RecoPulse MakePulse(double time);
  static I3DOMLaunch MakeLaunch(double time);
}


const I3Geometry& DCTestHelpers::GetTestGeometry()
{
    log_debug("Creating test geometry.");
    I3Geometry *geometry = new I3Geometry();
    I3OMGeoMap &geo = geometry->omgeo;

    for (int i=1; i<=9; ++i)
      for (int j=1; j<=3; ++j)
	geo[OMKey(i,j)].position = I3Position(10*(i%3-1),
					      10*(i/3-1),
					      10*(j-2));
    return *geometry;
}

I3RecoPulse DCTestHelpers::MakePulse(double time)
{
  I3RecoPulse *pulse = new I3RecoPulse();
  pulse->SetTime(time);
  pulse->SetCharge(1);
  return *pulse;
}

I3DOMLaunch DCTestHelpers::MakeLaunch(double time)
{
  I3DOMLaunch *launch = new I3DOMLaunch();
  launch->SetStartTime(time);
  return *launch;
}

#endif
