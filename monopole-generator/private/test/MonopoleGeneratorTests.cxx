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
#include "phys-services/empty-streams/I3EmptyStreamsFactory.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"

#include <string>

TEST_GROUP(monopole-generator);

/**
 *Test direction randomization
 */
TEST(TestDirRandomization)
{
  int countZG120=0;
  int countZ90120=0;
  int countZ6090=0;
  int countZ060=0;
  int countA090=0;
  int countA90180=0;
  int countA180270=0;
  int countA270360=0;

  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  for(int i=1;i<=100000;i++){
    I3Direction tmpDirTest = I3MonopoleGeneratorUtils::RandomizeDirection(random, 0.*I3Units::degree, 180.*I3Units::degree, 0.*I3Units::degree, 360.*I3Units::degree);
    //Sort zenith into four equal slices
    if((tmpDirTest.GetZenith()>=(2*I3Constants::pi)/3)&&(tmpDirTest.GetZenith()<=(I3Constants::pi)))
      countZG120++;
    else if((tmpDirTest.GetZenith()<(2*I3Constants::pi)/3)&&(tmpDirTest.GetZenith()>=(I3Constants::pi)/2))
      countZ90120++;
    else if((tmpDirTest.GetZenith()<(I3Constants::pi)/2)&&(tmpDirTest.GetZenith()>=(I3Constants::pi)/3)) 
      countZ6090++;
    else if((tmpDirTest.GetZenith()<(I3Constants::pi)/3)&&(tmpDirTest.GetZenith()>=0))
      countZ060++;
    else FAIL("Zenith not in range");

    //Sort azimuth into four equal slices
    if((tmpDirTest.GetAzimuth()>=0)&&(tmpDirTest.GetAzimuth()<(I3Constants::pi)/2))
      countA090++;
    else if((tmpDirTest.GetAzimuth()>=(I3Constants::pi)/2)&&(tmpDirTest.GetAzimuth()<(I3Constants::pi)))
      countA90180++;
    else if((tmpDirTest.GetAzimuth()>=(I3Constants::pi))&&(tmpDirTest.GetAzimuth()<(3*I3Constants::pi)/2))
      countA180270++;
    else if((tmpDirTest.GetAzimuth()>=(3*I3Constants::pi)/2)&&(tmpDirTest.GetAzimuth()<(2*I3Constants::pi)))
      countA270360++;
    else FAIL("Azimuth not in range");
  }
  //Allow ~3sigma (=3*sqrt(10^5*.25*.75)) deviation for the binomial test
  ENSURE_DISTANCE(countZG120,25000,410);
  ENSURE_DISTANCE(countZ90120,25000,410);
  ENSURE_DISTANCE(countZ6090,25000,410);
  ENSURE_DISTANCE(countZ060,25000,410);
  ENSURE_DISTANCE(countA090,25000,410);
  ENSURE_DISTANCE(countA90180,25000,410);
  ENSURE_DISTANCE(countA180270,25000,410);
  ENSURE_DISTANCE(countA270360,25000,410);
}


/**
 *Test rotation by giving specific points and ensuring disk
 *ends up where it should
 */
