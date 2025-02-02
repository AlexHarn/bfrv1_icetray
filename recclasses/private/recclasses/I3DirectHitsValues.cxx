/**
 * $Id$
 *
 * Copyright (C) 2012
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @file I3DirectHitsValues.cxx
 * @version $Revision$
 * @date $Date$
 * @author Martin Wolf <martin.wolf@icecube.wisc.edu>
 * @brief This file contains the implementation of the I3DirectHitsValues class,
 *        which is an I3FrameObject holding the values for a particular class of
 *        direct hits.
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
#include "icetray/I3Units.h"

#include "recclasses/I3DirectHitsValues.h"

//______________________________________________________________________________
template <class Archive>
void
I3DirectHitsValues::
serialize(Archive& ar, unsigned version)
{
    if(version > i3directhitsvalues_version_)
        log_fatal("Attempting to read version %u from file but running version "
                  "%u of I3DirectHitsValues class.",
                  version, i3directhitsvalues_version_
        );

    ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
    ar & make_nvp("NDirStrings",                       this->nDirStrings_);
    ar & make_nvp("NDirDoms",                          this->nDirDoms_);
    ar & make_nvp("NDirPulses",                        this->nDirPulses_);
    ar & make_nvp("QDirPulses",                        this->qDirPulses_);
    ar & make_nvp("NEarlyStrings",                     this->nEarlyStrings_);
    ar & make_nvp("NEarlyDoms",                        this->nEarlyDoms_);
    ar & make_nvp("NEarlyPulses",                      this->nEarlyPulses_);
    ar & make_nvp("QEarlyPulses",                      this->qEarlyPulses_);
    ar & make_nvp("NLateStrings",                      this->nLateStrings_);
    ar & make_nvp("NLateDoms",                         this->nLateDoms_);
    ar & make_nvp("NLatePulses",                       this->nLatePulses_);
    ar & make_nvp("QLatePulses",                       this->qLatePulses_);
    ar & make_nvp("DirTrackLength",                    this->dirTrackLength_);
    ar & make_nvp("DirTrackHitDistributionSmoothness", this->dirTrackHitDistributionSmoothness_);
}

I3_SERIALIZABLE(I3DirectHitsValues);

//______________________________________________________________________________
std::ostream&
operator<<(std::ostream& oss, const I3DirectHitsValues& rhs)
{
  return(rhs.Print(oss));
}

std::ostream&
I3DirectHitsValues::Print(std::ostream& oss) const
  {
    oss << "I3DirectHitsValues("                                                                    <<
        "NDirStrings: "                       << GetNDirStrings()                       << ", " <<
        "NDirDoms: "                          << GetNDirDoms()                          << ", " <<
        "NDirPulses: "                        << GetNDirPulses()                        << ", " <<
        "QDirPulses [PE]: "                   << GetQDirPulses()                        << ", " <<
        "NEarlyStrings: "                     << GetNEarlyStrings()                     << ", " <<
        "NEarlyDoms: "                        << GetNEarlyDoms()                        << ", " <<
        "NEarlyPulses: "                      << GetNEarlyPulses()                      << ", " <<
        "QEarlyPulses [PE]: "                 << GetQEarlyPulses()                      << ", " <<
        "NLateStrings: "                      << GetNLateStrings()                      << ", " <<
        "NLateDoms: "                         << GetNLateDoms()                         << ", " <<
        "NLatePulses: "                       << GetNLatePulses()                       << ", " <<
        "QLatePulses [PE]: "                  << GetQLatePulses()                       << ", " <<
        "DirTrackLength [m]: "                << GetDirTrackLength()/I3Units::m         << ", " <<
        "DirTrackHitDistributionSmoothness: " << GetDirTrackHitDistributionSmoothness() <<
        ")";
    return oss;
}
