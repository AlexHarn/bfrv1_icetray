/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.10 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Simple representation of an infinite muon
    track in the detector.  
*/

#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Utilities/IPdfException.h"
#include <iostream>
#include <cassert>

#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "icetray/I3Units.h"

using namespace IPDF;

typedef double energy_result;
typedef double distance_result;
typedef double angle_result;

InfiniteMuon::InfiniteMuon(
    double vx, double vy, double vz, 
    double dx, double dy, double dz, 
    double energy, double t0)
: min_dist(0.0),
  x_(vx/I3Units::m), y_(vy/I3Units::m), z_(vz/I3Units::m),
  dx_(dx), dy_(dy), dz_(dz),
  e_(energy/I3Units::GeV), t0_(t0/I3Units::ns),
  dn_(sqrt(dx*dx + dy*dy + dz*dz)),
  theta_(-INFINITY),phi_(-INFINITY)
{
  AssertX(std::isfinite(x_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(y_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(z_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(dx_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(dy_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(dz_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(t0_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(e_),IPDF::IllegalInputParameter);
  this->normalize();
}


InfiniteMuon::InfiniteMuon(const I3ParticleConstPtr i3track)
: min_dist(0.0),
  e_(i3track->GetEnergy()/I3Units::GeV), t0_(i3track->GetTime()/I3Units::nanosecond), 
  theta_(-INFINITY),phi_(-INFINITY)
{
  AssertX(std::isfinite(t0_),IPDF::IllegalInputParameter);
  this->init(i3track->GetPos(), i3track->GetDir());
}

InfiniteMuon::InfiniteMuon(const I3Position&  posn,
			   const I3Direction& dirn,
			   double energy, double t0)
: min_dist(0.0),
  e_(energy/I3Units::GeV), t0_(t0/I3Units::ns), 
  theta_(-INFINITY),phi_(-INFINITY)
{
  AssertX(std::isfinite(t0_),IPDF::IllegalInputParameter);
  this->init(posn,dirn);
}

void InfiniteMuon::init(const I3Position&  posn,
			const I3Direction& dirn) {
  x_=posn.GetX()/I3Units::meter;
  y_=posn.GetY()/I3Units::meter;
  z_=posn.GetZ()/I3Units::meter;

  /// Directional cosines refer to direction particle is going
  dx_=dirn.GetX();
  dy_=dirn.GetY();
  dz_=dirn.GetZ();

  AssertX(std::isfinite(x_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(y_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(z_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(dx_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(dy_),IPDF::IllegalInputParameter);
  AssertX(std::isfinite(dz_),IPDF::IllegalInputParameter);

  dn_ = sqrt(dx_*dx_ + dy_*dy_ + dz_*dz_);
  this->normalize();
}


void InfiniteMuon::normalize() {
  //
  // Check normalisation of direction cosine
  AssertX( std::isfinite(dn_) && (dn_>0.0), IPDF::IllegalInputParameter );
  dx_ = dx_/dn_;
  dy_ = dy_/dn_;
  dz_ = dz_/dn_;
  AssertX( (std::abs(dx_)<=1.)&&(std::abs(dy_)<=1.)&&(std::abs(dz_)<=1.),
  IPDF::NumericalError );
}

double InfiniteMuon::getTheta() const
{
  if(std::isinf(theta_)) {
    theta_=acos(dz_);
    while(theta_<0.) { theta_+=2.*M_PI; }
  }
  return theta_;
}

double InfiniteMuon::getPhi() const
{
  if(std::isinf(phi_)) {
    phi_=atan2(dy_,dx_);
    while(phi_<0.) { phi_+=2.*M_PI; }
  }
  return phi_;
}

// 'Astronomical' spherical coordinates
double InfiniteMuon::getZenith() const
{
  double zenith = M_PI - this->getTheta();
  if(zenith>M_PI) { zenith = 2.*M_PI - zenith; }
  if(zenith<0.)   { zenith = -zenith; }
  return zenith;
}

double InfiniteMuon::getAzimuth() const
{
  double azi = this->getPhi() + M_PI;
  while(azi>2.*M_PI) { azi -= 2.*M_PI; }
  while(azi<0.)	     { azi += 2.*M_PI; }
  return azi;
}

std::ostream& IPDF::operator<<(std::ostream& os,
    const InfiniteMuon& hyp) {
  return (os<<"InfiniteMuon: ( "
      <<"(x,y,z): ("<<hyp.x_<<"m,"<<hyp.y_<<"m,"<<hyp.z_<<"m), "
      <<"(dx,dy,dz): ("<<hyp.dx_<<","<<hyp.dy_<<","<<hyp.dz_<<"), "
      <<hyp.e_<<" GeV, T0 "<<hyp.t0_<<"ns, "
      <<"(theta,phi): ("<<hyp.getTheta()<<"rad,"<<hyp.getPhi()<<"rad)"
      <<" )");
}

