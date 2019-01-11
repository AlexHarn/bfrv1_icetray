#ifndef SimpleOmReceiver_H
#define SimpleOmReceiver_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file SimpleOmReceiver.h
    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Basic implementation of an OM receiver.
    Provides access to the geometry and hardware properties.
*/

namespace IPDF {

  /**
   * @brief Basic implementation of an OM receiver.
   *
   * Provides access to the geometry and hardware properties.
   */
  class SimpleOmReceiver {
  public:
  /**
    * @brief Ctor from simple cartesian coordinates and jitter in PMT hits (sigma)
    *
    * @param x position in IceCube coordinate system.
    * @param y position in IceCube coordinate system.
    * @param z position in IceCube coordinate system.
    * @param orientation is in AMANDA standard: 1.=up -1.=down (i.e. cos(zenith))
    * @param sigma is in nanoseconds, and is used in some PDFs as the width of a single photo-electron hit
    * @param sensit is the relative sensitivity of this PMT
    */
    SimpleOmReceiver(double x , double y, double z, 
	double orientation, double sigma, double sensit=1.0)
	:	x_(x),y_(y),z_(z),
    ori_(orientation), sigma_(sigma), sens_(sensit) { }

    /// @brief Access position in cartesian coordinates
    double getX() const  {return x_;};
    /// @brief Access position in cartesian coordinates
    double getY() const {return y_;};
    /// @brief Access position in cartesian coordinates
    double getZ() const {return z_;};
    /// @brief Access orientation (see Ctor docs)
    double getOrientation() const  {return ori_;};
    /// @brief Access PMT jitter (see Ctor docs)
    double getJitter() const {return sigma_;};
    /// @brief Access sensitivity of PMT
    double getRelativeSensitivity() const {return sens_;};
  private:
    double x_,y_,z_;
    double ori_;
    double sigma_;
    double sens_;
  };

} // namespace IPDF

#endif // SimpleOmReceiver_H
