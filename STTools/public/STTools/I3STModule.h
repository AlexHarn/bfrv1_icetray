/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/I3STModule.h
 * @date $Date$
 * @brief This file contains the definition of the I3STModule icetray module.
 *
 *        This icetray module should be used as a base class module for a
 *        particular icetray module, that implements ST algorithms.
 *
 *        This base module is derived from the I3ConditionalModule, so
 *        conditional execution of the module is possible.
 *
 *        It also provides pre-defined ST configuration parameters and a method
 *        to do the ST configuration setup procedure.
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
#ifndef STTOOLS_I3STMODULE_H_INCLUDED
#define STTOOLS_I3STMODULE_H_INCLUDED

#include <set>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/python.hpp>

#include "icetray/I3DefaultName.h"
#include "icetray/I3Frame.h"
#include "icetray/I3ConditionalModule.h"

#include "STTools/I3RUsageTimer.h"
#include "STTools/utilities.h"

namespace bp = boost::python;

//==============================================================================
template <
    class STConfigurationServiceType
>
class I3STModule
  : public I3ConditionalModule
{
  public:
    I3STModule(const I3Context& context);

    void Finish();

  protected:
    //__________________________________________________________________________
    /** The Geometry frame-stop method calls the SetupSTConfiguration method.
     *  So if the user re-implements this method, he needs to make sure that the
     *  SetupSTConfiguration method gets called, either by calling the Geometry
     *  method of this base module or by calling the SetupSTConfiguration
     *  directly.
     */
    void Geometry(I3FramePtr frame);

    //__________________________________________________________________________
    /** The SetupSContext method is pure virtual and must be implemented by the
     *  derived module.
     *
     *  This method should setup the internals of the SContext class module
     *  member variable sContext_, e.g. the spatial condition map based on a
     *  Geometry frame.
     *  This method gets called by the Geometry frame-stop method, whenever a
     *  new geometry frame passes by.
     */
    virtual
    void SetupSContext(I3FramePtr frame) = 0;

    //__________________________________________________________________________
    /** The RunSTAlgorithm method is pure virtual and must be implemented by the
     *  derived module.
     *  This is the actual work-horse method, that is running the particular
     *  ST algorithm.
     *  This method is registered to the configured frame type (a.k.a. stream).
     */
    virtual
    void RunSTAlgorithm(I3FramePtr frame) = 0;

    //__________________________________________________________________________
    /** Calls the most derived version of the RunSTAlgorithm method and does
     *  some book keeping.
     */
    void CallRunSTAlgorithm(I3FramePtr frame)
    {
        ++nCallsToRunSTAlgorithm_;
        totRUsageSTAlgorithmTimer_.Start();
        this->RunSTAlgorithm(frame);
        totRUsageSTAlgorithmTimer_.Stop();
    }

    //__________________________________________________________________________
    /** Internal configure function used by icetray to apply module
     *  configuration that is hidden to the user.
     *
     *  We use it here to ensure, that the ST configuration module parameters
     *  are defined and their values are retrieved.
     *  This way, the derived user ST module does not necessarily need to call
     *  the Configure() method of its base class module.
     *
     */
    void Configure_();

    //////
    // Define the module parameters.
    //__________________________________________________________________________
    /** The shared pointer to the ST configuration object, that provides the ST
     *  configuration for this ST icetray module.
     */
    boost::shared_ptr< STConfigurationServiceType > stConfigService_;

    //__________________________________________________________________________
    /** The name of the I3Geometry frame object that should be used for
     *  constructing the spatial context.
     */
    std::string geometryName_;

    //__________________________________________________________________________
    /** The name of the input hit series map frame object (for example an
     *  I3DOMLaunchSeriesMap, an I3RecoPulseSeriesMap, or an
     *  I3RecoPulseSeriesMapMask I3FrameObject). What particular type is
     *  supported depends on the support of the derived module.
     */
    std::string inputHitSeriesMapName_;

    //__________________________________________________________________________
    /** The name of the output hit series map frame object (for example an
     *  I3DOMLaunchSeriesMap, an I3RecoPulseSeriesMap, or an
     *  I3RecoPulseSeriesMapMask I3FrameObject). What particular type is
     *  supported depends on the support of the derived module.
     */
    std::string outputHitSeriesMapName_;

    //__________________________________________________________________________
    /** The set of I3Frame type (a.k.a. stream) for which the RunSTAlgorithm
     *  method will get executed.
     */
    std::set<I3Frame::Stream> runSTAlgorithmOnStreams_;

    //__________________________________________________________________________
    /** How many times the RunSTAlgorithm method has been called.
     */
    uint32_t nCallsToRunSTAlgorithm_;

    //__________________________________________________________________________
    /// The timer for collecting the total resource usage for running the ST
    /// algorithm.
    sttools::I3RUsageTimer totRUsageSTAlgorithmTimer_;

  private:
    SET_LOGGER("I3STModule");
};

