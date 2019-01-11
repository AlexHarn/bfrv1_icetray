/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/STTools/modules/I3SeededRTCleaningModule.cxx
 * @date $Date$
 * @brief This file contains the definition and implementation of the
 *        SeededRTCleaning I3Module (or better spoken I3STModule).
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

#include "icetray/I3PointerTypedefs.h"

#include "dataclasses/I3Map.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include "STTools/I3FourVector.h"
#include "STTools/I3STModule.h"
#include "STTools/utilities.h"

#include "STTools/algorithms/seededRT/I3SeededRTConfigurationService.h"
#include "STTools/algorithms/seededRT/I3SeededRTSTHitInfo.h"
#include "STTools/algorithms/seededRT/utilities.h"

using namespace std;

//==============================================================================
template <class HitType, class OutType = I3Map<OMKey, vector<HitType> > >
class I3SeededRTCleaningModule
  : public I3STModule< sttools::seededRT::I3SeededRTConfigurationService >
{
  public:
    I3SeededRTCleaningModule(const I3Context &context);

    void Configure();

    void Finish();

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

    //__________________________________________________________________________
    /** The name of the procedure to use for determining the initial seed
     *  hits.
     */
    std::string seedProcedure_;

    //__________________________________________________________________________
    /** Some seed procedures need a configured number of hits threshold.
     */
    uint32_t nHitsThreshold_;

    //__________________________________________________________________________
    /** The list of OMKey objects that should be used for the OMKeyList seed
     *  procedure.
     */
    std::vector<OMKey> seedOMKeyList_;

    //__________________________________________________________________________
    /** The name of the hit series map inside the current processed frame,
     *  whose hits should be used as seed hits.
     */
    std::string seedHitSeriesMapName_;

    //__________________________________________________________________________
    /** The switch if it is allowed that an event has no seed hits, i.e. no
     *  hits will be present in the output for those events.
     */
    bool allowNoSeedHits_;

    //__________________________________________________________________________
    /** The switch if result hit maps should be stored in the frame when they
     *  are empty.
     */
    bool storeEmptyResults_;

    //__________________________________________________________________________
    /** The maximal number of iterations for the iterative RT hits selection
     *  procedure. The value -1 meens no constraint.
     */
    int32_t maxNIterations_;

    //__________________________________________________________________________
    /** The number of events for which the configured seed procedure did not
     *  find any seed hits.
     */
    uint32_t nEventsWithNoSeedHits_;

    //__________________________________________________________________________
    /** BOOST function pointer that points to the configured seed procedure
     *  function.
     */
    boost::function<uint32_t (const I3Frame&, STHitInfoSeries&)> doSeedProcedure_;

    //__________________________________________________________________________
    /** The name of the optional output hit series map, that contains the
     *  cleaned away HLC hits.
     */
    std::string outputDiscardedHLCHitSeriesMapName_;

    //__________________________________________________________________________
    /** The name of the optional output hit series map, that contains the
     *  cleaned away SLC hits.
     */
    std::string outputDiscardedSLCHitSeriesMapName_;
    
    /** The last I3Geometry object seen */
    I3GeometryConstPtr currentGeometry_;

  private:
    SET_LOGGER("I3SeededRTCleaningModule");
};

