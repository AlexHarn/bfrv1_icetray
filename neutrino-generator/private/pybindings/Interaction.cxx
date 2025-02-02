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

#include <neutrino-generator/utils/EnumTypeDefs.h>
#include <neutrino-generator/Steering.h>
#include <phys-services/I3RandomService.h>
#include <neutrino-generator/interactions/InteractionBase.h>
#include <neutrino-generator/interactions/InteractionCC.h>
#include <neutrino-generator/interactions/InteractionNC.h>
#include <neutrino-generator/interactions/InteractionGR.h>
#include <neutrino-generator/interactions/InteractionCCDifferential.h>
#include <neutrino-generator/interactions/InteractionNCDifferential.h>
#include <neutrino-generator/interactions/TauDecay.h>
#include <string>
#include <vector>

namespace bp = boost::python;



void register_InteractionBase()
{

  bp::class_<nugen::InteractionBase, boost::shared_ptr<nugen::InteractionBase>,  boost::noncopyable >("InteractionBase", bp::no_init)
    .def("initialize_crossection_table", &nugen::InteractionBase::InitializeCrosssectionTable, 
         (bp::arg("filename")), "initialize crosssection table")
    .def("initialize_finalstate_table", &nugen::InteractionBase::InitializeFinalStateTable, 
         (bp::arg("filename")),"initialize finalstate table")
    .def("select_xy", &nugen::InteractionBase::SelectXY, (bp::arg("log10ene")), "Select X and Y") 
    .def("good_xy", &nugen::InteractionBase::GetGoodXY, (bp::arg("log10ene"), bp::arg("target_mass"),bp::arg("lepton_mass"), bp::arg("lepton_ptype"), bp::arg("x"), bp::arg("y"), bp::arg("ntrials")), "Get X and Y and Check XY range") 
    .def("xsec_cgs", &nugen::InteractionBase::GetXsecCGS, 
         (bp::arg("ene")), "cross section in cm^2")
    .def("calc_outgoing_costheta_simple", 
         &nugen::InteractionBase::CalcOutgoingCosThetaSimple, 
         (bp::arg("nu_ene"), bp::arg("x"), bp::arg("y")),"Calc outgoing costheta with a simple form") 
    .def("calc_outgoing_theta", 
         &nugen::InteractionBase::CalcOutgoingTheta, 
         (bp::arg("nu_ene"), bp::arg("out_ptype"), bp::arg("x"), bp::arg("y"), bp::arg("lepton_theta"), bp::arg("hadron_theta")),
          "Calc outgoing theta with taking into account of lepton mass") 
    ;
}

void register_InteractionCC()
{
  bp::class_<nugen::InteractionCC, boost::shared_ptr<nugen::InteractionCC>, bp::bases<nugen::InteractionBase>, boost::noncopyable >
         ("InteractionCC", bp::init<boost::shared_ptr<I3RandomService>, boost::shared_ptr<nugen::Steering> >())
     ;
}

void register_InteractionNC()
{
  bp::class_<nugen::InteractionNC, boost::shared_ptr<nugen::InteractionNC>, bp::bases<nugen::InteractionBase>, boost::noncopyable >
         ("InteractionNC", bp::init<boost::shared_ptr<I3RandomService>, boost::shared_ptr<nugen::Steering> >())
     ;
}

void register_InteractionGR()
{
  bp::class_<nugen::InteractionGR, boost::shared_ptr<nugen::InteractionGR>, bp::bases<nugen::InteractionBase>, boost::noncopyable >
         ("InteractionGR", bp::init<boost::shared_ptr<I3RandomService>, boost::shared_ptr<nugen::Steering> >())

    .def("xsec_cgs", &nugen::InteractionGR::GetXsecCGS, 
         (bp::arg("ene")), "cross section in cm^2")
    .def("debug_print", &nugen::InteractionGR::DebugPrint, (bp::arg("fname"),bp::arg("n")), "debug print")
    ;
}

void register_InteractionCCDifferential()
{
  bp::class_<nugen::InteractionCCDifferential, boost::shared_ptr<nugen::InteractionCCDifferential>, bp::bases<nugen::InteractionCC>, boost::noncopyable >
         ("InteractionCCDifferential", bp::init<boost::shared_ptr<I3RandomService>, boost::shared_ptr<nugen::Steering> >())

    .def("initialize_i3cross_section_tables", &nugen::InteractionCCDifferential::InitializeI3CrossSectionTables,
         (bp::arg("dxs_filename"),bp::arg("sigma_filename")), "initiallize differential cross section")
    .def("xsec_cgs", &nugen::InteractionCCDifferential::GetXsecCGS, 
         (bp::arg("ene")), "cross section in cm^2")
    ;
}

void register_InteractionNCDifferential()
{
  bp::class_<nugen::InteractionNCDifferential, boost::shared_ptr<nugen::InteractionNCDifferential>, bp::bases<nugen::InteractionNC>, boost::noncopyable >
         ("InteractionNCDifferential", bp::init<boost::shared_ptr<I3RandomService>, boost::shared_ptr<nugen::Steering> >())

    .def("initialize_i3cross_section_tables", &nugen::InteractionNCDifferential::InitializeI3CrossSectionTables,
         (bp::arg("dxs_filename"),bp::arg("sigma_filename")), "initiallize differential cross section")
    .def("xsec_cgs", &nugen::InteractionNCDifferential::GetXsecCGS, 
         (bp::arg("ene")), "cross section in cm^2")
    ;
}