TEST(TestDiskRotation)
{
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  I3Direction dirTest1(0,0);
  I3Position posTest(I3MonopoleGeneratorUtils::RandomizePosition(random,dirTest1,100,0));
  ENSURE_DISTANCE(posTest.GetX(),0.0,1e-6);
  ENSURE_DISTANCE(posTest.GetY(),0.0,1e-6);
  ENSURE_DISTANCE(posTest.GetZ(),100.0,1e-6);

  I3Direction dirTest2(180*I3Units::degree,0);
  I3Position posTest2(I3MonopoleGeneratorUtils::RandomizePosition(random,dirTest2,100,0));
  ENSURE_DISTANCE(posTest2.GetX(),0.0,1e-6);
  ENSURE_DISTANCE(posTest2.GetY(),0.0,1e-6);
  ENSURE_DISTANCE(posTest2.GetZ(),-100.0,1e-6);

  I3Direction dirTest3(90*I3Units::degree,0);
  I3Position posTest3(I3MonopoleGeneratorUtils::RandomizePosition(random,dirTest3,100,0));
  ENSURE_DISTANCE(posTest3.GetX(),100.0,1e-6);
  ENSURE_DISTANCE(posTest3.GetY(),0.0,1e-6);
  ENSURE_DISTANCE(posTest3.GetZ(),0.0,1e-6);

  I3Direction dirTest4(90*I3Units::degree,90*I3Units::degree);
  I3Position posTest4(I3MonopoleGeneratorUtils::RandomizePosition(random,dirTest4,100,0));
  ENSURE_DISTANCE(posTest4.GetX(),0.0,1e-6);
  ENSURE_DISTANCE(posTest4.GetY(),100.0,1e-6);
  ENSURE_DISTANCE(posTest4.GetZ(),0.0,1e-6);

}


/**
 * Test randomization by considering disk split into equal area halves
 * and comparing how many particles end up in each one
 */
TEST(TestDiskLocationRandomization)
{
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  I3Direction dirTest(0,0);
  I3Position locTest(I3MonopoleGeneratorUtils::RandomizePosition(random,dirTest,100,10));
  ENSURE((locTest.GetX()*locTest.GetX())+(locTest.GetY()*locTest.GetY())<=100.);
  int countInside=0;
  double r=0.0;
  double r_cutoff=sqrt(50.0); //should be roughly same inside and outside
  for(int i=1;i<=100000;i++){
    r=0.0;
    I3Position tmpPos(I3MonopoleGeneratorUtils::RandomizePosition(random,dirTest,100,10));
    r=sqrt(tmpPos.GetX()*tmpPos.GetX()+tmpPos.GetY()*tmpPos.GetY());
    if(r<=r_cutoff){
      countInside++;
    }
  }
  //Allow ~3sigma (=3*sqrt(10^5*.5*.5)) deviation for the binomial test
  ENSURE_DISTANCE(countInside,50000,480,"A large disagreement in where Monopoles are generating");
}


/**
 * Test randomization by splitting into equal regions
 * and comparing how many particles end up in each one.
 * This test checks a flat distribution.
 */
TEST(TestVelocityRandomizationFlat)
{
  int q0=0;
  int q1=0;
  int q2=0;
  int q3=0;
  int qfail=0;
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  for(int i=1;i<=100000;i++){
    double weight;
    double tmpbeta(I3MonopoleGeneratorUtils::RandomizeVelocity(random,0.5,0.9,NAN,&weight));
    if(tmpbeta >=0.5 && tmpbeta <0.6)
        q0++;
    else if(tmpbeta >=0.6 && tmpbeta <0.7)
        q1++;
    else if(tmpbeta >=0.7 && tmpbeta <0.8)
        q2++;
    else if(tmpbeta >=0.8 && tmpbeta <=0.9)
        q3++;
    else
        qfail++;
  }
  ENSURE(qfail==0);
  //Allow ~3sigma (=3*sqrt(10^5*.25*.75)) deviation for the binomial test
  ENSURE_DISTANCE(q0,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q1,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q2,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q3,25000,410,"A large disagreement in the Monopoles velocity distribution");
  
}

/**
 * Test randomization by splitting into equal regions
 * and comparing how many particles end up in each one.
 * This test checks a distribution with a powerlaw of 1.
 */
