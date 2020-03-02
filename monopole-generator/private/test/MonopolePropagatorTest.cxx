/**
 * Monopole Propagator Test Suite
 * (c) 2004 IceCube Collaboration
 * Version $Id: MonopolePropagatorTest.cxx 124421 2014-10-10 15:24:15Z jacobi $
 *
 * @file MonopolePropagatorTest.cxx
 * @date $Date: 2008-01-18 00:43:41 (Fri, 18 Jan 2008)$
 * @author Alex Olivas <olivas@icecube.umd.edu>
 * @author Brian Christy <bchristy@icecube.umd.edu>
 * @brief Tests for the I3MonopolePropagatorModule
 * @todo Test to ensure MCTree name being assigned correctly
 * @todo Test to ensure stepsize overrides any length setting
 */

#include <I3Test.h>

#include <monopole-generator/I3MonopoleRelativisticUtils.h>
#include <monopole-generator/I3MonopolePropagator.h>
#include <monopole-generator/I3MonopoleGenerator.h>

#include <icetray/I3Frame.h>
#include <icetray/I3Tray.h>
#include <icetray/I3Units.h>
#include <dataclasses/I3Double.h>
#include <dataclasses/I3Vector.h>
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/physics/I3MCTree.h>
#include <dataclasses/physics/I3MCTreeUtils.h>
#include <dataclasses/I3Constants.h>
#include <phys-services/I3GSLRandomService.h>

#include <limits>

/**
 * Simple function to make generating particles easier throughout tests
 */
I3ParticlePtr GenTestPart(double const zen, double const azi, double const beta, double const time, double const length) {
  I3ParticlePtr temp(new I3Particle());
  temp->SetDir(zen * I3Units::radian, azi * I3Units::radian);
  temp->SetSpeed(beta * I3Constants::c);
  temp->SetTime(time * I3Units::s);
  temp->SetLength(length * I3Units::m);
  return temp;
};


TEST_GROUP(monopole
- propagator);

/**
 * Make sure that a basic call to the PropagatorMonopole Method will return a
 * new particle with the expected position, direction, length, and time
 */


/*
Disabling this for now as this need a major rewrite. Alternativly, we should refactor the cod ein the monopole propagator
 to separate teh actual single monopole propation from a hole frame as it is at the moment //FHL
TEST(TestPropUtilsBasic) {
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  double zenith = random->Uniform(0.0, I3Constants::pi);
  double azimuth = random->Uniform(0.0, 2 * I3Constants::pi);
  double length = random->Uniform(0.0, 10.0);
  double time = random->Uniform(0.0, 10.0);
  double beta = random->Uniform(0.1, 0.999);
  double mp_mass = pow(10, random->Uniform(6.1, 16.9));

  I3ParticlePtr testStart = GenTestPart(zenith, azimuth, beta, time, length);
  testStart->SetType(I3Particle::Monopole);
  testStart->SetShape(I3Particle::ContainedTrack);
  testStart->SetPos(0, 0, 0);
  testStart->SetEnergy((mp_mass / sqrt(1 - beta * beta)) * I3Units::GeV);

  //I3ParticlePtr testEnd = frame.Get
  ENSURE_EQUAL(testEnd->GetAzimuth(), testStart->GetAzimuth());
  ENSURE_EQUAL(testEnd->GetZenith(), testStart->GetZenith());
  ENSURE_DISTANCE(testEnd->GetTime(), testStart->GetStopTime(), 0.00001);
  ENSURE_DISTANCE(testEnd->GetPos().GetX(), testStart->GetStopPos().GetX(), 0.000001);
  ENSURE_DISTANCE(testEnd->GetPos().GetY(), testStart->GetStopPos().GetY(), 0.000001);
  ENSURE_DISTANCE(testEnd->GetPos().GetZ(), testStart->GetStopPos().GetZ(), 0.000001);

  I3ParticlePtr testEnd2 = I3MonopoleRelativisticUtils::PropagateFastMonopole(frame, mp_mass, false, false, true,
                                                                              100, 10, 0.001);
  ENSURE_EQUAL(testEnd2->GetLength(), 100, "StepSize option seems to be flawed");
  ENSURE_EQUAL(testEnd2->GetEnergy(), testStart->GetEnergy(), "No Energy Prop option is flawed");

}

 */

