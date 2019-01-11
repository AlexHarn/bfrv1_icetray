/**
 * Copyright (C) 2014
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/pybindings/register_I3Vector_of.hpp
 * @date $Date$
 * @brief This file contains the definition if the register_I3Vector_of template
 *        function to expose vectors of I3FrameObjects to Python.
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
#ifndef STTOOLS_PY_REGISTER_I3VECTOR_OF_HPP_INCLUDED
#define STTOOLS_PY_REGISTER_I3VECTOR_OF_HPP_INCLUDED

#include <string>

#include <boost/shared_ptr.hpp>

#include <icetray/I3FrameObject.h>
#include <icetray/python/dataclass_suite.hpp>

#include <dataclasses/I3Vector.h>

namespace sttools {
namespace py {

template <typename T>
void
register_i3vector_of(const std::string& s)
{
    typedef I3Vector<T> vec_t;
    bp::class_<vec_t, bp::bases<I3FrameObject>, boost::shared_ptr<vec_t> > ((std::string("I3Vector") + s).c_str())
        .def(bp::dataclass_suite<vec_t>())
    ;
    register_pointer_conversions<vec_t>();
}

}// namespace py
}// namespace sttools

#endif // ! STTOOLS_PY_REGISTER_I3VECTOR_OF_HPP_INCLUDED
