#ifndef I3ICELAYERTILT_H
#define I3ICELAYERTILT_H
#include <string>

/**
 *@file
 *@brief I3IceLayerTilt
 *       A class for calculating ice layer tilt.
 *
 *@author Dmitry Chirkin
 *(c) the IceCube Collaboration
 */

class I3IceLayerTilt {

 private:

#define LMAX 6    // number of dust loggers
#define LYRS 170  // number of depth points

  bool tilt;
  int lnum, lpts, l0;
  double lmin, lrdz, r0;
  double lnx, lny;
  double lr[LMAX];
  double lp[LMAX][LYRS];

  void error(const std::string&);

 public:
  I3IceLayerTilt();

  void ini(std::string);
  void set_r0();
  double zshift(double, double, double);

};

#endif