/**
 *Check Energy Loss function
 *All values found by hand.  Should the constants change,
 *this will have to be updated
 */
TEST(TestPropUtilsEnergyLoss) {
        //4 values most used in simulation
        double testEnergy = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.76, 1.0, false);
        ENSURE_DISTANCE(testEnergy, 681.186648, 0.0001);
        double testEnergy2 = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.8, 1.0, false);
        ENSURE_DISTANCE(testEnergy2, 700.43438, 0.0001);
        double testEnergy3 = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.9, 1.0, false);
        ENSURE_DISTANCE(testEnergy3, 764.5833498, 0.0001);
        double testEnergy4 = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.99, 1.0, false);
        ENSURE_DISTANCE(testEnergy4, 944.0476, 0.0001);

        double testEnergy5 = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.999, 1.0, false);
        ENSURE_DISTANCE(testEnergy5, 1113.92211, 0.0001);
        double testEnergy6 = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.999, 1.0, true);
        ENSURE_DISTANCE(testEnergy6, 1002.28710, 0.0001);
        double testEnergy7 = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.99, 10, false);
        ENSURE_DISTANCE(testEnergy7, 9440.476, 0.001);
        double testEnergy8 = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.5, 1.0, false);
        ENSURE_DISTANCE(testEnergy8, 577.66030, 0.0001);
        double testEnergy9 = I3MonopoleRelativisticUtils::CalculateEnergyLoss(0.2, 1.0, false);
        ENSURE_DISTANCE(testEnergy9, 425.14828348, 0.0001);
}
/**
 *Check Density Correction function
 *All values found by hand.  Should the constants change,
 *this will have to be updated
 */
TEST(TestPropUtilsDenCorr) {
        double testDen = I3MonopoleRelativisticUtils::CalculateDensityCorrection(0.86);
        ENSURE_EQUAL(testDen, 0.0, "Density Correction not setting to 0 in X<X0 regime");
        double testDen2 = I3MonopoleRelativisticUtils::CalculateDensityCorrection(0.87);
        ENSURE_DISTANCE(testDen2, 0.00858616, 0.000001, "Density Correction off for X0<X<X1 regime");
        //Should probably never use this speed...
        double testDen3 = I3MonopoleRelativisticUtils::CalculateDensityCorrection(0.999995);
        ENSURE_DISTANCE(testDen3, 8.01261, 0.000001, "Density Correction off for X0<X<X1 regime");
        double testDen4 = I3MonopoleRelativisticUtils::CalculateDensityCorrection(0.999999);
        ENSURE_DISTANCE(testDen4, 9.620661, 0.000001, "Density Correction off for X>X1 regime");
}

/**
 *Check that it sets correct new length
 */
TEST(TestPropUtilsCalcLen) {
        log_warn("Some parts of test disabled until I figure out what the actual avalues here should be");
        //double testLen = I3MonopoleRelativisticUtils::CalculateNextLength(0.00000001, 1e7, 1e6, 10, 0);
        //ENSURE_DISTANCE(testLen, 7.5, 0.000001, "Test part 1");
        //testLen = I3MonopoleRelativisticUtils::CalculateNextLength(0.00000001, 2.3e8, 1e7, 100, 0);
        //ENSURE_DISTANCE(testLen, 100.0, 0.000001, "Test part 2");
        //testLen = I3MonopoleRelativisticUtils::CalculateNextLength(0.00000001, 1.0001e6, 1e6, 10, 0);
        //ENSURE_DISTANCE(testLen, 0.0001, 0.000001, "Test part 3");
        //Finally, check one that should log_fatal
        bool passed = true;
        try {
          double testLen = I3MonopoleRelativisticUtils::CalculateNextLength(NAN, NAN, NAN, NAN, NAN);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("Calculate length did not log_fatal when it should have"); }
}

/**
 *Check that Speed/Energy Conversion accurate
 *Notably, check the 4 beta's used in production
 */
