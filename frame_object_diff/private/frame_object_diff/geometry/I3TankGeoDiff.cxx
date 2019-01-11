#include <serialization/bitset.hpp>
#include <icetray/serialization.h>
#include <dataclasses/external/CompareFloatingPoint.h> 
#include <frame_object_diff/geometry/I3TankGeoDiff.h>

I3TankGeoDiff::I3TankGeoDiff(const I3TankGeo& base, const I3TankGeo& cur)
{
  Init_(base,cur);
}
I3TankGeoDiff::I3TankGeoDiff(I3TankGeoConstPtr base, I3TankGeoConstPtr cur)
{
  Init_(*base,*cur);
}

void
I3TankGeoDiff::Init_(const I3TankGeo& base, const I3TankGeo& cur)
{
  
  /*bits.set();
  if (base.position == cur.position)
    bits[PositionBit] = 0;
  else
    position_ = cur.position;
  if (CompareFloatingPoint::Compare_NanEqual(base.orientation,
          cur.orientation))
    bits[1] = 0;
  else
    orientation_ = cur.orientation;
  if (CompareFloatingPoint::Compare_NanEqual(base.tankradius,
          cur.tankradius))
    bits[2] = 0;
  else
    tankradius_ = cur.tankradius;
  if (CompareFloatingPoint::Compare_NanEqual(base.tankheight,
          cur.tankheight))
    bits[3] = 0;
  else
    tankheight_ = cur.tankheight;
  if (CompareFloatingPoint::Compare_NanEqual(base.fillheight,
          cur.fillheight))
    bits[4] = 0;
  else
    fillheight_ = cur.fillheight;
  if (base.omKeyList_ == cur.omKeyList_)
    bits[5] = 0;
  else
    omKeyList_ = I3VectorDiff<OMKey>("",base.omKeyList_,cur.omKeyList_);
  if (CompareFloatingPoint::Compare_NanEqual(base.snowheight,
          cur.snowheight))
    bits[6] = 0;
  else
    snowheight_ = cur.snowheight;
  if (base.tanktype == cur.tanktype)
    bits[7] = 0;
  else
    tanktype_ = cur.tanktype;*/
  using CompareFloatingPoint::Compare_NanEqual;
  
  if ((bits[PositionBit] = !(base.position == cur.position)))
    position_ = cur.position;
  
  if ((bits[OrientationBit] = !Compare_NanEqual(base.orientation,cur.orientation)))
    orientation_ = cur.orientation;
  
  if ((bits[RadiusBit] = !Compare_NanEqual(base.tankradius,cur.tankradius)))
    tankradius_ = cur.tankradius;
  
  if ((bits[HeightBit] = !Compare_NanEqual(base.tankheight,cur.tankheight)))
    tankheight_ = cur.tankheight;
  
  if ((bits[FillBit] = !Compare_NanEqual(base.fillheight,cur.fillheight)))
    fillheight_ = cur.fillheight;
  
  if ((bits[KeyListBit] = !(base.omKeyList_ == cur.omKeyList_)))
    omKeyList_ = I3VectorDiff<OMKey>("",base.omKeyList_,cur.omKeyList_);
  
  if ((bits[SnowHeightBit] = !Compare_NanEqual(base.snowheight,cur.snowheight)))
    snowheight_ = cur.snowheight;
  
  if ((bits[TankTypeBit] = !(base.tanktype == cur.tanktype)))
    tanktype_ = cur.tanktype;
}

I3TankGeoPtr
I3TankGeoDiff::Unpack(const I3TankGeo& base) const
{
  if (unpacked_)
    return unpacked_;
  
  unpacked_ = I3TankGeoPtr(new I3TankGeo());
  /*if (bits[PositionBit])
    unpacked_->position = position_;
  else
    unpacked_->position = base.position;
  if (bits[1])
    unpacked_->orientation = orientation_;
  else
    unpacked_->orientation = base.orientation;
  if (bits[2])
    unpacked_->tankradius = tankradius_;
  else
    unpacked_->tankradius = base.tankradius;
  if (bits[3])
    unpacked_->tankheight = tankheight_;
  else
    unpacked_->tankheight = base.tankheight;
  if (bits[4])
    unpacked_->fillheight = fillheight_;
  else
    unpacked_->fillheight = base.fillheight;
  if (bits[5])
    unpacked_->omKeyList_ = *(omKeyList_.Unpack(base.omKeyList_));
  else
    unpacked_->omKeyList_ = base.omKeyList_;
  if (bits[6])
    unpacked_->snowheight = snowheight_;
  else
    unpacked_->snowheight = base.snowheight;
  if (bits[7])
    unpacked_->tanktype = tanktype_;
  else
    unpacked_->tanktype = base.tanktype;*/
  
  unpacked_->position = (bits[PositionBit] ? position_ : base.position);
  unpacked_->orientation = (bits[OrientationBit] ? orientation_ : base.orientation);
  unpacked_->tankradius = (bits[RadiusBit] ? tankradius_ : base.tankradius);
  unpacked_->tankheight = (bits[HeightBit] ? tankheight_ : base.tankheight);
  unpacked_->fillheight = (bits[FillBit] ? fillheight_ : base.fillheight);
  unpacked_->omKeyList_ = (bits[KeyListBit] ?
                           *(omKeyList_.Unpack(base.omKeyList_)) :
                           base.omKeyList_);
  unpacked_->snowheight = (bits[SnowHeightBit] ? snowheight_ : base.snowheight);
  unpacked_->tanktype = (bits[TankTypeBit] ? tanktype_ : base.tanktype);
  
  return unpacked_;  
}
I3TankGeoPtr
I3TankGeoDiff::Unpack(I3TankGeoConstPtr base) const
{
  return Unpack(*base);
}

template <class Archive>
void
I3TankGeoDiff::serialize(Archive & ar, unsigned version)
{
  if (version>i3tankgeodiff_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3TankGeoDiff class.",
        version, i3tankgeodiff_version_);
  
  ar & make_nvp("bitset",bits);  
  
  if (bits[PositionBit])
    ar & make_nvp("position", position_);
  if (bits[1])
    ar & make_nvp("orientation", orientation_);
  if (bits[2])
    ar & make_nvp("tankradius", tankradius_);
  if (bits[3])
    ar & make_nvp("tankheight", tankheight_);
  if (bits[4])
    ar & make_nvp("fillheight", fillheight_);
  if (bits[5])
    ar & make_nvp("omKeyList", omKeyList_);
  if (bits[6])
    ar & make_nvp("snowheight", snowheight_);
  if (bits[7])
    ar & make_nvp("tanktype", tanktype_);
}

I3_SERIALIZABLE(I3TankGeoDiff);

I3_SERIALIZABLE(I3StationGeoDiff);

I3_SERIALIZABLE(I3StationGeoMapDiff);
