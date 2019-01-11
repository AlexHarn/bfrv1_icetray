/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/STTools/I3FourVector.cxx
 * @date $Date$
 * @brief This file contains the implementation of the I3FourVector class
 *        within the sttools namespace.
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
#include "STTools/I3FourVector.h"

//______________________________________________________________________________
std::ostream&
operator<<(std::ostream& os, const sttools::I3FourVector& rhs)
{
    os << "I3FourVector(" <<
        "X0: " << rhs.GetX0() << ", " <<
        "X1: " << rhs.GetX1() << ", " <<
        "X2: " << rhs.GetX2() << ", " <<
        "X3: " << rhs.GetX3() << ")";

    return os;
}

//##############################################################################
namespace sttools {

//______________________________________________________________________________
I3FourVector&
I3FourVector::
operator+=(const I3FourVector& rhs)
{
    x0_ += rhs.GetX0();
    x1_ += rhs.GetX1();
    x2_ += rhs.GetX2();
    x3_ += rhs.GetX3();

    return *this;
}

//______________________________________________________________________________
I3FourVector&
I3FourVector::
operator/=(const I3FourVector& rhs)
{
    x0_ /= rhs.GetX0();
    x1_ /= rhs.GetX1();
    x2_ /= rhs.GetX2();
    x3_ /= rhs.GetX3();

    return *this;
}

}/*sttools*/
//##############################################################################
