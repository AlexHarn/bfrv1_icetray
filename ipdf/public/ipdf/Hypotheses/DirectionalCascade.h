#ifndef IPDF_DirectionalCascade_H_INCLUDED
#define IPDF_DirectionalCascade_H_INCLUDED

/**
    copyright  (C) 2007
    the icecube collaboration
    $Id$

    @file DirectionalCascade.h
    @version $Revision$
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
    @author C Wiebusch <wiebusch@physik.uni-wuppertal.de>
    @author DJ Boersma <boersma@icecube.wisc.edu>
*/

#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"

#include <iosfwd>
#include <cmath>

#include "dataclasses/physics/I3Particle.h"

namespace IPDF {

  /**
    @brief simple representation of a simple cascade in the detector.  

    This class is almost identical to the PointCascade. The only
    difference is that the cascade has a direction: light is not emitted
    entirely spherically, but with some bias to forward angles (in the
    direction of the cascade). Technically the ONLY difference with
    PointCascade is that the emission angle is computed differently:
    instead of the angle of the light arrival direction on the OM with
    the OM axis it gives the angle between the cascade direction and
    the difference vector between the OM and the cascade vertex.

    The reason for having two separate classes is in the Pandel
    HitProbability class: for the directional cascade there is
    a direction-dependent correction to the estimation of the mean
    number of photoelectrons.

    This class has constructors, which take I3 classes in order 
    to use ipdf within the I3 framework.

    Accesor methods are provided for the neutino direction in 
    spherical polar coordinates using two conventions:
    @li getTheta() and getPhi() return the direction in 
    which the neutrino is travelling, the standard for ipdf.
    @li getZenith() and getAzimuth() return the direction 
    from which the neutrino originates, the standard in the I3 
    framework.
   */
class DirectionalCascade {
public:
  typedef double energy_result;
  typedef double distance_result;
  typedef double angle_result;

  /// @brief EmissionGeometry represents the geometry of light emission from a casccade
  /// 
  /// Other EmissionHypotheses represent a different EmissionGeometrys
  template<class TIce>
  class EmissionGeometry {
  public:
    /// @brief Determine the perpendicular distance and light emission 
    /// angle relative to a given OM position.
    ///
    /// Can be specialised for OmReceiver classes not matching 
    /// the interface used in the default implementation.
    template<class OmReceiver>
    EmissionGeometry(const DirectionalCascade&, const OmReceiver&);

    double propagationDistance() const;
    // double perpendicularDistance() const { return perpD_; }
    double cosEmissionAngle() const { return cEang_; }
    double geometricalTime() const { return geomT_; }
    double EmissionZCoordinate() const { return zE_; }

  private:
    // double perpD_;
    double cEang_;
    double propD_;
    double geomT_;
    double zE_;
  };

  template<class TIce> friend class EmissionGeometry;

  /// @brief Ctor in simple cartesian coordinates
  DirectionalCascade(double vx, double vy, double vz, 
      double dx, double dy, double dz, double energy=-1e99,
      double t0=0.);

  /// @brief Ctor from a core icetray class (I3Particle)
  explicit DirectionalCascade(const I3ParticleConstPtr);
  /// @brief Ctor using icetray geometry utility classes
  DirectionalCascade(const I3Position&,
	       const I3Direction&,
	       double energy=-1.e99, double t0=0.);
protected:
  /// @brief Conversion between internal structure and icetray
  void init(const I3Position&  posn, const I3Direction& dirn);
public:
  /// @brief Conversion from Self to icetray native (I3Particle)
  I3ParticlePtr makeI3Particle() const {
    I3ParticlePtr basic(new I3Particle(I3Particle::Cascade));
    basic->SetShape(I3Particle::InfiniteTrack);
    basic->SetPos(x_*I3Units::m,y_*I3Units::m,z_*I3Units::m);
    basic->SetDir(this->getZenith(),this->getAzimuth());
    basic->SetTime(t0_*I3Units::ns);
    basic->SetEnergy(t0_*I3Units::GeV);
    return basic;
  }

