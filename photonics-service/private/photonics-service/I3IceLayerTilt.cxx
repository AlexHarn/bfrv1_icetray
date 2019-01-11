#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include "icetray/I3Logging.h"

#include <photonics-service/I3IceLayerTilt.h>

I3IceLayerTilt::I3IceLayerTilt() : tilt(false), lnum(0), l0(0), r0(0) { }

void I3IceLayerTilt::error(const std::string& str){
  log_fatal(str.c_str());
  throw std::runtime_error(str);
}

void I3IceLayerTilt::ini(std::string dir){
  if(tilt || dir=="") return;
  const double cv=M_PI/180, thx=225;
  lnx=cos(cv*thx), lny=sin(cv*thx);

  std::ifstream inFile((dir+"/tilt.par").c_str(), std::ifstream::in);
  if(!inFile.fail()){
    int str;
    double aux;
    std::vector<double> vlr;
    while(inFile >> str >> aux){ if(aux==0) l0=str; vlr.push_back(aux); }
    inFile.close();

    int size=vlr.size();
    if(size>LMAX) error("File tilt.par defines too many dust maps");
    for(int i=1; i<size; i++) if(vlr[i]<vlr[i-1]) error("Tilt map does not use increasing range order");
    for(int i=0; i<size; i++) lr[i]=vlr[i];

    std::ifstream inFile((dir+"/tilt.dat").c_str(), std::ifstream::in);
    if(!inFile.fail()){
      lnum=size;
      double depth;
      std::vector<double> pts(lnum), ds;
      std::vector< std::vector<double> > vlp(lnum);
      while(inFile >> depth){
	int i=0;
	while(inFile >> pts[i++]) if(i>=lnum) break;
	if(i!=lnum) break;
	ds.push_back(depth);
	for(i=0; i<lnum; i++) vlp[i].push_back(pts[i]);
      }
      inFile.close();

      int size=ds.size();
      if(size>LYRS) error("File tilt.dat defines too many map points");
      for(int i=1; i<size; i++) if(ds[i]<ds[i-1]) error("Tilt map does not use increasing depth order");
      for(int i=0; i<lnum; i++) for(int j=0; j<size; j++) lp[i][j]=vlp[i][size-1-j];
      lpts=size;

      if(size<2) lnum=0;
      else{
	double vmin=ds[0], vmax=ds[size-1];
	double zoff=1948.07;  // depth of detector center
	lmin=zoff-vmax; lrdz=(lpts-1)/(vmax-vmin);
      }
    }
    else error("Failed to open file tilt.dat");
  }
  else error("Failed to open file tilt.par");

  if(lnum>0) log_info_stream("Loaded "<<lnum<<"x"<<lpts<<" dust layer points");
  else error("Not enough ice tilt points");

  tilt=true;
  set_r0();
}

void I3IceLayerTilt::set_r0(){
  // need access to geoemtry to avoid hard-coded value below
  // this is the distance to string 86 from center, along the tilt gradient, in meters
  if(l0==86) r0=3.00521;
  log_debug_stream("r0 is set to "<<r0);
}

double I3IceLayerTilt::zshift(double rx, double ry, double rz){
  if(!tilt) return 0;
  double z=(rz-lmin)*lrdz;
  int k=std::min(std::max((int)floor(z), 0), lpts-2);
  int l=k+1;

  double nr=lnx*rx+lny*ry-r0, result=0;
  for(int j=1; j<LMAX; j++) if(nr<lr[j] || j==lnum-1){
      int i=j-1;
      result = ( (lp[j][l]*(z-k)+lp[j][k]*(l-z))*(nr-lr[i]) +
		 (lp[i][l]*(z-k)+lp[i][k]*(l-z))*(lr[j]-nr) )/(lr[j]-lr[i]);
      break;
    }
  return result;
}