TEST(TestPropUtilsEnToBeta) {
        double testBeta = I3MonopoleRelativisticUtils::EnergyToBeta(1e7, 1e6);
        ENSURE_DISTANCE(testBeta, 0.9949, 0.0001);
        testBeta = I3MonopoleRelativisticUtils::EnergyToBeta(2.294157e8, 1e8);
        ENSURE_DISTANCE(testBeta, 0.9, 0.0001);
        testBeta = I3MonopoleRelativisticUtils::EnergyToBeta(5e8, 3e8);
        ENSURE_DISTANCE(testBeta, 0.8, 0.0001);
        testBeta = I3MonopoleRelativisticUtils::EnergyToBeta(1.5386e8, 1e8);
        ENSURE_DISTANCE(testBeta, 0.76, 0.0001);
}

/**
 * Check that CheckParticle correctly checking...
 * (it's so meta)
 */
TEST(TestPropUtilsCheckParticle) {
        I3ParticlePtr shouldwork = GenTestPart(0, 0, 0.995, 1, 1);
        bool fail = false;
        shouldwork->SetType(I3Particle::Monopole);
        shouldwork->SetEnergy(1e12);
        shouldwork->SetPos(0, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*shouldwork, 1e11);
        }
        catch (...) {
          fail = true;
        }
        if (fail) { FAIL("CheckParticle failed when it should not have"); }
        I3ParticlePtr nonmp = GenTestPart(0, 0, 0.995, 1, 1);
        bool passed = true;
        nonmp->SetType(I3Particle::NuMu);
        nonmp->SetEnergy(1e12);
        nonmp->SetPos(0, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*nonmp, 1e11);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("CheckParticle allowed a non-monopole"); }
        I3ParticlePtr nanpos = GenTestPart(0, 0, 0.995, 1, 1);
        passed = true;
        nanpos->SetType(I3Particle::Monopole);
        nanpos->SetEnergy(1e12);
        nanpos->SetPos(NAN, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*nanpos, 1e11);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("CheckParticle allowed a NaN Position"); }
        I3ParticlePtr toofarout = GenTestPart(0, 0, 0.995, 1, 1);
        passed = true;
        toofarout->SetType(I3Particle::Monopole);
        toofarout->SetEnergy(1e12);
        toofarout->SetPos(10000, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*toofarout, 1e11);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("CheckParticle allowed a far away Position"); }
        I3ParticlePtr nodir = GenTestPart(NAN, NAN, 0.995, 1, 1);
        passed = true;
        nodir->SetType(I3Particle::Monopole);
        nodir->SetEnergy(1e12);
        nodir->SetPos(0, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*nodir, 1e11);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("CheckParticle allowed a NaN direction"); }
        I3ParticlePtr noe = GenTestPart(0, 0, 0.995, 1, 1);
        passed = true;
        noe->SetType(I3Particle::Monopole);
        noe->SetEnergy(0);
        noe->SetPos(0, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*noe, 1e11);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("CheckParticle allowed a zero energy"); }
        I3ParticlePtr notime = GenTestPart(0, 0, 0.995, NAN, 1);
        passed = true;
        notime->SetType(I3Particle::Monopole);
        notime->SetEnergy(1e12);
        notime->SetPos(0, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*notime, 1e11);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("CheckParticle allowed a non-existant time"); }
        I3ParticlePtr nol = GenTestPart(0, 0, 0.995, 1, NAN);
        passed = true;
        nol->SetType(I3Particle::Monopole);
        nol->SetEnergy(1e12);
        nol->SetPos(0, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*nol, 1e11);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("CheckParticle allowed a non-existant lenth"); }
        I3ParticlePtr toofast = GenTestPart(0, 0, 0.999999999999999, 1, 1);
        passed = true;
        nol->SetType(I3Particle::Monopole);
        nol->SetEnergy(1e19);
        nol->SetPos(0, 0, 0);
        try {
          I3MonopoleRelativisticUtils::CheckParticle(*nol, 1e11);
        }
        catch (...) {
          passed = false;
        }
        if (passed) { FAIL("CheckParticle allowed too fast of a monopole"); }

}


