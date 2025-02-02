/**
 * $Id$
 *
 * Copyright (C) 2012
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @file I3HitMultiplicityValues.cxx
 * @version $Revision$
 * @date $Date$
 * @author Martin Wolf <martin.wolf@icecube.wisc.edu>
 * @brief This file contains the implementation of the I3HitMultiplicityValues
 *        class, which is an I3FrameObject holding the result values of hit
 *        multiplicity calculation.
 *
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
#include <iostream>

#include "icetray/serialization.h"

#include "recclasses/I3HitMultiplicityValues.h"

//______________________________________________________________________________
template <class Archive>
void
I3HitMultiplicityValues::
serialize(Archive& ar, unsigned version)
{
    if(version > i3hitmultiplicityvalues_version_)
        log_fatal("Attempting to read version %u from file but running version "
                  "%u of I3HitMultiplicityValues class.",
                  version, i3hitmultiplicityvalues_version_
        );

    ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
    ar & make_nvp("NHitStrings",      this->nHitStrings_);
    ar & make_nvp("NHitDoms",         this->nHitDoms_);
    ar & make_nvp("NHitDomsOnePulse", this->nHitDomsOnePulse_);
    ar & make_nvp("NPulses",          this->nPulses_);
}

I3_SERIALIZABLE(I3HitMultiplicityValues);

//______________________________________________________________________________
std::ostream&
I3HitMultiplicityValues::Print(std::ostream& oss) const
{
    oss << "I3HitMultiplicityValues(" <<
               "NHitStrings: "      << GetNHitStrings()      << ", " <<
               "NHitDoms: "         << GetNHitDoms()         << ", " <<
               "NHitDomsOnePulse: " << GetNHitDomsOnePulse() << ", " <<
               "NPulses: "          << GetNPulses()          <<
           ")";
    return oss;
}

//______________________________________________________________________________
std::ostream&
operator<<(std::ostream& oss, const I3HitMultiplicityValues& rhs)
{
  return(rhs.Print(oss));
}
