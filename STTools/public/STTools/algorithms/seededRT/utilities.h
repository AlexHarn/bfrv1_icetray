/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/algorithms/seededRT/utilities.h
 * @date $Date$
 * @brief This file contains the definitions of the utility functions for the
 *        seededRT algorithm within the sttools::seededRT namespace.
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
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_UTILITIES_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_UTILITIES_H_INCLUDED

#include <algorithm>
#include <cmath>
#include <stdint.h>
#include <vector>

#include <boost/implicit_cast.hpp>
#include <boost/function.hpp>

#include "icetray/OMKey.h"
#include "icetray/I3PointerTypedefs.h"
#include "icetray/I3Logging.h"

#include "dataclasses/ostream_overloads.hpp"
#include "dataclasses/I3Constants.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3Vector.h"
#include "dataclasses/geometry/I3OMGeo.h"

#include "STTools/I3FourVector.h"
#include "STTools/utilities.h"

#include "STTools/algorithms/seededRT/I3SeededRTSDataMap.h"
#include "STTools/algorithms/seededRT/I3SeededRTConfigurationService.h"
#include "STTools/algorithms/seededRT/I3SeededRTSTHitInfo.h"

#include "STTools/cpp/streamToString.hpp"

namespace sttools {
namespace seededRT {

inline
bool
equivalent(const OMKey &omKey1, const OMKey &omKey2)
{
    return (omKey1.GetString() == omKey2.GetString()) &&
        (omKey1.GetOM() == omKey2.GetOM());
}

//______________________________________________________________________________
/** Fills a given seededRT ST hit info series with all hits from a given hit
 *  series map, based on a given ST configuration defined via the given
 *  I3VectorSTConfiguration object.
 */
template <class HitType>
void
fillSeededRTSTHitInfoSeries(
    const I3SeededRTConfigurationService        &stConfigService,
    const I3Map< OMKey, std::vector<HitType> >  &inputHitSeriesMap,
    std::vector< I3SeededRTSTHitInfo<HitType> > &stHitInfoSeries
)
{
    const I3OMGeoMap& omGeoMap = stConfigService.GetOMGeoMap();

    typename I3Map< OMKey, std::vector<HitType> >::const_iterator inputHitSeriesMapCIter;
    typename std::vector< HitType >::const_iterator inputHitSeriesCIter;

    for(inputHitSeriesMapCIter = inputHitSeriesMap.begin();
        inputHitSeriesMapCIter != inputHitSeriesMap.end();
        ++inputHitSeriesMapCIter
       )
    {
        const OMKey &omKey = inputHitSeriesMapCIter->first;
        const std::vector< HitType > &inputHitSeries = inputHitSeriesMapCIter->second;

        I3OMGeoMap::const_iterator omGeoMapCIter = omGeoMap.find(omKey);
        if(omGeoMapCIter == omGeoMap.end()) {
            log_warn(
                "The %s was not found in the geometry! "
                "Skipping this hit OM ...",
                omKey.str().c_str());
            continue;
        }
        const I3Position& omPos = omGeoMapCIter->second.position;

        uint32_t idx = 0;
        for(inputHitSeriesCIter = inputHitSeries.begin();
            inputHitSeriesCIter != inputHitSeries.end();
            ++inputHitSeriesCIter, ++idx
           )
        {
            const HitType &hit = *inputHitSeriesCIter;

            if(std::isnan(getHitTime(hit)))
            {
                log_warn(
                    "Got a NaN time from a hit on OM %s! "
                    "Ignoring this hit!",
                    omKey.str().c_str());
                continue;
            }

            stHitInfoSeries.push_back(I3SeededRTSTHitInfo<HitType>(
                omKey, &omPos, &hit, idx
            ));
        }
    }

    // Sort the hits in hit time ascending.
    std::sort(stHitInfoSeries.begin(), stHitInfoSeries.end(), I3SeededRTSTHitInfo<HitType>::compareHitTimeAsc);
}

//______________________________________________________________________________
/** Counts the number of selected ST hits inside a given ST hit info series.
 */
template <class HitType>
uint32_t
countSelectedSTHits(std::vector< I3SeededRTSTHitInfo<HitType> > const & stHitInfoSeries)
{
    uint32_t nTotSelectedHits = 0;
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::const_iterator stHitIter = stHitInfoSeries.begin();
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::const_iterator stHitInfoSeries_end = stHitInfoSeries.end();
    for(; stHitIter != stHitInfoSeries_end; ++stHitIter)
    {
        if(stHitIter->IsHitSelected()) {
            nTotSelectedHits++;
        }
    }
    return nTotSelectedHits;
}

//______________________________________________________________________________
/** Selects hits for which the specified hit select function returns ``true``.
 *
 *  @param stHitInfoSeries The std::vector< I3SeededRTSTHitInfo< HitType > >
 *  @param hitSelectFctn The address of the hit select function that checks if
 *      the hit given by I3SeededRTSTHitInfo< HitType > object should get
 *      selected as seed (returns ``true``) or not (returns ``false``).
 *
 *  @returns The number of selected seed hits.
 */
template <class HitType>
uint32_t
selectHits(
    std::vector< I3SeededRTSTHitInfo<HitType> >&                stHitInfoSeries,
    boost::function<bool (const I3SeededRTSTHitInfo<HitType>&)> hitSelectFctn
)
{
    uint32_t nSelectedSeedHits = 0;
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitIter;
    for(stHitIter = stHitInfoSeries.begin(); stHitIter != stHitInfoSeries.end(); ++stHitIter)
    {
        I3SeededRTSTHitInfo<HitType>& stHit = *stHitIter;
        if(hitSelectFctn(stHit))
        {
            stHit.SetIsHitSelected(true);
            ++nSelectedSeedHits;
        }
    }

    return nSelectedSeedHits;
}

//______________________________________________________________________________
/** Checks if the two given times are in temporal causal connection to each
 *  other by using the given RT time and dustlayer correction length.
 *
 *  @note For performance reasons, we define this function as an inline
 *        function.
 *
 *  @returns ``true`` if the two times are temporally close enough, and
 *      ``false`` otherwise.
 */
inline
bool
areTimesInTemporalCausalConnection(
    const double time1,
    const double time2,
    const double rtTime,
    const double dustlayerCorrectionLength=0
)
{
    return (std::fabs(time2 - time1) < (rtTime + dustlayerCorrectionLength/I3Constants::c_ice));
}

//______________________________________________________________________________
/** Determines the selected RT hits of each selected RT hit and deselects those
 *  selected RT hits which have less RT hits than the given threshold.
 *
 *  @note The given ST hit info series must be ascended time ordered in hit
 *        time!
 *
 *  @returns The number of total selected hits.
 */
template <class HitType>
uint32_t
deselectRTHitsBelowRTPartnerHitsThreshold(
    const I3SeededRTConfigurationService&        stConfigService,
    std::vector< I3SeededRTSTHitInfo<HitType> >& stHitInfoSeries,
    uint32_t                                     threshold
)
{
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitIter1;
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitIter2;
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitInfoSeries_begin(stHitInfoSeries.begin());
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitInfoSeries_end(stHitInfoSeries.end());

    const I3SeededRTSContext &sContext = stConfigService.GetSContext();
    const I3SeededRTSDataMap &sDataMap = *sContext.GetSDataMap();
    const OMKeyHasher &omKeyHasher = sDataMap.GetOMKeyHasher();

    // Get the maximal possible RTTime from the ST configuration and add a
    // margin accounting for the maximal possible dustlayer correction length.
    // This max_dt variable is used to define a maximal RT hit search time
    // window just for optimal performance reasons.
    const double max_dt = stConfigService.GetMaxRTTime() +
                          sContext.GetMaxDustlayerCorrectionLength()/I3Constants::c_ice;

    const bool allowSelfCoincidence = stConfigService.GetAllowSelfCoincidence();

    // Go through the hits and determine for each selected hit the hits that
    // fulfill the seededRT conditions with this selected hit.
    for(stHitIter1 = stHitInfoSeries_begin; stHitIter1 != stHitInfoSeries_end; ++stHitIter1)
    {
        // Skip all unselected hits.
        if(! stHitIter1->IsHitSelected()) {
            continue;
        }

        const OMKey &srcOMKey = stHitIter1->GetOMKey();

        std::set<uint32_t>& stPartnerOMHashes = stHitIter1->GetSTPartnerOMHashes();

        // Look backward in time w.r.t. the time of the current hit.
        // Remember: stHitInfoSeries is ascended hit time sorted.
        if(stHitIter1 > stHitInfoSeries_begin)
        {
            stHitIter2 = stHitIter1 - 1;
            const double minTime = stHitIter1->GetHitTime() - max_dt;
            while((stHitIter2 >= stHitInfoSeries_begin) && (stHitIter2->GetHitTime() >= minTime))
            {
                if(stHitIter2->IsHitSelected())
                {
                    const OMKey &dstOMKey = stHitIter2->GetOMKey();

                    // Don't consider hits of the same OM if self coincidence
                    // is disabled.
                    if(!allowSelfCoincidence && equivalent(srcOMKey, dstOMKey)) {
                        --stHitIter2;
                        continue;
                    }

                    const I3SeededRTSData &sData = stConfigService.GetSDataForOMLink(srcOMKey, dstOMKey);
                    if(sData.stConfig_ != NULL)
                    {
                        // By definition, the hits are in spacial connection,
                        // when there is a ST configuration set for this OM
                        // link.
                        const double rtTime = sData.stConfig_->GetRTTime();

                        if(areTimesInTemporalCausalConnection(
                                stHitIter2->GetHitTime(), stHitIter1->GetHitTime(),
                                rtTime, sData.dustlayerCorrectionLength_)
                          )
                        {
                            stPartnerOMHashes.insert(omKeyHasher.HashOMKey(dstOMKey));
                            if(stPartnerOMHashes.size() >= threshold) {
                                break;
                            }
                        }
                    }
                }
                --stHitIter2;
            }
        }

        if(stPartnerOMHashes.size() >= threshold) {
            continue;
        }

        // Look forward in time w.r.t. the time of the current hit.
        // Remember: stHitInfoSeries is ascended hit time sorted.
        if(stHitIter1 != stHitInfoSeries_end)
        {
            stHitIter2 = stHitIter1 + 1;
            const double maxTime = stHitIter1->GetHitTime() + max_dt;
            while((stHitIter2 != stHitInfoSeries_end) && (stHitIter2->GetHitTime() <= maxTime))
            {
                if(stHitIter2->IsHitSelected())
                {
                    const OMKey &dstOMKey = stHitIter2->GetOMKey();

                    // Don't consider hits of the same OM if self coincidence
                    // is disabled.
                    if(!allowSelfCoincidence
                        && equivalent(srcOMKey, dstOMKey)) {
                        ++stHitIter2;
                        continue;
                    }

                    const I3SeededRTSData &sData = stConfigService.GetSDataForOMLink(srcOMKey, dstOMKey);
                    if(sData.stConfig_ != NULL)
                    {
                        const double rtTime = sData.stConfig_->GetRTTime();

                        if(areTimesInTemporalCausalConnection(
                            stHitIter1->GetHitTime(), stHitIter2->GetHitTime(),
                            rtTime, sData.dustlayerCorrectionLength_)
                        )
                        {
                            stPartnerOMHashes.insert(omKeyHasher.HashOMKey(dstOMKey));
                            if(stPartnerOMHashes.size() >= threshold) {
                                break;
                            }
                        }
                    }
                }
                ++stHitIter2;
            }
        }
    }

    // Count the total number of selected hits after this procedure.
    uint32_t nTotSelectedHits = 0;
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitIter;
    for(stHitIter = stHitInfoSeries_begin; stHitIter != stHitInfoSeries_end; ++stHitIter)
    {
        if(stHitIter->IsHitSelected())
        {
            if(stHitIter->GetSTPartnerOMHashes().size() < threshold) {
                stHitIter->SetIsHitSelected(false);
            }
            else {
                ++nTotSelectedHits;
            }
        }
    }

    return nTotSelectedHits;
}

//______________________________________________________________________________
/** Selects new hits which fulfill the seededRT conditions for already selected
 *  hits. In order to select all hits, that fulfill the seededRT conditions,
 *  this function has to be called several times, until it does not select more
 *  hits.
 *
 *  @note The given ST hit info series must be ascended time ordered in hit
 *        time!
 *
 *  @returns The number of total (old+new) selected hits.
 */
template <class HitType>
uint32_t
doSelectRTHitsIteration(
    const I3SeededRTConfigurationService        &stConfigService,
    std::vector< I3SeededRTSTHitInfo<HitType> > &stHitInfoSeries
)
{
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitIter1;
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitIter2;
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitInfoSeries_begin(stHitInfoSeries.begin());
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitInfoSeries_end(stHitInfoSeries.end());

    const I3SeededRTSContext &sContext = stConfigService.GetSContext();

    // Get the maximal possible RTTime from the ST configuration and add a
    // margin accounting for the maximal possible dustlayer correction length.
    // This max_dt variable is used to define a maximal RT hit search time
    // window just for optimal performance reasons.
    const double max_dt = stConfigService.GetMaxRTTime() +
                          sContext.GetMaxDustlayerCorrectionLength()/I3Constants::c_ice;

    const bool allowSelfCoincidence = stConfigService.GetAllowSelfCoincidence();

    // Go through the hits and check if a non-selected hit fulfills the
    // seededRT conditions with an already selected hit.
    for(stHitIter1 = stHitInfoSeries_begin; stHitIter1 != stHitInfoSeries_end; ++stHitIter1)
    {
        // Do not newly select already selected hits.
        if(stHitIter1->IsHitSelected()) {
            continue;
        }

        const OMKey &dstOMKey = stHitIter1->GetOMKey();

        // Look backward in time w.r.t. the time of the current hit.
        // Remember: stHitInfoSeries is ascended hit time sorted.
        if(stHitIter1 > stHitInfoSeries_begin)
        {
            stHitIter2 = stHitIter1 - 1;
            const double minTime = stHitIter1->GetHitTime() - max_dt;
            while((stHitIter2 >= stHitInfoSeries_begin) && (stHitIter2->GetHitTime() >= minTime))
            {
                if(stHitIter2->IsHitSelected())
                {
                    const OMKey &srcOMKey = stHitIter2->GetOMKey();

                    // Don't consider hits of the same OM if self coincidence
                    // is disabled.
                    if(!allowSelfCoincidence && equivalent(srcOMKey, dstOMKey)) {
                        --stHitIter2;
                        continue;
                    }

                    log_trace("Getting SData for OM link %s-%s.",
                        srcOMKey.str().c_str(), dstOMKey.str().c_str());
                    const I3SeededRTSData &sData = stConfigService.GetSDataForOMLink(srcOMKey, dstOMKey);
                    if(sData.stConfig_ != NULL)
                    {
                        // By definition, the hits are in spacial connection,
                        // when there is a ST configuration set for this OM
                        // link.
                        const double rtTime = sData.stConfig_->GetRTTime();

                        if(areTimesInTemporalCausalConnection(
                                stHitIter2->GetHitTime(), stHitIter1->GetHitTime(),
                                rtTime, sData.dustlayerCorrectionLength_)
                          )
                        {
                            stHitIter1->SetIsHitNewlySelected(true);
                            break;
                        }
                    }
                    else
                    {
                        log_trace("No SData for OM link %s-%s available.",
                            srcOMKey.str().c_str(), dstOMKey.str().c_str()
                        );
                    }
                }
                --stHitIter2;
            }
        }

        if(stHitIter1->IsHitNewlySelected()) {
            continue;
        }

        // Look forward in time w.r.t. the time of the current hit.
        // Remember: stHitInfoSeries is ascended hit time sorted.
        if(stHitIter1 != stHitInfoSeries_end)
        {
            stHitIter2 = stHitIter1 + 1;
            const double maxTime = stHitIter1->GetHitTime() + max_dt;
            while((stHitIter2 != stHitInfoSeries_end) && (stHitIter2->GetHitTime() <= maxTime))
            {
                if(stHitIter2->IsHitSelected())
                {
                    const OMKey &srcOMKey = stHitIter2->GetOMKey();

                    // Don't consider hits of the same OM if self coincidence
                    // is disabled.
                    if(!allowSelfCoincidence && equivalent(srcOMKey, dstOMKey)) {
                        ++stHitIter2;
                        continue;
                    }

                    log_trace("Getting SData for OM link %s-%s.",
                        srcOMKey.str().c_str(), dstOMKey.str().c_str());
                    const I3SeededRTSData &sData = stConfigService.GetSDataForOMLink(srcOMKey, dstOMKey);
                    if(sData.stConfig_ != NULL)
                    {
                        // By definition, the hits are in spacial connection,
                        // when there is a ST configuration set for this OM
                        // link.
                        const double rtTime = sData.stConfig_->GetRTTime();

                        if(areTimesInTemporalCausalConnection(
                                stHitIter1->GetHitTime(), stHitIter2->GetHitTime(),
                                rtTime, sData.dustlayerCorrectionLength_)
                          )
                        {
                            stHitIter1->SetIsHitNewlySelected(true);
                            break;
                        }
                    }
                    else
                    {
                        log_trace("No SData for OM link %s-%s available.",
                            srcOMKey.str().c_str(), dstOMKey.str().c_str()
                        );
                    }
                }
                ++stHitIter2;
            }
        }
    }

    // Count the total number of selected hits after this RT hit selection
    // iteration.
    uint32_t nTotSelectedHits = 0;
    typename std::vector< I3SeededRTSTHitInfo<HitType> >::iterator stHitIter;
    for(stHitIter = stHitInfoSeries_begin; stHitIter != stHitInfoSeries_end; ++stHitIter)
    {
        if(stHitIter->IsHitSelected()) {
            nTotSelectedHits++;
        }
        else if(stHitIter->IsHitNewlySelected())
        {
            stHitIter->SetIsHitNewlySelected(false);
            stHitIter->SetIsHitSelected(true);
            nTotSelectedHits++;
        }
    }

    return nTotSelectedHits;
}

//______________________________________________________________________________
/** Does the iterative seededRT procedure of adding RT hits to the selections
 *  of already selected hits. This function makes use of the
 *  ``doSelectRTHitsIteration`` function.
 */
template <class HitType>
uint32_t
doIterativeSelectRTHitsProcedure(
    const I3SeededRTConfigurationService        &stConfigService,
    std::vector< I3SeededRTSTHitInfo<HitType> > &stHitInfoSeries,
    int32_t                                     maxNIterations=-1
)
{
    if(maxNIterations < -1) {
        log_fatal("The number of maximal iterations has been set to %d. "
                  "But it must be >= -1!", maxNIterations);
    }

    if(maxNIterations == 0)
    {
        // No iteration should be done, but we need the number of already
        // selected hits for the return value of this function. So count them.
        return countSelectedSTHits<HitType>(stHitInfoSeries);
    }

    int32_t nIterations = 0;
    uint32_t nTotSelectedHitsOld = 0;
    uint32_t nTotSelectedHitsNew = 0;

    do
    {
        ++nIterations;
        nTotSelectedHitsOld = nTotSelectedHitsNew;
        log_debug("Starting iteration #%d",
            nIterations);
        nTotSelectedHitsNew = doSelectRTHitsIteration(stConfigService, stHitInfoSeries);
        log_debug("NTotSelectedHitsOld: %u, NTotSelectedHitsNew: %u",
            nTotSelectedHitsOld, nTotSelectedHitsNew);
    }
    while((nTotSelectedHitsOld < nTotSelectedHitsNew) &&
          (maxNIterations == -1 || (maxNIterations > 0 && nIterations < maxNIterations))
         );

    return nTotSelectedHitsNew;
}

//______________________________________________________________________________
/** Function object to determine if a hit is selected.
 */
template <class HitType>
struct isSelectedHit
{
    bool operator()(const I3SeededRTSTHitInfo<HitType>& stHitInfo)
    {
        return stHitInfo.IsHitSelected();
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is unselected.
 */
template <class HitType>
struct isUnselectedHit
{
    bool operator()(const I3SeededRTSTHitInfo<HitType>& stHitInfo)
    {
        return (! stHitInfo.IsHitSelected());
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is an unselected HLC hit.
 */
template <class HitType>
struct isUnselectedHLCHit
{
    isUnselectedHit<HitType> isUnselectedHit_;
    isHLCHit<HitType>        isHLCHit_;

    bool operator()(const I3SeededRTSTHitInfo<HitType>& stHitInfo)
    {
        return (isUnselectedHit_(stHitInfo) && isHLCHit_(stHitInfo));
    }
};

//______________________________________________________________________________
/** Function object to determine if a hit is an unselected SLC hit.
 */
template <class HitType>
struct isUnselectedSLCHit
{
    isUnselectedHit<HitType> isUnselectedHit_;
    isSLCHit<HitType>        isSLCHit_;

    bool operator()(const I3SeededRTSTHitInfo<HitType>& stHitInfo)
    {
        return (isUnselectedHit_(stHitInfo) && isSLCHit_(stHitInfo));
    }
};

//______________________________________________________________________________
/** Function to create an output frame object of type OutType, and filling it
 *  with all selected hits from a given I3SeededRTSTHitInfo series.
 */
template <
    class HitType,
    class OutType
>
boost::shared_ptr<OutType>
createAndFillOutputFrameObject(
    const I3Frame&                                              frame,
    const std::string&                                          inputHitSeriesMapName,
    const std::vector< I3SeededRTSTHitInfo<HitType> >&          stHitInfoSeries,
    boost::function<bool (const I3SeededRTSTHitInfo<HitType>&)> hitSelectFctn = isSelectedHit<HitType>()
)
{
    boost::shared_ptr<OutType> output = createOutputFrameObject<OutType>(frame, inputHitSeriesMapName);

    typename std::vector< I3SeededRTSTHitInfo<HitType> >::const_iterator citer;
    for(citer = stHitInfoSeries.begin(); citer != stHitInfoSeries.end(); ++citer)
    {
        const I3SeededRTSTHitInfo<HitType>& stHitInfo = *citer;
        if(hitSelectFctn(stHitInfo))
        {
            sttools::addHitToOutputFrameObject<HitType, OutType>(
                *output, stHitInfo.GetOMKey(), *(stHitInfo.GetHitPtr()));
        }
    }

    return output;
}

//______________________________________________________________________________
/** Checks if the two given positions are in spatial causal connection to each
 *  other by using the given ST configuration service and the given ST
 *  configuration object.
 *
 *  This method needs to calculate the dustlayer correction length for the
 *  distance between the given positions. The value of the calculated dustlayer
 *  correction length can be gathered by specifying a pointer to a double value,
 *  which will be set if the pointer is non-zero and if the two positions are
 *  in spatial range.
 *
 *  @note For performance reasons, we define this function as an inline
 *        function.
 *
 *  @returns ``true`` if the two position are spatially close enough, and
 *      ``false`` otherwise. If ``false`` is returned, the double value of the
 *      dustlayer correction length is not set!
 */
inline
bool
arePositionsInSpatialCausalConnection(
    const I3SeededRTConfigurationService& stConfigService,
    const I3SeededRTConfiguration&        stConfig,
    const I3Position&                     srcPos,
    const I3Position&                     dstPos,
    double*                               dustlayerCorrectionLengthPtr
)
{
    // Calculate the main dustlayer correction. By the definition of the
    // dustlayer correction length, the maximal value of the correction length
    // is the z-coordinate width of the dust layer.
    double dustlayerCorrectionLength = 0;
    if(stConfigService.GetUseDustlayerCorrection())
    {
        const double srcPosZ = srcPos.GetZ();
        const double dstPosZ = dstPos.GetZ();

        const double posUpperZ = std::max(srcPosZ, dstPosZ);
        const double posLowerZ = std::min(srcPosZ, dstPosZ);

        // Calculate a dustlayer correction length if both positions are not
        // above and not below the dustlayer.
        const double dustlayerUpperZBoundary = stConfigService.GetDustlayerUpperZBoundary();
        const double dustlayerLowerZBoundary = stConfigService.GetDustlayerLowerZBoundary();
        if(!(posUpperZ > dustlayerUpperZBoundary &&
             posLowerZ > dustlayerUpperZBoundary
            )
           &&
           !(posUpperZ < dustlayerLowerZBoundary &&
             posLowerZ < dustlayerLowerZBoundary
            )
          )
        {
            dustlayerCorrectionLength =   std::min(posUpperZ, dustlayerUpperZBoundary)
                                        - std::max(posLowerZ, dustlayerLowerZBoundary);
        }
    }
    if(dustlayerCorrectionLength < 0) {
        log_fatal(
            "The calculated dustlayer correction length became negative!");
    }

    // Check if the two DOM are in spatial reach of each other.
    if(stConfig.GetRTCoordSys() == I3SeededRTConfiguration::Sph)
    {
        const double rtRadius = stConfig.GetRTRadius();

        const double xDist = srcPos.GetX() - dstPos.GetX();
        const double yDist = srcPos.GetY() - dstPos.GetY();
        const double zDist = srcPos.GetZ() - dstPos.GetZ();

        const double xyzDist2 = xDist*xDist + yDist*yDist + zDist*zDist;

        const double r = rtRadius + dustlayerCorrectionLength;
        if(xyzDist2 <= r*r)
        {
            // The DOM positions are close enough.
            *dustlayerCorrectionLengthPtr = dustlayerCorrectionLength;
            return true;
        }
    }
    else if(stConfig.GetRTCoordSys() == I3SeededRTConfiguration::Cyl)
    {
        const double rtRadius = stConfig.GetRTRadius();
        const double rtHeight = stConfig.GetRTHeight();

        const double xDist = srcPos.GetX() - dstPos.GetX();
        const double yDist = srcPos.GetY() - dstPos.GetY();
        const double zDist = srcPos.GetZ() - dstPos.GetZ();

        const double xyDist2 = xDist*xDist + yDist*yDist;

        if(xyDist2 <= rtRadius*rtRadius &&
           fabs(zDist) <= rtHeight + dustlayerCorrectionLength)
        {
            // The DOM positions are close enough.
            *dustlayerCorrectionLengthPtr = dustlayerCorrectionLength;
            return true;
        }
    }

    // If none of the above conditions are fulfilled, the two DOMs are just not
    // close enough.
    return false;
}

//______________________________________________________________________________
/** Function object to determine if a hit is in ST connection to a given OM.
 *  This function object can then be used as callback function for the
 *  selectHits function.
 */
template <class HitType>
struct isHitInSTConnectionToOM
{
    const I3SeededRTConfigurationService& stConfigService_;
    const OMKey                           omKey_;
    const I3Position                      omKeyPos_;
    const double                          omKeyHitTime_;

    isHitInSTConnectionToOM(
        const I3SeededRTConfigurationService& stConfigService,
        const OMKey&                          omKey,
        const I3Position&                     omKeyPos,
        const double                          omKeyHitTime
    )
      : stConfigService_(stConfigService),
        omKey_(omKey),
        omKeyPos_(omKeyPos),
        omKeyHitTime_(omKeyHitTime)
    {}

    bool operator()(const I3SeededRTSTHitInfo<HitType>& stHitInfo)
    {
        const I3Position& srcPos = stHitInfo.GetOMPosition();

        // Get ST configuration for the OM link between the hit and the
        // reference OM.
        const I3SeededRTConfiguration& stConfig = stConfigService_.GetSTConfigurationForOMLink(stHitInfo.GetOMKey(), omKey_);

        double dustlayerCorrectionLength = 0;
        if(! arePositionsInSpatialCausalConnection(
                stConfigService_, stConfig, srcPos, omKeyPos_, &dustlayerCorrectionLength)
          ) {
            return false;
        }

        if(! areTimesInTemporalCausalConnection(
                stHitInfo.GetHitTime(), omKeyHitTime_, stConfig.GetRTTime(), dustlayerCorrectionLength)
          ) {
            return false;
        }

        return true;
    }
};

//______________________________________________________________________________
/** Function object to determine if a ST hit is part of a hit series map.
 */
template <class HitType>
struct isHitSeriesMapHit
{
    std::vector< I3STHitInfo<HitType> > hitInfoSeries_;
    typename std::vector< I3STHitInfo<HitType> >::const_iterator hitInfoSeriesCIter_;
    typename std::vector< I3STHitInfo<HitType> >::const_iterator hitInfoSeries_begin_;
    typename std::vector< I3STHitInfo<HitType> >::const_iterator hitInfoSeries_end_;

    //__________________________________________________________________________
    isHitSeriesMapHit(
        const I3Map< OMKey, std::vector<HitType> >& hitSeriesMap
    )
    {
        // We need to convert the hit series map into a time sorted hit info
        // series for faster access.
        typename I3Map< OMKey, std::vector<HitType> >::const_iterator hitSeriesMapCIter;
        typename std::vector< HitType >::const_iterator hitSeriesCIter;
        typename std::vector< HitType >::const_iterator hitSeries_end;

        for(hitSeriesMapCIter = hitSeriesMap.begin();
            hitSeriesMapCIter != hitSeriesMap.end();
            ++hitSeriesMapCIter
           )
        {
            const OMKey &omKey = hitSeriesMapCIter->first;
            const std::vector< HitType > &hitSeries = hitSeriesMapCIter->second;
            const I3Position omPos;

            hitSeriesCIter = hitSeries.begin();
            hitSeries_end  = hitSeries.end();
            uint32_t idx = 0;
            for(; hitSeriesCIter != hitSeries_end;
                ++hitSeriesCIter, ++idx
               )
            {
                const HitType &hit = *hitSeriesCIter;

                if(std::isnan(getHitTime(hit)))
                {
                    log_warn(
                        "Got a NaN time from a seed hit on OM %s! "
                        "Ignoring this hit as a seed hit!",
                        omKey.str().c_str());
                    continue;
                }

                hitInfoSeries_.push_back(I3STHitInfo<HitType>(
                    omKey, &omPos, &hit, idx));
            }
        }

        std::sort(hitInfoSeries_.begin(), hitInfoSeries_.end(), I3STHitInfo<HitType>::compareHitTimeAsc);

        hitInfoSeriesCIter_ = hitInfoSeries_.begin();

        hitInfoSeries_begin_ = hitInfoSeries_.begin();
        hitInfoSeries_end_   = hitInfoSeries_.end();
    }

    //__________________________________________________________________________
    bool
    operator()(const I3SeededRTSTHitInfo<HitType>& stHitInfo)
    {
        // Since the successive hits given through the successive
        // I3SeededRTSTHitInfo objects are ascended time sorted, and the hits
        // stored in the hitInfoSeries_ object are also ascended time sorted,
        // we can implement a fast decision algorithm here.

        // Check if there are defined seed hits at all.
        if(hitInfoSeries_begin_ == hitInfoSeries_end_)
        {
            return false;
        }

        const double hitTime = stHitInfo.GetHitTime();

        // Check if a new iteration of hits has started.
        if(hitTime < hitInfoSeriesCIter_->GetHitTime())
        {
            hitInfoSeriesCIter_ = hitInfoSeries_begin_;
        }

        // Move the hitInfoSeriesCIter_ forward to the position in the
        // hitInfoSeries_ vector where the hit time is equal to
        // the time of the hit in question at the last time.
        while(
            ((hitInfoSeriesCIter_+1) != hitInfoSeries_end_) &&
            ((hitInfoSeriesCIter_+1)->GetHitTime() <= hitTime)
        )
        {
            ++hitInfoSeriesCIter_;
        }

        // Look backward in time to find the hit series map hit.
        typename std::vector< I3STHitInfo<HitType> >::const_iterator citer;
        citer = hitInfoSeriesCIter_;
        do
        {
            if( ( citer->GetOMKey() == stHitInfo.GetOMKey() ) &&
                ( *(citer->GetHitPtr()) == *(stHitInfo.GetHitPtr()) )
              )
            {
                hitInfoSeriesCIter_ = citer;
                return true;
            }

            if(citer == hitInfoSeries_begin_) break;
            else                              --citer;
        }
        while(citer->GetHitTime() >= hitTime);

        return false;
    }
};

//______________________________________________________________________________
/** Function object to seed with all hits, which are HLC hits.
 */
template <class HitType>
struct seedWithAllHLCHits
{
    uint32_t
    operator()(const I3Frame& frame, std::vector< I3SeededRTSTHitInfo< HitType > >& stHitInfoSeries)
    {
        uint32_t nSeedHits = selectHits<HitType>(stHitInfoSeries, isHLCHit<HitType>());
        if(nSeedHits == 0)
        {
            log_info("No HLC hits found!");
        }

        return nSeedHits;
    }
};

//______________________________________________________________________________
/** Function object to seed with all hits that have at least nHitsThreshold
 *  partner ST hits.
 *
 *  @note If the allowNoSeedHits option is set to ``false`` and  no hits
 *        fulfill the requirements above, all hits will be used as
 *        seed hits!
 */
template <class HitType>
struct seedWithAllCoreHits
{
    const I3SeededRTConfigurationService& stConfigService_;
    uint32_t                              nHitsThreshold_;
    bool                                  allowNoSeedHits_;

    //__________________________________________________________________________
    seedWithAllCoreHits(
        const I3SeededRTConfigurationService& stConfigService,
        uint32_t                              nHitsThreshold,
        bool                                  allowNoSeedHits
    )
      : stConfigService_(stConfigService),
        nHitsThreshold_(nHitsThreshold),
        allowNoSeedHits_(allowNoSeedHits)
    {}

    //__________________________________________________________________________
    uint32_t
    operator()(const I3Frame& frame, std::vector< I3SeededRTSTHitInfo< HitType > >& stHitInfoSeries)
    {
        // First select all hits and then deselect those hits, which have less
        // than nHitsThreshold_ partner ST hits.
        uint32_t nSeedHits = selectHits<HitType>(stHitInfoSeries, isAnyHit<HitType>());
        if(nSeedHits == 0)
        {
            log_info("No hits found at all!");
        }
        else
        {
            nSeedHits = deselectRTHitsBelowRTPartnerHitsThreshold<HitType>(stConfigService_, stHitInfoSeries, nHitsThreshold_);
            if(nSeedHits == 0)
            {
                if(! allowNoSeedHits_)
                {
                    log_info(
                        "No hits fulfill the requirements "
                        "(nHitsThreshold = %u) for the AllCoreHits seed "
                        "procedure. "
                        "Taking all hits as seed hits (allowNoSeedHits = False) ...",
                        nHitsThreshold_);
                    nSeedHits = selectHits<HitType>(stHitInfoSeries, isAnyHit<HitType>());
                }
                else
                {
                    log_info(
                        "No hits fulfill the requirements "
                        "(nHitsThreshold = %u) for the AllCoreHits seed "
                        "procedure.",
                        nHitsThreshold_);
                }
            }
        }

        return nSeedHits;
    }
};

//______________________________________________________________________________
/** Function object to seed with all HLC hits that have at least nHitsThreshold
 *  partner ST HLC hits.
 *
 *  @note If the allowNoSeedHits option is set to ``false`` and  no HLC hits
 *        fulfill the requirements above, all HLC hits will be used as
 *        seed hits!
 */
template <class HitType>
struct seedWithHLCCoreHits
{
    const I3SeededRTConfigurationService& stConfigService_;
    uint32_t                              nHitsThreshold_;
    bool                                  allowNoSeedHits_;

    //__________________________________________________________________________
    seedWithHLCCoreHits(
        const I3SeededRTConfigurationService& stConfigService,
        uint32_t                              nHitsThreshold,
        bool                                  allowNoSeedHits
    )
      : stConfigService_(stConfigService),
        nHitsThreshold_(nHitsThreshold),
        allowNoSeedHits_(allowNoSeedHits)
    {}

    //__________________________________________________________________________
    uint32_t
    operator()(const I3Frame& frame, std::vector< I3SeededRTSTHitInfo< HitType > >& stHitInfoSeries)
    {
        // First select all HLC hits and then deselect those selected hits,
        // which have less than nHitsThreshold_ partner HLC ST hits.
        uint32_t nSeedHits = selectHits<HitType>(stHitInfoSeries, isHLCHit<HitType>());
        if(nSeedHits == 0)
        {
            log_info("No HLC hits found!");
        }
        else
        {
            nSeedHits = deselectRTHitsBelowRTPartnerHitsThreshold<HitType>(stConfigService_, stHitInfoSeries, nHitsThreshold_);
            if(nSeedHits == 0)
            {
                if(! allowNoSeedHits_)
                {
                    log_info(
                        "No HLC hits fulfill the requirements "
                        "(nHitsThreshold = %u) for the HLCCoreHits seed "
                        "procedure. "
                        "Taking all HLC hits as seed hits (allowNoSeedHits = False) ...",
                        nHitsThreshold_);
                    nSeedHits = selectHits<HitType>(stHitInfoSeries, isHLCHit<HitType>());
                }
                else
                {
                    log_info(
                        "No HLC hits fulfill the requirements "
                        "(nHitsThreshold = %u) for the HLCCoreHits seed "
                        "procedure.",
                        nHitsThreshold_);
                }
            }
        }

        return nSeedHits;
    }
};

//______________________________________________________________________________
/** Function object to seed with hits which fulfill the ST conditions around the
 *  Center-of-Gravity (COG) of all HLC hits.
 *
 *  @note If the allowNoSeedHits option is set to ``false`` and no hits
 *        fulfill the requirements above, all HLC hits will be used as
 *        seed hits!
 */
template <class HitType>
struct seedWithHLCCOGSTHits
{
    const I3SeededRTConfigurationService& stConfigService_;
    bool                                  allowNoSeedHits_;

    //__________________________________________________________________________
    seedWithHLCCOGSTHits(
        const I3SeededRTConfigurationService& stConfigService,
        bool                                  allowNoSeedHits
    )
      : stConfigService_(stConfigService),
        allowNoSeedHits_(allowNoSeedHits)
    {}

    //__________________________________________________________________________
    uint32_t
    operator()(const I3Frame& frame, std::vector< I3SeededRTSTHitInfo< HitType > >& stHitInfoSeries)
    {
        uint32_t nSeedHits = 0;

        I3FourVector hlcCOG = calcHitsCOGFourPosition<HitType, I3SeededRTSTHitInfo>(stHitInfoSeries, isHLCHit<HitType>());
        if(hlcCOG.IsNAN())
        {
            // Note: If the COG of the HLC hits could not be determined, then
            //       there are simply no HLC hits!
            log_info(
                "No HLC hits found for which the COG could be determined!");
        }
        else
        {
            // Determine the nearest OM w.r.t. the COG.
            I3Position hlcCOGPos = hlcCOG.GetThreeVectorAsI3Position();
            OMKey omKey = getOMKeyClosestToPosition(stConfigService_.GetOMGeoMap(), hlcCOGPos);

            // We use the COG position to determine if a particular hit is in
            // ST connection to the COG. But we use the DOM link from the
            // closted DOM to the particular hit for determining the ST config.
            nSeedHits = selectHits<HitType>(stHitInfoSeries,
                isHitInSTConnectionToOM<HitType>(stConfigService_, omKey, hlcCOGPos, hlcCOG.GetX0()));
            if(nSeedHits == 0)
            {
                if(! allowNoSeedHits_)
                {
                    log_info(
                        "No hits are in ST connection to the HLC COG (%s)! "
                        "Taking all HLC hits as seed hits (allowNoSeedHits = False) ...",
                        hlcCOG.str().c_str());
                    nSeedHits = selectHits<HitType>(stHitInfoSeries, isHLCHit<HitType>());
                }
                else
                {
                    log_info(
                        "No hits are in ST connection to the HLC COG (%s)!",
                        hlcCOG.str().c_str());
                }
            }
        }

        return nSeedHits;
    }
};

//______________________________________________________________________________
/** Function object to seed with all hits which belong to the OMs present in the
 *  given std::vector< OMKey > object.
 */
template <class HitType>
struct seedWithOMKeyHits
{
    std::vector<OMKey> omKeys_;

    //__________________________________________________________________________
    seedWithOMKeyHits(
        const std::vector<OMKey>& omKeys
    )
      : omKeys_(omKeys)
    {}

    //__________________________________________________________________________
    uint32_t
    operator()(const I3Frame& frame, std::vector< I3SeededRTSTHitInfo< HitType > >& stHitInfoSeries)
    {
        uint32_t nSeedHits = selectHits<HitType>(stHitInfoSeries, isOMKeyHit<HitType>(omKeys_));
        if(nSeedHits == 0)
        {
            log_info(
                "No hits from configured OMs (%s) found!",
                streamToString< std::vector<OMKey> >(omKeys_).c_str());
        }

        return nSeedHits;
    }
};

//______________________________________________________________________________
/** Function object to seed with all hits which belong to the OMs present in the
 *  given std::vector< OMKey > object and which are the N-th hits within the
 *  OM's hit series.
 */
template <class HitType>
struct seedWithNthOMKeyHits
{
    uint32_t nth_;
    std::vector<OMKey> omKeys_;

    //__________________________________________________________________________
    seedWithNthOMKeyHits(
        uint32_t                  nth,
        const std::vector<OMKey>& omKeys
    )
      : nth_(nth),
        omKeys_(omKeys)
    {}

    //__________________________________________________________________________
    uint32_t
    operator()(const I3Frame& frame, std::vector< I3SeededRTSTHitInfo< HitType > >& stHitInfoSeries)
    {
        uint32_t nSeedHits = selectHits<HitType>(stHitInfoSeries, isNthOMKeyHit<HitType>(nth_, omKeys_));
        if(nSeedHits == 0)
        {
            log_info(
                "No %u-th hits from configured OMs (%s) found!",
                nth_, streamToString< std::vector<OMKey> >(omKeys_).c_str());
        }

        return nSeedHits;
    }
};

//______________________________________________________________________________
/** Function object to seed with all hits which are contained inside an
 *  I3Map< OMKey, std::vector< HitType > > object named ``seedHitSeriesMapName``
 *  present in the current processed frame.
 *
 *  @note It must be made sure, that the hit series map exists inside the frame!
 *      Otherwise it will raise a runtime error exception.
 */
template <class HitType>
struct seedWithHitSeriesMapHitsFromFrame
{
    std::string seedHitSeriesMapName_;

    //__________________________________________________________________________
    seedWithHitSeriesMapHitsFromFrame(
        const std::string& seedHitSeriesMapName
    )
      : seedHitSeriesMapName_(seedHitSeriesMapName)
    {}

    //__________________________________________________________________________
    uint32_t
    operator()(const I3Frame& frame, std::vector< I3SeededRTSTHitInfo< HitType > >& stHitInfoSeries)
    {
        boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > hitSeriesMap =
            getHitSeriesMap<HitType>(frame, seedHitSeriesMapName_);
        uint32_t nSeedHits = selectHits<HitType>(stHitInfoSeries, isHitSeriesMapHit<HitType>(
            *hitSeriesMap));
        if(nSeedHits == 0)
        {
            log_info(
                "No seed hits found from hit series map!");
        }

        return nSeedHits;
    }
};

//______________________________________________________________________________
/** Utility function to do a seededRT cleaning on a given hit series map stored
 *  inside a given I3Frame object.
 */
template <
    class HitType,
    class OutType
>
boost::shared_ptr< OutType >
doSeededRTCleaning(
    const I3SeededRTConfigurationService&                                                    stConfigService,
    boost::function<uint32_t (const I3Frame&, std::vector< I3SeededRTSTHitInfo<HitType> >&)> doSeedProcedure,
    const I3Frame&                                                                           frame,
    const std::string&                                                                       inputHitSeriesMapName,
    int32_t                                                                                  maxNIterations=-1,
    bool                                                                                     allowNoSeedHits=false
)
{
    //--------------------------------------------------------------------------
    // Define convenient type aliases.
    typedef I3Map<OMKey, std::vector<HitType> > InputHitSeriesMap;
    typedef boost::shared_ptr<const InputHitSeriesMap> InputHitSeriesMapConstPtr;

    typedef I3SeededRTSTHitInfo<HitType> STHitInfo;

    typedef std::vector<STHitInfo> STHitInfoSeries;
    typedef boost::shared_ptr<OutType> OutTypePtr;

    //--------------------------------------------------------------------------
    // Get the input hit series map from the frame.
    InputHitSeriesMapConstPtr inputHitSeriesMap = getHitSeriesMap<HitType>(frame, inputHitSeriesMapName);

    //--------------------------------------------------------------------------
    // Create the vector for storing the ST information of each hit and fill
    // this vector with all the hits. No hit will be selected and the hits will
    // be ascended time sorted.
    STHitInfoSeries stHitInfoSeries;
    fillSeededRTSTHitInfoSeries(stConfigService, *inputHitSeriesMap, stHitInfoSeries);

    //--------------------------------------------------------------------------
    // Select the initial seed hits based on the given seed procedure.

    uint32_t nSeedHits = doSeedProcedure(frame, stHitInfoSeries);
    log_trace("Did doSeedProcedure: nSeedHits = %ud", nSeedHits);
    if(nSeedHits == 0)
    {
        if(allowNoSeedHits) {
            log_debug("The output will be empty (nSeedHits = 0, allowNoSeedHits = True)");
        }
        else {
            log_fatal("No seed hits found! (nSeedHits = 0, allowNoSeedHits = False)");
        }
    }

    //--------------------------------------------------------------------------
    // Do the seededRT iterative procedure with the given maximal
    // number of iterations.
    if(nSeedHits > 0)
    {
        doIterativeSelectRTHitsProcedure(
            stConfigService, stHitInfoSeries, maxNIterations);
    }

    //--------------------------------------------------------------------------
    // Create and fill the output frame object and return it.
    OutTypePtr output = createAndFillOutputFrameObject<HitType,OutType>(
        frame, inputHitSeriesMapName, stHitInfoSeries, isSelectedHit<HitType>());

    return output;
}

}// namespace seededRT
}// namespace sttools

#endif// ! STTOOLS_ALGORITHMS_SEEDEDRT_UTILITIES_H_INCLUDED