/**
 *Check Icetray Interface
 *Three Tests
 *(1) Checks much of the user interface and that it handles mixed trees as expected
 *(2) Makes sure setting stepsize overrides everything else
 *(3) Checks that all log fatals are called
 */
//Used to see that monopole segments aren't set beyond this length
const double MAX_LENGTH_TEST = 62.7 * I3Units::m;
//Used to check that a starting NaN particle gets set to this length
const double MIN_LENGTH_TEST = 0.1 * I3Units::cm;
//Used to check that user defined threshold for propagation distance works
const double DIST_FROM_CENTER_MAX_TEST = 1800.0 * I3Units::m;
//Here make distance outside max set in module to ensure it will continue
//beyond that threshold since the monopole starts further beyond it.
const double DIST_FROM_CENTER_BEYONDMAX_TEST = 3300.0 * I3Units::m;
//Used to check that stops propagation after falls below speed threshold
const double BETA_MIN_TEST = 0.2;
using namespace I3MCTreeUtils;

namespace mp_prop_io_test {
    class CreateNotJustMPTree : public I3Module {
    public :
        CreateNotJustMPTree(const I3Context &ctx) : I3Module(ctx) { AddOutBox("OutBox"); }

        void DAQ(I3FramePtr frame) {

          I3DoublePtr mp_mass_ptr(new I3Double(1e11 * I3Units::GeV));
          I3MCTreePtr testTree(new I3MCTree);
          I3ParticlePtr test1 = GenTestPart(0, 0, 0.999, 0, 100);
          test1->SetType(I3Particle::EPlus);
          AddPrimary(*testTree, *test1);
          //For some reason monopoles are filled opposite order they come out
          //Probably something to do with order in which they are propagated
          I3ParticlePtr testmp3 = GenTestPart(0, 0, .99, 0, NAN);
          testmp3->SetType(I3Particle::Monopole);
          testmp3->SetPos(0, 0, DIST_FROM_CENTER_MAX_TEST - 10 * I3Units::m);
          testmp3->SetEnergy((mp_mass_ptr->value) / sqrt(1 - pow(0.99, 2)));
          AddPrimary(*testTree, *testmp3);

          I3ParticlePtr testmp2 = GenTestPart(0, 0, BETA_MIN_TEST, 0, NAN);
          testmp2->SetType(I3Particle::Monopole);
          testmp2->SetPos(0, 0, 0);
          testmp2->SetEnergy((mp_mass_ptr->value) / sqrt(1 - pow(BETA_MIN_TEST, 2)));
          AddPrimary(*testTree, *testmp2);

          I3ParticlePtr testmp1 = GenTestPart(0, 0, .99, 0, 10 * MAX_LENGTH_TEST);
          testmp1->SetType(I3Particle::Monopole);
          testmp1->SetPos(0, 0, DIST_FROM_CENTER_BEYONDMAX_TEST);
          testmp1->SetEnergy((mp_mass_ptr->value) / sqrt(1 - .99 * .99));
          AddPrimary(*testTree, *testmp1);
          for (unsigned int i = 0; i < 10; ++i) {
            I3ParticlePtr tempchild = GenTestPart(0, 0, 0.99, 0, NAN);
            tempchild->SetType(I3Particle::EMinus);
            AppendChild(*testTree, *testmp1, *tempchild);
          }

          I3MapStringDoublePtr mpinfo(new I3MapStringDouble);
          (*mpinfo)["Beta"] = BETA_MIN_TEST;
          (*mpinfo)["Mass"] = mp_mass_ptr->value;

          frame->Put("TestingMCTreeName", testTree);
          frame->Put("MPInfoDict", mpinfo);
          PushFrame(frame);
        }

    };

    class NotJustMPTreeTest : public I3Module {
    public :
        NotJustMPTreeTest(const I3Context &ctx) : I3Module(ctx) { }