//______________________________________________________________________________
template <class HitType, class OutType>
I3SeededRTCleaningModule<HitType, OutType>::
I3SeededRTCleaningModule(const I3Context& context)
  : I3STModule< sttools::seededRT::I3SeededRTConfigurationService >(context),
    seedProcedure_("AllHLCHits"),
    nHitsThreshold_(2),
    seedHitSeriesMapName_(""),
    allowNoSeedHits_(false),
    storeEmptyResults_(true),
    maxNIterations_(-1),
    nEventsWithNoSeedHits_(0),
    outputDiscardedHLCHitSeriesMapName_(""),
    outputDiscardedSLCHitSeriesMapName_("")
{
    //--------------------------------------------------------------------------
    AddParameter("SeedProcedure",
        "The name of the procedure to use for determining the initial seed "
        "hits. The following values are possible: "
        "\"AllHLCHits\": "
            "All HLC hits will be used as seed hits. "
        "\"AllCoreHits\": "
            "All hits that have at least NHitsThreshold partner ST hits will "
            "be used as seed hits. "
        "\"HLCCoreHits\": "
            "Only HLC hits, that have a certain amount of partner "
            "ST HLC hits. If the AllowNoSeedHits parameter is set to "
            "``false`` and the HLC core is empty, all HLC hits "
            "will be used as seed hits. "
        "\"HLCCOGSTHits\": Hits which fulfill the ST conditions around the "
            "Center-of-Gravity of all HLC hits, are used as seed hits. "
        "\"OMKeyHits\": "
            "Select all hits as seed hits which belong to the OMs present in "
            "the configured (SeedOMKeyList) list of OMKey objects. "
        "\"FirstOMKeyHits\": "
            "Select hits as seed hits which belong to the OMs present in "
            "the configured (SeedOMKeyList) list of OMKey objects and are the "
            "first hits within the OM's hit series. (This seed procedure was "
            "used by the original SeededRTCleaning module when configured with "
            "Seeds='Brightest'.) "
        "\"HitSeriesMapHitsFromFrame\": "
            "Selects hits as seed hits which are contained inside an hit "
            "series map named ``SeedHitSeriesMapName`` inside the current "
            "processed frame.",
        seedProcedure_);

    AddParameter("NHitsThreshold",
        "Some seed procedures need a configured number of hits threshold. This "
        "is it. The value ``0`` means no threshold.",
        nHitsThreshold_);

    AddParameter("AllowNoSeedHits",
        "The switch if it is allowed that an event has no seed hits, i.e. no "
        "hits will be present in the output for those events.",
        allowNoSeedHits_);

    AddParameter("StoreEmptyResults",
        "The switch if resulting hit maps should be put in the frame when they "
        "are empty.",
        storeEmptyResults_);

    AddParameter("SeedOMKeyList",
        "The list of OMKey objects, that should be used by the OMHits seed "
        "procedure.",
        seedOMKeyList_);

    AddParameter("SeedHitSeriesMapName",
        "The name of the hit series map inside the current processed frame, "
        "whose hits should be used as seed hits. So this hit series map must "
        "be a subset of the hit series map named ``InputHitSeriesMapName``. "
        "Otherwise no hits will be used as seeds at all.",
        seedHitSeriesMapName_);

    //--------------------------------------------------------------------------
    AddParameter("MaxNIterations",
        "The maximal number of iterations for the iterative RT hits selection "
        "procedure. The value ``-1`` means no constraint on the number of "
        "iterations.",
        maxNIterations_);

    //--------------------------------------------------------------------------
    AddParameter("OutputDiscardedHLCHitSeriesMapName",
        "The frame object name of the optional hit series map output for HLC "
        "hits that were cleaned away by the seededRT algorithm. An empty \"\" "
        "name disables this output.",
        outputDiscardedHLCHitSeriesMapName_);

    AddParameter("OutputDiscardedSLCHitSeriesMapName",
        "The frame object name of the optional hit series map output for SLC "
        "hits that were cleaned away by the seededRT algorithm. An empty \"\" "
        "name disables this output.",
        outputDiscardedSLCHitSeriesMapName_);

    AddOutBox("OutBox");
}

