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

void register_I3LikelihoodEllipse()
{
  // Simplify stuff by creating some function pointers
  I3LikelihoodEllipse (I3LikelihoodEllipse::*profile_single)(uint) = &I3LikelihoodEllipse::Profile;
  I3LikelihoodEllipse (I3LikelihoodEllipse::*profile_multi)(std::vector<uint>) = &I3LikelihoodEllipse::Profile;

  class_<I3LikelihoodEllipse,  bp::bases<I3FrameObject>, boost::shared_ptr<I3LikelihoodEllipse> >("I3LikelihoodEllipse", 
	 "A class to store a single likelihood ellipse from MultiNest")

    .def(init<std::vector<double>, I3Matrix, double, std::vector<std::string> >((
        arg("center"), arg("inverse_covariance"), arg("nllh"), arg("axis_names")), "Constructor for an ellipse given the raw values"))

    .def(init<const I3LikelihoodEllipse&>((arg("ellipse")), "Copy constructor for an ellipse"))
    
    .add_property("center", &I3LikelihoodEllipse::GetCenter, &I3LikelihoodEllipse::SetCenter)
    .add_property("inverse_covariance", &I3LikelihoodEllipse::GetInverseCovariance, &I3LikelihoodEllipse::SetInverseCovariance)
    .add_property("nllh", &I3LikelihoodEllipse::GetNLLH, &I3LikelihoodEllipse::SetNLLH)
    .add_property("axis_names", &I3LikelihoodEllipse::GetAxisNames, &I3LikelihoodEllipse::SetAxisNames)

    .def("__repr__", &I3LikelihoodEllipse::repr)
    .def("profile", profile_single, "Get the collection profiled over the given dimension index")
    .def("profile", profile_multi, "Get the collection profiled over the given dimension indices")
    .def("contains", (bool(I3LikelihoodEllipse::*)(const std::vector<double>&))&I3LikelihoodEllipse::Contains)
    .def(self == self)
    .def(self != self)
    .def(bp::dataclass_suite<I3LikelihoodEllipse>())
    ;
  register_pointer_conversions<I3LikelihoodEllipse>();
  
  def("ConvertToUnitVectors", &I3LikelihoodEllipse::ConvertToUnitVectors, (arg("azimuth"), arg("zenith"),
									   arg("nx"), arg("ny"), arg("nz")));

}


