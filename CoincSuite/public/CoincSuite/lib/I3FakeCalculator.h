/**
 * copyright  (C) 2004
 * the IceCube collaboration
 * $Id$
 *
 * @file I3FakeCalculator.h
 * @version $Revision$
 * @date $Date$
 * @author mzoll <marcel.zoll@fysik.su.se>
 */

#ifndef I3FAKECALCULATOR_H
#define I3FAKECALCULATOR_H

#include <cmath>
#include <utility>
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3OMGeo.h"
#include "dataclasses/I3Direction.h"

/**
 * This is a namespace, which provides a collection of stand-alone
 * functions which can seamlessly be integrated into
 * "phys-services/I3Calculator.h/cxx"
 */

namespace I3FakeCalculator {
  //Single Track operations
  /// Get Point in along track in this distance from vertex [distance in i3units::meter]
  inline I3Position PointAlongTrack(const I3Particle& track, const double& distance) {
    return I3Position(track.GetPos().GetX()+ distance*track.GetDir().GetX(),
                      track.GetPos().GetY()+ distance*track.GetDir().GetY(),
                      track.GetPos().GetZ()+ distance*track.GetDir().GetZ());
  };
  /// Get Point along track which is time later than the vertex [time in i3units::ns]
  inline I3Position TimeAlongTrack(const I3Particle& track, const double& time) {
    return PointAlongTrack(track, time* track.GetSpeed());
  };

  //analytic linear geometry
  ///a reprensentation of a Line in 3d space
  class GeoLine{
    I3Position pos;
    I3Direction dir;
  public:
    GeoLine(const I3Position p, const I3Direction d):pos(p),dir(d){};
    GeoLine(I3Particle part):pos(part.GetPos()),dir(part.GetDir()) {};
    inline I3Position GetPos() const {return pos;};
    inline I3Direction GetDir() const {return dir;};
  };

  ///a representation of a Surface in 3d space
  class GeoSurface{
    I3Position pos;
    I3Direction dir; //representing the normal
  public:
    GeoSurface(const I3Position p, const I3Direction d):pos(p),dir(d){};
    GeoSurface(I3Particle part):pos(part.GetPos()),dir(part.GetDir()) {};
    inline I3Position GetPos() const {return pos;};
    inline I3Direction GetDir() const {return dir;};
  };

  ///walk along a line from its base-factor along the positive direction
  inline I3Position PointAlongLine(const GeoLine& line, const double& distance) {
    return I3Position(line.GetPos().GetX()+ distance*line.GetDir().GetX(),
                      line.GetPos().GetY()+ distance*line.GetDir().GetY(),
                      line.GetPos().GetZ()+ distance*line.GetDir().GetZ());
  };

  /// returns the closest distance between two input tracks (distance of closest approach)
  double TrackDistance(const GeoLine& p1, const GeoLine& p2); 
  
  /// return point on track1 which is closest to track2
  I3Position TrackClosestPoint(const GeoLine& track1, const GeoLine& track2);
  
  /// Get the intersection point of track with surface
  I3Position TrackSurfaceIntersection(const GeoSurface& surf, const GeoLine& track);

  ///distance of a point towards a surface
  inline double PointSurfaceDistance(const I3Position& point, const GeoSurface& surf)
    {return surf.GetDir()*point - surf.GetDir()*surf.GetPos();};

  /// return the distance of this point to the origin (length of vector), or to another point 
  double DistanceTowardsPoint (const I3Position& point, const I3Position& origin= I3Position(0,0,0));

};

#endif
