#ifndef IPDF_PointCascade_H_INCLUDED
#define IPDF_PointCascade_H_INCLUDED

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file PointCascade.h
    @version $Revision: 1.9 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
    @author C Wiebusch <wiebusch@physik.uni-wuppertal.de>
*/

#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"

#include <iosfwd>
#include <cmath>

#include "dataclasses/physics/I3Particle.h"

namespace IPDF {

  /**
    @brief simple representation of a simple cascade in the detector.  

    This is a first attempt at this class.  I assume a point source 
    of light, at the vertex, with light propagating directly (spherically) 
    to the OMs.

    This class has constructors, which take I3 classes in order 
    to use ipdf within the I3 framework.

   */
class PointCascade {
public:
  typedef double energy_result;
  typedef double distance_result;
  // typedef double angle_result;

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
    EmissionGeometry(const PointCascade&, const OmReceiver&);

    double propagationDistance() const;
    // double perpendicularDistance() const { return perpD_; }
    // double cosEmissionAngle() const { return cEang_; }
    double geometricalTime() const { return geomT_; }
    double EmissionZCoordinate() const { return zE_; }

  private:
    // double perpD_;
    // double cEang_;
    double propD_;
    double geomT_;
    double zE_;
  };

  template<class TIce> friend class EmissionGeometry;

  /// @brief Ctor in simple cartesian coordinates
  PointCascade(double vx, double vy, double vz,
          double energy=-1e99, double t0=0.);

  /// @brief Ctor from a core icetray class (I3Particle)
  explicit PointCascade(const I3ParticleConstPtr);
  /// @brief Ctor using icetray geometry utility classes
  PointCascade(const I3Position& p,
	       const I3Direction& d,
	       double energy=-1.e99, double t0=0.);
protected:
  /// @brief Conversion between internal structure and icetray
  void init(const I3Position&  posn);
  void init(const I3Position&  posn, const I3Direction& dirn){init(posn);}
public:
  /// @brief Conversion from Self to icetray native (I3Particle)
  I3ParticlePtr makeI3Particle() const {
    I3ParticlePtr basic(new I3Particle(I3Particle::Cascade));
    basic->SetShape(I3Particle::Cascade);
    basic->SetPos(x_*I3Units::m,y_*I3Units::m,z_*I3Units::m);
    basic->SetTime(t0_*I3Units::ns);
    basic->SetEnergy(e_*I3Units::GeV);
    return basic;
  }

  /// @brief Access cartesian track description
  distance_result getPointX() const {return x_*I3Units::m;}; 
  /// @brief Access cartesian track description
  distance_result getPointY() const {return y_*I3Units::m;}; 
  /// @brief Access cartesian track description
  distance_result getPointZ() const {return z_*I3Units::m;}; 
  /// @brief Energy in GeV
  energy_result   getEnergy() const {return e_*I3Units::GeV;};
  /// @brief Timing in the geometry are relative to T0
  double          getTZero() const {return t0_*I3Units::ns;};

  friend std::ostream& operator<<(std::ostream&, const PointCascade&);

  /** Minimum distance (for relative emission/OM vector) */ 
  const double min_dist; // meters
  
private:
  double x_,y_,z_,e_,t0_,dn_;
};

std::ostream& operator<<(std::ostream& os, 
    const IPDF::PointCascade&);

template<typename TIce>
template<typename OmReceiver>
PointCascade::EmissionGeometry<TIce>::EmissionGeometry(
		const PointCascade& eh,
		const OmReceiver& omr)
{
  double rx = omr.getX() - eh.x_;        /* distance vertex -point */
  double ry = omr.getY() - eh.y_;
  double rz = omr.getZ() - eh.z_;
  double rlen = sqrt(rx*rx + ry*ry + rz*rz);  /* distance vertex - om */
  propD_ = (rlen>eh.min_dist) ? rlen : eh.min_dist;
  /* cosinus alpha */
  // double cosalpha = (rlen>0.) ? ((rx*eh.dx_ + ry*eh.dy_ + rz*eh.dz_)/rlen) : (0.);
  // AssertX( std::abs(cosalpha)<=1., IPDF::NumericalError );
  /* distance = sin(alpha)*rn */
  // double dperp = rlen; //  *sqrt( (1.-cosalpha) * (1.+cosalpha) );

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
  geomT_ = tgeo;
  zE_ = eh.z_;
}

template<typename TIce>
double PointCascade::EmissionGeometry<TIce>::propagationDistance() const
{
  return propD_;
}

} // namespace IPDF

#endif // IPDF_PointCascade_H_INCLUDED
