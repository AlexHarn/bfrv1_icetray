/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/STTools/utilities.cxx
 * @date $Date$
 * @brief This file contains the implementation of utility functions within the
 *        sttools namespace useful for user defined ST algorithms.
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
#include <string>
#include <sstream>

#include <boost/make_shared.hpp>

#include "icetray/I3Logging.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Units.h"

#include "STTools/utilities.h"

using namespace std;

//##############################################################################
namespace sttools {

//______________________________________________________________________________
// Define the STTools logger.
// SET_LOGGER("STTools");

//______________________________________________________________________________
// Specialized function to check if an I3RecoPulseSeriesMapMask output frame
// object is empty.
template <>
bool
isOutputFrameObjectEmpty(const I3RecoPulseSeriesMapMask &output)
{
    return (output.GetSum() == 0);
}

//______________________________________________________________________________
// Specialized function to create an output I3FrameObject of type
// I3RecoPulseSeriesMapMask.
template <>
I3RecoPulseSeriesMapMaskPtr
createOutputFrameObject(
    const I3Frame &frame,
    const string  &inputHitSeriesMapName
)
{
    I3RecoPulseSeriesMapMaskPtr mask =
        boost::make_shared<I3RecoPulseSeriesMapMask>(frame, inputHitSeriesMapName);

    mask->SetNone();

    return mask;
}

//______________________________________________________________________________
// Specialized function to add a hit to an I3RecoPulseSeriesMapMask output
// frame object.
template <>
void
addHitToOutputFrameObject(
    I3RecoPulseSeriesMapMask &output,
    const OMKey              &omkey,
    const I3RecoPulse        &hit
)
{
    output.Set(omkey, hit, true);
}

//______________________________________________________________________________
OMKey
getOMKeyClosestToPosition(I3OMGeoMap const & omgeomap, I3Position const & pos)
{
    I3OMGeoMap::const_iterator it = omgeomap.begin();
    I3OMGeoMap::const_iterator const it_end = omgeomap.end();
    OMKey closestOM;
    double closestDist = std::numeric_limits<double>::max();
    for(; it!=it_end; ++it)
    {
        OMKey const & omkey = it->first;
        I3Position const & ompos = it->second.position;
        I3Position const posdiff = pos - ompos;
        double const dist = (posdiff)*(posdiff);
        if(dist < closestDist)
        {
            closestDist = dist;
            closestOM = omkey;
        }
    }
    return closestOM;
}

}/*sttools*/
//##############################################################################