TEST(TestVelocityRandomizationPowerLaw1)
{
  int q0=0;
  int q1=0;
  int q2=0;
  int q3=0;
  int qfail=0;
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  for(int i=1;i<=100000;i++){
    double weight;
    double tmpbeta(I3MonopoleGeneratorUtils::RandomizeVelocity(random,0.5,0.9,1.,&weight));
    if(tmpbeta >=0.5 && tmpbeta <0.579146)            // 25-percentile
        q0++;
    else if(tmpbeta >=0.579146 && tmpbeta <0.670820)  // 50-percentile
        q1++;
    else if(tmpbeta >=0.670820 && tmpbeta <0.777006)  // 75-percentile
        q2++;
    else if(tmpbeta >=0.777006 && tmpbeta <=0.9)      // 100-percentile
        q3++;
    else
        qfail++;
  }  
  ENSURE(qfail==0);
  //Allow ~3sigma (=3*sqrt(10^5*.25*.75)) deviation for the binomial test
  ENSURE_DISTANCE(q0,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q1,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q2,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q3,25000,410,"A large disagreement in the Monopoles velocity distribution");
  
}

/**
 * Test randomization by splitting into equal regions
 * and comparing how many particles end up in each one.
 * This test checks a distribution with a powerlaw of 5.
 */
TEST(TestVelocityRandomizationPowerLaw5)
{
  int q0=0;
  int q1=0;
  int q2=0;
  int q3=0;
  int qfail=0;
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  for(int i=1;i<=100000;i++){
    double weight;
    double tmpbeta(I3MonopoleGeneratorUtils::RandomizeVelocity(random,0.5,0.9,5.,&weight));
    if(tmpbeta >=0.5 && tmpbeta <0.533102505)               // 25-percentile
        q0++;
    else if(tmpbeta >=0.533102505 && tmpbeta <0.581230251)  // 50-percentile
        q1++;
    else if(tmpbeta >=0.581230251 && tmpbeta <0.664038667)  // 75-percentile
        q2++;
    else if(tmpbeta >=0.664038667 && tmpbeta <=0.9)         // 100-percentile
        q3++;
    else
        qfail++;
  }  
  ENSURE(qfail==0);
  //Allow ~3sigma (=3*sqrt(10^5*.25*.75)) deviation for the binomial test
  ENSURE_DISTANCE(q0,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q1,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q2,25000,410,"A large disagreement in the Monopoles velocity distribution");
  ENSURE_DISTANCE(q3,25000,410,"A large disagreement in the Monopoles velocity distribution");
  
}

/**
 *Helper function to pass given parameters to an icetray script
 *with I3MonopoleGenerator module
 */
void TestingInterface(double mass, double gamma, double betaMin, double betaMax, double zenithMin, double zenithMax, double azimuthMin, double azimuthMax, double diskRadius, double diskDistance, double length, bool shouldPass);


/**
 * Test the various sanity checks are working for I3MonopoleGenerator
 */
TEST(monopole_mass_limits_tests){
  TestingInterface((0.9e5)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((2e17)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
}

TEST(monopole_gamma_limits_tests){
  TestingInterface((1e8)*I3Units::GeV,0.9,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,1001,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,1.0,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
}

TEST(monopole_beta_limits_tests){
  TestingInterface((1e8)*I3Units::GeV,NAN,0.9e-6,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,NAN,1.000001,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,NAN,1.0,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
 
  TestingInterface((1e8)*I3Units::GeV,NAN,0.1,1.000001,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,NAN,1.000001,0.1,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,NAN,NAN,0.1,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false); 
  
}

TEST(monopole_angle_limits_tests){
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,-1*I3Units::degree,0.0,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,181*I3Units::degree,0.0,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,-1*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,181*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,-1*I3Units::degree,0.0,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,361*I3Units::degree,0.0,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,0.0,-1*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,0.0,361*I3Units::degree,10,10,10,false);

  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,180.*I3Units::degree,0.0,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,360*I3Units::degree,0.0,10,10,10,false);
}

TEST(monopole_disk_limits_tests){
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,-1,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,-1,10,false);
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10*I3Units::km,10,false);
}

TEST(monopole_length_limits_tests){
  TestingInterface((1e8)*I3Units::GeV,2,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,-10,false);
}

