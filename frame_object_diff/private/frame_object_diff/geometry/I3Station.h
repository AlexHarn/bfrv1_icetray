/**
 * copyright  (C) 2004
 * the icecube collaboration
 * @version $Id: I3Station.h 128631 2015-02-04 10:34:22Z jvansanten $
 * @file I3Station.h
 * @date $Date: 2015-02-04 04:34:22 -0600 (Wed, 04 Feb 2015) $
 */

#ifndef I3STATION_H_INCLUDED
#define I3STATION_H_INCLUDED

#include <iostream>
#include <vector>
#include <map>

#include "dataclasses/I3Position.h"
#include "dataclasses/Utility.h"
#include "icetray/OMKey.h"

#include "dataclasses/geometry/I3TankGeo.h"

static const unsigned i3station_version_ = 0;

namespace station {

  static const unsigned tank_version_ = 0;
  
  /**
   * Store the variables that change tank-to-tank.
   */
  class Tank
  {
  public:
    I3Position position; //!< tank x,y,z position
    double snowheight; //!< snow "overburden" (use I3Units)
    std::vector<OMKey> oms; //!< doms in tank
    
    bool operator==(const Tank& rhs) const
    {
      return (position == rhs.position &&
              snowheight == rhs.snowheight &&
              oms == rhs.oms);
    }
    bool operator!=(const Tank& rhs) const
    {
      return !operator==(rhs);
    }
    
  private:
    friend class icecube::serialization::access;
    template <class Archive> void serialize(Archive & ar, unsigned version);
  };
}

I3_CLASS_VERSION(station::Tank, station::tank_version_);

/**
 * Each station has a number of tanks.
 * Shared properties are listed as part of the station.
 * Modified properties are listed for each tank.
 */
class I3Station
{
public:
  I3Station() {}
  
  /**
   * Convert from I3StationGeo to I3Station.
   */
  I3Station(const I3StationGeo& rhs);
  
  virtual ~I3Station();

  typedef I3TankGeo::TankType TankType;

  double orientation; //!< relative angular rotation of tanks
  double tankradius; //!< tank radius (I3Units!)
  double tankheight; //!< tank height (I3Units!)
  double fillheight; //!< water/ice level height (I3Units!)
  TankType tanktype; //!< Type of tanks
  std::vector<station::Tank> tanks;
  
  /**
   * Convert back to I3StationGeo.
   */
  I3StationGeo GetStationGeo();
  
  bool operator==(const I3Station& rhs) const
  {
    return (orientation == rhs.orientation &&
            tankradius == rhs.tankradius &&
            fillheight == rhs.fillheight &&
            tanktype == rhs.tanktype &&
            tanks == rhs.tanks);
  }
  bool operator!=(const I3Station& rhs) const
  {
    return !operator==(rhs);
  }

private:
  friend class icecube::serialization::access;

  template <class Archive> void load(Archive & ar, unsigned version);
  template <class Archive> void save(Archive & ar, unsigned version) const;
  
  I3_SERIALIZATION_SPLIT_MEMBER();
};

I3_CLASS_VERSION(I3Station, i3station_version_);
I3_POINTER_TYPEDEFS(I3Station);

typedef std::map<int, I3Station> I3StationMap;
I3_POINTER_TYPEDEFS(I3StationMap);

#endif //I3STATION_H_INCLUDED
