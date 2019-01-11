#include <I3Test.h>

#include <boost/foreach.hpp>

#include <tensor-of-inertia/I3TensorOfInertiaCalculator.hpp>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/geometry/I3OMGeo.h>
#include <phys-services/I3Cuts.h>

TEST_GROUP(I3TensorOfInertiaCalculatorTests);

TEST(calculate_toi)
{
 
  I3RecoPulseSeriesMapPtr rpsmap(new I3RecoPulseSeriesMap);
  I3Geometry geometry;

  I3OMGeo g1;
  g1.position = I3Position(100,0,0);
  geometry.omgeo[OMKey(0,0)] = g1;

  I3OMGeo g2;
  g2.position = I3Position(0,75,0);
  geometry.omgeo[OMKey(0,1)] = g2;

  I3OMGeo g3;
  g3.position = I3Position(-100,0,0);
  geometry.omgeo[OMKey(0,2)] = g3;

  I3OMGeo g4;
  g4.position = I3Position(0,-75,0);
  geometry.omgeo[OMKey(0,3)] = g4;

  I3OMGeo g5;
  g5.position = I3Position(0,0,50);
  geometry.omgeo[OMKey(1,0)] = g5;

  I3OMGeo g6;
  g6.position = I3Position(0,0,50);
  geometry.omgeo[OMKey(2,0)] = g6;

  BOOST_FOREACH(I3OMGeoMap::value_type& key_value_pair, geometry.omgeo){
    I3RecoPulse rp;
    rp.SetTime(0.);
    rp.SetCharge(1.);
    rp.SetWidth(10. * I3Units::ns);
    std::vector<I3RecoPulse> rps;
    rps.push_back(rp);
    (*rpsmap)[key_value_pair.first] = rps;
  }

  I3Position cog = I3Cuts::COG(geometry, *rpsmap);
  
  I3TensorOfInertiaCalculator toi_calculator(1.);
  I3Matrix toi = toi_calculator.CalculateTOI(rpsmap, geometry.omgeo, cog);
  
}

