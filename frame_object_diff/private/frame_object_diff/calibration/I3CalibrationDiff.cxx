#include <icetray/serialization.h>
#include <frame_object_diff/calibration/I3CalibrationDiff.h>

I3CalibrationDiff::I3CalibrationDiff(const std::string filename, const I3Calibration& base,
    const I3Calibration& cal)
    : base_filename(filename),
      startTime(cal.startTime),
      endTime(cal.endTime),
      domCal(base.domCal,cal.domCal),
      vemCal(base.vemCal,cal.vemCal)
{
  log_info_stream( "domCal size:" << domCal.size() << " " 
        << sizeof(I3DOMCalibrationDiff) << std::endl
        << "vemCal size:" << vemCal.size() << " "
        << sizeof(I3VEMCalibration) << std::endl);
}

I3CalibrationDiff::I3CalibrationDiff(const std::string filename, I3CalibrationConstPtr base,
    I3CalibrationConstPtr cal)
    : base_filename(filename),
      startTime(cal->startTime),
      endTime(cal->endTime),
      domCal(base->domCal,cal->domCal),
      vemCal(base->vemCal,cal->vemCal)
{
  std::bitset<29> mask;
  for(I3DOMCalibrationMapDiff::plus_iterator i(domCal.begin_plus());
      i != domCal.end_plus();i++)
      mask |= i->second.bits;
  log_info_stream( "domCal size:" << domCal.size() << " " 
        << sizeof(I3DOMCalibrationDiff) << std::endl
        << "    obj1:" << domCal.begin_plus()->second.bits << std::endl
        << "    all:" << mask << std::endl
        << "vemCal size:" << vemCal.size() << " "
        << sizeof(I3VEMCalibration) << std::endl);
}

I3CalibrationPtr
I3CalibrationDiff::Unpack(const I3Calibration& base) const
{
  if (unpacked_)
    return unpacked_;
  
  unpacked_ = I3CalibrationPtr(new I3Calibration());
  unpacked_->startTime = startTime;
  unpacked_->endTime = endTime;
  unpacked_->domCal = *(domCal.Unpack(base.domCal));
  unpacked_->vemCal = *(vemCal.Unpack(base.vemCal));
  return unpacked_;
}

I3CalibrationPtr
I3CalibrationDiff::Unpack(I3CalibrationConstPtr base) const
{
  return Unpack(*base);
}

template <class Archive>
void 
I3CalibrationDiff::serialize(Archive& ar, unsigned version)
{
  if (version>i3calibrationdiff_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3CalibrationDiff class.",
        version, i3calibrationdiff_version_);

  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));

  ar & make_nvp("filename", base_filename);
  ar & make_nvp("StartTime",startTime);
  ar & make_nvp("EndTime",endTime);
  ar & make_nvp("DOMCal", domCal);
  ar & make_nvp("VEMCal", vemCal);
}

I3_SERIALIZABLE(I3CalibrationDiff);
