#include <cstddef>

#include <icetray/serialization.h>
#include <frame_object_diff/geometry/I3Station.h>

#include <boost/foreach.hpp>

template <class Archive>
void
station::Tank::serialize(Archive & ar, unsigned version)
{
  ar & make_nvp("Position",position);
  ar & make_nvp("SnowHeight",snowheight);
  ar & make_nvp("OMs",oms);
}

I3_SERIALIZABLE(station::Tank);

I3Station::I3Station(const I3StationGeo& rhs)
{
  if (!rhs.empty())
  {
    orientation = rhs[0].orientation;
    tankradius = rhs[0].tankradius;
    tankheight = rhs[0].tankheight;
    fillheight = rhs[0].fillheight;
    tanktype = rhs[0].tanktype;
    
    std::size_t num_tanks(rhs.size());
    tanks.reserve(num_tanks);
    std::size_t num_oms(rhs[0].omKeyList_.size());
    BOOST_FOREACH(I3TankGeo g,rhs)
    {
      station::Tank t;
      //4 Apr 17- removed.  Tank Orientations are nan in gcdserver
      //   And this causes much pain in comparisons
      //i3_assert(orientation == g.orientation);
      i3_assert(tankradius == g.tankradius);
      i3_assert(tankheight == g.tankheight);
      i3_assert(fillheight == g.fillheight);
      i3_assert(tanktype == g.tanktype);
      i3_assert(num_oms == g.omKeyList_.size());
      t.position = g.position;
      t.snowheight = g.snowheight;
      t.oms.reserve(num_oms);
      for(std::size_t i=0;i<num_oms;i++)
        t.oms.push_back(g.omKeyList_[i]);
      tanks.push_back(t);
    }
  }
}

I3Station::~I3Station() {}

I3StationGeo I3Station::GetStationGeo()
{
  I3StationGeo ret;
  BOOST_FOREACH(station::Tank t,tanks)
  {
    I3TankGeo g;
    g.position = t.position;
    g.orientation = orientation;
    g.tankradius = tankradius;
    g.tankheight = tankheight;
    g.fillheight = fillheight;
    g.snowheight = t.snowheight;
    g.tanktype = tanktype;
    BOOST_FOREACH(OMKey k,t.oms)
      g.omKeyList_.push_back(k);
    ret.push_back(g);
  }
  return ret;
}

template <class Archive>
void
I3Station::load(Archive& ar, unsigned version)
{
  if (version>i3station_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3Station class.",version,i3station_version_);
  
  ar & make_nvp("Orientation", orientation);
  ar & make_nvp("TankRadius", tankradius);
  ar & make_nvp("TankHeight", tankheight);
  ar & make_nvp("FillHeight", fillheight);
  ar & make_nvp("TankType", tanktype);
  
  std::size_t num_tanks;
  ar & make_nvp("NumTanks", num_tanks);
  if (num_tanks > 0) {
    tanks.reserve(num_tanks);
    std::size_t num_oms;
    ar & make_nvp("NumOMs", num_oms);
    for(;num_tanks>0;num_tanks--)
    {
      station::Tank t;
      ar & make_nvp("Position", t.position);
      ar & make_nvp("SnowHeight", t.snowheight);
      t.oms.reserve(num_oms);
      for(std::size_t i=0;i<num_oms;i++)
      {
        OMKey k;
        ar & make_nvp("OM", k);
        t.oms.push_back(k);
      }
      tanks.push_back(t);
    }
  }
}

template <class Archive>
void
I3Station::save(Archive& ar, unsigned version) const
{
  ar & make_nvp("Orientation", orientation);
  ar & make_nvp("TankRadius", tankradius);
  ar & make_nvp("TankHeight", tankheight);
  ar & make_nvp("FillHeight", fillheight);
  ar & make_nvp("TankType", tanktype);

  std::size_t num_tanks(tanks.size());
  ar & make_nvp("NumTanks", num_tanks);
  if (!tanks.empty())
  {
    std::size_t num_oms(tanks[0].oms.size());
    ar & make_nvp("NumOMs", num_oms);
    BOOST_FOREACH(station::Tank t,tanks)
    {
      ar & make_nvp("Position", t.position);
      ar & make_nvp("SnowHeight", t.snowheight);
      for(std::size_t i=0;i<num_oms;i++)
        ar & make_nvp("OM", t.oms[i]);
    }
  }
}

I3_SPLIT_SERIALIZABLE(I3Station);
//I3_SERIALIZABLE(I3StationMap);