//______________________________________________________________________________
template <class HitType, class OutType>
void
I3SeededRTCleaningModule<HitType, OutType>::
Configure()
{
    //--------------------------------------------------------------------------
    GetParameter("SeedProcedure", seedProcedure_);
    GetParameter("NHitsThreshold", nHitsThreshold_);
    GetParameter("AllowNoSeedHits", allowNoSeedHits_);
    GetParameter("StoreEmptyResults", storeEmptyResults_);
    GetParameter("SeedOMKeyList", seedOMKeyList_);
    GetParameter("SeedHitSeriesMapName", seedHitSeriesMapName_);

    //--------------------------------------------------------------------------
    GetParameter("MaxNIterations", maxNIterations_);

    //--------------------------------------------------------------------------
    GetParameter("OutputDiscardedHLCHitSeriesMapName", outputDiscardedHLCHitSeriesMapName_);
    GetParameter("OutputDiscardedSLCHitSeriesMapName", outputDiscardedSLCHitSeriesMapName_);

    //--------------------------------------------------------------------------
    // Set the seed precedure function.
    if(seedProcedure_ == "AllHLCHits")
    {
        doSeedProcedure_ = sttools::seededRT::seedWithAllHLCHits<HitType>();
    }
    else if(seedProcedure_ == "AllCoreHits")
    {
        doSeedProcedure_ = sttools::seededRT::seedWithAllCoreHits<HitType>(
            *stConfigService_, nHitsThreshold_, allowNoSeedHits_);
    }
    else if(seedProcedure_ == "HLCCoreHits")
    {
        doSeedProcedure_ = sttools::seededRT::seedWithHLCCoreHits<HitType>(
            *stConfigService_, nHitsThreshold_, allowNoSeedHits_);
    }
    else if(seedProcedure_ == "HLCCOGSTHits")
    {
        doSeedProcedure_ = sttools::seededRT::seedWithHLCCOGSTHits<HitType>(
            *stConfigService_, allowNoSeedHits_);
    }
    else if(seedProcedure_ == "OMKeyHits")
    {
        if(seedOMKeyList_.empty())
        {
            log_fatal(
                "The \"OMKeyHits\" seed procedure has been chosen but no "
                "OMKey objects have been specified via the SeedOMKeyList "
                "module parameter!");
        }

        doSeedProcedure_ = sttools::seededRT::seedWithOMKeyHits<HitType>(
            seedOMKeyList_);
    }
    else if(seedProcedure_ == "FirstOMKeyHits")
    {
        if(seedOMKeyList_.empty())
        {
            log_fatal(
                "The \"FirstOMKeyHits\" seed procedure has been chosen but no "
                "OMKey objects have been specified via the SeedOMKeyList "
                "module parameter!");
        }

        doSeedProcedure_ = sttools::seededRT::seedWithNthOMKeyHits<HitType>(
            1, seedOMKeyList_);
    }
    else if(seedProcedure_ == "HitSeriesMapHitsFromFrame")
    {
        if(seedHitSeriesMapName_.empty())
        {
            log_fatal(
                "The \"HitSeriesMapHitsFromFrame\" seed procedure has been "
                "chosen but no name for the hit series map inside the frame "
                "have been specified via the SeedHitSeriesMapName module "
                "parameter!");
        }

        doSeedProcedure_ = sttools::seededRT::seedWithHitSeriesMapHitsFromFrame<HitType>(
            seedHitSeriesMapName_);
    }
    else
    {
        log_fatal(
            "The SeedProcedure named \"%s\" is not supported!",
            seedProcedure_.c_str());
    }
}

//______________________________________________________________________________
template <class HitType, class OutType>
void
I3SeededRTCleaningModule<HitType, OutType>::
SetupSContext(I3FramePtr frame)
{
    log_trace("Setup the spatial context by using the seededRT algorithm.");

    //--------------------------------------------------------------------------
    // Get the I3OMGeoMap from the frame.
    currentGeometry_ = frame->Get<I3GeometryConstPtr>(geometryName_);
    i3_assert(currentGeometry_ != NULL);
    const I3OMGeoMap& omGeoMap = currentGeometry_->omgeo;

    //--------------------------------------------------------------------------
    // Setup the spatial context of the ST configuration service.
    stConfigService_->SetupSContext(omGeoMap);

    log_trace("Spatial context setup completed.");
    PushFrame(frame, "OutBox");
}

