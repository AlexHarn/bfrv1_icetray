/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.10 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Simple representation of a simple casccade
    in the detector.  
*/

#include "ipdf/Hypotheses/DirectionalCascade.h"
#include "ipdf/Utilities/IPdfException.h"
#include <iostream>
#include <cmath>

#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "icetray/I3Units.h"

using namespace IPDF;
using std::abs;
using std::isfinite;

typedef double energy_result;
typedef double distance_result;
typedef double angle_result;

DirectionalCascade::DirectionalCascade(
    double vx, double vy, double vz, 
    double dx, double dy, double dz, 
    double energy, double t0)
: min_dist(0.0),
  dx_(dx), dy_(dy), dz_(dz),
  x_(vx/I3Units::m), y_(vy/I3Units::m), z_(vz/I3Units::m),
  e_(energy/I3Units::GeV), t0_(t0/I3Units::ns), 
  dn_(sqrt(dx*dx + dy*dy + dz*dz)),
  theta_(-INFINITY),phi_(-INFINITY)
{
  AssertX(isfinite(dx_),IPDF::IllegalInputParameter);
  AssertX(isfinite(dy_),IPDF::IllegalInputParameter);
  AssertX(isfinite(dz_),IPDF::IllegalInputParameter);
  AssertX(isfinite(x_),IPDF::IllegalInputParameter);
  AssertX(isfinite(y_),IPDF::IllegalInputParameter);
  AssertX(isfinite(z_),IPDF::IllegalInputParameter);
  AssertX(isfinite(t0_),IPDF::IllegalInputParameter);
  AssertX(isfinite(e_),IPDF::IllegalInputParameter);
  this->normalize();
}

DirectionalCascade::DirectionalCascade(const I3ParticleConstPtr i3particle)
: min_dist(0.0),
  e_(i3particle->GetEnergy()/I3Units::GeV),
  t0_(i3particle->GetTime()/I3Units::nanosecond), 
  theta_(-INFINITY),phi_(-INFINITY)
{
  AssertX(isfinite(t0_),IPDF::IllegalInputParameter);
  AssertX(isfinite(e_),IPDF::IllegalInputParameter);
  this->init(i3particle->GetPos(), i3particle->GetDir());
}

DirectionalCascade::DirectionalCascade(const I3Position&  posn,
				     const I3Direction& dirn,
				     double energy, double t0)
: min_dist(0.0),
  e_(energy/I3Units::GeV), t0_(t0/I3Units::ns), 
  theta_(-INFINITY),phi_(-INFINITY)
{
  AssertX(isfinite(t0_),IPDF::IllegalInputParameter);
  AssertX(isfinite(e_),IPDF::IllegalInputParameter);
  this->init(posn,dirn);
}

void DirectionalCascade::init(const I3Position&  posn,
			const I3Direction& dirn) {
  x_=posn.GetX()/I3Units::meter;
  y_=posn.GetY()/I3Units::meter;
  z_=posn.GetZ()/I3Units::meter;

  /// Directional cosines refer to direction particle is going
  dx_=dirn.GetX();
  dy_=dirn.GetY();
  dz_=dirn.GetZ();

  AssertX(isfinite(x_),IPDF::IllegalInputParameter);
  AssertX(isfinite(y_),IPDF::IllegalInputParameter);
  AssertX(isfinite(z_),IPDF::IllegalInputParameter);
  AssertX(isfinite(dx_),IPDF::IllegalInputParameter);
  AssertX(isfinite(dy_),IPDF::IllegalInputParameter);
  AssertX(isfinite(dz_),IPDF::IllegalInputParameter);

  dn_ = sqrt(dx_*dx_ + dy_*dy_ + dz_*dz_);
  this->normalize();
}

void DirectionalCascade::normalize() {
  // 
  // Check normalisation of direction cosine
  AssertX( isfinite(dn_) && (dn_>0.0), IPDF::IllegalInputParameter );
  dx_ = dx_/dn_;
  dy_ = dy_/dn_;
  dz_ = dz_/dn_;
  AssertX( (abs(dx_)<=1.)&&(abs(dy_)<=1.)&&(abs(dz_)<=1.),
      IPDF::NumericalError );
}

double DirectionalCascade::getTheta() const
{
  if(std::isinf(theta_)) {
    theta_=acos(dz_);
    while(theta_<0.) { theta_+=2.*M_PI; }
  }
  return theta_;
}

double DirectionalCascade::getPhi() const
{
  if(std::isinf(phi_)) {
    phi_=atan2(dy_,dx_);
    while(phi_<0.) { phi_+=2.*M_PI; }
  }
  return phi_;
}

// 'Astronomical' spherical coordinates
double DirectionalCascade::getZenith() const
{
  double zenith = M_PI - this->getTheta();
  if(zenith>M_PI) { zenith = 2.*M_PI - zenith; }
  if(zenith<0.)   { zenith = -zenith; }
  return zenith;
}

double DirectionalCascade::getAzimuth() const
{
  double azi = this->getPhi() + M_PI;
  while(azi>2.*M_PI) { azi -= 2.*M_PI; }
  while(azi<0.)	     { azi += 2.*M_PI; }
  return azi;
}

std::ostream& IPDF::operator<<(std::ostream& os,
    const DirectionalCascade& hyp) {
  return (os<<"DirectionalCascade: ( "
      <<"(x,y,z): ("<<hyp.x_<<"m,"<<hyp.y_<<"m,"<<hyp.z_<<"m), "
      <<"(dx,dy,dz): ("<<hyp.dx_<<","<<hyp.dy_<<","<<hyp.dz_<<"), "
      <<hyp.e_<<" GeV, T0 "<<hyp.t0_<<" ns, "
      <<"(theta,phi): ("<<hyp.getTheta()<<"rad,"<<hyp.getPhi()<<"rad)"
      <<" )");
}
