/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/STTools/modules/I3RTVetoModule.cxx
 * @date $Date$
 * @brief This file contains the definition and implementation of the RTVeto
 *        I3Module (or better I3STModule).
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
#include <vector>

#include "icetray/OMKey.h"
#include "icetray/I3Frame.h"
#include "icetray/I3PointerTypedefs.h"

#include "dataclasses/I3Map.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include "STTools/I3STModule.h"
#include "STTools/algorithms/seededRT/utilities.h"

using namespace std;

//==============================================================================
template <class HitType, class OutType = I3Map<OMKey, vector<HitType> > >
class I3RTVetoModule
  : public I3STModule< sttools::seededRT::I3SeededRTConfigurationService >
{
  public:
    I3RTVetoModule(const I3Context &context);

  protected:
    // Define convenient type aliases.
    //__________________________________________________________________________
    typedef I3Map<OMKey, vector<HitType> > InputHitSeriesMap;
    I3_POINTER_TYPEDEFS(InputHitSeriesMap);

    typedef sttools::seededRT::I3SeededRTSTHitInfo<HitType> STHitInfo;

    typedef vector< STHitInfo > STHitInfoSeries;
    I3_POINTER_TYPEDEFS(STHitInfoSeries);

    I3_POINTER_TYPEDEFS(OutType);

    //__________________________________________________________________________
    void SetupSContext(I3FramePtr frame);

    //__________________________________________________________________________
    void RunSTAlgorithm(I3FramePtr frame);

  private:
    SET_LOGGER("I3RTVetoModule");
};

//______________________________________________________________________________
template <class HitType, class OutType>
I3RTVetoModule<HitType, OutType>::
I3RTVetoModule(const I3Context& context)
  : I3STModule< sttools::seededRT::I3SeededRTConfigurationService >(context)
{
    AddOutBox("OutBox");
}

//______________________________________________________________________________
template <class HitType, class OutType>
void
I3RTVetoModule<HitType, OutType>::
SetupSContext(I3FramePtr frame)
{
    log_trace(
        "Setup the spatial context by using the seededRT algorithm.");

    //--------------------------------------------------------------------------
    // Get the I3OMGeoMap from the frame.
    const I3Geometry& geometry = frame->Get<I3Geometry>(geometryName_);
    const I3OMGeoMap& omGeoMap = geometry.omgeo;

    //--------------------------------------------------------------------------
    // Setup the spatial context of the ST configuration service.
    stConfigService_->SetupSContext(omGeoMap);

    log_trace(
        "Spatial context setup completed.");
    PushFrame(frame, "OutBox");
}

//______________________________________________________________________________
template <class HitType, class OutType>
void
I3RTVetoModule<HitType, OutType>::
RunSTAlgorithm(I3FramePtr frame)
{
    log_debug("Entering RunSTAlgorithm.");

    if(! frame->Has(inputHitSeriesMapName_)) {
        log_debug(
            "The frame does not have the input hit series map named \"%s\"! "
            "Skipping this frame.",
            inputHitSeriesMapName_.c_str());
        PushFrame(frame, "OutBox");
        return;
    }

    if(! stConfigService_->GetSContext().GetSDataMap())
    {
        log_fatal(
            "The spatial data map has not been created! Was there no "
            "Geometry frame before this current frame?");
    }

    InputHitSeriesMapConstPtr inputHitSeriesMap = sttools::getHitSeriesMap<HitType>(*frame, inputHitSeriesMapName_);

    //--------------------------------------------------------------------------
    // Create the vector for storing the ST information of each hit and fill
    // this vector with all the hits. No hit will be selected and the hits will
    // be ascended time sorted.
    STHitInfoSeries stHitInfoSeries;
    sttools::seededRT::fillSeededRTSTHitInfoSeries(*stConfigService_, *inputHitSeriesMap, stHitInfoSeries);

    //--------------------------------------------------------------------------
    log_debug(
        "InputHitSeriesMap has %zu hits. Will run seededRT algorithm %zu times "
        "with each one of the hit as a seed.",
        stHitInfoSeries.size(), stHitInfoSeries.size());

    STHitInfoSeries bestSTHitInfoSeries;
    uint32_t maxNSelectedRTHits = 0;
    uint32_t nSameMaxNSelectedRTHits = 1;
    for(size_t i=0; i<stHitInfoSeries.size(); ++i)
    {
        // Seed with one hit.
        STHitInfoSeries currSTHitInfoSeries(stHitInfoSeries);
        currSTHitInfoSeries[i].SetIsHitSelected(true);

        // Do the seededRT iterative procedure with no maximal number of
        // iterations.
        uint32_t nSelectedRTHits = sttools::seededRT::doIterativeSelectRTHitsProcedure(
            *stConfigService_,
            currSTHitInfoSeries);

        if(nSelectedRTHits > maxNSelectedRTHits)
        {
            maxNSelectedRTHits = nSelectedRTHits;
            bestSTHitInfoSeries = currSTHitInfoSeries;
            nSameMaxNSelectedRTHits = 1;
        }
        else if(nSelectedRTHits == maxNSelectedRTHits) {
            ++nSameMaxNSelectedRTHits;
        }
    }
    if(nSameMaxNSelectedRTHits > 1)
    {
        log_info(
            "Found %u best hit possibilities for this event with %u kept hits!",
            nSameMaxNSelectedRTHits, maxNSelectedRTHits);
    }

    log_debug(
        "OutputHitSeriesMap '%s' will contain %u hits.",
        outputHitSeriesMapName_.c_str(), maxNSelectedRTHits);

    //--------------------------------------------------------------------------
    // Create and fill the output frame object and put it into the frame.
    OutTypePtr output = sttools::seededRT::createAndFillOutputFrameObject<HitType,OutType>(
        *frame, inputHitSeriesMapName_, bestSTHitInfoSeries, sttools::seededRT::isSelectedHit<HitType>());
    frame->Put(outputHitSeriesMapName_, output);

    PushFrame(frame, "OutBox");
}

//==============================================================================

//______________________________________________________________________________
typedef I3RTVetoModule<I3RecoPulse, I3RecoPulseSeriesMap>     I3RTVeto_RecoPulse_Module;
typedef I3RTVetoModule<I3RecoPulse, I3RecoPulseSeriesMapMask> I3RTVeto_RecoPulseMask_Module;
typedef I3RTVetoModule<I3DOMLaunch, I3DOMLaunchSeriesMap>     I3RTVeto_DOMLaunch_Module;

I3_MODULE(I3RTVeto_RecoPulse_Module);
I3_MODULE(I3RTVeto_RecoPulseMask_Module);
I3_MODULE(I3RTVeto_DOMLaunch_Module);
