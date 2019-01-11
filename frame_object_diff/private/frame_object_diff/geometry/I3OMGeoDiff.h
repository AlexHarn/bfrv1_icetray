/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3OMGeoDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3OMGEODIFF_H_INCLUDED
#define I3OMGEODIFF_H_INCLUDED

#include "dataclasses/geometry/I3OMGeo.h"
#include "frame_object_diff/I3MapDiff.h"

typedef I3MapDiff<OMKey, I3OMGeo> I3OMGeoMapDiff;
I3_POINTER_TYPEDEFS(I3OMGeoMapDiff);
I3_CLASS_VERSION(I3OMGeoMapDiff, mapdiff_version_);

#endif // I3OMGEODIFF_H_INCLUDED
