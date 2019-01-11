/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/I3PySContext.h
 * @date $Date$
 * @brief This file contains the definition of the I3PySContext_interface
 *        template. It is a boost::python visitor, that helps to
 *        expose I3SContext C++ classes of particular ST algorithms to Python.
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
#ifndef STTOOLS_I3PYSCONTEXT_H_INCLUDED
#define STTOOLS_I3PYSCONTEXT_H_INCLUDED

#include <boost/python.hpp>
#include <boost/python/def_visitor.hpp>

namespace bp = boost::python;

//##############################################################################
namespace sttools {

//==============================================================================
template <
    class SDataType,
    class SContextType,
    class PySContext_interfaceType
>
class I3PySContext_interface
  : public bp::def_visitor< PySContext_interfaceType >
{
  public:
    template <class classT>
    void
    visit(classT &cls) const
    {
        cls.def("ConstructSDataMap",
            &SContextType::ConstructSDataMap,
            (bp::arg("self"),
             bp::arg("omGeoMap"),
             bp::arg("sDataMapSym")
            ),
            "Constructs a new spatial data map and sets it to the context.       \n"
            "                                                                    \n"
            ":param omGeoMap: The I3OMGeoMap object for which the spatial data   \n"
            "    map will be valid.                                              \n"
            "                                                                    \n"
            ":param sDataMapSym: The OMKey pair symmetry of the spatial data map.\n"
            "                                                                    \n"
            ".. note:                                                            \n"
            "                                                                    \n"
            "    An already constructed spatial data map will get destroyed      \n"
            "    when calling this method!                                       \n"
            "                                                                    \n");
    }
};

//==============================================================================

}/*sttools*/
//##############################################################################

#endif//STTOOLS_I3PYSCONTEXT_H_INCLUDED
