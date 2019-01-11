/**
 * Copyright (C) 2014
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/pybindings/algorithms/seededRT/I3SeededRTConfiguration.cxx
 * @date $Date$
 * @brief This file contains the Python bindings for the I3SeededRTConfiguration
 *        class.
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

#include "STTools/algorithms/seededRT/I3SeededRTConfiguration.h"
#include "STTools/pybindings/register_I3Vector_of.hpp"

namespace bp = boost::python;

namespace sttools {
namespace seededRT {
namespace py {

void register_I3SeededRTConfiguration()
{
    // Tell boost::python that it should create the python docstrings with
    // user-defined docstrings, python signatures, but no C++ signatures.
    bp::docstring_options docstring_opts(true, true, false);

    {
    bp::scope I3SeededRTConfiguration_scope =
    bp::class_<I3SeededRTConfiguration, bp::bases<I3STConfiguration>, I3SeededRTConfigurationPtr>(
        "I3SeededRTConfiguration",
        "The I3SeededRTConfiguration class provides a ST configuration class for \n"
        "the seededRT ST algorithm.                                              \n",
        bp::init<
             const std::string&,
             const I3VectorOMKeyLinkSet&,
             I3SeededRTConfiguration::SeededRTCoordSys,
             double,
             double,
             double
            >(
            (bp::arg("self"),
             bp::arg("name"),
             bp::arg("omKeyLinkSets"),
             bp::arg("rtCoordSys"),
             bp::arg("rtTime"),
             bp::arg("rtRadius"),
             bp::arg("rtHeight")
            ),
            "Constructs a new I3SeededRTConfiguration object with specific values     \n"
            "for all settings.                                                        \n"
        ))
        .add_property("rt_coord_sys",
            &I3SeededRTConfiguration::GetRTCoordSys,
            &I3SeededRTConfiguration::SetRTCoordSys,
            "The type of the coordinate system to use for seededRT spatial causality  \n"
            "calculations.                                                            \n"
        )
        .add_property("rt_time",
            &I3SeededRTConfiguration::GetRTTime,
            &I3SeededRTConfiguration::SetRTTime,
            "The time interval to use for seededRT ST causality calculations.         \n"
        )
        .add_property("rt_radius",
            &I3SeededRTConfiguration::GetRTRadius,
            &I3SeededRTConfiguration::SetRTRadius,
            "The distance radius to use for seededRT ST causality calculations.       \n"
        )
        .add_property("rt_height",
            &I3SeededRTConfiguration::GetRTHeight,
            &I3SeededRTConfiguration::SetRTHeight,
            "If rt_coord_sys is set to ``Cyl``, this specifies the height of the      \n"
            "cylinder that should be used around an OM for calculating the seededRT   \n"
            "spatial ST causality. The radius of the cylinder is then defined through \n"
            "the value of rt_radius.                                                  \n"
        )
        .add_property("st_volumetime",
            &I3SeededRTConfiguration::GetSTVolumetime,
            "The ST volume-time (ST volume times ST time) that is spaned up by this   \n"
            "ST configuration. This value can be used to compare two ST               \n"
            "configurations in terms of their effectiveness.                          \n"
        )
        .def(bp::dataclass_suite<I3SeededRTConfiguration>())
        ;

    bp::enum_<I3SeededRTConfiguration::SeededRTCoordSys>("SeededRTCoordSys")
        .value("Sph", I3SeededRTConfiguration::Sph)
        .value("Cyl", I3SeededRTConfiguration::Cyl)
        .export_values()
        ;
    }//END OF I3SeededRTConfiguration_scope

    // The register_pointer_conversions<T>() can only be used for
    // I3FrameObjects!
    // It tells python about implicit conversions to/from const and FrameObject
    // boost shared pointers.
    register_pointer_conversions<I3SeededRTConfiguration>();

    //__________________________________________________________________________
    // Expose the I3VectorSeededRTConfiguration class.
    sttools::py::register_i3vector_of<I3SeededRTConfiguration>("SeededRTConfiguration");
}

}// namespace py
}// namespace seededRT
}// namespace sttools
