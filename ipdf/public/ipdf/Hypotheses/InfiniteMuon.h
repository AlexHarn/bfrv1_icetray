#ifndef IPDF_InfiniteMuon_H_INCLUDED
#define IPDF_InfiniteMuon_H_INCLUDED

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file InfiniteMuon.h
    @version $Revision: 1.9 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>, David Boersma <boersma@icecube.wisc.edu>
*/

#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"

#include <iosfwd>
#include <cmath>

#include "dataclasses/physics/I3Particle.h"
#include "icetray/I3Logging.h"

namespace IPDF {

  /**
    @brief simple representation of an infinite muon track in the detector.  

    This class has constructors, which take I3 classes in order 
    to use ipdf within the I3 framework.

    Accesor methods are provided for the muon direction in 
    spherical polar coordinates using two conventions:
    @li getTheta() and getPhi() return the direction in 
    which the muon is travelling, the standard for ipdf.
    @li getZenith() and getAzimuth() return the direction 
    from which the muon originates, the standard in the I3
    framework:
    http://icecube.umd.edu/dule/icecube/axes.html

    @todo Refactor EmissionGeometry ctor
    @todo Check corrected cherenkov angle in geometrical time 
    @todo Why do I3Particles have infinite energy, handle it?
   */
class InfiniteMuon {
public:
  typedef double energy_result;
  typedef double distance_result;
  typedef double angle_result;

  template<class TIce> friend class EmissionGeometry;

  /// @brief Ctor in simple cartesian coordinates
  InfiniteMuon(double vx, double vy, double vz, 
      double dx, double dy, double dz, double energy=-1e99,
      double t0=0.);

  /// @brief EmissionGeometry represents the geometry of Cherenkov light emission
  ///
  /// Other EmissionHypotheses represent different EmissionGeometrys
  template<class TIce>
  class EmissionGeometry {
  public:
    /// @brief Determine the perpendicular distance and light emission 
    /// angle relative to a given OM position.
    ///
    /// Can be specialised for OmReceiver classes not matching 
    /// the interface used in the default implementation.
    template<class OmReceiver>
    explicit EmissionGeometry(const InfiniteMuon&, const OmReceiver&);

    /// @brief Use the IceModel to determine the photon propagation distance
    double propagationDistance() const;
    double perpendicularDistance() const { return perpD_; }
    double cosEmissionAngle() const { return cEang_; }
    double geometricalTime() const { return geomT_; }
    double EmissionZCoordinate() const { return zE_; }

  private:
    double perpD_;
    double cEang_;
    double geomT_;
    double zE_;
  };

  /// @brief Ctor from a core icetray class (I3Particle)
  explicit InfiniteMuon(const I3ParticleConstPtr);
  /// @brief Ctor using icetray geometry utility classes
  InfiniteMuon(const I3Position&,
	       const I3Direction&,
	       double energy=-1.e99, double t0=0.);
protected:
  /// @brief Conversion between internal structure and icetray
  void init(const I3Position&  posn, const I3Direction& dirn);
public:
  /// @brief Conversion from Self to icetray native (I3Particle)
  I3ParticlePtr makeI3Particle() const {
    I3ParticlePtr basic(new I3Particle(I3Particle::InfiniteTrack));
    basic->SetPos(x_*I3Units::m,y_*I3Units::m,z_*I3Units::m);
    // I3 use astronomical coordinates (zenith, azimuth):
    basic->SetDir(this->getZenith(),this->getAzimuth());
    //basic->SetDirection(-dx_,-dy_,-dz_);  // maybe this is better?
    basic->SetTime(t0_*I3Units::ns);
    basic->SetEnergy(e_*I3Units::GeV);
    return basic;
  }

  SET_LOGGER("ipdf");

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

  friend std::ostream& operator<<(std::ostream&, const InfiniteMuon&);

  /** Minimum distance (for relative emission/OM vector) */ 
  const double min_dist; // meters
  
protected:
  void normalize();
private:
  double x_,y_,z_,dx_,dy_,dz_,e_,t0_,dn_;
  mutable double theta_,phi_;
};

std::ostream& operator<<(std::ostream& os, 
    const IPDF::InfiniteMuon&);

template<typename TIce>
template<typename OmReceiver>
InfiniteMuon::EmissionGeometry<TIce>::EmissionGeometry(
		const InfiniteMuon& eh,
		const OmReceiver& omr)
{
  double rx = omr.getX() - eh.x_;        /* distance vertex -point */
  double ry = omr.getY() - eh.y_;
  double rz = omr.getZ() - eh.z_;
  double rlen = sqrt(rx*rx + ry*ry + rz*rz);  /* distance vertex - om */
  rlen = (rlen<eh.min_dist) ? eh.min_dist : rlen;
  /* cosinus alpha */
  double cosalpha = (rlen>0.) ? ((rx*eh.dx_ + ry*eh.dy_ + rz*eh.dz_)/rlen) : (0.);
  cosalpha = (fabs(cosalpha)>1.) ? cosalpha/fabs(cosalpha) : cosalpha; /* stop unidentified round-off problem */
  AssertX( fabs(cosalpha)<=1., IPDF::NumericalError );
  /* distance = sin(alpha)*rn */
  double dperp = rlen *sqrt( (1.-cosalpha) * (1.+cosalpha) );

  // Calculate the emission angle relative to the OM:
  double ori = omr.getOrientation();
  log_trace( "ori=%f", ori );
  AssertX((ori<=1.)&&(ori>=-1.),IPDF::IllegalInputParameter);

  double dist = dperp / IPdfConstants::SN_CER_ICE_P; /* distance of light */
  double dlon = dist * IPdfConstants::CS_CER_ICE_P;  /* distance of cer light to perp */
  double vde  = rlen * cosalpha - dlon; /* distance from point to vertex  */
  double zE = eh.z_ + vde * (eh.getDircosZ()); /* z coordinate of (direct) Cherenkov emission point */
  double zdist = rz - vde * (eh.getDircosZ()); /* z diff of (direct) Cherenkov emission point to OM */
  angle_result csang = (dist >0.) ? -1. * zdist/dist : 0.; /*angle in global coord */
  csang *= ori; /* if ori =-1 = downlooking the value is inverted */

  // calculate the geometrical propogation time from R0 to OM:
  double tgeo = dperp * IPdfConstants::TG_CER_TKOEFF; // corrected TAN(CER_ANGLE) should provide better angular resolution
  tgeo += eh.dx_*rx + eh.dy_*ry + eh.dz_*rz;
  tgeo /= IPdfConstants::C_VACUUM;
  tgeo += eh.getTZero();
  perpD_ = dperp;
  cEang_ = csang;
  geomT_ = tgeo;
  zE_ = zE;
}


template<typename TIce>
double InfiniteMuon::EmissionGeometry<TIce>::propagationDistance() const
{
     return ( TIce::TD_DIST_P1*perpD_
	    + TIce::TD_DIST_P0_CS0
	    + TIce::TD_DIST_P0_CS1*cEang_
	    + TIce::TD_DIST_P0_CS2*cEang_*cEang_ );
}

} // namespace IPDF

#endif // IPDF_InfiniteMuon_H_INCLUDED
