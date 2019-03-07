/**
 *  $Id: I3LineFitParams.cxx 142581 2016-02-27 04:04:33Z hdembinski $
 *  
 *  Copyright (C) 2008
 *  Troy D. Straszheim  <troy@icecube.umd.edu>
 *  and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *  
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *  
 */
#include <icetray/python/dataclass_suite.hpp>
#include "recclasses/I3LikelihoodEllipse.h"
#include <boost/python.hpp>

using namespace boost::python;




void register_I3LikelihoodEllipseCollection()
{
  I3LikelihoodEllipseCollection (I3LikelihoodEllipseCollection::*profile_single)(uint) = &I3LikelihoodEllipseCollection::Profile;
  I3LikelihoodEllipseCollection (I3LikelihoodEllipseCollection::*profile_multi)(std::vector<uint>) = &I3LikelihoodEllipseCollection::Profile;

  class_<I3LikelihoodEllipseCollection, bp::bases<I3FrameObject>, boost::shared_ptr<I3LikelihoodEllipseCollection> >("I3LikelihoodEllipseCollection", 
	 "A class to store a list of likelihood ellipses from MultiNest")
    
    .add_property("ellipses", &I3LikelihoodEllipseCollection::GetEllipses, &I3LikelihoodEllipseCollection::SetEllipses)
    .add_property("axis_names", &I3LikelihoodEllipseCollection::GetAxisNames)
    .def("profile", profile_single, "Get the collection profiled over the given dimension index")
    .def("profile", profile_multi, "Get the collection profiled over the given dimension indices")
    .def("contains", (bool(I3LikelihoodEllipseCollection::*)(const std::vector<double>&))&I3LikelihoodEllipseCollection::Contains,
	 "Is this point contained within any of the ellipses?")
    .def("prune", &I3LikelihoodEllipseCollection::Prune, (arg("max_delta_llh")=15.0), "Find the minimal set of ellipses to contain a set of points.")
    .def("__repr__", &I3LikelihoodEllipse::repr)
    .def("__len__", &I3LikelihoodEllipseCollection::GetSize)
    .def("__getitem__", &I3LikelihoodEllipseCollection::GetEllipse)
    .def("__setitem__", &I3LikelihoodEllipseCollection::SetEllipse)
    .def("__delitem__", &I3LikelihoodEllipseCollection::RemoveEllipse)
    .def("__contains__", &I3LikelihoodEllipseCollection::In)
    .def("append", &I3LikelihoodEllipseCollection::AddEllipse)
    .def("extend", &I3LikelihoodEllipseCollection::Extend)
    .def("clear", &I3LikelihoodEllipseCollection::Clear)
    .def(self == self)
    .def(self != self)
    .def(bp::dataclass_suite<I3LikelihoodEllipseCollection>())
    ;
  register_pointer_conversions<I3LikelihoodEllipseCollection>();
  
}
