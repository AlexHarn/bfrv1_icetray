/**
 * Copyright (C) 2014
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/pybindings/algorithms/seededRT/I3SeededRTConfigurationService.cxx
 * @date $Date$
 * @brief This file contains the Python bindings for the
 *        I3SeededRTConfigurationService class.
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
#include <boost/python.hpp>

#include "STTools/algorithms/seededRT/I3PySeededRTConfigurationService.h"

namespace bp = boost::python;

namespace sttools {
namespace seededRT {
namespace py {

void register_I3SeededRTConfigurationService()
{
    // Tell boost::python that it should create the python docstrings with
    // user-defined docstrings, python signatures, but no C++ signatures.
    bp::docstring_options docstring_opts(true, true, false);

    // We need to have a boost::python wrapper around the
    // I3SeededRTConfigurationService class because we want to be able to write
    // special seededRT ST configuration services in python, which are derived
    // from the I3SeededRTConfigurationService C++ class.
    bp::class_<sttools::seededRT::I3PySeededRTConfigurationService, boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService>, boost::noncopyable>(
        "I3SeededRTConfigurationService",
        bp::init<bool, bool, double, double>(
            (bp::arg("self"),
             bp::arg("allowSelfCoincidence"),
             bp::arg("useDustlayerCorrection"),
             bp::arg("dustlayerUpperZBoundary"),
             bp::arg("dustlayerLowerZBoundary")
            ),
            "Constructs a new I3SeededRTConfigurationService object with the given \n"
            "global settings."
        ))
        .def(sttools::seededRT::I3PySeededRTConfigurationService_interface())
        ;

    bp::implicitly_convertible<
            boost::shared_ptr< sttools::seededRT::I3PySeededRTConfigurationService >,
            boost::shared_ptr< sttools::seededRT::I3SeededRTConfigurationService >
        >();
}

}// namespace py
}// namespace seededRT
}// namespace sttools
