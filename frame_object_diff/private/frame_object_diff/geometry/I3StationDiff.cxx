#include <serialization/bitset.hpp>
#include <icetray/serialization.h>
#include <dataclasses/external/CompareFloatingPoint.h> 
#include <frame_object_diff/geometry/I3StationDiff.h>

I3StationDiff::I3StationDiff(const I3Station& base, const I3Station& cur)
{
  Init_(base,cur);
}
I3StationDiff::I3StationDiff(I3StationConstPtr base, I3StationConstPtr cur)
{
  Init_(*base,*cur);
}

void
I3StationDiff::Init_(const I3Station& base, const I3Station& cur)
{
  bits.set();
  if (CompareFloatingPoint::Compare_NanEqual(base.orientation,
          cur.orientation))
    bits[0] = 0;
  else
    orientation_ = cur.orientation;
  if (CompareFloatingPoint::Compare_NanEqual(base.tankradius,
          cur.tankradius))
    bits[1] = 0;
  else
    tankradius_ = cur.tankradius;
  if (CompareFloatingPoint::Compare_NanEqual(base.tankheight,
          cur.tankheight))
    bits[2] = 0;
  else
    tankheight_ = cur.tankheight;
  if (CompareFloatingPoint::Compare_NanEqual(base.fillheight,
          cur.fillheight))
    bits[3] = 0;
  else
    fillheight_ = cur.fillheight;
  if (base.tanktype == cur.tanktype)
    bits[4] = 0;
  else
    tanktype_ = cur.tanktype;
  if (base.tanks == cur.tanks)
    bits[5] = 0;
  else
    tanks_ = VectorDiff<station::Tank>(base.tanks,cur.tanks);
}

I3StationPtr
I3StationDiff::Unpack(const I3Station& base) const
{
  if (unpacked_)
    return unpacked_;
  
  unpacked_ = I3StationPtr(new I3Station());
  if (bits[0])
    unpacked_->orientation = orientation_;
  else
    unpacked_->orientation = base.orientation;
  if (bits[1])
    unpacked_->tankradius = tankradius_;
  else
    unpacked_->tankradius = base.tankradius;
  if (bits[2])
    unpacked_->tankheight = tankheight_;
  else
    unpacked_->tankheight = base.tankheight;
  if (bits[3])
    unpacked_->fillheight = fillheight_;
  else
    unpacked_->fillheight = base.fillheight;
  if (bits[4])
    unpacked_->tanktype = tanktype_;
  else
    unpacked_->tanktype = base.tanktype;
  if (bits[5])
    unpacked_->tanks = *(tanks_.Unpack(base.tanks));
  else
    unpacked_->tanks = base.tanks;
  return unpacked_;  
}
I3StationPtr
I3StationDiff::Unpack(I3StationConstPtr base) const
{
  return Unpack(*base);
}

template <class Archive>
void
I3StationDiff::serialize(Archive & ar, unsigned version)
{
  if (version>i3stationdiff_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3StationDiff class.",
        version, i3stationdiff_version_);
  
  ar & make_nvp("bitset",bits);
  
  if (bits[0])
    ar & make_nvp("orientation", orientation_);
  if (bits[1])
    ar & make_nvp("tankradius", tankradius_);
  if (bits[2])
    ar & make_nvp("tankheight", tankheight_);
  if (bits[3])
    ar & make_nvp("fillheight", fillheight_);
  if (bits[4])
    ar & make_nvp("tanktype", tanktype_);
  if (bits[5])
    ar & make_nvp("tanks", tanks_);
}

I3_SERIALIZABLE(I3StationDiff);

I3_SERIALIZABLE(I3StationMapDiff);
