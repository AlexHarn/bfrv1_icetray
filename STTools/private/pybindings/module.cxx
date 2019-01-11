/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/pybindings/module.cxx
 * @date $Date$
 * @brief This file contains the Python bindings for the STTools python package.
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

#include "icetray/load_project.h"

//##############################################################################
namespace sttools {

//______________________________________________________________________________
// Forward declare the registration function of the seededRT python module.
namespace seededRT {
namespace py {
void register_module();
}// namespace py
}// namespace seededRT

namespace py {

void register_I3RUsageTimer();

//______________________________________________________________________________
void register_module()
{
    register_I3RUsageTimer();

    seededRT::py::register_module();
}

}// namespace py
}// namespace sttools

//______________________________________________________________________________
BOOST_PYTHON_MODULE(STTools)
{
    load_project("STTools", false);

    sttools::py::register_module();
}
