/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3DetectorStatus.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3DetectorStatusDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3DETECTORSTATUSDIFF_H_INCLUDED
#define I3DETECTORSTATUSDIFF_H_INCLUDED

#include "icetray/I3FrameObject.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "frame_object_diff/status/I3DOMStatusDiff.h"
#include "frame_object_diff/status/I3TriggerStatusDiff.h"

static const unsigned i3detectorstatusdiff_version_ = 0;

/**
 * Store the difference between two I3DetectorStatus objects.
 */
class I3DetectorStatusDiff : public I3FrameObject
{
public:
  std::string base_filename; //!< filename of base object
  I3Time startTime; //!< start time
  I3Time endTime; //!< end time
  I3DOMStatusMapDiff domStatus; //!< dom status
  I3TriggerStatusMapDiff triggerStatus; //!< trigger status
  std::string daqConfigurationName; //!< daq configuration name
  
  I3DetectorStatusDiff() { }
  
  /**
   * Create a Diff against a base, for the cur (det) object.
   */
  I3DetectorStatusDiff(const std::string filename, const I3DetectorStatus& base, 
      const I3DetectorStatus& det);
  I3DetectorStatusDiff(const std::string filename, I3DetectorStatusConstPtr base, 
      I3DetectorStatusConstPtr det);
  
  virtual ~I3DetectorStatusDiff() { }

  /**
   * Unpack the Diff, returning a shared pointer to
   * the original object.
   *
   * Takes the base that was originally provided to the
   * constructor as input.
   */
  I3DetectorStatusPtr Unpack(const I3DetectorStatus& base) const;
  I3DetectorStatusPtr Unpack(I3DetectorStatusConstPtr base) const;
  
private:
  /**
   * A shared pointer to the unpacked data, so we don't
   * have to regenerate this for subsequent calls.
   */
  mutable I3DetectorStatusPtr unpacked_;
  
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version);
};

I3_DEFAULT_NAME(I3DetectorStatusDiff);
I3_POINTER_TYPEDEFS(I3DetectorStatusDiff);
I3_CLASS_VERSION(I3DetectorStatusDiff, i3detectorstatusdiff_version_);

#endif // I3DETECTORSTATUSDIFF_H_INCLUDED
