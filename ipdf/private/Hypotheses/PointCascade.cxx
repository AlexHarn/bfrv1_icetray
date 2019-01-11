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

#include "ipdf/Hypotheses/PointCascade.h"
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

PointCascade::PointCascade(
    double vx, double vy, double vz, 
    double energy, double t0)
: min_dist(0.0),
  x_(vx/I3Units::m), y_(vy/I3Units::m), z_(vz/I3Units::m),
    e_(energy/I3Units::GeV), t0_(t0/I3Units::ns)
{
  AssertX(isfinite(x_),IPDF::IllegalInputParameter);
  AssertX(isfinite(y_),IPDF::IllegalInputParameter);
  AssertX(isfinite(z_),IPDF::IllegalInputParameter);
  AssertX(isfinite(t0_),IPDF::IllegalInputParameter);
  AssertX(isfinite(e_),IPDF::IllegalInputParameter);
}


PointCascade::PointCascade(const I3ParticleConstPtr i3particle)
: min_dist(0.0),
  e_(i3particle->GetEnergy()/I3Units::GeV), t0_(i3particle->GetTime()/I3Units::nanosecond)
{
  AssertX(isfinite(t0_),IPDF::IllegalInputParameter);
  AssertX(isfinite(e_),IPDF::IllegalInputParameter);
  this->init(i3particle->GetPos());
}

PointCascade::PointCascade( const I3Position&  posn,
                            const I3Direction& dirn,
                            double energy, double t0)
: min_dist(0.0),
  e_(energy/I3Units::GeV), t0_(t0/I3Units::ns)
{
  AssertX(isfinite(t0_),IPDF::IllegalInputParameter);
  AssertX(isfinite(e_),IPDF::IllegalInputParameter);
  this->init(posn);
}

void PointCascade::init(const I3Position&  posn) {
  x_=posn.GetX()/I3Units::meter;
  y_=posn.GetY()/I3Units::meter;
  z_=posn.GetZ()/I3Units::meter;

  AssertX(isfinite(x_),IPDF::IllegalInputParameter);
  AssertX(isfinite(y_),IPDF::IllegalInputParameter);
  AssertX(isfinite(z_),IPDF::IllegalInputParameter);
}

std::ostream& IPDF::operator<<(std::ostream& os,
    const PointCascade& hyp) {
  return (os<<"PointCascade: ( "
      <<"(x,y,z): ("<<hyp.x_<<"m,"<<hyp.y_<<"m,"<<hyp.z_<<"m), "
      <<hyp.e_<<" GeV, T0 "<<hyp.t0_<<"ns, "
      <<" )");
}
