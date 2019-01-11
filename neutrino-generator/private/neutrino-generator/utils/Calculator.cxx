/**
 *  copyright  (C) 2005
 *  the IceCube collaboration
 *  $Id: $
 *
 *  @version $Revision: $
 *  @date    $Date: $
 *  @author  Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *
 *  @brief   Calculator IMPLEMENTATION FILE
 */

#include <neutrino-generator/utils/Calculator.h>
#include "neutrino-generator/Particle.h"

using namespace std;

namespace nugen {

//_________________________________________________________
double Calculator::CalcPowerLawEnergyFactor(
                                double primaryE,
                                double gamma,
                                double eminLog,
                                double emaxLog)
{
  //
  // calculate energy weight
  // this is a part of oneweight. See Gary's note
  // http://www.icecube.wisc.edu/~hoshina/docs/nusim.pdf
  //

  // integral of energy spectrum
  double emax = pow(10.0, emaxLog) / I3Units::GeV;
  double emin = pow(10.0, eminLog) / I3Units::GeV;
  double eintg = 0; // integral of energy range

  if (gamma == 1) {
     // if E^-1 then integral over Emin and Emax is
     eintg = log(emax / emin);

  } else {
     // if not E^-1 then integral over Emin and Emax is
     eintg = (pow(emax , (1.- gamma)) -
           pow(emin, (1.- gamma))) / (1.- gamma);
  }

  double efactor = pow( primaryE , - gamma );

  return  eintg / efactor;

}

//_________________________________________________________
double Calculator::CalcSolidAngleFactor(
                            double minzen,
                            double maxzen,
                            double minazi,
                            double maxazi)
{
  // all angles must be in radian.
  double maxcoszen = cos(minzen);
  double mincoszen = cos(maxzen);
  double solidAngle = (maxcoszen - mincoszen)*(maxazi - minazi);
  if (solidAngle == 0) {
      // solid angle falls to zero, that means you are interested 
      // in a limited direction. just return 1.0.
      return 1.0;
  }
  return solidAngle;
}

//_________________________________________________________________

double Calculator::DistanceToNextBoundary(
                       earthmodel::EarthModelServiceConstPtr earth,
                       const I3Position &posCE,
                       const I3Direction &dirCE)
{
   // this function returns zero when the posCE is exactly on boundary.
   double dist = earth->DistanceToNextBoundaryCrossing(posCE, dirCE);

   const double tinydist = 1.0e-6 * I3Units::m; 
   if (dist < tinydist) {
      // the position is almost on boundary, and no one care about 1um difference.
      // Step forward with tiny distance, make cross boundary, and find the 
      // next boundary.
      dist = earth->DistanceToNextBoundaryCrossing(
                        posCE + tinydist * dirCE, dirCE);
      dist += tinydist;
   }
   return dist;
}

//_________________________________________________________________
I3Position Calculator::Rotate(const I3Position &pos, 
                              double angle,
                              const I3Direction &axis)
{
   //rotate vector
   // code imported from TRotation:
   //const double ll = axis.Mag();
   const double ll = 1.0; // by definition

   if (ll == 0.0) {
      log_warn("zero axis, returns original position.");
      return pos;
   }

   double fX = pos.GetX();
   double fY = pos.GetY();
   double fZ = pos.GetZ();

   const double sa = std::sin(angle), ca = std::cos(angle);
   const double dx = axis.GetX()/ll, dy = axis.GetY()/ll, dz = axis.GetZ()/ll;
   const double fxx = ca+(1.-ca)*dx*dx;    const double fxy = (1.-ca)*dx*dy-sa*dz; const double fxz = (1.-ca)*dx*dz+sa*dy;
   const double fyx = (1.-ca)*dy*dx+sa*dz; const double fyy = ca+(1.-ca)*dy*dy;    const double fyz = (1.-ca)*dy*dz-sa*dx;
   const double fzx = (1.-ca)*dz*dx-sa*dy; const double fzy = (1.-ca)*dz*dy+sa*dx; const double fzz = ca+(1.-ca)*dz*dz;

   const double newfX = fxx*fX + fxy*fY + fxz*fZ;
   const double newfY = fyx*fX + fyy*fY + fyz*fZ;
   const double newfZ = fzx*fX + fzy*fY + fzz*fZ;

   fX = newfX;
   fY = newfY;
   fZ = newfZ;

   return I3Position(fX, fY, fZ);
}


//_________________________________________________________________
I3Position Calculator::RotateUz(const I3Position &pos, 
                                const I3Direction &dir)
{
   double u1 = dir.GetX();
   double u2 = dir.GetY();
   double u3 = dir.GetZ();
   double up = u1*u1 + u2*u2;

   double fX = pos.GetX();
   double fY = pos.GetY();
   double fZ = pos.GetZ();

   if (up) {
      up = std::sqrt(up);
      double px = fX,  py = fY,  pz = fZ;
      fX = (u1*u3*px - u2*py + u1*up*pz)/up;
      fY = (u2*u3*px + u1*py + u2*up*pz)/up;
      fZ = (u3*u3*px -    px + u3*up*pz)/up;
   } else if (u3 < 0.) { fX = -fX; fZ = -fZ; }      // phi=0  teta=pi
   else {};

   return I3Position(fX, fY, fZ);
}

//_________________________________________________________________
double Calculator::Angle(const I3Position &p, const I3Position &q)
{
   // return the angle w.r.t. another 3-vector
   double ptot2 = p.Mag2()*q.Mag2();
   if(ptot2 <= 0) {
      return 0.0;
   } else {
      double arg = (p*q)/std::sqrt(ptot2);
      if(arg >  1.0) arg =  1.0;
      if(arg < -1.0) arg = -1.0;
      return std::acos(arg);
   }
}

//_________________________________________________________________
double Calculator::Angle(const I3Position &p, const I3Direction &qd)
{
   I3Position q(qd);
   return Angle(p, q);
}

//_________________________________________________________________
double Calculator::Angle(const I3Direction &pd, const I3Direction &qd)
{
   I3Position p(pd);
   I3Position q(qd);
   return Angle(p, q);
}

}
