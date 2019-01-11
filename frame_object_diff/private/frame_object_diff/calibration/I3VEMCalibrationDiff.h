/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3VEMCalibrationDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3VEMCALIBRATIONDIFF_H_INCLUDED
#define I3VEMCALIBRATIONDIFF_H_INCLUDED

#include "dataclasses/calibration/I3Calibration.h"
#include "frame_object_diff/I3MapDiff.h"

typedef MapDiff<OMKey, I3VEMCalibration> I3VEMCalibrationMapDiff;
I3_POINTER_TYPEDEFS(I3VEMCalibrationMapDiff);
I3_CLASS_VERSION(I3VEMCalibrationMapDiff, mapdiff_version_);

#endif // I3VEMCALIBRATIONDIFF_H_INCLUDED
