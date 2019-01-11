/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file I3SeededRTSContext.h
 * @date $Date$
 * @brief This file contains the definition of the I3SeededRTSContext class, a
 *        specialization of the I3SContext template for the seededRT algorithm
 *        within the sttools::seededRT namespace.
 *
 *        ----------------------------------------------------------------------
 *        This file is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation; either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSCONTEXT_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSCONTEXT_H_INCLUDED

#include "icetray/I3PointerTypedefs.h"

#include "STTools/OMKeyPairMap.h"
#include "STTools/I3SContext.h"

#include "STTools/algorithms/seededRT/I3SeededRTSData.h"

//##############################################################################
namespace sttools { namespace seededRT {

//==============================================================================
class I3SeededRTSContext
  : public sttools::I3SContext<I3SeededRTSData>
{
  public:
    //__________________________________________________________________________
    /** The default constructor.
     */
    I3SeededRTSContext()
      : sttools::I3SContext<I3SeededRTSData>(),
        maxDustlayerCorrectionLength_(0)
    {}

    //__________________________________________________________________________
    I3SeededRTSContext(const I3OMGeoMap &omGeoMap, sttools::OMKeyPairMap<I3SeededRTSData>::Symmetry sDataMapSym)
      : sttools::I3SContext<I3SeededRTSData>(omGeoMap, sDataMapSym),
        maxDustlayerCorrectionLength_(0)
    {}

    //__________________________________________________________________________
    inline
    double GetMaxDustlayerCorrectionLength() const {
        return maxDustlayerCorrectionLength_;
    }

    //__________________________________________________________________________
    inline
    void SetMaxDustlayerCorrectionLength(double l) {
        maxDustlayerCorrectionLength_ = l;
    }

  protected:
    //__________________________________________________________________________
    /// The maximal dustlayer correction length calculated for the spatial data
    /// map.
    double maxDustlayerCorrectionLength_;
};

I3_POINTER_TYPEDEFS(I3SeededRTSContext);

//==============================================================================

}/*seededRT*/}/*sttools*/
//##############################################################################

#endif//STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSCONTEXT_H_INCLUDED
