/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/hit_info_getters.h
 * @date $Date$
 * @brief This file contains the definitions of utility functions to get
 *        information about hits within the sttools namespace.
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
#ifndef STTOOLS_HIT_INFO_GETTERS_H_INCLUDED
#define STTOOLS_HIT_INFO_GETTERS_H_INCLUDED

//##############################################################################
namespace sttools {

//______________________________________________________________________________
/** Function to get the charge of a hit type object.
 */
template <class HitType>
double
getHitCharge(const HitType &hit);

//______________________________________________________________________________
/** Function to get the Local Coincidence bit from a hit type object.
 */
template <class HitType>
bool
getHitLCBit(const HitType &hit);

//______________________________________________________________________________
/** Function to get the time from a hit type object.
 */
template <class HitType>
double
getHitTime(const HitType &hit);

}/*sttools*/
//##############################################################################

#endif//STTOOLS_HIT_INFO_GETTERS_H_INCLUDED
