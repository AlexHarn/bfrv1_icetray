#ifndef I3OmReceiver_H
#define I3OmReceiver_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file I3OmReceiver.h
    @version $Revision: 1.4 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @todo Where to get the relative efficiency of the OM?
*/

#include <iosfwd>

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/geometry/I3OMGeo.h"

namespace IPDF {

/**
 * @brief Implementation of an OM receiver via the Icecube framework classes.
 *
 * Provides access to the geometry and hardware properties.
 */
  class I3OmReceiver {
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
    I3OmReceiver(double x , double y, double z, 
	double orientation, double sigma, double sensit=1.0)
	:	x_(x),y_(y),z_(z),
    ori_(orientation), sigma_(sigma), sens_(sensit) { }

  /// @brief Ctor from icetray native OM geometry (I3OMGeo)
  explicit I3OmReceiver(const I3OMGeo&);
protected:
  /// @brief Calculate the orientation using icetray native OM orientation ENUM
  void setOrientation(const I3Orientation&);
public:

  ///  @brief Access position in cartesian coordinates
   double getX() const  {return x_;};
  ///  @brief Access position in cartesian coordinates
    double getY() const {return y_;};
  ///  @brief Access position in cartesian coordinates
    double getZ() const {return z_;};
  ///  @brief Access orientation (see Ctor docs)
    double getOrientation() const  {return ori_;};
  ///  @brief Access PMT jitter (see Ctor docs)
    double getJitter() const {return sigma_;};
  ///  @brief Access sensitivity of PMT
    double getRelativeSensitivity() const {return sens_;};

    friend std::ostream& operator<<(std::ostream&, const I3OmReceiver&);
  private:
//    const I3Position& posn_;  /// @todo Use I3Position here?
    double x_,y_,z_;
    double ori_;
    double sigma_;
    double sens_;
  };

  std::ostream& operator<<(std::ostream& os, 
      const IPDF::I3OmReceiver&);

} // namespace IPDF

#endif // I3OmReceiver_H