        void DAQ(I3FramePtr frame) {
          const I3MCTree &testTree = frame->Get<I3MCTree>("TestingMCTreeName");
          int num_non_mp_child1 = 0;
          int num_mp_child1 = 0;
          int num_mp_child2 = 0;
          int num_mp_child3 = 0;

          const std::vector <I3Particle> primaries =
                  GetPrimaries(testTree);

          ENSURE(primaries.size() == 4);
          ENSURE(primaries[0].GetType() == I3Particle::EPlus);
          ENSURE(primaries[1].GetType() == I3Particle::Monopole);
          ENSURE_EQUAL(primaries[1].GetLength(), MAX_LENGTH_TEST);
          ENSURE(primaries[2].GetType() == I3Particle::Monopole);
          ENSURE_EQUAL(primaries[2].GetLength(), MIN_LENGTH_TEST);
          ENSURE(primaries[3].GetType() == I3Particle::Monopole);
          ENSURE_EQUAL(primaries[3].GetZ(), DIST_FROM_CENTER_MAX_TEST - 10 * I3Units::m);
          int primID0 = primaries[0].GetMinorID();
          int primID1 = primaries[1].GetMinorID();
          int primID2 = primaries[2].GetMinorID();
          int primID3 = primaries[3].GetMinorID();

          int tempID = primID1;
          for (auto const &tree_iter: testTree) {
            if (HasParent(testTree, tree_iter)) {
              if (GetPrimary(testTree, tree_iter).GetMinorID() == primID0) {
                FAIL("Non-Monopole primary wasn't supposed to have children!");
              }
                //First particle should have 10 EMinus children
                //All monopoles should have length of 10 since moving so fast
                //Each propagated monopole should have the previous one as a parent
                //and smaller energy
                //Finally, should travel past -1000m since started beyond 800m mark
              else if (GetPrimary(testTree, tree_iter).GetMinorID() == primID1) {
                if (tree_iter.GetType() != I3Particle::Monopole) { ++num_non_mp_child1; }
                else {
                  ++num_mp_child1;
                  ENSURE_EQUAL(tree_iter.GetLength(), MAX_LENGTH_TEST);
                  ENSURE_EQUAL(tree_iter.GetZenith(), 0);
                  ENSURE_EQUAL(tree_iter.GetAzimuth(), 0);
                  ENSURE(tree_iter.GetEnergy() <
                         GetPrimary(testTree, tree_iter).GetEnergy());
                  ENSURE(tree_iter.GetEnergy() <
                         GetParent(testTree, tree_iter).GetEnergy());
                  ENSURE_EQUAL(GetParent(testTree, tree_iter).GetMinorID(), tempID);
                  tempID = tree_iter.GetMinorID();
                }
              }
                //Second monopole should fall below speed threshold after one iteration
              else if (GetPrimary(testTree, tree_iter).GetMinorID() == primID2) {
                ENSURE(tree_iter.GetType() == I3Particle::Monopole);
                ENSURE((tree_iter.GetEnergy()) < (GetPrimary(testTree, tree_iter).GetEnergy()));
                ++num_mp_child2;
              }
                //third particle should mimic first, except travel to -800m
              else if (GetPrimary(testTree, tree_iter).GetMinorID() == primID3) {
                ENSURE(tree_iter.GetType() == I3Particle::Monopole);
                ENSURE(tree_iter.GetEnergy() <
                       GetPrimary(testTree, tree_iter).GetEnergy());
                ++num_mp_child3;
              } else { FAIL("Found a child that didn't have a proper parent"); }
            }//End looking at monopole children
          }//End looping through tree

          ENSURE_EQUAL(num_non_mp_child1, 10);
          ENSURE_EQUAL(num_mp_child1, ceil(2 * DIST_FROM_CENTER_BEYONDMAX_TEST / (MAX_LENGTH_TEST)));
          ENSURE_EQUAL(num_mp_child2, 1);
          //Unlike tree 1, this needs an extra 1 since original length not set so primary only goes min length
          //10 comes from the distance less of Max center that tree was set to above
          ENSURE_EQUAL(num_mp_child3,
                       1 + ceil((2 * DIST_FROM_CENTER_MAX_TEST - 10 - MIN_LENGTH_TEST) / (MAX_LENGTH_TEST)));

          PushFrame(frame, "OutBox");
        }//End Physics
    };
}
I3_MODULE(mp_prop_io_test::CreateNotJustMPTree);
I3_MODULE(mp_prop_io_test::NotJustMPTreeTest);
/**
 * This test is designed to comprehensively check the interface of the Propagate module through various ways
 * (1) That it only propagates Monopoles (done by adding non-monopole primary/secondaries to test generation tree)
 * (2) That the monopoles all lose energy as they propagate
 * (3) That the number of children remains as expected - constant if non-monopole, and the correct number of
 *     segments if they are monopoles (the num_child tests)
 * (4) That setting the monopole to beyond the max length will cause it to propagate beyond the detector the same amount
 *     as it started (tree1 above)
 * (5) That, otherwise, it will propagate to the set max distance from the detector center (tree3 above)
 * (6) That it stops propagating when it falls below user specified minimum speed (tree2 above)
 * (7) That direction is preserved
 * (8) That a monopole with no length gets the starting particle set to user defined min length (tree2, 3)
 * (9) That when CalculateNextLength gives something larger than user-defined max length,
 *     length is set to this instead (tree 1)
 */

