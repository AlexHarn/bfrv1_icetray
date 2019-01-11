/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3StationDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3STATIONDIFF_H_INCLUDED
#define I3STATIONDIFF_H_INCLUDED

#include "frame_object_diff/geometry/I3Station.h"
#include "frame_object_diff/I3MapDiff.h"
#include "frame_object_diff/I3VectorDiff.h"

static const unsigned i3stationdiff_version_ = 0;

class I3StationDiff
{
public:
  std::bitset<6> bits;
  
  I3StationDiff() { }
  
  I3StationDiff(const I3Station& base, const I3Station& cur);
  I3StationDiff(I3StationConstPtr base, I3StationConstPtr cur);
  
  I3StationPtr Unpack(const I3Station& base) const;

  I3StationPtr Unpack(I3StationConstPtr base) const;

private:
  double orientation_;
  double tankradius_;
  double tankheight_;
  double fillheight_;
  I3Station::TankType tanktype_;
  VectorDiff<station::Tank> tanks_;
  
  mutable I3StationPtr unpacked_;
  
  void Init_(const I3Station& base, const I3Station& cur);
  
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version);
};

I3_POINTER_TYPEDEFS(I3StationDiff);
I3_CLASS_VERSION(I3StationDiff, i3stationdiff_version_);

typedef MapDiff<int, I3StationDiff, I3Station> I3StationMapDiff;
I3_POINTER_TYPEDEFS(I3StationMapDiff);
I3_CLASS_VERSION(I3StationMapDiff, mapdiff_version_);

#endif // I3STATIONDIFF_H_INCLUDED
