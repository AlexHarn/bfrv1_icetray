/**
 * $Id$
 *
 * Copyright (C) 2012
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @file I3TrackCharacteristicsValues.cxx
 * @version $Revision$
 * @date $Date$
 * @author Martin Wolf <martin.wolf@icecube.wisc.edu>
 * @brief This file contains the implementation of the
 *        I3TrackCharacteristicsValues class, which is an I3FrameObject holding
 *        the result values of a track characteristics calculation.
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
#include "icetray/serialization.h"
#include "icetray/I3Units.h"

#include "recclasses/I3TrackCharacteristicsValues.h"

//______________________________________________________________________________
template <class Archive>
void
I3TrackCharacteristicsValues::
serialize(Archive& ar, unsigned version)
{
    if(version > i3trackcharacteristicsvalues_version_)
        log_fatal("Attempting to read version %u from file but running version "
                  "%u of I3TrackCharacteristicsValues class.",
                  version, i3trackcharacteristicsvalues_version_
        );

    ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
    ar & make_nvp("AvgDomDistQTotDom",               this->avgDomDistQTotDom_);
    ar & make_nvp("EmptyHitsTrackLength",            this->emptyHitsTrackLength_);
    ar & make_nvp("TrackHitsSeparationLength",       this->trackHitsSeparationLength_);
    ar & make_nvp("TrackHitsDistributionSmoothness", this->trackHitsDistributionSmoothness_);
}

I3_SERIALIZABLE(I3TrackCharacteristicsValues);

//______________________________________________________________________________
std::ostream&
operator<<(std::ostream& os, const I3TrackCharacteristicsValues& rhs)
{
  return(rhs.Print(os));
}

//______________________________________________________________________________
std::ostream& I3TrackCharacteristicsValues::Print(std::ostream& oss) const
{
    oss << "I3TrackCharacteristicsValues("                                                           <<
        "AvgDomDistQTotDom [m]: "           << GetAvgDomDistQTotDom()/I3Units::m         << ", " <<
        "EmptyHitsTrackLength [m]: "        << GetEmptyHitsTrackLength()/I3Units::m      << ", " <<
        "TrackHitsSeparationLength [m]: "   << GetTrackHitsSeparationLength()/I3Units::m << ", " <<
        "TrackHitsDistributionSmoothness: " << GetTrackHitsDistributionSmoothness()      <<
        ")";
    return oss;
}