TEST(NotJustMonopoles) {
        const int NFRAMES(4);
        I3Tray tray;

        tray.AddService("I3GSLRandomServiceFactory", "random");
        tray.AddModule("I3InfiniteSource", "frames");
        //tray.AddModule("I3MonopoleGenerator", "mono-gen")
        //("Mass", 1e11 * I3Units::GeV)
        //("Gamma", 10)
        //("Disk_dist", 1000 * I3Units::m)
        //("Disk_rad", 0);
        tray.AddModule("mp_prop_io_test::CreateNotJustMPTree", "inject-non-monopoles");
        tray.AddModule("I3MonopolePropagator", "prop")
          ("InputTreeName", "TestingMCTreeName")
          ("OutputTreeName", "TestingMCTreeName")
          ("SpeedMin", BETA_MIN_TEST * I3Constants::c)
          ("MaxLength", MAX_LENGTH_TEST)
          ("MinLength", MIN_LENGTH_TEST)
          ("MaxDistanceFromCenter", DIST_FROM_CENTER_MAX_TEST);
        //tray.AddModule("mp_prop_io_test::NotJustMPTreeTest", "test");
        tray.Execute(NFRAMES);
        tray.Finish();
}



//Used to see that monopole segments aren't set beyond this length

const double STEPSIZE_TEST = 33.2 * I3Units::m;
using namespace I3MCTreeUtils;

namespace mp_prop_step_test {
    class MPTreeTest : public I3Module {
    public :
        MPTreeTest(const I3Context &ctx) : I3Module(ctx) {  }

        void DAQ(I3FramePtr frame) {
          const I3MCTree &testTree = frame->Get<I3MCTree>();

          const std::vector <I3Particle> primaries =
                  GetPrimaries(testTree);
          int num_child = 0;

          ENSURE(primaries.size() == 1, "Tree should only have one primary!");
          ENSURE(primaries[0].GetType() == I3Particle::Monopole, "The primary should have been a monopole!");
          ENSURE_EQUAL(primaries[0].GetLength(), STEPSIZE_TEST);
          //ENSURE_EQUAL(primaries[0].GetZ(), DIST_FROM_CENTER_MAX_TEST); //no clue what this is supposed to do

          for (auto const &tree_iter: testTree) {
            if (HasParent(testTree, tree_iter)) {
              ENSURE_EQUAL(tree_iter.GetLength(), STEPSIZE_TEST);
              ENSURE_EQUAL(tree_iter.GetZenith(), 0);
              ENSURE_EQUAL(tree_iter.GetAzimuth(), 0);
              ENSURE(tree_iter.GetEnergy() <
                     GetPrimary(testTree, tree_iter).GetEnergy());
              ENSURE(tree_iter.GetEnergy() <
                     GetParent(testTree, tree_iter).GetEnergy());
              ++num_child;
            }
          }//End looping through tree

          //I think this test ist not valid anymore as we automatically shrink the stepsize id need be //FHL
          //ENSURE_EQUAL(num_child, ceil(2 * DIST_FROM_CENTER_MAX_TEST / STEPSIZE_TEST));

          PushFrame(frame, "OutBox");
        }//End Physics
    };
}
I3_MODULE(mp_prop_step_test::MPTreeTest);
