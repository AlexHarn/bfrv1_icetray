/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/algorithms/seededRT/I3SeededRTConfigurationService.h
 * @date $Date$
 * @brief This file contains the definition of the
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
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTCONFIGURATIONSERVICE_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTCONFIGURATIONSERVICE_H_INCLUDED

#include <cmath>

#include "icetray/I3Logging.h"

#include "STTools/I3STConfigurationService.h"

#include "STTools/algorithms/seededRT/I3SeededRTSContext.h"
#include "STTools/algorithms/seededRT/I3SeededRTConfiguration.h"

//##############################################################################
namespace sttools { namespace seededRT {

//==============================================================================
class I3SeededRTConfigurationService
  : public I3STConfigurationService<I3SeededRTConfiguration, I3SeededRTSContext>
{
  public:
    //__________________________________________________________________________
    I3SeededRTConfigurationService(
        bool   allowSelfCoincidence,
        bool   useDustlayerCorrection,
        double dustlayerUpperZBoundary,
        double dustlayerLowerZBoundary
    )
      : I3STConfigurationService<I3SeededRTConfiguration, I3SeededRTSContext>(),
        allowSelfCoincidence_(allowSelfCoincidence),
        useDustlayerCorrection_(useDustlayerCorrection),
        dustlayerUpperZBoundary_(dustlayerUpperZBoundary),
        dustlayerLowerZBoundary_(dustlayerLowerZBoundary),
        maxRTTime_(NAN)
    {}

    //__________________________________________________________________________
    bool GetAllowSelfCoincidence() const {
        return allowSelfCoincidence_;
    }
    void SetAllowSelfCoincidence(bool b)
    {
        if(IsSTConfigurationFrozen())
        {
            log_fatal(
                "The ST configuration is frozen, so the AllowSelfCoincidence "
                "property cannot be set! Call the UnfreezeSTConfiguration "
                "method first!");
        }
        allowSelfCoincidence_ = b;
    }

    //__________________________________________________________________________
    /** Returns the global flag if the main dustlayer correction should be used.
     */
    bool GetUseDustlayerCorrection() const {
        return useDustlayerCorrection_;
    }
    void SetUseDustlayerCorrection(bool b)
    {
        if(IsSTConfigurationFrozen())
        {
            log_fatal(
                "The ST configuration is frozen, so the UseDustlayerCorrection "
                "property cannot be set! Call the UnfreezeSTConfiguration "
                "method first!");
        }
        useDustlayerCorrection_ = b;
    }

    //__________________________________________________________________________
    /** Returns the global setting for the upper z boundary of the main dust
     *  layer.
     */
    double GetDustlayerUpperZBoundary() const {
        return dustlayerUpperZBoundary_;
    }
    //--------------------------------------------------------------------------
    void SetDustlayerUpperZBoundary(double z)
    {
        if(IsSTConfigurationFrozen())
        {
            log_fatal(
                "The ST configuration is frozen, so the DustlayerUpperZBoundary "
                "property cannot be set! Call the UnfreezeSTConfiguration "
                "method first!");
        }
        if(std::isnan(z) && useDustlayerCorrection_)
        {
            log_fatal(
                "The DustlayerUpperZBoundary property must not be NAN, "
                "if the UseDustlayerCorrection property is set to true!");
        }
        dustlayerUpperZBoundary_ = z;
    }

    //__________________________________________________________________________
    /** Returns the global setting for the lower z boundary of the main dust
     *  layer.
     */
    double GetDustlayerLowerZBoundary() const {
        return dustlayerLowerZBoundary_;
    }
    //--------------------------------------------------------------------------
    void SetDustlayerLowerZBoundary(double z)
    {
        if(IsSTConfigurationFrozen())
        {
            log_fatal(
                "The ST configuration is frozen, so the DustlayerLowerZBoundary "
                "property cannot be set! Call the UnfreezeSTConfiguration "
                "method first!");
        }
        if(std::isnan(z) && useDustlayerCorrection_)
        {
            log_fatal(
                "The DustlayerLowerZBoundary property must not be NAN, "
                "if the UseDustlayerCorrection property is set to true!");
        }
        dustlayerLowerZBoundary_ = z;
    }

    //__________________________________________________________________________
    /** Checks if the global settings are set consistantly.
     *
     *  @param throwException If set to ``true``, a log_fatal with an error
     *      message will be issued when a setting is not consistant.
     */
    virtual
    bool
    AreGlobalSettingsConsistant(bool throwException=false) const;

    //__________________________________________________________________________
    /** Freezes the ST configuration for optimal performance. It gets the
     *  maximal configured RTTime.
     */
    void
    FreezeSTConfiguration();

    //__________________________________________________________________________
    /** Unfreezes the ST configuration.
     */
    void
    UnfreezeSTConfiguration();

    //__________________________________________________________________________
    /** Returns the maximal
     */
    inline
    double
    GetMaxRTTime() const
    {
        if(! IsSTConfigurationFrozen()) {
            log_fatal(
                "The ST configuration is not frozen, so the maximal configured "
                "RT time is undefined! Call the FreezeSTConfiguration method "
                "first!");
        }
        return maxRTTime_;
    }

    //__________________________________________________________________________
    /** Setups the spatial data map inside the I3SeededRTSContext object of this
     *  I3SeededRTConfigurationService object, based on the given I3OMGeo
     *  object, using the seededRT procedure.
     *
     *  @note The spatial data map of the I3SeededRTSContext will be configured
     *        with an asymetric OMKey pair symmetry.
     */
    bool
    SetupSContext(const I3OMGeoMap &omGeoMap);

  protected:
    /** The global flag if hits of the same OM can be in causial ST connection.
     */
    bool allowSelfCoincidence_;

    //__________________________________________________________________________
    /** The global flag if the main dust layer correction should be used (true)
     *  or not (false).
     */
    bool useDustlayerCorrection_;

    //__________________________________________________________________________
    /** The global setting for the upper z boundary of the main dust layer in
     *  IceCube detector coordinates.
     */
    double dustlayerUpperZBoundary_;

    //__________________________________________________________________________
    /** The global setting for the lower z boundary of the main dust layer in
     *  IceCube detector coordinates.
     */
    double dustlayerLowerZBoundary_;

    //__________________________________________________________________________
    /** The cached value for the maximal configured RT time.
     */
    double maxRTTime_;

  private:
    SET_LOGGER("I3SeededRTConfigurationService");
};

//==============================================================================

//______________________________________________________________________________
I3_POINTER_TYPEDEFS(I3SeededRTConfigurationService);

}/*seededRT*/}/*sttools*/
//##############################################################################

#endif//STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTCONFIGURATIONSERVICE_H_INCLUDED
