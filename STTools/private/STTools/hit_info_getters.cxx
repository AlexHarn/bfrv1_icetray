/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/STTools/hit_info_getters.cxx
 * @date $Date$
 * @brief This file contains the implementation of the utility functions to
 *        get information about hits within the sttools namespace.
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
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

#include "STTools/hit_info_getters.h"

//##############################################################################
namespace sttools {

//______________________________________________________________________________
// Function to get the charge of a hit type object ...
// ... from an I3DOMLaunch object.
template <>
double
getHitCharge<I3DOMLaunch>(const I3DOMLaunch& hit)
{
    // There is no charge defined for a DOM launch! For weighting of the COG of
    // the hits, return the value ``1.``.
    return 1.;
}

// ... from an I3RecoPulse object.
template <>
double
getHitCharge<I3RecoPulse>(const I3RecoPulse& hit)
{
    return hit.GetCharge();
}

//______________________________________________________________________________
// Function to get the Local Coincidence bit from a hit type object ...
// ... from an I3DOMLaunch object.
template <>
bool
getHitLCBit<I3DOMLaunch>(const I3DOMLaunch& hit)
{
    return hit.GetLCBit();
}

// ... from an I3RecoPulse object.
template <>
bool
getHitLCBit<I3RecoPulse>(const I3RecoPulse& hit)
{
    return (hit.GetFlags() & I3RecoPulse::LC);
}

//______________________________________________________________________________
// Function to get the time from a hit type object ...
// ... from an I3DOMLaunch object.
template <>
double
getHitTime<I3DOMLaunch>(const I3DOMLaunch& hit)
{
    return hit.GetStartTime();
}

// ... from an I3RecoPulse object.
template <>
double
getHitTime<I3RecoPulse>(const I3RecoPulse& hit)
{
    return hit.GetTime();
}

}/*sttools*/
//##############################################################################
