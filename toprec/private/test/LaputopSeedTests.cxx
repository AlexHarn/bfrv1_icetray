/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: I3CalculatorTest.cxx 9161 2005-06-14 16:44:58Z pretz $

    @version $Revision: 1.2 $
    @date $Date: 2005-06-14 12:44:58 -0400 (Tue, 14 Jun 2005) $
    @author dule

    @todo
*/

#include <I3Test.h>
#include <stdio.h>

#include "interfaces/I3GeometryService.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "phys-services/source/I3GCDFileService.h"

#include "toprec/I3LaputopSeedService.h"
#include "recclasses/I3TopRecoPlaneFitParams.h"
#include "recclasses/I3TopLateralFitParams.h"


// Fake pulseseries containing 18 pulses:
extern I3RecoPulseSeriesMapPtr testPSM_tweaked();  // <-- defined in LaputopLikelihoodTests.cxx


TEST_GROUP(Seeds);


TEST(FromCOGAndPlane)
{
  printf("Init! \n");
  // This one has to make a "guess" at S125, which needs pulses and geometry, so
  // gotta fill that stuff. *sigh*
  std::string gcd(getenv("I3_TESTDATA"));
  gcd = gcd+"/sim/GeoCalibDetectorStatus_IC79.55380_corrected.i3.gz";
  // Create a fake frame, and put some stuff in it
  I3FramePtr frame(new I3Frame(I3Frame::Physics));
  I3Time time1(2010,171195332233603703ULL ); // first event in my test file
  // G
  I3GeometryServicePtr geoservice(new I3GCDFileGeometryService(gcd));
  assert (geoservice);
  I3GeometryConstPtr geometry1 = geoservice->GetGeometry(time1);
  assert (geometry1);
  frame->Put("I3Geometry", geometry1, I3Frame::Geometry);
  // C and D not required here.

  // Fake pulses
  printf("About to call.. \n");
  I3RecoPulseSeriesMapConstPtr psmptr = testPSM_tweaked();
  frame->Put("FakePulseSeriesMap",psmptr);

  // Particles for the seed: COG and Plane (with PlaneParams)
  // Also based (loosely) on the first event from Level2a_Laputop_IT73_0718_116195.i3
  I3ParticlePtr pc( new I3Particle);
  pc->SetPos(-235, 202, 1950);   // x, y, z    <-- this one has NO direction
  pc->SetTime(10260.0);         // <-- not used; time comes from PlaneParams (adjusted)
  pc->SetFitStatus(I3Particle::OK);
  frame->Put("PreviousCOG", pc);
  I3ParticlePtr pp( new I3Particle);
  pp->SetDir(M_PI/5, M_PI/4);   // zen/az    <-- this one has NO position or time
  pp->SetFitStatus(I3Particle::OK);
  frame->Put("PreviousPlane", pp);
  I3TopRecoPlaneFitParamsPtr plpar(new I3TopRecoPlaneFitParams);
  /*  Here's what a typical one looks like:
<I3FrameObject class_name="I3TopRecoPlaneFitParams" version="1">
  <ET>4786140333</ET>
  <DeltaT>91094841.900000006</DeltaT>
  <T0>10255.007</T0>
  <X0>-235.73944</X0>
  <Y0>211.09944</Y0>
  <Chi2>4.914196</Chi2>
  <NPulses>18</NPulses>
</I3FrameObject>
  */
  plpar->X0 = -240;
  plpar->Y0 = 210;
  plpar->T0 = 10255.0;
  plpar->Chi2 = 5.0;  // <-- doesn't matter what this is, just checks that it is >0
  frame->Put("PreviousPlaneParams", plpar);

  // Create the service
  I3LaputopSeedService lservice("LaputopSeed", "PreviousPlane", "PreviousCOG", "",
				4.0, "FakePulseSeriesMap");
  // Call the functions
  unsigned int se = lservice.SetEvent(*frame);
  I3EventHypothesis eh = lservice.GetSeed(0);
  // Test the results
  ENSURE(se == 1);
  ENSURE_DISTANCE(eh.particle->GetPos().GetX(), -235, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetPos().GetY(), 202, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetPos().GetZ(), 1950, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetDir().GetZenith(), M_PI/5, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetDir().GetAzimuth(), M_PI/4, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetTime(), 10259.1591, 0.0001);  //<-- adjusted from the PlaneParams T0
  I3LaputopParamsConstPtr lpp = boost::dynamic_pointer_cast<I3LaputopParams>(eh.nonstd);
  ENSURE_DISTANCE(lpp->GetValue(Laputop::Parameter::Log10_S125), 0.009717, 0.000001);
  ENSURE_DISTANCE(lpp->GetValue(Laputop::Parameter::Beta), 4.0, 0.0001);

}


