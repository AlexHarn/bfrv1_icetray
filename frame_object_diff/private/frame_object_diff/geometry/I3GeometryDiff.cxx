#include <icetray/serialization.h>
#include <frame_object_diff/geometry/I3GeometryDiff.h>

#include <boost/foreach.hpp>

I3GeometryDiff::I3GeometryDiff(const std::string filename, const I3Geometry& base,
    const I3Geometry& geo)
    : base_filename(filename),
      omgeo("",base.omgeo,geo.omgeo),
      startTime(geo.startTime),
      endTime(geo.endTime)
{
  InitStation_(base.stationgeo,geo.stationgeo);
}

I3GeometryDiff::I3GeometryDiff(const std::string filename, I3GeometryConstPtr base,
    I3GeometryConstPtr geo)
    : base_filename(filename),
      omgeo("",base->omgeo,geo->omgeo),
      startTime(geo->startTime),
      endTime(geo->endTime)
{
  InitStation_(base->stationgeo,geo->stationgeo);
}

void I3GeometryDiff::InitStation_(const I3StationGeoMap& base,
    const I3StationGeoMap& cur)
{
  I3StationMap base_station;
  I3StationMap cur_station;
  BOOST_FOREACH(I3StationGeoMap::value_type sg,base)
    base_station[sg.first] = I3Station(sg.second);
  BOOST_FOREACH(I3StationGeoMap::value_type sg,cur)
    cur_station[sg.first] = I3Station(sg.second);
  stationgeo = I3StationMapDiff(base_station,cur_station);
}

I3GeometryPtr
I3GeometryDiff::Unpack(const I3Geometry& base) const
{
  if (unpacked_)
    return unpacked_;
  
  unpacked_ = I3GeometryPtr(new I3Geometry());
  unpacked_->omgeo = *(omgeo.Unpack(base.omgeo));
  I3StationMap base_station;
  BOOST_FOREACH(I3StationGeoMap::value_type sg,base.stationgeo)
    base_station[sg.first] = I3Station(sg.second);
  I3StationMap st = *(stationgeo.Unpack(base_station));
  BOOST_FOREACH(I3StationMap::value_type s,st)
    unpacked_->stationgeo[s.first] = s.second.GetStationGeo();
  unpacked_->startTime = startTime;
  unpacked_->endTime = endTime;
  return unpacked_;
}

I3GeometryPtr
I3GeometryDiff::Unpack(I3GeometryConstPtr base) const
{
  return Unpack(*base);
}

template <class Archive>
void 
I3GeometryDiff::serialize(Archive& ar, unsigned version)
{
  if (version>i3geometrydiff_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3GeometryDiff class.",
        version, i3geometrydiff_version_);

  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));

  ar & make_nvp("filename", base_filename);
  ar & make_nvp("OMGeo", omgeo);
  ar & make_nvp("StationGeo", stationgeo);
  ar & make_nvp("StartTime",startTime);
  ar & make_nvp("EndTime",endTime);
}

I3_SERIALIZABLE(I3GeometryDiff);
