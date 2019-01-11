/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/I3FourVector.h
 * @date $Date$
 * @brief This file contains the definition of the I3FourVector class inside the
 *        sttools namespace. The I3FourVector class is used to store four
 *        components (in case of a four-position, a time and a 3-position).
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
#ifndef STTOOLS_I3FOURVECTOR_H_INCLUDED
#define STTOOLS_I3FOURVECTOR_H_INCLUDED

#include <stdint.h>

#include <cmath>
#include <ostream>
#include <string>

#include "icetray/I3Logging.h"

#include "dataclasses/I3Position.h"

//______________________________________________________________________________
// We need to forward declare the operator<< function and so also the
// I3FourVector class, in order to be able to use the streamToString function
// within in the I3FourVector class itself.
namespace sttools { class I3FourVector; }
std::ostream& operator<<(std::ostream& os, const sttools::I3FourVector& rhs);

#include "STTools/cpp/streamToString.hpp"

//##############################################################################
namespace sttools {

//==============================================================================
class I3FourVector
{
  public:
    //__________________________________________________________________________
    I3FourVector()
    {
        SetToNAN();
    }

    //__________________________________________________________________________
    I3FourVector(double x0, double x1, double x2, double x3)
    {
        Set(x0, x1, x2, x3);
    }

    //__________________________________________________________________________
    I3FourVector(const I3FourVector &rhs)
    {
        Set(rhs.GetX0(), rhs.GetX1(), rhs.GetX2(), rhs.GetX3());
    }

    //////
    // Property getter methods.
    //__________________________________________________________________________
    inline double GetX0() const { return x0_; }
    inline double GetX1() const { return x1_; }
    inline double GetX2() const { return x2_; }
    inline double GetX3() const { return x3_; }

    //////
    // Property setter methods.
    //__________________________________________________________________________
    inline void SetX0(double x) { x0_ = x; }
    inline void SetX1(double x) { x1_ = x; }
    inline void SetX2(double x) { x2_ = x; }
    inline void SetX3(double x) { x3_ = x; }

    //////
    // Public interface methods.
    //__________________________________________________________________________
    inline I3Position GetThreeVectorAsI3Position() const {
        return I3Position(x1_, x2_, x3_, I3Position::car);
    }

    //__________________________________________________________________________
    /** @returns ``true`` if at least one component is nan and ``false``
     *      otherwise.
     */
    inline
    bool
    IsNAN() const {
        return (std::isnan(x0_) ||
                std::isnan(x1_) ||
                std::isnan(x2_) ||
                std::isnan(x3_)
               );
    }

    //__________________________________________________________________________
    /** Sets all four components to the given values.
     */
    inline
    void Set(double x0, double x1, double x2, double x3)
    {
        x0_ = x0;
        x1_ = x1;
        x2_ = x2;
        x3_ = x3;
    }

    //__________________________________________________________________________
    /** Sets all four components to NAN.
     */
    inline
    void SetToNAN()
    {
        Set(NAN, NAN, NAN, NAN);
    }

    //__________________________________________________________________________
    /** @returns A std::string representation of this I3FourVector class.
     */
    inline
    std::string
    str()
    {
        return streamToString<I3FourVector>(*this);
    }

    //////
    // Operator methods.
    //__________________________________________________________________________
    template <typename T>
    I3FourVector& operator+=(const T& rhs)
    {
        x0_ += rhs;
        x1_ += rhs;
        x2_ += rhs;
        x3_ += rhs;

        return *this;
    }
    //--------------------------------------------------------------------------
    I3FourVector& operator+=(const I3FourVector& rhs);

    //__________________________________________________________________________
    template <typename T>
    I3FourVector& operator/=(const T& rhs)
    {
        x0_ /= rhs;
        x1_ /= rhs;
        x2_ /= rhs;
        x3_ /= rhs;

        return *this;
    }
    //--------------------------------------------------------------------------
    I3FourVector& operator/=(const I3FourVector& rhs);

  protected:
    //__________________________________________________________________________
    /** The four components of the vector.
     */
    double x0_, x1_, x2_, x3_;

  private:
    SET_LOGGER("I3FourVector");
};

//==============================================================================

}/*sttools*/
//##############################################################################

#endif//STTOOLS_I3FOURVECTOR_H_INCLUDED
