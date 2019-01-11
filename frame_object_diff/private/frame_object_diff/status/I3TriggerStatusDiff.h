/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3TriggerStatusDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3TRIGGERSTATUSDIFF_H_INCLUDED
#define I3TRIGGERSTATUSDIFF_H_INCLUDED

#include "dataclasses/status/I3DetectorStatus.h"
#include "frame_object_diff/I3MapDiff.h"

typedef MapDiff<TriggerKey, I3TriggerStatus> I3TriggerStatusMapDiff;
I3_POINTER_TYPEDEFS(I3TriggerStatusMapDiff);
I3_CLASS_VERSION(I3TriggerStatusMapDiff, mapdiff_version_);

#endif // I3TRIGGERSTATUSDIFF_H_INCLUDED
