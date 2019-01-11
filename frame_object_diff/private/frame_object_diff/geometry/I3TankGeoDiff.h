/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3TankGeoDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3TANKGEODIFF_H_INCLUDED
#define I3TANKGEODIFF_H_INCLUDED

#include "dataclasses/geometry/I3TankGeo.h"
#include "frame_object_diff/I3MapDiff.h"
#include "frame_object_diff/I3VectorDiff.h"
#include "frame_object_diff/I3FixedPositionVectorDiff.h"

static const unsigned i3tankgeodiff_version_ = 0;

/**
 * This code is not currently used, and superceded by I3Station.h
 *
 * TODO: Consider outright removal of this file.
 */

/**
 * Diff of I3TankGeo
 */
class I3TankGeoDiff
{
public:
  std::bitset<8> bits;
  
  I3TankGeoDiff() { }
  
  I3TankGeoDiff(const I3TankGeo& base, const I3TankGeo& cur);
  
  I3TankGeoDiff(I3TankGeoConstPtr base, I3TankGeoConstPtr cur);
  
  I3TankGeoPtr Unpack(const I3TankGeo& base) const;

  I3TankGeoPtr Unpack(I3TankGeoConstPtr base) const;

private:
  I3Position position_;
  double orientation_;
  double tankradius_;
  double tankheight_;
  double fillheight_;
  I3VectorDiff<OMKey> omKeyList_;
  double snowheight_;
  I3TankGeo::TankType tanktype_;
  
  mutable I3TankGeoPtr unpacked_;
  
  void Init_(const I3TankGeo& base, const I3TankGeo& cur);
  
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version);
  
  enum{ PositionBit=0, OrientationBit, RadiusBit, HeightBit,
    FillBit, KeyListBit, SnowHeightBit, TankTypeBit };
};

I3_POINTER_TYPEDEFS(I3TankGeoDiff);
I3_CLASS_VERSION(I3TankGeoDiff, i3tankgeodiff_version_);

typedef I3FixedPositionVectorDiff<I3TankGeoDiff,I3TankGeo> I3StationGeoDiff;
I3_POINTER_TYPEDEFS(I3StationGeoDiff);
I3_CLASS_VERSION(I3StationGeoDiff, vectordiff_version_);

typedef I3MapDiff<int, I3StationGeoDiff, I3StationGeo> I3StationGeoMapDiff;
I3_POINTER_TYPEDEFS(I3StationGeoMapDiff);
I3_CLASS_VERSION(I3StationGeoMapDiff, mapdiff_version_);

#endif // I3TANKGEODIFF_H_INCLUDED
