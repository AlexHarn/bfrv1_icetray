/**
 * \file IdealGeometry.h
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * tools to create an ideal I3Geometry for testcases and stuff
 */

#include "tools/IdealGeometry.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

I3Geometry GenerateTestGeometry () {
  /*
  construct an artificially ideal IceCube geometry, that is:
  1. exactly centered at string 36
  2. the major axis of the hexagonal grid follows the x-axis
  3. all strings are at their exact position in the hexagonal structure
  4. The charcteristic inter string spacing is 125 meters
  5. the detector center is 1948 meter below the ice-surface
  6. OM30 on regular IC-strings rests in the detector XY plane
  7. DOMs are spaced 17m in IC, 10/7m in DC
  8. IceTop tanks are located exactly on top of the strings at the surface
  */
  std::map<unsigned int, I3Position> string_position_map;
  
  const double Xic = 125.0;
  const double Yic = Xic*sqrt(3./4.);
  const double Xip = Xic/3.;
  const double Yip = Yic/3.; 
  
  string_position_map[ 1]=I3Position(  -3*Xic,-4*Yic, 0.);
  string_position_map[ 2]=I3Position(  -2*Xic,-4*Yic, 0.);
  string_position_map[ 3]=I3Position(  -1*Xic,-4*Yic, 0.);
  string_position_map[ 4]=I3Position(       0,-4*Yic, 0.);
  string_position_map[ 5]=I3Position(   1*Xic,-4*Yic, 0.);
  string_position_map[ 6]=I3Position(   2*Xic,-4*Yic, 0.);
  string_position_map[ 7]=I3Position(-3.5*Xic,-3*Yic, 0.);
  string_position_map[ 8]=I3Position(-2.5*Xic,-3*Yic, 0.);
  string_position_map[ 9]=I3Position(-1.5*Xic,-3*Yic, 0.);
  string_position_map[10]=I3Position(-0.5*Xic,-3*Yic, 0.);
  string_position_map[11]=I3Position( 0.5*Xic,-3*Yic, 0.);
  string_position_map[12]=I3Position( 1.5*Xic,-3*Yic, 0.);
  string_position_map[13]=I3Position( 2.5*Xic,-3*Yic, 0.);
  string_position_map[14]=I3Position(  -4*Xic,-2*Yic, 0.);
  string_position_map[15]=I3Position(  -3*Xic,-2*Yic, 0.);
  string_position_map[16]=I3Position(  -2*Xic,-2*Yic, 0.);
  string_position_map[17]=I3Position(  -1*Xic,-2*Yic, 0.);
  string_position_map[18]=I3Position(       0,-2*Yic, 0.);
  string_position_map[19]=I3Position(   1*Xic,-2*Yic, 0.);
  string_position_map[20]=I3Position(   2*Xic,-2*Yic, 0.);
  string_position_map[21]=I3Position(   3*Xic,-2*Yic, 0.);
  string_position_map[22]=I3Position(-4.5*Xic,-1*Yic, 0.);
  string_position_map[23]=I3Position(-3.5*Xic,-1*Yic, 0.);
  string_position_map[24]=I3Position(-2.5*Xic,-1*Yic, 0.);
  string_position_map[25]=I3Position(-1.5*Xic,-1*Yic, 0.);
  string_position_map[26]=I3Position(-0.5*Xic,-1*Yic, 0.);
  string_position_map[27]=I3Position( 0.5*Xic,-1*Yic, 0.);
  string_position_map[28]=I3Position( 1.5*Xic,-1*Yic, 0.);
  string_position_map[29]=I3Position( 2.5*Xic,-1*Yic, 0.);
  string_position_map[30]=I3Position( 3.5*Xic,-1*Yic, 0.);
  string_position_map[31]=I3Position(  -5*Xic,     0, 0.);
  string_position_map[32]=I3Position(  -4*Xic,     0, 0.);
  string_position_map[33]=I3Position(  -3*Xic,     0, 0.);
  string_position_map[34]=I3Position(  -2*Xic,     0, 0.);
  string_position_map[35]=I3Position(  -1*Xic,     0, 0.);
  string_position_map[36]=I3Position(       0,     0, 0.); //center
  string_position_map[37]=I3Position(   1*Xic,     0, 0.);
  string_position_map[38]=I3Position(   2*Xic,     0, 0.);
  string_position_map[39]=I3Position(   3*Xic,     0, 0.);
  string_position_map[40]=I3Position(   4*Xic,     0, 0.);
  string_position_map[41]=I3Position(-4.5*Xic, 1*Yic, 0.);
  string_position_map[42]=I3Position(-3.5*Xic, 1*Yic, 0.);
  string_position_map[43]=I3Position(-2.5*Xic, 1*Yic, 0.);
  string_position_map[44]=I3Position(-1.5*Xic, 1*Yic, 0.);
  string_position_map[45]=I3Position(-0.5*Xic, 1*Yic, 0.);
  string_position_map[46]=I3Position( 0.5*Xic, 1*Yic, 0.);
  string_position_map[47]=I3Position( 1.5*Xic, 1*Yic, 0.);
  string_position_map[48]=I3Position( 2.5*Xic, 1*Yic, 0.);
  string_position_map[49]=I3Position( 3.5*Xic, 1*Yic, 0.);
  string_position_map[50]=I3Position( 4.5*Xic, 1*Yic, 0.);
  string_position_map[51]=I3Position(  -4*Xic, 2*Yic, 0.);
  string_position_map[52]=I3Position(  -3*Xic, 2*Yic, 0.);
  string_position_map[53]=I3Position(  -2*Xic, 2*Yic, 0.);
  string_position_map[54]=I3Position(  -1*Xic, 2*Yic, 0.);
  string_position_map[55]=I3Position(       0, 2*Yic, 0.);
  string_position_map[56]=I3Position(   1*Xic, 2*Yic, 0.);
  string_position_map[57]=I3Position(   2*Xic, 2*Yic, 0.);
  string_position_map[58]=I3Position(   3*Xic, 2*Yic, 0.);
  string_position_map[59]=I3Position(   4*Xic, 2*Yic, 0.);
  string_position_map[60]=I3Position(-3.5*Xic, 3*Yic, 0.);
  string_position_map[61]=I3Position(-2.5*Xic, 3*Yic, 0.);
  string_position_map[62]=I3Position(-1.5*Xic, 3*Yic, 0.);
  string_position_map[63]=I3Position(-0.5*Xic, 3*Yic, 0.);
  string_position_map[64]=I3Position( 0.5*Xic, 3*Yic, 0.);
  string_position_map[65]=I3Position( 1.5*Xic, 3*Yic, 0.);
  string_position_map[66]=I3Position( 2.5*Xic, 3*Yic, 0.);
  string_position_map[67]=I3Position( 3.5*Xic, 3*Yic, 0.);
  string_position_map[68]=I3Position(  -3*Xic, 4*Yic, 0.);
  string_position_map[69]=I3Position(  -2*Xic, 4*Yic, 0.);
  string_position_map[70]=I3Position(  -1*Xic, 4*Yic, 0.);
  string_position_map[71]=I3Position(       0, 4*Yic, 0.);
  string_position_map[72]=I3Position(   1*Xic, 4*Yic, 0.);
  string_position_map[73]=I3Position(   2*Xic, 4*Yic, 0.);
  string_position_map[74]=I3Position(   3*Xic, 4*Yic, 0.);
  string_position_map[75]=I3Position(-2.5*Xic, 5*Yic, 0.);
  string_position_map[76]=I3Position(-1.5*Xic, 5*Yic, 0.);
  string_position_map[77]=I3Position(-0.5*Xic, 5*Yic, 0.);
  string_position_map[78]=I3Position( 0.5*Xic, 5*Yic, 0.);
  string_position_map[79]=I3Position(-0.5*Xip,-1*Yip, 0.);
  string_position_map[80]=I3Position( 0.5*Xip,-1*Yip, 0.);
  string_position_map[81]=I3Position(       0, 2*Yip, 0.);
  string_position_map[82]=I3Position( 1.5*Xip, 1*Yip, 0.);
  string_position_map[83]=I3Position( 1.5*Xip,-1*Yip, 0.);
  string_position_map[84]=I3Position(       0,-2*Yip, 0.);
  string_position_map[85]=I3Position(-1.5*Xip,-1*Yip, 0.);
  string_position_map[86]=I3Position(-1.5*Xip, 1*Yip, 0.);
 

  const double vspacing_ic = 17.; // 17m OM spacing on regular IC strings
  const double vspacing_dc_top = 10.; // 10m OM spacing on DC-strings in the top 10 DOMS
  const double vspacing_dc_bottom = 7.; // 10m OM spacing on DC-strings in the bottom 50 DOMs
  const double Zsurface = 1948.; //distance from the zero-point to the surface
  
  const double OM_zero = 30.; // OM30 lies at z=0
  
  const double dc_veto = 190.; //depth of OM1 on DC-strings
  const double dc_infill = -160.; //depth of OM11 on DC-strings
  //All definitions are done; now work it!
  
  I3Geometry geometry;

  geometry.startTime = I3Time(0,0);
  geometry.endTime = I3Time(0,0);
  I3OMGeoMap &omgeomap = geometry.omgeo;
  
//   BOOST_FOREACH(const std::pair<unsigned int, I3Position> &str_pos_pair, string_position_map) {
  for (std::map<unsigned int, I3Position>::const_iterator str_pos_pair = string_position_map.begin(); str_pos_pair!= string_position_map.end(); ++str_pos_pair) {
    const unsigned int& str = str_pos_pair->first;
    const I3Position& pos = str_pos_pair->second;
    if ( 1 <= str && str <= 78) {
      for (uint om=1; om<=60; ++om) {
        I3OMGeo omgeo;
        omgeo.omtype = I3OMGeo::IceCube;
        omgeo.position = I3Position(pos.GetX(), pos.GetY(), (OM_zero-om)*vspacing_ic);
        omgeo.orientation = I3Orientation(I3Direction(0, 0),I3Direction(M_PI, 0)); //straight_down
        omgeo.area = 0.0443999990821;
        omgeomap[OMKey(str, om)] = omgeo;
      }
    }
    else if ( 79 <= str && str <= 86) { //its infill DC
      for (unsigned int om=1; om<=10; ++om) {
        I3OMGeo omgeo;
        omgeo.omtype = I3OMGeo::IceCube;
        omgeo.position = I3Position(pos.GetX(), pos.GetY(), dc_veto-(om*vspacing_dc_top));
        omgeo.orientation = I3Orientation(I3Direction(0, 0),I3Direction(M_PI, 0)); //straight_down
        omgeo.area = 0.0443999990821;
        omgeomap[OMKey(str, om)] = omgeo;
      }
      
      for (unsigned int om=11; om<=60; ++om) {
        I3OMGeo omgeo;
        omgeo.omtype = I3OMGeo::IceCube;
        omgeo.position = I3Position(pos.GetX(), pos.GetY(), dc_infill-((om-11)*vspacing_dc_bottom));
        omgeo.orientation = I3Orientation(I3Direction(0, 0),I3Direction(M_PI, 0)); //straight_down
        omgeo.area = 0.0443999990821;
        omgeomap[OMKey(str, om)] = omgeo;
      }
    }
  }
  
  
  //ICETOP
  for (unsigned int str=1; str<=80; ++str) {
    const I3Position& pos = string_position_map[str];
    for (unsigned int om=61; om<=64; om++) {
      const OMKey omkey(str, om);
      const I3Position dom_pos(pos.GetX(), pos.GetY(), Zsurface);
      
      I3OMGeo omgeo;
      omgeo.omtype = I3OMGeo::IceTop;
      omgeo.position = I3Position(dom_pos);
      omgeo.orientation = I3Orientation(I3Direction(0, 0),I3Direction(M_PI, 0)); //straight_down
      omgeo.area = 0.0443999990821;
      omgeomap[OMKey(str, om)] = omgeo;
    }
  }
  uint str  = 82; //extra
  const I3Position& pos = string_position_map[str];
  for (unsigned int om=61; om<=64; ++om) {
    const OMKey omkey(str, om);
    const I3Position dom_pos(pos.GetX(), pos.GetY(), Zsurface);

    I3OMGeo omgeo;
    omgeo.omtype = I3OMGeo::IceTop;
    omgeo.position = I3Position(dom_pos);
    omgeo.orientation = I3Orientation(I3Direction(0, 0),I3Direction(M_PI, 0)); //straight_down
    omgeo.area = 0.0443999990821;
    omgeomap[OMKey(str, om)] = omgeo;
  }
  return geometry;
};

//========= CLASS I3GeoDeliver ============

/// A Generator Module for to make a I3Geometry
class I3GeoDeliver: public I3Module{
public: //interaction with the tray
  I3GeoDeliver(const I3Context& context);
  void Configure();
  void Process();
};
I3_MODULE(I3GeoDeliver);

I3GeoDeliver::I3GeoDeliver(const I3Context& context):I3Module(context) {AddOutBox("OutBox");};
void I3GeoDeliver::Configure() {};
void I3GeoDeliver::Process() {
  const I3Geometry geo = GenerateTestGeometry();
  I3FramePtr gframe= boost::make_shared<I3Frame>(I3Frame::Geometry);
  gframe->Put("I3Geometry", boost::make_shared<I3Geometry>(geo));
  PushFrame(gframe);
  RequestSuspension();
};

