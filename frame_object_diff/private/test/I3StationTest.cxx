#include <I3Test.h>
#include <iostream>
#include "dataclasses/geometry/I3TankGeo.h"
#include "frame_object_diff/geometry/I3Station.h"


TEST_GROUP(I3StationTest);

I3Station make_station(double offset)
{
  I3Station ret;
  ret.orientation = 10+offset;
  ret.tankradius = 20+offset;
  ret.fillheight = 30+offset;
  ret.tankheight = 35+offset;
  ret.tanktype = I3TankGeo::Zirconium_Lined;
  station::Tank t;
  t.position = I3Position(1,2,3);
  t.snowheight = 40+offset;
  t.oms.push_back(OMKey(1+offset,2+offset));
  ret.tanks.push_back(t);
  return ret;
}

TEST(equality)
{
  I3Station a = make_station(0);
  I3Station b = a;
  
  ENSURE(a == b,"equality fails");
  
  I3Station c = make_station(1);
  
  ENSURE(a != c, "inequality fails");
}

TEST(I3StationGeo)
{
  I3StationGeo old_station;
  I3TankGeo old_tank1,old_tank2;
  old_tank1.position = I3Position(1,2,3);
  old_tank1.orientation = 10;
  old_tank1.tankradius = 20;
  old_tank1.tankheight = 30;
  old_tank1.fillheight = 40;
  old_tank1.omKeyList_.push_back(OMKey(1,2));
  old_tank1.snowheight = 50;
  old_tank1.tanktype = I3TankGeo::Zirconium_Lined;
  old_station.push_back(old_tank1);
  old_tank2.position = I3Position(4,5,6);
  old_tank2.orientation = 10;
  old_tank2.tankradius = 20;
  old_tank2.tankheight = 30;
  old_tank2.fillheight = 40;
  old_tank2.omKeyList_.push_back(OMKey(1,3));
  old_tank2.snowheight = 60;
  old_tank2.tanktype = I3TankGeo::Zirconium_Lined;
  old_station.push_back(old_tank2);
  
  I3Station s(old_station);
  
  ENSURE(s.tanks.size() == 2, "doesn't have 2 tanks");
  ENSURE(s.tanks[0].oms == old_station[0].omKeyList_, "missing oms");
  ENSURE(s.tanks[1].oms == old_station[1].omKeyList_, "missing oms");
  
  I3StationGeo new_station = s.GetStationGeo();
  
  ENSURE(old_station.size() == new_station.size(),"sizes not equal");
  ENSURE(old_station[0] == new_station[0],"first tank not equal");
  ENSURE(old_station[1] == new_station[1],"second tank not equal");
  ENSURE(old_station == new_station,"station conversion failed");
}
