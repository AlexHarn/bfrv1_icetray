/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3CalibrationDiff.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3CalibrationDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3CALIBRATIONDIFF_H_INCLUDED
#define I3CALIBRATIONDIFF_H_INCLUDED

#include "icetray/I3FrameObject.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "frame_object_diff/calibration/I3DOMCalibrationDiff.h"
#include "frame_object_diff/calibration/I3VEMCalibrationDiff.h"

static const unsigned i3calibrationdiff_version_ = 0;

class I3CalibrationDiff : public I3FrameObject
{
public:
  std::string base_filename; //!< filename for base object
  I3Time startTime; //!< start time
  I3Time endTime; //!< end time
  I3DOMCalibrationMapDiff domCal; //!< DOMCal
  I3VEMCalibrationMapDiff vemCal; //!< VEMCal
  
  I3CalibrationDiff() { }
  
  /**
   * Create a Diff against a base, for the cur (cal) object.
   */
  I3CalibrationDiff(const std::string filename, const I3Calibration& base, 
      const I3Calibration& cal);
  I3CalibrationDiff(const std::string filename, I3CalibrationConstPtr base, 
      I3CalibrationConstPtr cal);
  
  /**
   * Unpack the Diff, returning a shared pointer to
   * the original object.
   *
   * Takes the base that was originially provided to the
   * constructor as input.
   */
  I3CalibrationPtr Unpack(const I3Calibration& base) const;
  I3CalibrationPtr Unpack(I3CalibrationConstPtr base) const;

private:
  /**
   * A shared pointer to the unpacked data, so we don't
   * have to regenerate this for subsequent calls.
   */
  mutable I3CalibrationPtr unpacked_;
  
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version);
};

I3_DEFAULT_NAME(I3CalibrationDiff);
I3_POINTER_TYPEDEFS(I3CalibrationDiff);
I3_CLASS_VERSION(I3CalibrationDiff, i3calibrationdiff_version_);

#endif // I3CALIBRATIONDIFF_H_INCLUDED
