/**
 *  $Id: ithon.cxx 43705 2008-03-26 20:54:18Z kjmeagher $
 *  
 *  Copyright (C) 2004, 2005, 2006, 2007
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


#include <boost/python.hpp>
#include <boost/preprocessor.hpp>

#include <string>
#include <iostream>
#include <iomanip>
#include <signal.h>

#include <icetray/I3Logging.h>
#include <icetray/I3Tray.h>
#include <icetray/OMKey.h>

using std::string;

// AddService: same as AddModule
I3Tray::param_setter (I3Tray::*AddServiceV)(const std::string& classname, 
					    const std::string& instancename)
  = &I3Tray::AddService;

//
// Manual enumeration of overloads for parameters.  For each type in
// I3_PARAM_TYPES a pointer to a member function with an appropriate
// signature is generated.  Each of these must be individually exposed
// to python via "def()"
//
typedef std::vector<int> ithon_vector_int;
typedef std::vector<double> ithon_vector_double;
typedef std::vector<std::string> ithon_vector_string;
typedef std::vector<OMKey> ithon_vector_omkey;

#define ITHON_I3_PARAM_TYPES			\
  (ithon_vector_int)				\
    (ithon_vector_double)			\
    (ithon_vector_string)			\
    (ithon_vector_omkey)			\
    (string)					\
    (double)					\
    (int)					\
    (int64_t)					\
    (OMKey)


#define I3_SETPARAM_OVERLOAD(r,data,t) \
  bool (I3Tray::*BOOST_PP_CAT(sp_,t) )(const std::string& something, const std::string& somethingelse, const t&) = &I3Tray::SetParameter;

BOOST_PP_SEQ_FOR_EACH(I3_SETPARAM_OVERLOAD, ~, ITHON_I3_PARAM_TYPES);

// this is used in the interface definition to expose these function
// to python
#define I3_SETPARAM_OVERLOAD_DEF(r,data,t) .def("SetParameter", BOOST_PP_CAT(sp_,t)) 


//
// Main python interface definition.  The name "ithon" must match
// the filename of the library that it is loaded from.  A C-linkage
// function named initithon() is generated and put in the library.
// When python loads a file name libX.so it calls a function named
// initX().  If it can't find it, it bails out.
//
using namespace boost::python;

static std::string I3TrayString(I3Tray &tray) {
  std::stringstream str;
  str << tray;
  return str.str();
}

std::string I3TrayRepr(I3Tray &tray) {
  std::stringstream str;
  BOOST_FOREACH(std::string service, tray.factories_in_order) {
    if (service == "__config") // Skip internal tray info service
      continue;
    I3ContextPtr context_p = tray.factory_contexts[service];
    const I3Configuration &config = context_p->Get<I3Configuration>();
    str << "AddService(";
    str << "'" << config.ClassName() << "', ";
    str << "'" << config.InstanceName() << "'";
    BOOST_FOREACH(std::string param, config.keys()) {
      std::string repr = boost::python::extract<std::string>(config.Get(param).attr("__repr__")());
      str << ", " << param << "=" << repr;
    }
    str << ")\n";
  }
  BOOST_FOREACH(std::string mod, tray.modules_in_order) {
    I3ContextPtr context_p = tray.module_contexts[mod];
    const I3Configuration &config = context_p->Get<I3Configuration>();
    str << "AddModule(";
    if (config.ClassName() == "PythonModule") {
      std::string pymod = boost::python::extract<std::string>(
        context_p->Get<boost::shared_ptr<boost::python::object> >
        ("class")->attr("__module__"));
      std::string pyname = boost::python::extract<std::string>(
        context_p->Get<boost::shared_ptr<boost::python::object> >
        ("class")->attr("__name__"));
      str << pymod << "." << pyname << ", ";
    } else if (config.ClassName() == "PythonFunction") {
      std::string repr = boost::python::extract<std::string>(
        context_p->Get<boost::shared_ptr<boost::python::object> >
        ("object")->attr("__repr__")());
      str << repr << ", ";
    } else {
      str << "'" << config.ClassName() << "', ";
    }
    str << "'" << config.InstanceName() << "'";
    BOOST_FOREACH(std::string param, config.keys()) {
      boost::python::object obj = config.Get(param);
      std::string repr = boost::python::extract<std::string>(config.Get(param).attr("__repr__")());
      str << ", " << param << "=" << repr;
    }
    str << ")\n";
  }
  return str.str();
}

void register_I3Tray()
{

  class_<I3Tray::param_setter>("param_setter", 
			       init<const I3Tray::param_setter&>());

  void (I3Tray::*Execute_0)(void)              = &I3Tray::Execute;
  void (I3Tray::*Execute_1)(unsigned)          = &I3Tray::Execute;

  class_<I3Tray, boost::noncopyable>("I3Tray")
    .def("Execute", Execute_0)
    .def("Execute", Execute_1)
    .def("Usage", &I3Tray::Usage)
    .def("Finish", &I3Tray::Finish)
    .def("Print", &I3Tray::Print)
    .def("__str__", &I3TrayString)
    .def("__repr__", &I3TrayRepr)
    .def("AddService", 
	 (I3Tray::param_setter (I3Tray::*)(const std::string&, const std::string&))
	 &I3Tray::AddService)
    .def("AddModule", 
	 (I3Tray::param_setter (I3Tray::*)(boost::python::object, const std::string&))
	 &I3Tray::AddModule)
    .def("MoveModule", &I3Tray::MoveModule, (arg("self"), arg("name"), arg("anchor"), arg("after")=false))
    .def("ConnectBoxes", &I3Tray::ConnectBoxes)
    
    // SetParameter exposure: BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS
    // does not work for some reason...  Compiler can't determine the
    // type of &I3Tray::SetParameter.  Dunno why.
    // .def("SetParameter", &I3Tray::SetParameter, SetParameterOverloads()) 

    // instead we do it manually with a little preprocessor
    // metaprogramming, and five-point-palm exploding heart
    // technique...
    .def("SetParameter", (bool (I3Tray::*)(const std::string&,
					   const std::string&,
					   const boost::python::object&))&I3Tray::SetParameter)
	 
    // BOOST_PP_SEQ_FOR_EACH(I3_SETPARAM_OVERLOAD_DEF, ~, ITHON_I3_PARAM_TYPES)
    ;
  
}


