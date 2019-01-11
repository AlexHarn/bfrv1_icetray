#ifndef TANKPULSE_H_INCLUDED
#define TANKPULSE_H_INCLUDED

#include "icetray/OMKey.h"

struct tankPulse {
  tankPulse(){}
  tankPulse(OMKey k, double xp, double yp, double zp, double tp, double widthp, double logvemp, double snowdepthp, double usepulsetimep):
    omkey(k),
    x(xp),
    y(yp),
    z(zp),
    t(tp),
    width(widthp),
    logvem(logvemp),
    snowdepth(snowdepthp),
    usepulsetime(usepulsetimep)
  {}

  OMKey omkey;  // needed for LC retriggering
  double x;
  double y;
  double z;
  double t;
  double width;
  double logvem;  // because don't really need vem in THIS llh, for saturated charges : log10(saturatedCharge/VEM)
  double snowdepth;
  bool usepulsetime; //for cutting on highly fluctuating pulses inside laputop (and still using them for charge)
};

#endif // define TANKPULSE_H_INCLUDED
