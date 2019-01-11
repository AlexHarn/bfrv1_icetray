/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/pybindings/I3RUsageTimer.cxx
 * @date $Date$
 * @brief This file contains the Python bindings for the I3RUsageTimer class.
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
#include <boost/shared_ptr.hpp>

#include "STTools/I3RUsageTimer.h"

namespace bp = boost::python;

namespace sttools {
namespace py {

void register_I3RUsageTimer()
{
    // Tell boost::python that it should create the python docstrings with
    // user-defined docstrings, python signatures, but no C++ signatures.
    bp::docstring_options docstring_opts(true, true, false);

    bp::class_<sttools::I3RUsageTimer, boost::shared_ptr<sttools::I3RUsageTimer>, boost::noncopyable>("I3RUsageTimer",
        bp::init<>(
            (bp::arg("self")),
            //----------------------------------------------------------------------------------
            "Creates a new I3RUsageTimer object with internal storage for the total system,  \n"
            "user, and wallclock times."))
        .def(bp::init<I3RUsagePtr, bool, bool>(
            (bp::arg("self"),
             bp::arg("rusage"),
             bp::arg("startImmediately")=false,
             bp::arg("resetTotTimes")=false
            ),
            "Creates a new I3RUsageTimer object with external storage for the total system,  \n"
            "user, and wallclock times through the provided I3RUsage class object."))

        .def("start", &sttools::I3RUsageTimer::Start,
            (bp::arg("self"),
             bp::arg("resetTotTimes")=false),
            "Starts the timer.                                                               \n"
            "                                                                                \n"
            ":param resetTotTimes: If set to ``True``, the total system, user, and wallclock \n"
            "    time will be reset to zero.                                                 \n"
            "                                                                                \n"
            ":returns bool: The status of the timer (``True`` if it is active, ``False``     \n"
            "    otherwise (i.e. an error occurred).                                         \n")

        .def("stop",
            &sttools::I3RUsageTimer::Stop,
            (bp::arg("self")),
            "Stops the timer. On success, it updates the system, user, and wallclock time by \n"
            "adding the measuring time duration.                                             \n"
            "                                                                                \n"
            ":returns bool: The status of the timer (``True`` if it is still active after    \n"
            "    this call (i.e. an error occurred), ``False`` otherwise.                    \n")

        .def("reset_total_times",
            &sttools::I3RUsageTimer::ResetTotalTimes,
            (bp::arg("self")),
            "Resets the total system, user, and wallclock times to zero.                     \n")

        .def("get_current_rusage",
            &sttools::I3RUsageTimer::GetCurrentRUsage,
            (bp::arg("self")),
            "Returns the current resource usage by returning an I3RUsage frame object, that  \n"
            "could be put into an I3Frame. If there was an error, ``None`` is returned       \n"
            "instead.                                                                        \n"
            "                                                                                \n"
            ".. note:                                                                        \n"
            "                                                                                \n"
            "    The method assumes, that the Start and Stop methods had been called before. \n"
            "    If not, the values of the returned I3RUsage object are arbitrary!           \n"
            "                                                                                \n")

        .def("get_total_rusage",
            &sttools::I3RUsageTimer::GetTotalRUsage,
            (bp::arg("self")),
            "Returns the total resource usage by returning an I3RUsage frame object, that    \n"
            "could be put into an I3Frame.                                                   \n")
    ;

    bp::implicitly_convertible<
            boost::shared_ptr<sttools::I3RUsageTimer>,
            boost::shared_ptr<const sttools::I3RUsageTimer>
        >();

    bp::def("convert_I3RUsage_to_str", &sttools::convertI3RUsageToString,
        (bp::arg("rusage")),
        "Converts a given I3RUsage object into a str object."
    );
}

}// namespace py
}// namespace sttools
