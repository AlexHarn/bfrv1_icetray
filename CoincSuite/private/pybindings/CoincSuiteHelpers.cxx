//
//   Copyright (c) 2014   Marcel ZOll and the IceCube Collaboration 
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

#include <boost/python.hpp>

#include "CoincSuite/lib/CoincSuiteHelpers.h"
#include "dataclasses/I3TimeWindow.h"

double TimeSeparation_double (const double startA,
                              const double endA,
                              const double startB,
                              const double endB)
{
  return CoincSuite::TimeSeparation<double>(startA, endA, startB, endB);
};

double TimeSeparation_i3tw (const I3TimeWindow &twA,
                            const I3TimeWindow &twB)
{
  return CoincSuite::TimeSeparation<double>(twA.GetStart(), twA.GetStop(), twB.GetStart(), twB.GetStop());
};


namespace bp = boost::python;

void register_CoincSuiteHelpers() {

  bp::def("frame_id_string", &CoincSuite::FrameIDstring,
          (bp::arg("frame")),
          "Gives a nicely formated identification string for a frame");

  bp::def("time_separation_doubles", &TimeSeparation_double,
          (bp::arg("startA"), bp::arg("endA"), bp::arg("startB"), bp::arg("endB")),
           "the time-separation of two time intervals; specified as [startA,endA] and [startB,endB];"
           "Returns: (NAN) full inclusion,"
           "(negative value) partial inclusion by so many units," 
           "(positive value) separated by that many units");
          

  bp::def("time_separation_tw", &TimeSeparation_i3tw,
          (bp::arg("twA"), bp::arg("twB")),
           "the time-separation of two time intervals; specified as timewindndowA and timewindowB;"
           "Returns: (NAN) full inclusion,"
           "(negative value) partial inclusion by so many units," 
           "(positive value) separated by that many units");
          
          
  bp::def("GetMaskAncestry", &CoincSuite::GetMaskAncestry,
          (bp::arg("frame"), bp::arg("key")),
          "get a complete list of the maps that this possible mask found at 'key' derives from.");

  bp::def("GetCommonMaskAncestry", &CoincSuite::GetCommonMaskAncestry,
          (bp::arg("frame"), bp::arg("keyA"), bp::arg("keyB")),
          "get a complete list of ancestors which are common to these two possible masks.");
  
  bp::def("UniteRecoMaps", &CoincSuite::UniteRecoMaps,
          (bp::arg("mapA"), bp::arg("mapB")),
          "A convenience function to unite two RecoMaps into a single one");
  
  bp::def("UniteTriggerHierarchies", &CoincSuite::UniteTriggerHierarchies,
          (bp::arg("trigA"), bp::arg("trigB")),
          "A convenience function to unite two TriggererHierarchies into a single one.");
};

