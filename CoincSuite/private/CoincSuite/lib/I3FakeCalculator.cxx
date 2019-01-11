/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$
    @file I3FakeCalculator.cxx 
    @version $Revision$
    @date $Date$
    @author mzoll <marcel.zoll@fysik.su.se>
*/

#include "CoincSuite/lib/I3FakeCalculator.h"

#include "phys-services/I3Calculator.h"

using namespace std;
using namespace I3Calculator;
using namespace I3FakeCalculator;

//______________________________________________________________________________
double I3FakeCalculator::TrackDistance(const GeoLine& p1, const GeoLine& p2) { 
  if (p1.GetDir() == p2.GetDir() || p1.GetDir()==GetReverseDirection(p2.GetDir())) {
    log_debug("these are parallel");
    return DistanceTowardsPoint(p2.GetPos(), p1.GetPos());
  }
  I3Position tcp12 = TrackClosestPoint(p1, p2);
  I3Position tcp21 = TrackClosestPoint(p2, p1);
  if (tcp12 == I3Position(NAN, NAN, NAN) && tcp21==I3Position(NAN, NAN, NAN)) {
    log_debug("these are parallel");
    return DistanceTowardsPoint(p2.GetPos(), p1.GetPos());
  }
  if (tcp12 == tcp21) {
    log_debug("these are intersecting");
    return 0.;
  }
  
  const I3Direction &dir1= p1.GetDir();
  const I3Direction &dir2= p2.GetDir();
  const I3Position &pos1= p1.GetPos();
  const I3Position &pos2= p2.GetPos();

  const double n_x=dir1.GetY()*dir2.GetZ()-dir1.GetZ()*dir2.GetY();
  const double n_y=dir1.GetZ()*dir2.GetX()-dir1.GetX()*dir2.GetZ();
  const double n_z=dir1.GetX()*dir2.GetY()-dir1.GetY()*dir2.GetX();
  const double n_norm=sqrt(n_x*n_x+n_y*n_y+n_z*n_z);

  const double dist = std::abs(((pos2.GetX()-pos1.GetX())*n_x+(pos2.GetY()-pos1.GetY())*n_y+(pos2.GetZ()-pos1.GetZ())*n_z)/n_norm);
  //log_trace("Distance at closest approach is %f", dist);
  return dist;
}

//______________________________________________________________________________
I3Position I3FakeCalculator::TrackClosestPoint(const GeoLine& track1, const GeoLine& track2) {
  static const double wiggle = 1E-10; //A small wiggle within lines are considered intersecting
  
  const double scalar = track1.GetDir()*track2.GetDir();
  
  if (std::abs(scalar)==1.) { // equal to p1.GetDir() == p2.GetDir() || p1.GetDir()==GetReverseDirection(p2.GetDir()))
    log_debug("These are parallel; return constructor");
    return I3Position(NAN, NAN, NAN);
  }
  const I3Position p3 = (track1.GetPos()- track2.GetPos());
  
  const double p3d1 = p3*track1.GetDir();
  
  const double s = ((p3d1*scalar) - (p3*track2.GetDir())) / (scalar*scalar-1);
  
  const double t = scalar * s - p3d1;
  
  const I3Position track1_closest = PointAlongLine(track1, t);
  const I3Position track2_closest = PointAlongLine(track2, s);
  if (sqrt((track1_closest-track2_closest)*(track1_closest-track2_closest)) <= wiggle) {
    log_debug("These are intersecting with a small wiggle of %f", wiggle);
    return track1_closest + ((track1_closest-track2_closest)/2);
  }
  return track1_closest; 
}

//______________________________________________________________________________
I3Position I3FakeCalculator::TrackSurfaceIntersection(const GeoSurface& surf, const GeoLine& track) {
  const double dir_scalar = surf.GetDir()*track.GetDir();
  if (dir_scalar==0.) {
    log_debug("track and surface are parallel; return blank constructor");
    return I3Position();
  }

  const double lambda=-(surf.GetDir().GetX()*track.GetPos().GetX() + surf.GetDir().GetX()*surf.GetPos().GetX()
                       +surf.GetDir().GetY()*track.GetPos().GetY() + surf.GetDir().GetY()*surf.GetPos().GetY()
                       +surf.GetDir().GetZ()*track.GetPos().GetZ() + surf.GetDir().GetZ()*surf.GetPos().GetZ())/(dir_scalar);

  return PointAlongLine(track, lambda);
}

//______________________________________________________________________________
double I3FakeCalculator::DistanceTowardsPoint(const I3Position& point, const I3Position& origin) {
  const I3Position trans = point - origin;
  
  return sqrt(trans.GetX()*trans.GetX()
    + trans.GetY()*trans.GetY()
    + trans.GetZ()*trans.GetZ());
}

