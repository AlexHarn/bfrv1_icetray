//
//   Copyright (c) 2008   Troy D. Straszheim and the IceCube Collaboration 
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

using namespace boost::python;
namespace bp = boost::python;

#include "polyplopia/PolyplopiaUtils.h"
#include "icetray/load_project.h"

void register_I3MapStringString();
void register_I3GeneratorService();

I3Frame MergeFrames(I3Frame frame1, I3Frame frame2, I3Map<std::string,std::string> names, float delta_t)
{ 
    return PolyplopiaUtils::MergeFrames(frame1, frame2, names,delta_t);
}

BOOST_PYTHON_MODULE(polyplopia)
{
  load_project("polyplopia", false);
  import("icecube.icetray");

  // export stuff in the PolyplopiaUtils namespace  
  def("GetFirstHitTime", PolyplopiaUtils::GetFirstHitTime);
  def("MergeMMCInfo", PolyplopiaUtils::MergeMMCInfo);
  def("MergeMCTrees", PolyplopiaUtils::MergeMCTrees);
  def("CopyWeights", PolyplopiaUtils::CopyWeights);
  def("OffsetTime", PolyplopiaUtils::OffsetTime);
  def("MergeEvents", MergeFrames);
  
  register_I3MapStringString();
  register_I3GeneratorService();
}
