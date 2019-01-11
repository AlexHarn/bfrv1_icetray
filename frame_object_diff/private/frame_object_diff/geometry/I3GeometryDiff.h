/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3GeometryDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3GEOMETRYDIFF_H_INCLUDED
#define I3GEOMETRYDIFF_H_INCLUDED

#include "icetray/I3FrameObject.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "frame_object_diff/geometry/I3StationDiff.h"
//#include "frame_object_diff/geometry/I3TankGeoDiff.h"
#include "frame_object_diff/geometry/I3OMGeoDiff.h"

static const unsigned i3geometrydiff_version_ = 0;

/**
 * Store the difference between two I3Geometry objects.
 */
class I3GeometryDiff : public I3FrameObject
{
public:
  std::string base_filename; //!< filename of base object
  I3OMGeoMapDiff omgeo; //!< diff of I3OMGeoMap
  I3StationMapDiff stationgeo; //!< diff of I3StationMap
  I3Time startTime; //!< start time
  I3Time endTime; //!< end time
  
  I3GeometryDiff() { }
  
  /**
   * Create a Diff against a base, for the cur (geo) object.
   */
  I3GeometryDiff(const std::string filename, const I3Geometry& base, 
      const I3Geometry& geo);

  I3GeometryDiff(const std::string filename, I3GeometryConstPtr base, 
      I3GeometryConstPtr geo);

  /**
   * Unpack the Diff, returning a shared pointer to
   * the original object.
   *
   * Takes the base that was originally provided to the
   * construtor as inpu.
   */
  I3GeometryPtr Unpack(const I3Geometry& base) const;
  
  I3GeometryPtr Unpack(I3GeometryConstPtr base) const;

private:
  /**
   * A shared pointer to the unpacked data, so we don't   
   * have to regenerate this for subsequent calls.    
   */
  mutable I3GeometryPtr unpacked_;

  /**
   * Initialize the I3StationMap separately, because of the
   * conversion from I3StationGeo to I3Station.
   */
  void InitStation_(const I3StationGeoMap& base, const I3StationGeoMap& cur);
  
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version);
};

I3_DEFAULT_NAME(I3GeometryDiff);
I3_POINTER_TYPEDEFS(I3GeometryDiff);
I3_CLASS_VERSION(I3GeometryDiff, i3geometrydiff_version_);

#endif // I3GEOMETRYDIFF_H_INCLUDED
