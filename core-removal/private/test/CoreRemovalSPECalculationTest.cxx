/**
 *
 * Unit test for the core-removal
 *
 * (c) 2009
 * the IceCube Collaboration
 * $Id$
 *
 * @file CoreRemovalSPECalculationTest.cxx
 * @date $Date$
 * @author panknin
 *
 */

#include <math.h>

#include <I3Test.h>
#include "I3CascadeFitCoreRemovalTester.h"
#include "dataclasses/physics/I3Particle.h"

boost::shared_ptr<I3CascadeFitCoreRemoval> getRemover ();

TEST_GROUP(CoreRemovalSPECalculationTest);

TEST(smallEnergy) {
  
  I3CascadeFitCoreRemovalTester coreRemover;
  // for E=1 we get Rmin
  ENSURE_DISTANCE (coreRemover.CalculateSPERadius (1), 5, 1e-12);
  // for low energy it is prop to (lnE)^2, so 0.1 and 10 get same result
  ENSURE_DISTANCE (coreRemover.CalculateSPERadius (0.1), 
  		   coreRemover.CalculateSPERadius (0.1), 1e-12);
  // set lnE=1, so we get Rmin plus spline coefficent
  // this is c_s=lambda_att/(2 lnEc)
  ENSURE_DISTANCE (coreRemover.CalculateSPERadius (std::exp (1)),
  		   5. + 29. / 2. / std::log (750), 1e-12) 
}

TEST(highEnergy) {
  I3CascadeFitCoreRemovalTester coreRemover;
  double cLambda = 5. - 29. * std::log (750) / 2.;
  // calculation by hand for two values
  ENSURE_DISTANCE (coreRemover.CalculateSPERadius (1000), 
		   29. * std::log (1000) + cLambda, 1e-12);
  ENSURE_DISTANCE (coreRemover.CalculateSPERadius (2000), 
		   29. * std::log (2000) + cLambda, 1e-12);
}

TEST(ParticelAndEnergyVersionMatch) {
  I3CascadeFitCoreRemovalTester coreRemover;
  // particle and double function should return the same result
  I3ParticlePtr particle (new I3Particle);
  // low energy
  particle->SetEnergy (10);
  ENSURE_DISTANCE (coreRemover.CalculateSPERadius (particle), 
		   coreRemover.CalculateSPERadius (10), 1e-12);
  // high energy
  particle->SetEnergy (1000);
  ENSURE_DISTANCE (coreRemover.CalculateSPERadius (particle), 
		   coreRemover.CalculateSPERadius (1000), 1e-12);

}

TEST(BorderMatch) {
  I3CascadeFitCoreRemovalTester coreRemover;
  // spline should fit close to critical energy
  ENSURE_DISTANCE (coreRemover.CalculateSPERadius (750. - 1e-12), 
		   coreRemover.CalculateSPERadius (750. + 1e-12), 1e-6);
}