//______________________________________________________________________________
template <
    class STConfigurationServiceType
>
I3STModule<STConfigurationServiceType>::
I3STModule(const I3Context& context)
  : I3ConditionalModule(context),
    geometryName_(I3DefaultName<I3Geometry>::value()),
    inputHitSeriesMapName_(""),
    outputHitSeriesMapName_(""),
    nCallsToRunSTAlgorithm_(0)
{
    //--------------------------------------------------------------------------
    AddParameter("GeometryName",
        "Name of the I3Geometry frame object that should be used for "
        "constructing the spatial context.",
        geometryName_);

    //--------------------------------------------------------------------------
    AddParameter("InputHitSeriesMapName",
        "Name of the input hit series map.",
        inputHitSeriesMapName_);
    AddParameter("OutputHitSeriesMapName",
        "Name of the output hit series map.",
        outputHitSeriesMapName_);

    //--------------------------------------------------------------------------
    AddParameter("STConfigService",
        "The ST configuration service object, that provides the ST "
        "configuration for this ST icetray module.",
        stConfigService_);

    //--------------------------------------------------------------------------
    bp::list defaultStreams;
    defaultStreams.append(I3Frame::Physics);
    AddParameter("Streams",
        "The set of I3Frame types (a.k.a. streams) for which the "
        "RunSTAlgorithm method will get executed.",
        defaultStreams);

    AddOutBox("OutBox");
}

//______________________________________________________________________________
template <
    class STConfigurationServiceType
>
void
I3STModule<STConfigurationServiceType>::
Configure_()
{
    //--------------------------------------------------------------------------
    GetParameter("GeometryName", geometryName_);

    //--------------------------------------------------------------------------
    GetParameter("STConfigService", stConfigService_);

    //--------------------------------------------------------------------------
    GetParameter("InputHitSeriesMapName",  inputHitSeriesMapName_);
    GetParameter("OutputHitSeriesMapName", outputHitSeriesMapName_);

    //--------------------------------------------------------------------------
    std::vector<I3Frame::Stream> streamVec;
    GetParameter("Streams", streamVec);
    runSTAlgorithmOnStreams_ = std::set<I3Frame::Stream>(streamVec.begin(), streamVec.end());
    BOOST_FOREACH(const std::set<I3Frame::Stream>::value_type &stream, runSTAlgorithmOnStreams_)
    {
        Register(stream, &I3STModule::CallRunSTAlgorithm);
    }

    //////
    // Check configuration for consistancy.
    if(geometryName_ == "") {
        log_fatal("The GeometryName must not be \"\"!");
    }
    if(! stConfigService_) {
        log_fatal("The ST configuration service object was not set!");
    }
    if(inputHitSeriesMapName_ == "") {
        log_fatal("The InputHitSeriesMapName must not be \"\"!");
    }
    if(outputHitSeriesMapName_ == "") {
        log_fatal("The OutputHitSeriesMapName must not be \"\"!");
    }

    // Call the Configure_() method of I3ConditionalModule, which
    // will call the Configure_() method of I3Module, which then will call the
    // Configure() method of the most derived class instance.
    I3ConditionalModule::Configure_();
}

//______________________________________________________________________________
template <
    class STConfigurationServiceType
>
void
I3STModule<STConfigurationServiceType>::
Geometry(I3FramePtr frame)
{
    SetupSContext(frame);
}

//______________________________________________________________________________
template <
    class STConfigurationServiceType
>
void
I3STModule<STConfigurationServiceType>::
Finish()
{
    I3RUsagePtr totalRUsage = totRUsageSTAlgorithmTimer_.GetTotalRUsage();
    log_info(
        "%s: %u calls to RunSTAlgorithm: %s, %.2fms per call",
        GetName().c_str(),
        nCallsToRunSTAlgorithm_,
        sttools::convertI3RUsageToString(*totalRUsage).c_str(),
        (nCallsToRunSTAlgorithm_ ? totalRUsage->wallclocktime/I3Units::millisecond/nCallsToRunSTAlgorithm_ : 0));
}

//==============================================================================

#endif//STTOOLS_I3STMODULE_H_INCLUDED
