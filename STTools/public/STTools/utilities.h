/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/utilities.h
 * @date $Date$
 * @brief This file contains the definitions of the utility functions within the
 *        sttools namespace useful for ST user defined algorithms.
 *
 *        The functions are C++ templated via the template parameters
 *        ``HitType`` and ``OutType``, so they can be used both with
 *        I3DOMLaunchSeriesMaps and I3RecoPulseSeriesMaps as input data.
 *
 *        @note This file should not be included directly into the user's
 *              application. Instead, the user should include the api.h header
 *              file.
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
#ifndef STTOOLS_UTILITIES_H_INCLUDED
#define STTOOLS_UTILITIES_H_INCLUDED

#include <sstream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "icetray/OMKey.h"
#include "icetray/I3Frame.h"
#include "icetray/I3PhysicsTimer.h"

#include "dataclasses/I3Map.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/geometry/I3OMGeo.h"

#include "STTools/OMKeyPairMap.h"
#include "STTools/I3FourVector.h"
#include "STTools/I3STHitInfo.h"

//##############################################################################
namespace sttools {

//______________________________________________________________________________
/** Creates a new empty shared std::vector< STHitInfoType< HitType > > object
 *  for storing ST hit information about hits of type HitType.
 *  @returns A BOOST shared pointer to the newly created ST hit information
 *      series.
 */
template<
    class HitType,
    template <class HitType_> class STHitInfoType
>
boost::shared_ptr< std::vector< STHitInfoType<HitType> > >
createSTHitInfoSeries()
{
    return boost::make_shared< std::vector< STHitInfoType<HitType> > >();
}


//______________________________________________________________________________
/** Function object to determine if a hit is a hit, i.e. returns always
 *  ``true``.
 */
template <class HitType>
struct isAnyHit
{
    bool
    operator()(const I3STHitInfo<HitType>& stHitInfo)
    {
        return true;
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is a HLC hit.
 */
template <class HitType>
struct isHLCHit
{
    bool
    operator()(const I3STHitInfo<HitType>& stHitInfo)
    {
        return stHitInfo.IsHLCHit();
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is a SLC hit.
 */
template <class HitType>
struct isSLCHit
{
    bool
    operator()(const I3STHitInfo<HitType>& stHitInfo)
    {
        return stHitInfo.IsSLCHit();
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is the first hit within the OM's hit
 *  series.
 */
template <class HitType>
struct isFirstOMHit
{
    bool
    operator()(const I3STHitInfo<HitType>& stHitInfo)
    {
        return stHitInfo.IsFirstOMHit();
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is the N-th hit within the OM's hit
 *  series.
 *
 *  @note: The first hit has the number (nth parameter) ``1``!
 */
template <class HitType>
struct isNthOMHit
{
    uint32_t idx_;

    //__________________________________________________________________________
    isNthOMHit(uint32_t nth)
      : idx_(nth-1)
    {}

    //__________________________________________________________________________
    bool
    operator()(const I3STHitInfo<HitType>& stHitInfo)
    {
        return (stHitInfo.GetHitIdxWithinOMHitSeries() == idx_);
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is part of an OM present in a given
 *  OMKey list.
 */
template <class HitType>
struct isOMKeyHit
{
    std::vector<OMKey> omKeys_;

    //__________________________________________________________________________
    isOMKeyHit(const std::vector<OMKey>& omKeys)
      : omKeys_(omKeys)
    {}

    //__________________________________________________________________________
    bool
    operator()(const I3STHitInfo<HitType>& stHitInfo)
    {
        const OMKey& omKey = stHitInfo.GetOMKey();
        if(std::find(omKeys_.begin(), omKeys_.end(), omKey) != omKeys_.end()) {
            return true;
        }
        return false;
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is the n-th hit of an OM that is
 *  present in a given OMKey list.
 *
 *  @note: The first hit has the number (nth parameter) ``1``!
 */
template <class HitType>
struct isNthOMKeyHit
{
    isNthOMHit<HitType> isNthOMHit_;
    isOMKeyHit<HitType> isOMKeyHit_;

    //__________________________________________________________________________
    isNthOMKeyHit(
        uint32_t nth,
        const std::vector<OMKey>& omKeys
    )
      : isNthOMHit_(isNthOMHit<HitType>(nth)),
        isOMKeyHit_(isOMKeyHit<HitType>(omKeys))
    {}

    //__________________________________________________________________________
    bool
    operator()(const I3STHitInfo<HitType>& stHitInfo)
    {
        return (isNthOMHit_(stHitInfo) && isOMKeyHit_(stHitInfo));
    }
};

//______________________________________________________________________________
/** Calculates the Center-of-Gravity of the given hits for which the given
 *  hit select function returns ``true``.
 */
template <
    class HitType,
    template<class HitType_> class STHitInfoType
>
I3FourVector
calcHitsCOGFourPosition(
    const std::vector< STHitInfoType<HitType> >&                    stHitInfoSeries,
    boost::function<bool (const STHitInfoType<HitType> &rtHitInfo)> hitSelectFctn = isAnyHit<HitType>()
)
{
    I3FourVector cog(0,0,0,0);

    uint32_t nSelectedHits = 0;
    typename std::vector< STHitInfoType<HitType> >::const_iterator stHitCIter;
    for(stHitCIter = stHitInfoSeries.begin(); stHitCIter != stHitInfoSeries.end(); ++stHitCIter)
    {
        const STHitInfoType<HitType>& stHit = *stHitCIter;
        if(hitSelectFctn(stHit))
        {
            cog += I3FourVector(stHit.GetHitTime(),
                                stHit.GetOMPosition().GetX(),
                                stHit.GetOMPosition().GetY(),
                                stHit.GetOMPosition().GetZ());
            ++nSelectedHits;
        }
    }
    if(nSelectedHits > 0) {
        cog /= nSelectedHits;
    }
    else {
        cog.SetToNAN();
    }

    log_debug(
        "Considered %u hits for calculating the four-position of their CoG ("
        "%s).",
        nSelectedHits, cog.str().c_str());

    return cog;
}

//______________________________________________________________________________
/** Gets a shared pointer to the given named constant hit series map inside the
 *  given I3Frame object.
 *
 *  @throws RuntimeException If the given hit series map does not exists in the
 *      frame, or could not be loaded.
 */
template <class HitType>
inline
boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > >
getHitSeriesMap(
    const I3Frame     &frame,
    const std::string &hitSeriesMapName
)
{
    boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > hitSeriesMap =
        frame.template Get<boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > >(hitSeriesMapName);
    if(! hitSeriesMap)
    {
        log_fatal(
            "The hit series map named \"%s\" could not be loaded correctly! "
            "The frame returned a null-pointer. Does it actually exists within "
            "the frame?",
            hitSeriesMapName.c_str());
    }
    return hitSeriesMap;
}

//______________________________________________________________________________
/** Function to create an output I3FrameObject of type OutType.
 *  The created output frame object will be empty.
 *
 * @returns A BOOST shared pointer pointing to the created frame object.
 */
template <class OutType>
boost::shared_ptr<OutType>
createOutputFrameObject(
    const I3Frame     &frame,
    const std::string &inputHitSeriesMapName
)
{
    return boost::make_shared<OutType>();
}

//______________________________________________________________________________
// Specialized function to create an output I3FrameObject of type
// I3RecoPulseSeriesMapMask.
template <>
I3RecoPulseSeriesMapMaskPtr
createOutputFrameObject(
    const I3Frame     &frame,
    const std::string &inputHitSeriesMapName
);

//______________________________________________________________________________
/** Function to check if an output frame object is empty.
 *
 *  @param output The output frame object.
 *
 *  @returns The boolean value ``true`` if the output frame object is empty,
 *      ``false`` otherwise.
 */
template <class OutType>
bool
isOutputFrameObjectEmpty(const OutType &output)
{
    return output.empty();
}

// Specialized version for I3RecoPulseSeriesMapMask objects.
template <>
bool
isOutputFrameObjectEmpty<I3RecoPulseSeriesMapMask>(const I3RecoPulseSeriesMapMask & output);

//______________________________________________________________________________
/** Function to add a hit to the output frame object.
 */
template <
    class HitType,
    class OutType
>
inline
void
addHitToOutputFrameObject(
    OutType       &output,
    const OMKey   &omkey,
    const HitType &hit
)
{
    output[omkey].push_back(hit);
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
);

//______________________________________________________________________________
// Determines the OMKey that is located closest to the given position in the
// detector.
OMKey
getOMKeyClosestToPosition(I3OMGeoMap const & omgeomap, I3Position const & pos);

}/*sttools*/
//##############################################################################

#endif//STTOOLS_UTILITIES_H_INCLUDED
