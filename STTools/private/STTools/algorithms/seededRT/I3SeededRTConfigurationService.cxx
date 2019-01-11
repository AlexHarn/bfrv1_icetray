/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/STTools/algorithms/seededRT/I3SeededRTConfigurationService.cxx
 * @date $Date$
 * @brief This file contains the implementation of the
 *        I3SeededRTConfigurationService class within the sttools::seededRT
 *        namespace.
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
#include <algorithm>
#include <cmath>

#include <boost/foreach.hpp>

#include "icetray/I3Units.h"

#include "dataclasses/I3Constants.h"

#include "STTools/I3RUsageTimer.h"
#include "STTools/utilities.h"

#include "STTools/algorithms/seededRT/I3SeededRTSDataMap.h"
#include "STTools/algorithms/seededRT/I3SeededRTConfiguration.h"
#include "STTools/algorithms/seededRT/I3SeededRTConfigurationService.h"
#include "STTools/algorithms/seededRT/utilities.h"

using namespace std;

namespace sttools {

//__________________________________________________________________________
/** Creates the default SeededRT ST configuration object. With this
 *  ST configuration any two hits will never be in causal connection.
 */
template<> template<>
I3SeededRTConfiguration
I3STConfigurationService<I3SeededRTConfiguration, seededRT::I3SeededRTSContext>::
CreateDefaultSTConfiguration<I3SeededRTConfiguration>() const
{
    return I3SeededRTConfiguration(
               "Default Null ST Configuration",
               I3VectorOMKeyLinkSet(),
               I3SeededRTConfiguration::Sph,
               0, 0, 0
           );
}

namespace seededRT {

//______________________________________________________________________________
bool
I3SeededRTConfigurationService::
AreGlobalSettingsConsistant(bool throwException) const
{
    if(useDustlayerCorrection_)
    {
        if(std::isnan(dustlayerUpperZBoundary_)) {
            if(throwException)
                log_fatal(
                    "The DustlayerUpperZBoundary global setting is set to "
                    "NAN, but the UseDustlayerCorrection global setting is "
                    "set to true!");
            return false;
        }
        if(std::isnan(dustlayerLowerZBoundary_)) {
            if(throwException)
                log_fatal(
                    "The DustlayerLowerZBoundary global setting is set to "
                    "NAN, but the UseDustlayerCorrection global setting is "
                    "set to true!");
            return false;
        }
        if(dustlayerUpperZBoundary_ < dustlayerLowerZBoundary_) {
            if(throwException)
                log_fatal(
                    "The DustlayerUpperZBoundary global setting value is "
                    "less than the DustlayerLowerZBoundary global setting "
                    "value!");
            return false;
        }
    }

    return true;
}

//______________________________________________________________________________
void
I3SeededRTConfigurationService::
FreezeSTConfiguration()
{
    // Get the maximal configured RT time.
    maxRTTime_ = NAN;
    BOOST_FOREACH(const I3SeededRTConfiguration &stConfig, *((*this).stConfigVecPtr_))
    {
        const double rtTime = stConfig.GetRTTime();
        if(std::isnan(maxRTTime_) || rtTime > maxRTTime_) {
            maxRTTime_ = rtTime;
        }
    }

    // Call this method of the base class.
    I3STConfigurationService<I3SeededRTConfiguration, I3SeededRTSContext>::FreezeSTConfiguration();
}

//______________________________________________________________________________
void
I3SeededRTConfigurationService::
UnfreezeSTConfiguration()
{
    maxRTTime_ = NAN;

    // Call this method of the base class.
    I3STConfigurationService<I3SeededRTConfiguration, I3SeededRTSContext>::UnfreezeSTConfiguration();
}

//______________________________________________________________________________
bool
I3SeededRTConfigurationService::
SetupSContext(const I3OMGeoMap &omGeoMap)
{
    // Call the base class method, that will tell us if a new SContext needs to
    // get setup.
    if(I3STConfigurationService<I3SeededRTConfiguration, I3SeededRTSContext>::SetupSContext(omGeoMap))
    {
        return true;
    }

    log_debug("Setup SContext for the seededRT algorithm.");

    //--------------------------------------------------------------------------
    // Construct the spatial data map for OMKey pairs within the
    // I3SeededRTSContext class instance using the current geometry.
    if(allowSelfCoincidence_)
    {
        sContext_.ConstructSDataMap(omGeoMap, I3SeededRTSDataMap::SymAndEqualOMKeysAreAllowed);
    }
    else
    {
        sContext_.ConstructSDataMap(omGeoMap, I3SeededRTSDataMap::SymAndEqualOMKeysAreForbidden);
    }

    log_debug(
        "Constructed SDataMap of the I3SeededRTSContext with %u elements and "
        "a memory size of %zu bytes.",
        sContext_.GetSDataMap()->GetSizeOfMapArray(),
        sContext_.GetSDataMap()->GetMemSizeOfMapArray());

    I3SeededRTSDataMapPtr sDataMap = sContext_.GetSDataMap();

    uint32_t count = 0;

    I3RUsage rusage;
    {I3RUsageTimer timer(rusage, /*startImmediately=*/true, /*resetGlobalTotTimes=*/true);

    const I3OMGeoMap::const_iterator citerOmGeoMapEnd = omGeoMap.end();
    I3OMGeoMap::const_iterator citerSrc = omGeoMap.begin();
    I3OMGeoMap::const_iterator citerDst;

    // Loop over the lower triangular src-dst DOM matrix (including the
    // diagonal if self coincidence is enabled and skipping it if disabled).
    for(; citerSrc != citerOmGeoMapEnd; ++citerSrc)
    {
        const I3OMGeoMap::value_type &srcKey = *citerSrc;
        const OMKey &srcOMKey = srcKey.first;
        const I3Position &srcPos = srcKey.second.position;
        if (srcOMKey.GetPMT() != 0)
            continue;

        citerDst = citerSrc;
        if(!allowSelfCoincidence_) {
            ++citerDst;
        }
        for(; citerDst != citerOmGeoMapEnd; ++citerDst)
        {
            const I3OMGeoMap::value_type &dstKey = *citerDst;
            const OMKey &dstOMKey = dstKey.first;
            const I3Position &dstPos = dstKey.second.position;
            if (dstOMKey.GetPMT() != 0)
                continue;

            // Get the ST configuration for the OM link.
            const I3SeededRTConfiguration &stConfig = GetSTConfigurationForOMLink(srcOMKey, dstOMKey);

            double dustlayerCorrectionLength = NAN;
            if(arePositionsInSpatialCausalConnection(
                *this, stConfig, srcPos, dstPos, &dustlayerCorrectionLength))
            {
                // Configure the OM link by setting the ST configuration for
                // that OM link.

                // Get the spatial data structure for the OM link.
                I3SeededRTSData &sData = sDataMap->at(srcOMKey, dstOMKey);

                // Check if it wasn't already configured.
                if(sData.stConfig_ != NULL)
                {
                    log_fatal("The OM link (%s, %s) was already configured. "
                              "It seems the ST configuration is not unique!",
                              srcOMKey.str().c_str(), dstOMKey.str().c_str());
                }

                // Set the ST configuration for this OM link.
                sData.stConfig_ = &stConfig;

                // Store the maximal dustlayer correction length within the
                // SContext.
                if(dustlayerCorrectionLength > sContext_.GetMaxDustlayerCorrectionLength()) {
                    sContext_.SetMaxDustlayerCorrectionLength(dustlayerCorrectionLength);
                }

                sData.dustlayerCorrectionLength_ = dustlayerCorrectionLength;

                log_trace("Set ST configuration for OM link %s-%s to %s",
                    srcOMKey.str().c_str(), dstOMKey.str().c_str(), stConfig.str().c_str()
                );
            }

            ++count;
        }
    }
    }//I3RUsageScopeTimer

    log_debug(
        "%u spatial OMKey pair information calculated: %s, %.3f ms/calculation.",
        count, convertI3RUsageToString(rusage).c_str(),
        (count ? rusage.wallclocktime/I3Units::millisecond/count : 0));

    return true;
}

}// namespace seededRT
}// namespace sttools
