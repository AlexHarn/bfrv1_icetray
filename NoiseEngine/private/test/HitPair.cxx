#include <I3Test.h>

#include "icetray/I3Units.h"
#include "NoiseEngine/HitPair.h"

#include <boost/random.hpp>

TEST_GROUP(HitPairTest);

TEST(HitPair){

  const double FIRST(100*I3Units::ns);
  const double LAST(1100.*I3Units::ns);
  const double length = 2000*I3Units::ns;

  /**
   * make and plant the first hit
   */
  I3RecoPulse first_hit;
  first_hit.SetTime(FIRST);
  I3Position pos1(0,0,0);
  
  /**
   * make and plant the last hit
   */
  I3RecoPulse last_hit;
  last_hit.SetTime(LAST);
  OMKey second_om(21,35);
  I3Position pos2(100,100,0);

  HitPair p;
  p.SetTimes(FIRST, LAST);
  p.SetAngles(pos1,pos2);

  double RealZen = 1.570796326794897; // pi/2
  double RealAzi = 0.785398163397448; // pi/4

  std::cerr << "hit1: x="<< pos1.GetX() << ", y=" << pos1.GetY() << ", z=" << pos1.GetZ() << ", t=" << FIRST << std::endl;
  std::cerr << "hit2: x="<< pos2.GetX() << ", y=" << pos2.GetY() << ", z=" << pos2.GetZ() << ", t=" << LAST << std::endl;
  std::cerr << "Real Zenith=" << RealZen << ", Real Azimuth=" << RealAzi << std::endl;
  std::cerr << "Calculated Zen="<<p.GetZenith() << ", Calculated Azimuth=" << p.GetAzimuth() << std::endl;
  
  ENSURE( p.InTimeWindow(FIRST, length), "Time window check failed. Hits apparently further than 2000 ns apart!");
  ENSURE( p.InVelocityWindow( 0.1, 1.0), "Outside of velocity window?");
  ENSURE_DISTANCE(p.GetZenith(), RealZen, 0.001, "Zenith angle failed.");
  ENSURE_DISTANCE(p.GetAzimuth(), RealAzi, 0.001, "Azimuthal angle failed.");
}
