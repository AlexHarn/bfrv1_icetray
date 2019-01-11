#include <icetray/serialization.h>
#include <frame_object_diff/status/I3DetectorStatusDiff.h>


I3DetectorStatusDiff::I3DetectorStatusDiff(const std::string filename,
    const I3DetectorStatus& base, const I3DetectorStatus& det)
    : base_filename(filename),
      startTime(det.startTime),
      endTime(det.endTime),
      domStatus(base.domStatus,det.domStatus),
      triggerStatus(base.triggerStatus,det.triggerStatus),
      daqConfigurationName(det.daqConfigurationName)
{ }

I3DetectorStatusDiff::I3DetectorStatusDiff(const std::string filename,
    I3DetectorStatusConstPtr base, I3DetectorStatusConstPtr det)
    : base_filename(filename),
      startTime(det->startTime),
      endTime(det->endTime),
      domStatus(base->domStatus,det->domStatus),
      triggerStatus(base->triggerStatus,det->triggerStatus),
      daqConfigurationName(det->daqConfigurationName)
{ }

I3DetectorStatusPtr
I3DetectorStatusDiff::Unpack(const I3DetectorStatus& base) const
{
  if (unpacked_)
    return unpacked_;
  
  unpacked_ = I3DetectorStatusPtr(new I3DetectorStatus());
  unpacked_->startTime = startTime;
  unpacked_->endTime = endTime;
  unpacked_->domStatus = *(domStatus.Unpack(base.domStatus));
  unpacked_->triggerStatus = *(triggerStatus.Unpack(base.triggerStatus));
  unpacked_->daqConfigurationName = daqConfigurationName;
  return unpacked_;
}

I3DetectorStatusPtr
I3DetectorStatusDiff::Unpack(I3DetectorStatusConstPtr base) const
{
  return Unpack(*base);
}

template <class Archive>
void 
I3DetectorStatusDiff::serialize(Archive& ar, unsigned version)
{
  if (version>i3detectorstatusdiff_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3DetectorStatusDiff class.",
        version, i3detectorstatusdiff_version_);

  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));

  ar & make_nvp("filename", base_filename);
  ar & make_nvp("StartTime",startTime);
  ar & make_nvp("EndTime",endTime);
  ar & make_nvp("DOMStatus", domStatus);
  ar & make_nvp("TriggerStatus", triggerStatus);
  ar & make_nvp("DaqConfigurationName", daqConfigurationName);
}

I3_SERIALIZABLE(I3DetectorStatusDiff);