  /// @brief Access cartesian track description
  distance_result getPointX() const {return x_*I3Units::m;}; 
  /// @brief Access cartesian track description
  distance_result getPointY() const {return y_*I3Units::m;}; 
  /// @brief Access cartesian track description
  distance_result getPointZ() const {return z_*I3Units::m;}; 
  /// @brief Access cartesian track description
  distance_result getDircosX() const {return dx_;}; 
  /// @brief Access cartesian track description
  distance_result getDircosY() const {return dy_;}; 
  /// @brief Access cartesian track description
  distance_result getDircosZ() const {return dz_;}; 
  /// @brief Energy in GeV
  energy_result   getEnergy() const {return e_*I3Units::GeV;};
  /// @brief Timing in the geometry are relative to T0
  double          getTZero() const {return t0_*I3Units::ns;};
  /// @brief Access the direction via polar coords
  double getTheta() const;
  /// @brief Access the direction via polar coords
  double getPhi() const;

  /// @brief Access the direction via astronomical coords
  double getZenith() const;
  /// @brief Access the direction via astronomical coords
  double getAzimuth() const;

  friend std::ostream& operator<<(std::ostream&, const DirectionalCascade&);

  /** Minimum distance (for relative emission/OM vector) */ 
  const double min_dist; // meters
  
protected:
  void normalize();
private:
  double dx_,dy_,dz_,x_,y_,z_,e_,t0_,dn_;
  mutable double theta_,phi_;
};

std::ostream& operator<<(std::ostream& os, 
    const IPDF::DirectionalCascade&);

template<typename TIce>
template<typename OmReceiver>
DirectionalCascade::EmissionGeometry<TIce>::EmissionGeometry(
		const DirectionalCascade& eh,
		const OmReceiver& omr)
{
  double rx = omr.getX() - eh.x_;        /* distance vertex -point */
  double ry = omr.getY() - eh.y_;
  double rz = omr.getZ() - eh.z_;
  double rlen = sqrt(rx*rx + ry*ry + rz*rz);  /* distance vertex - om */
  propD_ = (rlen<eh.min_dist) ? eh.min_dist : rlen;
  /* cosinus alpha */
  double cosalpha = (rlen>0.) ? ((rx*eh.dx_ + ry*eh.dy_ + rz*eh.dz_)/rlen) : (0.);
  AssertX( fabs(cosalpha)<=1., IPDF::NumericalError );
  /* distance = sin(alpha)*rn */
  // double dperp = rlen *sqrt( (1.-cosalpha) * (1.+cosalpha) );

  // Calculate the emission angle relative to the OM:
  // double ori = omr.getOrientation();
  // AssertX((ori<=1.)&&(ori>=-1.),IPDF::IllegalInputParameter);

//  double dist = dperp / IPdfConstants::SN_CER_ICE_P; /* distance of light */
//  double dlon = dist * IPdfConstants::CS_CER_ICE_P;  /* distance of cer light to perp */
  // angle_result csang = (rlen >0.) ? -1. * rz/rlen : 0.; /*angle in global coord */
  // csang *= ori; /* if ori =-1 = downlooking the value is inverted */

  // calculate the geometrical propogation time from R0 to OM:
  double tgeo = propD_ / IPdfConstants::C_ICE_G;
  tgeo += eh.getTZero();
  // perpD_ = dperp;
  // cEang_ = csang;
  cEang_ = cosalpha;  // THIS IS THE DIFFERENCE WITH POINTCASCADE !!!
  geomT_ = tgeo;
  zE_ = eh.z_;
}

template<typename TIce>
double DirectionalCascade::EmissionGeometry<TIce>::propagationDistance() const
{
  return propD_;
}

} // namespace IPDF

#endif // IPDF_DirectionalCascade_H_INCLUDED
