/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/STTools/algorithms/seededRT/I3SeededRTConfiguration.cxx
 * @date $Date$
 * @brief This file contains the implementation of the I3SeededRTConfiguration
 *        class.
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
#include <sstream>

#include "icetray/serialization.h"

#include "STTools/utilities.h"
#include "STTools/algorithms/seededRT/I3SeededRTConfiguration.h"

//______________________________________________________________________________
I3SeededRTConfiguration::
~I3SeededRTConfiguration()
{}

//______________________________________________________________________________
template <class Archive>
void
I3SeededRTConfiguration::
serialize(Archive &ar, unsigned version)
{
    if(version > i3seededrtconfiguration_version_) {
        log_fatal("Attempting to read version %u from file but running version "
                  "%u of I3SeededRTConfiguration class.",
                  version, i3seededrtconfiguration_version_
        );
    }

    ar & make_nvp("I3STConfiguration", base_object<I3STConfiguration>(*this));
    ar & make_nvp("RTCoordSys", rtCoordSys_);
    ar & make_nvp("RTTime",     rtTime_);
    ar & make_nvp("RTRadius",   rtRadius_);
    ar & make_nvp("RTHeight",   rtHeight_);
}

I3_SERIALIZABLE(I3SeededRTConfiguration);
I3_SERIALIZABLE(I3VectorSeededRTConfiguration);

//______________________________________________________________________________
std::string
I3SeededRTConfiguration::
GetPrettySettingsStr(unsigned int nLeadingWS) const
{
    std::string ws(nLeadingWS, ' ');

    std::stringstream ss;
    ss << ws << "I3STConfiguration:"                            << std::endl <<
          I3STConfiguration::GetPrettySettingsStr(nLeadingWS+4) << std::endl <<
          ws << "RTCoordSys:   " << GetRTCoordSys()             << std::endl <<
          ws << "RTTime [ns]:  " << GetRTTime()/I3Units::ns     << std::endl <<
          ws << "RTRadius [m]: " << GetRTRadius()/I3Units::m    << std::endl <<
          ws << "RTHeight [m]: " << GetRTHeight()/I3Units::m
          ;
    return ss.str();
}

//______________________________________________________________________________
std::ostream&
operator<<(std::ostream& os, const I3SeededRTConfiguration &rhs)
{
    os << "I3SeededRTConfiguration:" << std::endl
       << rhs.GetPrettySettingsStr(4);
    return os;
}
