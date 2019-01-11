//
//   Copyright (c) 2004, 2005, 2006, 2007   Troy D. Straszheim  
//   
//   $Id$
//
//   This file is part of IceTray.
//
//   IceTray is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 3 of the License, or
//   (at your option) any later version.
//
//   IceTray is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <neutrino-generator/utils/ZenithSampler.h>
#include <string>
#include <vector>

namespace bp = boost::python;

// dummy class just used as a namespace.
namespace {
  struct dummy { };
}

void register_ZenithSampler()
{
  using namespace ZenithSampler;

  bp::def("simple_slope_sampler", &SimpleSlopeSampler,(bp::arg("alpha"), bp::arg("min"), bp::arg("max"), bp::arg("random")), "sampling coszen with simple slope weight");

  bp::class_<FlatZenithEmulator, 
             boost::shared_ptr<FlatZenithEmulator>, 
             boost::noncopyable >
         ("FlatZenithEmulator", bp::init<>())
    .def("sampling", &FlatZenithEmulator::Sampling, (bp::arg("random")),"sampling coszen with emulation mode of flat-zenith distribution")
    .def("initialize", &FlatZenithEmulator::Initialize, (bp::arg("mincos"), bp::arg("maxcos")), "initializing functions")
    ;

  bp::class_<dummy>("ZenithSampler")
  ;
}

