//
//   Copyright (c) 2010
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

#include <icetray/I3FrameObject.h>
#include <icetray/load_project.h>

using namespace boost::python;
namespace bp = boost::python;
#include <boost/preprocessor.hpp>

#include "vuvuzela/VuvuzelaFunctions.h"

//
//  Add the class you are registering to this list of parenthesized typenames.
//  Don't forget to watch that the newlines are backslashed.
//  To register class Nick, add (Nick) to the list below, add
//  Nick.cxx to the list of i3_add_library out in CMakeLists.txt,
//  and create a file Nick.cxx that contains a function 
//    void register_Nick();
//  that does the boost.python registration for class Nick.
//
#define I3_REGISTRATION_FN_DECL(r, data, t) void BOOST_PP_CAT(register_,t)();
#define I3_REGISTER(r, data, t) BOOST_PP_CAT(register_,t)();

#if __cplusplus >= 201103L
namespace {

std::vector<double>
generate_noise(
        const double thermal_rate,
        const double rate, 
        const double hits,
        const double mean, 
        const double sigma,
        const double length, 
        I3RandomServicePtr rng) {

    std::set<double> buffer;
    std::vector<double> out;
    
    MakeThermalHits(rng, buffer, 0, thermal_rate, 0, length);
    MakeNonThermalHits(rng, buffer, 0, rate, hits, mean, sigma, 0, length, true);
    
    std::copy_if(buffer.begin(), buffer.end(), std::back_inserter(out), [length](double t) { return t >= 0 && t < length; });
    
    return out;
}

}
#endif

static void register_VuvuzelaFunctions()
{
#if __cplusplus >= 201103L
    def("generate_noise", &generate_noise, (bp::arg("thermal_rate"), "rate", "hits", "mean", "sigma", "length", "rng"));
#endif
}


I3_PYTHON_MODULE(vuvuzela)
{
  load_project("vuvuzela", false);
  
  register_VuvuzelaFunctions();
}