//______________________________________________________________________________
template <class HitType, class OutType>
void
I3SeededRTCleaningModule<HitType, OutType>::
RunSTAlgorithm(I3FramePtr frame)
{
    log_trace("Entering RunSTAlgorithm.");

    if(! stConfigService_->GetSContext().GetSDataMap())
    {
        log_fatal(
            "The spatial data map has not been created! Was there no "
            "Geometry frame before this current frame?");
    }

    if(! frame->Has(inputHitSeriesMapName_)) {
        log_info(
            "The frame does not have the input hit series map named \"%s\"! "
            "Skipping this frame ...",
            inputHitSeriesMapName_.c_str());
        PushFrame(frame, "OutBox");
        return;
    }

    // We need to check if the hit series map of the seeds exists in the frame,
    // otherwise the seed procedure function will fail with an runtime error
    // (what is on purpose).
    if(seedProcedure_ == "HitSeriesMapHitsFromFrame" &&
       !frame->Has(seedHitSeriesMapName_)
      ) {
        log_info(
            "The frame does not have the seed hit series map named \"%s\"! "
            "Skipping this frame ...",
            seedHitSeriesMapName_.c_str());
        PushFrame(frame, "OutBox");
        return;
    }

    InputHitSeriesMapConstPtr inputHitSeriesMap = sttools::getHitSeriesMap<HitType>(*frame, inputHitSeriesMapName_);

    //--------------------------------------------------------------------------
    // Create the vector for storing the ST information of each hit and fill
    // this vector with all the hits. No hit will be selected and the hits will
    // be ascended time sorted.
    STHitInfoSeries stHitInfoSeries;
    sttools::seededRT::fillSeededRTSTHitInfoSeries(*stConfigService_, *inputHitSeriesMap, stHitInfoSeries);

    //--------------------------------------------------------------------------
    // Select the initial seed hits based on the configured seed procedure.
    uint32_t nSeedHits = doSeedProcedure_(*frame, stHitInfoSeries);
    if(nSeedHits == 0)
    {
        ++nEventsWithNoSeedHits_;
    }

    //--------------------------------------------------------------------------
    // Do the seededRT iterative procedure with the configured maximal
    // number of iterations.
    if(nSeedHits > 0)
    {
        sttools::seededRT::doIterativeSelectRTHitsProcedure(
            *stConfigService_, stHitInfoSeries, maxNIterations_);
    }

    //--------------------------------------------------------------------------
    // Create and fill the output frame object and put it into the frame.
    OutTypePtr output = sttools::seededRT::createAndFillOutputFrameObject<HitType,OutType>(
        *frame, inputHitSeriesMapName_, stHitInfoSeries, sttools::seededRT::isSelectedHit<HitType>());
    if(!(storeEmptyResults_ == false && sttools::isOutputFrameObjectEmpty<OutType>(*output))) {
        frame->Put(outputHitSeriesMapName_, output);
    }

    //--------------------------------------------------------------------------
    // Create and fill the optional output frame object for discarded HLC hits.
    if(! outputDiscardedHLCHitSeriesMapName_.empty())
    {
        OutTypePtr discardedHLCOutput = sttools::seededRT::createAndFillOutputFrameObject<HitType,OutType>(
            *frame, inputHitSeriesMapName_, stHitInfoSeries, sttools::seededRT::isUnselectedHLCHit<HitType>());
        if(!(storeEmptyResults_ == false && sttools::isOutputFrameObjectEmpty<OutType>(*discardedHLCOutput))) {
            frame->Put(outputDiscardedHLCHitSeriesMapName_, discardedHLCOutput);
        }
    }

    //--------------------------------------------------------------------------
    // Create and fill the optional output frame object for discarded SLC hits.
    if(! outputDiscardedSLCHitSeriesMapName_.empty())
    {
        OutTypePtr discardedSLCOutput = sttools::seededRT::createAndFillOutputFrameObject<HitType,OutType>(
            *frame, inputHitSeriesMapName_, stHitInfoSeries, sttools::seededRT::isUnselectedSLCHit<HitType>());
        if(!(storeEmptyResults_ == false && sttools::isOutputFrameObjectEmpty<OutType>(*discardedSLCOutput))) {
            frame->Put(outputDiscardedSLCHitSeriesMapName_, discardedSLCOutput);
        }
    }

    PushFrame(frame, "OutBox");
}

//______________________________________________________________________________
template <class HitType, class OutType>
void
I3SeededRTCleaningModule<HitType, OutType>::
Finish()
{
    if(nEventsWithNoSeedHits_ > 0)
    {
        log_info(
            "The SeededRTCleaningModule \"%s\" was run with seed procedure "
            "\"%s\". There have been %u events without any seed hits! "
            "Just to be aware.",
            GetName().c_str(), seedProcedure_.c_str(), nEventsWithNoSeedHits_);
    }

    // Call the base class method, which will print out some performance info.
    I3STModule< sttools::seededRT::I3SeededRTConfigurationService >::Finish();
}

//==============================================================================

//______________________________________________________________________________
typedef I3SeededRTCleaningModule<I3RecoPulse, I3RecoPulseSeriesMap>     I3SeededRTCleaning_RecoPulse_Module;
typedef I3SeededRTCleaningModule<I3RecoPulse, I3RecoPulseSeriesMapMask> I3SeededRTCleaning_RecoPulseMask_Module;
typedef I3SeededRTCleaningModule<I3DOMLaunch, I3DOMLaunchSeriesMap>     I3SeededRTCleaning_DOMLaunch_Module;

I3_MODULE(I3SeededRTCleaning_RecoPulse_Module);
I3_MODULE(I3SeededRTCleaning_RecoPulseMask_Module);
I3_MODULE(I3SeededRTCleaning_DOMLaunch_Module);