TEST(monopole_velocity_settings_tests){
  TestingInterface((1e8)*I3Units::GeV,NAN,NAN,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
  TestingInterface((1e8)*I3Units::GeV,10,1e-3,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,10,10,10,false);
}


/**
 * Make sure a generic particle will pass
 */
TEST(monopole_generic_set_direction){
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  double mass = random->Uniform(1e5,1e17);
  double zenith = (I3Constants::pi)*(random->Uniform(0,1));
  double azimuth = (I3Constants::pi)*(random->Uniform(0,2));
  double gamma = random->Uniform(1,1000);
  double rad = random->Uniform(0,1000);
  double dist = random->Uniform(0,5000);
  double length = random->Uniform(0,1000);
  TestingInterface(mass*I3Units::GeV,gamma,NAN,NAN,zenith*I3Units::radian,zenith*I3Units::radian,azimuth*I3Units::radian,azimuth*I3Units::radian,rad*I3Units::m,dist*I3Units::m,length*I3Units::m,true);
}  

TEST(monopole_generic_rand_direction){
  I3GSLRandomServicePtr random(new I3GSLRandomService(time(NULL)));
  double mass = random->Uniform(1e5,1e17);
  double beta = random->Uniform(1e-6,1);
  double rad = random->Uniform(0,1000);
  double dist = random->Uniform(0,5000);
  double length = random->Uniform(0,1000);
  TestingInterface(mass*I3Units::GeV,NAN,beta,NAN,0.0,180.*I3Units::degree,0.0,360.*I3Units::degree,rad*I3Units::m,dist*I3Units::m,length*I3Units::m,true);
}  


/**
 *Helper function to pass given parameters to an icetray script
 *with I3MonopoleGenerator module
 */
void TestingInterface(double mass, double gamma, double betaMin, double betaMax, double zenithMin, double zenithMax, double azimuthMin, double azimuthMax, double diskRadius, double diskDistance, double length, bool shouldPass){
  const int NFRAMES(100);
  bool pass(true);
  
  std::vector<double> betaRange;
  std::vector<double> zenithRange;
  std::vector<double> azimuthRange;
  
  betaRange.resize(2);
  betaRange[0]=betaMin; //beta min
  betaRange[1]=betaMax; //beta max
  
  zenithRange.resize(2);
  zenithRange[0]=zenithMin; //min zenith
  zenithRange[1]=zenithMax; //max zenith
  
  azimuthRange.resize(2);
  azimuthRange[0]=azimuthMin; //min azimuth
  azimuthRange[1]=azimuthMax; //max azimuth
  
  
  try{
  I3Tray tray;

  tray.AddService("I3GSLRandomServiceFactory", "random");

  tray.AddModule("I3InfiniteSource","frames");
  tray.AddModule("I3MonopoleGenerator","mono-gen")
    ("Mass",mass)
    ("Gamma",gamma)
    ("BetaRange",betaRange)
    ("Disk_dist",diskDistance)
    ("Disk_rad",diskRadius)
    ("ZenithRange", zenithRange)
    ("AzimuthRange", azimuthRange)
    ("Length",length);
  tray.AddModule("TrashCan","trash");

  tray.Execute(NFRAMES);
  tray.Finish();
  }
  catch(...){
    pass=false;
  }

  if(shouldPass!=pass){FAIL("Generator Passed when it should have failed or vice versa");}

}


/**
 * Simple script to make sure check of random generator is working
 */
TEST(missing_random_generator){
  const int NFRAMES(100);
  bool pass(true);

  try{
  I3Tray tray;

  tray.AddModule("I3InfiniteSource","frames");
  tray.AddModule("I3MonopoleGenerator","mono-gen")
    ("Mass",(1e8)*I3Units::GeV)
    ("Gamma",10)
    ("Disk_dist",10)
    ("Disk_rad",10)
    ("Length",1.0);
  tray.AddModule("TrashCan","trash");

  tray.Execute(NFRAMES);
  tray.Finish();
  }
  catch(...){
    pass=false;
  }

  if(pass){FAIL("Generator Passed when it should have failed (no random gen)");}
}

