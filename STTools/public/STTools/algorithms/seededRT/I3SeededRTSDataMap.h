/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/algorithms/seededRT/I3SeededRTSDataMap.h
 * @date $Date$
 * @brief This file contains the definition of the I3SeededRTSDataMap class
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
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSDATAMAP_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSDATAMAP_H_INCLUDED

#include "icetray/I3PointerTypedefs.h"

#include "STTools/OMKeyPairMap.h"

#include "STTools/algorithms/seededRT/I3SeededRTSData.h"

//##############################################################################
namespace sttools { namespace seededRT {

typedef OMKeyPairMap<I3SeededRTSData> I3SeededRTSDataMap;
I3_POINTER_TYPEDEFS(I3SeededRTSDataMap);

}/*seededRT*/}/*sttools*/
//##############################################################################

#endif//STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSDATAMAP_H_INCLUDED