TEST(FromOldI3TopLateralFitParams)
{
  printf("Init I3TopLateralFitParams test! \n");
  // Create a fake frame, and put some stuff in it
  I3FramePtr frame(new I3Frame(I3Frame::Physics));

  // A particle for the seed
  I3ParticlePtr p( new I3Particle);
  p->SetPos(10, 20, 1730);   // x, y, z
  p->SetDir(M_PI/6, M_PI/4);  // zenith=30deg, azimuth=45deg
  p->SetTime(5000.0);
  p->SetFitStatus(I3Particle::OK);
  frame->Put("PreviousFit", p);

  // Some (old) Params for the seed
  I3TopLateralFitParamsPtr oldp(new I3TopLateralFitParams);
  oldp->S125 = 1.234;
  oldp->Beta = 5.0;
  frame->Put("PreviousFitParams", oldp);

  // Create the service
  I3LaputopSeedService lservice("LaputopSeed", "PreviousFit", "PreviousFit", "PreviousFitParams",
				-1, "NothingHere");
  // Call the functions
  unsigned int se = lservice.SetEvent(*frame);
  I3EventHypothesis eh = lservice.GetSeed(0);
  // Test the results
  ENSURE(se == 1);
  ENSURE_DISTANCE(eh.particle->GetPos().GetX(), 10, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetPos().GetY(), 20, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetPos().GetZ(), 1730, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetDir().GetZenith(), M_PI/6, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetDir().GetAzimuth(), M_PI/4, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetTime(), 5000.0, 0.0001);
  I3LaputopParamsConstPtr lpp = boost::dynamic_pointer_cast<I3LaputopParams>(eh.nonstd);
  ENSURE_DISTANCE(lpp->GetValue(Laputop::Parameter::Log10_S125), log10(1.234), 0.0001);
  ENSURE_DISTANCE(lpp->GetValue(Laputop::Parameter::Beta), 5.0, 0.0001);

}

TEST(FromNewLaputopParams)
{
  printf("Init I3LaputopParams test! \n");
  // Create a fake frame, and put some stuff in it
  I3FramePtr frame(new I3Frame(I3Frame::Physics));

  // A particle for the seed
  I3ParticlePtr p( new I3Particle);
  p->SetPos(10, 20, 1730);   // x, y, z
  p->SetDir(M_PI/6, M_PI/4);  // zenith=30deg, azimuth=45deg
  p->SetTime(5000.0);
  p->SetFitStatus(I3Particle::OK);
  frame->Put("PreviousFit2", p);

  // Some (new) Params for the seed
  I3LaputopParamsPtr newp(new I3LaputopParams);
  newp->SetValue(Laputop::Parameter::Log10_S125, 1.234);
  newp->SetValue(Laputop::Parameter::Beta, 6.0);
  frame->Put("PreviousFit2Params", newp);

  // Create the service
  I3LaputopSeedService lservice("LaputopSeed2", "PreviousFit2", "PreviousFit2", "PreviousFit2Params",
				-1, "NothingHere");
  // Call the functions
  unsigned int se = lservice.SetEvent(*frame);
  I3EventHypothesis eh = lservice.GetSeed(0);
  // Test the results
  ENSURE(se == 1);
  ENSURE_DISTANCE(eh.particle->GetPos().GetX(), 10, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetPos().GetY(), 20, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetPos().GetZ(), 1730, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetDir().GetZenith(), M_PI/6, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetDir().GetAzimuth(), M_PI/4, 0.0001);
  ENSURE_DISTANCE(eh.particle->GetTime(), 5000.0, 0.0001);
  I3LaputopParamsConstPtr lpp = boost::dynamic_pointer_cast<I3LaputopParams>(eh.nonstd);
  ENSURE_DISTANCE(lpp->GetValue(Laputop::Parameter::Log10_S125), 1.234, 0.0001);
  ENSURE_DISTANCE(lpp->GetValue(Laputop::Parameter::Beta), 6.0, 0.0001);

}
