/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/algorithms/seededRT/I3PySeededRTConfigurationService.h
 * @date $Date$
 * @brief This file contains the definition of the
 *        I3PySeededRTConfigurationService class.
 *        It is a boost::python wrapper class derived from the
 *        I3PySTConfigurationService template, that helps to
 *        expose the I3SeededRTConfigurationService C++ class to Python.
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
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_I3PYSEEDEDRTCONFIGURATIONSERVICE_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_I3PYSEEDEDRTCONFIGURATIONSERVICE_H_INCLUDED

#include <boost/python.hpp>

#include "icetray/I3Logging.h"

#include "STTools/I3PySTConfigurationService.h"

#include "STTools/algorithms/seededRT/I3SeededRTConfiguration.h"
#include "STTools/algorithms/seededRT/I3SeededRTConfigurationService.h"
#include "STTools/algorithms/seededRT/I3SeededRTSContext.h"

namespace bp = boost::python;

namespace sttools {
namespace seededRT {

//==============================================================================
class I3PySeededRTConfigurationService
  : public I3SeededRTConfigurationService,
    public bp::wrapper<I3SeededRTConfigurationService>
{
  public:
    //__________________________________________________________________________
    I3PySeededRTConfigurationService(
        bool   allowSelfCoincidence,
        bool   useDustlayerCorrection,
        double dustlayerUpperZBoundary,
        double dustlayerLowerZBoundary
    )
      : I3SeededRTConfigurationService(
            allowSelfCoincidence,
            useDustlayerCorrection,
            dustlayerUpperZBoundary,
            dustlayerLowerZBoundary
        ),
        bp::wrapper<I3SeededRTConfigurationService>()
    {
        log_trace("%s", __PRETTY_FUNCTION__);
    }

    //////
    // I3STConfigurationService wrapper methods.
    //__________________________________________________________________________
    I3PySTConfigurationService_IMPLEMENTATION(I3SeededRTConfiguration, I3SeededRTConfigurationService)

    //////
    // I3SeededRTConfigurationService specific wrapper methods.
    //__________________________________________________________________________
    bool
    PyAreGlobalSettingsConsistant(bool throwException=false) const
    {
        log_trace("%s", __PRETTY_FUNCTION__);
        return I3SeededRTConfigurationService::AreGlobalSettingsConsistant(throwException);
    }
    //--------------------------------------------------------------------------
    bool
    AreGlobalSettingsConsistant(bool throwException=false) const
    {
        log_trace("%s", __PRETTY_FUNCTION__);
        if(bp::override f = this->get_override("AreGlobalSettingsConsistant")) {
            return f(throwException);
        }
        else {
            // Python class doesn't implement "AreGlobalSettingsConsistant",
            // so we will call the corresponding C++ class method.
            return I3SeededRTConfigurationService::AreGlobalSettingsConsistant(throwException);
        }
    }

  private:
    SET_LOGGER("I3PySeededRTConfigurationService");
};

//==============================================================================
class I3PySeededRTConfigurationService_interface
  : public I3PySTConfigurationService_interface<
                I3SeededRTConfiguration,
                I3SeededRTSContext,
                I3PySeededRTConfigurationService,
                I3PySeededRTConfigurationService_interface >
{
  private:
    typedef I3PySTConfigurationService_interface<
                I3SeededRTConfiguration,
                I3SeededRTSContext,
                I3PySeededRTConfigurationService,
                I3PySeededRTConfigurationService_interface >
            I3PySTConfigurationService_interface_t;
  public:
    template <class classT>
    void
    visit(classT &cls) const
    {
        // Invoke the visit method of the base class.
        I3PySTConfigurationService_interface_t::visit(cls);

        cls.def("AreGlobalSettingsConsistant",
            &I3PySeededRTConfigurationService::PyAreGlobalSettingsConsistant,
            (bp::arg("self"),
             bp::arg("throwException")=false
            ),
            "Checks if the global settings are set consistantly.             \n"
            "                                                                \n"
            ":param throwException: If set to ``true``, a RuntimeError with  \n"
            "    an error message will be issued when a setting is not       \n"
            "    consistant.                                                 \n"
            "                                                                \n"
            ":return bool: ``True`` if the global settings are set           \n"
            "    consistantly, and ``False`` otherwise.                      \n");

        cls.add_property("allow_self_coincidence",
            &I3PySeededRTConfigurationService::GetAllowSelfCoincidence,
            &I3PySeededRTConfigurationService::SetAllowSelfCoincidence,
            "The global flag if two hits on the same OM can be in causial    \n"
            "connection.                                                     \n"
            "                                                                \n"
            ".. note:                                                        \n"
            "                                                                \n"
            "    If the ST configuration is frozen, this property cannot be  \n"
            "    set until the UnfreezeSTConfiguration method has been       \n"
            "    called!                                                     \n"
            "                                                                \n");

        cls.add_property("use_dustlayer_correction",
            &I3PySeededRTConfigurationService::GetUseDustlayerCorrection,
            &I3PySeededRTConfigurationService::SetUseDustlayerCorrection,
            "The global flag if the dustlayer correction should be used      \n"
            "(``True``, or not ``False``).                                   \n"
            "                                                                \n"
            ".. note:                                                        \n"
            "                                                                \n"
            "    If the ST configuration is frozen, this property cannot be  \n"
            "    set until the UnfreezeSTConfiguration method has been       \n"
            "    called!                                                     \n"
            "                                                                \n");

        cls.add_property("dustlayer_upper_z_boundary",
            &I3PySeededRTConfigurationService::GetDustlayerUpperZBoundary,
            &I3PySeededRTConfigurationService::SetDustlayerUpperZBoundary,
            "The global setting for the upper z boundary of the main dust    \n"
            "layer in IceCube detector coordinates.                          \n"
            "                                                                \n"
            ".. note:                                                        \n"
            "                                                                \n"
            "    If the ST configuration is frozen, this property cannot be  \n"
            "    set until the UnfreezeSTConfiguration method has been       \n"
            "    called!                                                     \n"
            "                                                                \n");

        cls.add_property("dustlayer_lower_z_boundary",
            &I3PySeededRTConfigurationService::GetDustlayerLowerZBoundary,
            &I3PySeededRTConfigurationService::SetDustlayerLowerZBoundary,
            "The global setting for the lower z boundary of the main dust    \n"
            "layer in IceCube detector coordinates.                          \n"
            "                                                                \n"
            ".. note:                                                        \n"
            "                                                                \n"
            "    If the ST configuration is frozen, this property cannot be  \n"
            "    set until the UnfreezeSTConfiguration method has been       \n"
            "    called!                                                     \n"
            "                                                                \n");

        cls.add_property("max_rt_time",
            &I3PySeededRTConfigurationService::GetMaxRTTime,
            "*read-only*                                                     \n"
            "The maximal configured RT time.                                 \n"
            "                                                                \n"
            ".. note:                                                        \n"
            "                                                                \n"
            "    This property is only defined after the ST configuration has\n"
            "    been frozen. Otherwise a RuntimeError will be thrown.       \n"
            "                                                                \n");
    }
};

//==============================================================================

}// namespace seededRT
}// namespace sttools

#endif// ! STTOOLS_ALGORITHMS_SEEDEDRT_I3PYSEEDEDRTCONFIGURATIONSERVICE_H_INCLUDED
