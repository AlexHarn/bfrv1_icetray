/**
 * \file IdealGeometry.h
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * tool to create an ideal I3Geometry for test-cases and stuff
 */

#ifndef IDEALGEOMETRY_H
#define IDEALGEOMETRY_H

#include "icetray/I3Module.h"
#include "dataclasses/geometry/I3Geometry.h"

#include <boost/make_shared.hpp>

/** Generate a TestGeometry
    construct an artificially ideal IceCube geometry, that is:
  1. exactly centered at string 36
  2. the major axis of the hexagonal grid follows the x-axis
  3. all strings are at their exact position in the hexagonal structure
  4. The charcteristic inter string spacing is 125 meters
  5. the detector center is 1948 meter below the ice-surface
  6. OM30 on regular IC-strings rests in the detector XY plane
  7. DOMs are spaced 17m in IC, 10/7m in DC
  8. IceTop tanks are located exactly on top of the strings at the surface
  */
I3Geometry GenerateTestGeometry ();

#endif //IDEALGEOMETRY_H
