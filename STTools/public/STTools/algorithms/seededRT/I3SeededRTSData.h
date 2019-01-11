/**
 * Copyright (C) 2013 - 2014
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/algorithms/seededRT/I3SeededRTSData.h
 * @date $Date$
 * @brief This file contains the definition of the I3SeededRTSData data
 *        structure within the sttools::seededRT namespace, which is used by
 *        the seededRT algorithm to store spatial map data.
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
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSDATA_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSDATA_H_INCLUDED

#include "STTools/algorithms/seededRT/I3SeededRTConfiguration.h"

namespace sttools {
namespace seededRT {

/** The I3SeededRTSData struct holds data about the spatial causal condition
 *  between two OMs. This data struct is used in an OMKeyPairMap to store
 *  these information for a pair of OMKeys, i.e. one OM link.
 */
struct I3SeededRTSData
{
    //__________________________________________________________________________
    /** The pointer to the ST configuration object valid for the particular
     *  OM link. If this pointer is set to NULL, the two OMs of the OM link are
     *  not in spatial connection.
     */
    I3SeededRTConfiguration const * stConfig_;

    //__________________________________________________________________________
    /** The length of the spatial correction to account for (longer, due to
     *  higher scattering) propagation of light inside the main dustlayer.
     */
    double dustlayerCorrectionLength_;

    //__________________________________________________________________________
    I3SeededRTSData()
      : stConfig_(NULL),
        dustlayerCorrectionLength_(NAN)
    {}
};

}// namespace seededRT
}// namespace sttools

#endif// ! STTOOLS_ALGORITHMS_CLASSICST_I3CLASSICRTSDATA_H_INCLUDED
