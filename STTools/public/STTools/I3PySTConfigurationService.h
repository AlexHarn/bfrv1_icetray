/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/I3PySTConfigurationService.h
 * @date $Date$
 * @brief This file contains the definition of the I3PySTConfigurationService
 *        template. It is a boost::python wrapper template, that helps to
 *        expose I3STConfigurationService C++ classes of particular ST
 *        algorithms to Python.
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
#ifndef STTOOLS_I3PYSTCONFIGURATIONSERVICE_H_INCLUDED
#define STTOOLS_I3PYSTCONFIGURATIONSERVICE_H_INCLUDED

#include <vector>

#include <boost/python.hpp>
#include <boost/python/def_visitor.hpp>

#include "icetray/OMKey.h"
#include "icetray/I3Logging.h"

#include "dataclasses/geometry/I3OMGeo.h"

namespace bp = boost::python;

namespace sttools {

//==============================================================================
template <
    class STConfigurationType,
    class SContextType,
    class STConfigurationServiceType,
    class PySTConfigurationService_interfaceType
>
class I3PySTConfigurationService_interface
  : public bp::def_visitor< PySTConfigurationService_interfaceType >
{
  public:
    template <class classT>
    void
    visit(classT &cls) const
    {
        cls.def("GetSTConfigurationForOMLink",
            &STConfigurationServiceType::PyGetSTConfigurationForOMLink, bp::return_internal_reference<1>(),
            (bp::arg("self"), bp::arg("omKey1"), bp::arg("omKey2")),
            "Returns the ST configuration object that applies to the given       \n"
            "OM link.");

        cls.def("IsSTConfigurationComplete",
            &STConfigurationServiceType::PyIsSTConfigurationComplete,
            (bp::arg("self"), bp::arg("omGeoMap")),
            "Checks if the ST configuration provided by this ST configuration    \n"
            "service is complete, i.e., that all OMKeys present in the given     \n"
            "geometry are also present in this ST configuration.");

        cls.def("FreezeSTConfiguration",
            &STConfigurationServiceType::PyFreezeSTConfiguration,
            (bp::arg("self")),
            "Freezes the ST configuration (i.e. the st_config_vec member         \n"
            "variable) and creates a lookup-table for the ST configuration       \n"
            "objects for each OMKey. This will speed-up the                      \n"
            "GetSTConfigurationForOMKey method.");

        cls.def("UnfreezeSTConfiguration",
            &STConfigurationServiceType::PyUnfreezeSTConfiguration,
            (bp::arg("self")),
            "Unfreezes the ST configuration (i.e. the st_config_vec member       \n"
            "variable) and deletes the lookup-table for the ST configuration     \n"
            "objects for each OMKey. Future calls to the                         \n"
            "GetSTConfigurationForOMKey method will be noticable slower.");

        cls.def("IsSTConfigurationFrozen",
            &STConfigurationServiceType::PyIsSTConfigurationFrozen,
            (bp::arg("self")),
            "Returns ``True`` if the ST configuration (i.e. the ``st_config_vec``\n"
            "property) is frozen and ``false`` otherwise.                        \n"
            "                                                                    \n"
            ".. note:                                                            \n"
            "                                                                    \n"
            "    If the ST configuration is frozen, it must not be changed until \n"
            "    the ``UnfreezeSTConfiguration`` method has been called!         \n"
            "                                                                    \n");

        cls.def("SetupSContext",
            &STConfigurationServiceType::PySetupSContext,
            (bp::arg("self"),
             bp::arg("omGeoMap")
            ),
            "The setupSContext method should be re-implemented by the derived    \n"
            "class, i.e the particular ST algorithm, and should setup the spatial\n"
            "ST context based on the given I3OMGeoMap object if, and only if,    \n"
            "this method returns ``false``. Otherwise it should just do nothing. \n"
            "This method here checks if the ST configuration is frozen and       \n"
            "throws an exception if not. Furthermore it checks if the ST         \n"
            "configuration is complete and unique, and if the given              \n"
            "I3OMGeoMap object is seen by the first time by this ST configuration\n"
            "service.");

        cls.add_property("st_config_vec",
            bp::make_function( (boost::shared_ptr< I3Vector<STConfigurationType> > (STConfigurationServiceType::*)()) &STConfigurationServiceType::GetSTConfigVecPtr),
            (void (STConfigurationServiceType::*)(boost::shared_ptr< I3Vector<STConfigurationType> >&)) &STConfigurationServiceType::SetSTConfigVecPtr,
            "The ST configuration vector holding all ST configuration objects,   \n"
            "that are managed by this ST configuration service.");

        cls.add_property("om_geo_map",
            bp::make_function( (const I3OMGeoMap& (STConfigurationServiceType::*)()) &STConfigurationServiceType::GetOMGeoMap, bp::return_internal_reference<1>() ),
            "*read-only*                                                         \n"
            "                                                                    \n"
            "The I3OMGeoMap object, that is used by this ST configuration        \n"
            "service (i.e. to setup the spatial context).");

        cls.add_property("s_context",
            bp::make_function( (SContextType& (STConfigurationServiceType::*)()) &STConfigurationServiceType::GetSContext, bp::return_internal_reference<1>() ),
            "*read-only*                                                         \n"
            "                                                                    \n"
            "The spatial context object.");
    }
};

//==============================================================================
// Since it is not possible to derive a boost::python::wrapper from an other
// boost::python::wrapper, we create a macro here that can be used by the most
// derived class that derives then also from boost::python::wrapper.
//
// The macro arguments are:
//
//     STConfigurationType:
//         This is the ST configuration class, that is
//         used by the ST configuration service.
//
//     STConfigurationServiceType:
//         This is the ST configuration service class, that should get
//         boost::python::wrapper'ed.
//
#define I3PySTConfigurationService_IMPLEMENTATION(STConfigurationType, STConfigurationServiceType) \
    /*________________________________________________________________________*/\
    const STConfigurationType&                                                  \
    PyGetSTConfigurationForOMLink(const OMKey &omKey1, const OMKey &omKey2) const\
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        return STConfigurationServiceType::GetSTConfigurationForOMLink(omKey1, omKey2);\
    }                                                                           \
    /*------------------------------------------------------------------------*/\
    const STConfigurationType&                                                  \
    GetSTConfigurationForOMLink(const OMKey &omKey1, const OMKey &omKey2) const \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        if(bp::override f = this->get_override("GetSTConfigurationForOMLink")) {\
            return f(omKey1, omKey2);                                           \
        }                                                                       \
        else {                                                                  \
            /* Python class doesn't implement "GetSTConfigurationForOMLink",  */\
            /* so we will call the corresponding C++ class method.            */\
            return STConfigurationServiceType::GetSTConfigurationForOMLink(omKey1, omKey2);\
        }                                                                       \
    }                                                                           \
                                                                                \
    /*________________________________________________________________________*/\
    bool                                                                        \
    PyIsSTConfigurationComplete(const I3OMGeoMap &omGeoMap) const               \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        return STConfigurationServiceType::IsSTConfigurationComplete(omGeoMap); \
    }                                                                           \
    /*------------------------------------------------------------------------*/\
    bool                                                                        \
    IsSTConfigurationComplete(const I3OMGeoMap &omGeoMap) const                 \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        if(bp::override f = this->get_override("IsSTConfigurationComplete")) {  \
            return f(omGeoMap);                                                 \
        }                                                                       \
        else {                                                                  \
            /* Python class doesn't implement "IsSTConfigurationComplete",    */\
            /* so we will call the corresponding C++ class method.            */\
            return STConfigurationServiceType::IsSTConfigurationComplete(omGeoMap);\
        }                                                                       \
    }                                                                           \
                                                                                \
    /*________________________________________________________________________*/\
    void                                                                        \
    PyFreezeSTConfiguration()                                                   \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        STConfigurationServiceType::FreezeSTConfiguration();                    \
    }                                                                           \
    /*------------------------------------------------------------------------*/\
    void                                                                        \
    FreezeSTConfiguration()                                                     \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        if(bp::override f = this->get_override("FreezeSTConfiguration")) {      \
            f();                                                                \
        }                                                                       \
        else {                                                                  \
            /* Python class doesn't implement "FreezeSTConfiguration",        */\
            /* so we will call the corresponding C++ class method.            */\
            STConfigurationServiceType::FreezeSTConfiguration();                \
        }                                                                       \
    }                                                                           \
                                                                                \
    /*________________________________________________________________________*/\
    void                                                                        \
    PyUnfreezeSTConfiguration()                                                 \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        STConfigurationServiceType::UnfreezeSTConfiguration();                  \
    }                                                                           \
    /*------------------------------------------------------------------------*/\
    void                                                                        \
    UnfreezeSTConfiguration()                                                   \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        if(bp::override f = this->get_override("UnfreezeSTConfiguration")) {    \
            f();                                                                \
        }                                                                       \
        else {                                                                  \
            /* Python class doesn't implement "UnfreezeSTConfiguration",      */\
            /* so we will call the corresponding C++ class method.            */\
            STConfigurationServiceType::UnfreezeSTConfiguration();              \
        }                                                                       \
    }                                                                           \
                                                                                \
    /*________________________________________________________________________*/\
    bool                                                                        \
    PyIsSTConfigurationFrozen() const                                           \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        return STConfigurationServiceType::IsSTConfigurationFrozen();           \
    }                                                                           \
    /*------------------------------------------------------------------------*/\
    bool                                                                        \
    IsSTConfigurationFrozen() const                                             \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        if(bp::override f = this->get_override("IsSTConfigurationFrozen")) {    \
            return f();                                                         \
        }                                                                       \
        else {                                                                  \
            /* Python class doesn't implement "IsSTConfigurationFrozen",      */\
            /* so we will call the corresponding C++ class method.            */\
            return STConfigurationServiceType::IsSTConfigurationFrozen();       \
        }                                                                       \
    }                                                                           \
                                                                                \
    /*________________________________________________________________________*/\
    bool                                                                        \
    PySetupSContext(const I3OMGeoMap &omGeoMap)                                 \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        return STConfigurationServiceType::SetupSContext(omGeoMap);             \
    }                                                                           \
    /*------------------------------------------------------------------------*/\
    bool                                                                        \
    SetupSContext(const I3OMGeoMap &omGeoMap)                                   \
    {                                                                           \
        log_trace("%s", __PRETTY_FUNCTION__);                                   \
        if(bp::override f = this->get_override("SetupSContext")) {              \
            return f(omGeoMap);                                                 \
        }                                                                       \
        else {                                                                  \
            /* Python class doesn't implement "SetupSContext",                */\
            /* so we will call the corresponding C++ class method.            */\
            return STConfigurationServiceType::SetupSContext(omGeoMap);         \
        }                                                                       \
    }

}// namespace sttools

#endif// ! STTOOLS_I3PYSTCONFIGURATIONSERVICE_H_INCLUDED
