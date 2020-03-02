/**
 * A Monopole Generator Test Suite
 * (c) 2004 - 2014 IceCube Collaboration
 * Version $Id$
 *
 * @file MonopoleGeneratorTests.cxx
 * @date $Date$
 * @author jacobi
 * @author bchristy
 * @author olivas
 * @brief Tests for I3MonopoleGenerator module
 */

#include <I3Test.h>

#include <monopole-generator/I3MonopoleGenerator.h>
#include <monopole-generator/I3MonopoleGeneratorUtils.h>

#include "icetray/I3Tray.h"
#include "icetray/I3Units.h"
#include "phys-services/I3GSLRandomService.h"
#include "phys-services/I3GSLRandomServiceFactory.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"

#include <string>


TEST_GROUP(monopole - generator);

TEST(TestRandomNumberGeneratorUniform) {
        int q0 = 0;
        int q1 = 0;
        int q2 = 0;
        int q3 = 0;
        int qfail = 0;
        unsigned const N = 10000000;
        I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
        for (unsigned int i = 0; i < N; ++i) {
          double tmpbeta(I3MonopoleGeneratorUtils::RandomUniformSampled(random, 0.5, 0.9));
          if (tmpbeta >= 0.5 && tmpbeta < 0.6)
            ++q0;
          else if (tmpbeta >= 0.6 && tmpbeta < 0.7)
            ++q1;
          else if (tmpbeta >= 0.7 && tmpbeta < 0.8)
            ++q2;
          else if (tmpbeta >= 0.8 && tmpbeta <= 0.9)
            ++q3;
          else
            ++qfail;
        }
        ENSURE(qfail == 0);
        //Allow ~3sigma (=3*sqrt(10^5*.25*.75)) deviation for the binomial test
        unsigned int const d = 3 * sqrt(N * 0.25 * 0.75);
        ENSURE_DISTANCE(q0, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
        ENSURE_DISTANCE(q1, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
        ENSURE_DISTANCE(q2, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
        ENSURE_DISTANCE(q3, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
}


/**
 * Test randomization by splitting into equal regions
 * and comparing how many particles end up in each one.
 * This test checks a flat distribution.
 */
TEST(TestRandomNumberGeneratorPowerLawForFlattness) {
  int q0 = 0;
  int q1 = 0;
  int q2 = 0;
  int q3 = 0;
  int qfail = 0;
  unsigned const N = 10000000;
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  for (unsigned int i = 0; i < N; ++i) {
    double tmpbeta(I3MonopoleGeneratorUtils::RandomPowerLawSampled(random, 0.5, 0.9, NAN));
    if (tmpbeta >= 0.5 && tmpbeta < 0.6)
      ++q0;
    else if (tmpbeta >= 0.6 && tmpbeta < 0.7)
      ++q1;
    else if (tmpbeta >= 0.7 && tmpbeta < 0.8)
      ++q2;
    else if (tmpbeta >= 0.8 && tmpbeta <= 0.9)
      ++q3;
    else
      ++qfail;
  }
  ENSURE(qfail == 0);
  //Allow ~3sigma (=3*sqrt(10^5*.25*.75)) deviation for the binomial test
  unsigned int const d = 3 * sqrt(N * 0.25 * 0.75);
  ENSURE_DISTANCE(q0, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q1, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q2, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q3, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
}


TEST(TestRandomNumberGeneratorPowerLaw1) {
  int q0 = 0;
  int q1 = 0;
  int q2 = 0;
  int q3 = 0;
  int qfail = 0;
  unsigned const N = 10000000;
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  for (unsigned int i = 0; i < N; ++i) {
    double tmpbeta(I3MonopoleGeneratorUtils::RandomPowerLawSampled(random, 0.5, 0.9, 1.));
    if (tmpbeta >= 0.5 && tmpbeta < 0.579146)            // 25-percentile
      ++q0;
    else if (tmpbeta >= 0.579146 && tmpbeta < 0.670820)  // 50-percentile
      ++q1;
    else if (tmpbeta >= 0.670820 && tmpbeta < 0.777006)  // 75-percentile
      ++q2;
    else if (tmpbeta >= 0.777006 && tmpbeta <= 0.9)      // 100-percentile
      ++q3;
    else
      ++qfail;
  }
  ENSURE(qfail == 0);
  //Allow ~3sigma (=3*sqrt(10^5*.25*.75)) deviation for the binomial test
  unsigned int const d = 3 * sqrt(N * 0.25 * 0.75);
  ENSURE_DISTANCE(q0, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q1, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q2, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q3, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
}

TEST(TestRandomNumberGeneratorPowerLaw5) {
  int q0 = 0;
  int q1 = 0;
  int q2 = 0;
  int q3 = 0;
  int qfail = 0;
  unsigned const N = 10000000;
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  for (unsigned int i = 0; i < N; ++i) {
    double tmpbeta(I3MonopoleGeneratorUtils::RandomPowerLawSampled(random, 0.5, 0.9, 5.));
    if (tmpbeta >= 0.5 && tmpbeta < 0.533102505)               // 25-percentile
      ++q0;
    else if (tmpbeta >= 0.533102505 && tmpbeta < 0.581230251)  // 50-percentile
      ++q1;
    else if (tmpbeta >= 0.581230251 && tmpbeta < 0.664038667)  // 75-percentile
      ++q2;
    else if (tmpbeta >= 0.664038667 && tmpbeta <= 0.9)         // 100-percentile
      ++q3;
    else
      ++qfail;
  }
  ENSURE(qfail == 0);
  //Allow ~3sigma (=3*sqrt(10^5*.25*.75)) deviation for the binomial test
  unsigned int const d = 3 * sqrt(N * 0.25 * 0.75);
  ENSURE_DISTANCE(q0, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q1, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q2, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q3, (unsigned int) N / 4, d, "A large disagreement in the Monopoles velocity distribution");
}
