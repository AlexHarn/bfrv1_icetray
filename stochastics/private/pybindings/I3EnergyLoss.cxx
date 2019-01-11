/**
 *  $Id$
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

#include <icetray/load_project.h>
#include <stochastics/I3EnergyLoss.h>
#include <tableio/converter/pybindings.h>
#include <stochastics/converter/I3EnergyLossConverter.h>
#include <icetray/python/dataclass_suite.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(stochastics)
{
  class_<I3EnergyLoss, bases<I3FrameObject>, I3EnergyLossPtr>("I3EnergyLoss")
    .def(dataclass_suite<I3EnergyLoss>())
    .def_readwrite("eLoss_1000", &I3EnergyLoss::Eloss_1000)
    .def_readwrite("eLoss_1500", &I3EnergyLoss::Eloss_1500)
    .def_readwrite("eLoss_1600", &I3EnergyLoss::Eloss_1600)
    .def_readwrite("eLoss_1700", &I3EnergyLoss::Eloss_1700)
    .def_readwrite("eLoss_1800", &I3EnergyLoss::Eloss_1800)
    .def_readwrite("eLoss_1900", &I3EnergyLoss::Eloss_1900)
    .def_readwrite("eLoss_2000", &I3EnergyLoss::Eloss_2000)
    .def_readwrite("eLoss_2100", &I3EnergyLoss::Eloss_2100)
    .def_readwrite("eLoss_2200", &I3EnergyLoss::Eloss_2200)
    .def_readwrite("eLoss_2300", &I3EnergyLoss::Eloss_2300)
    .def_readwrite("eLoss_2400", &I3EnergyLoss::Eloss_2400)
    .def_readwrite("eLoss_3000", &I3EnergyLoss::Eloss_3000)

    .def_readwrite("primMassEstimate", &I3EnergyLoss::primMassEstimate)
    .def_readwrite("primEnergyEstimate", &I3EnergyLoss::primEnergyEstimate)
    .def_readwrite("primMassEstimate_err", &I3EnergyLoss::primMassEstimate_err)
    .def_readwrite("primEnergyEstimate_err", &I3EnergyLoss::primEnergyEstimate_err)

    .def_readwrite("nHEstoch", &I3EnergyLoss::nHEstoch)
    .def_readwrite("avStochEnergy", &I3EnergyLoss::avStochEnergy)
    .def_readwrite("avRelStochEnergy", &I3EnergyLoss::avRelStochEnergy)
    .def_readwrite("highestStochEnergy", &I3EnergyLoss::highestStochEnergy)
    .def_readwrite("highestRelStochEnergy", &I3EnergyLoss::highestRelStochEnergy)
    .def_readwrite("totalStochEnergy", &I3EnergyLoss::totalStochEnergy)
    .def_readwrite("totalRelStochEnergy", &I3EnergyLoss::totalRelStochEnergy)
    .def_readwrite("chi2", &I3EnergyLoss::chi2)
    .def_readwrite("chi2_red", &I3EnergyLoss::chi2_red)
    .def_readwrite("avStochDepth", &I3EnergyLoss::avStochDepth)
    .def_readwrite("status", &I3EnergyLoss::status)
    ;

  register_pointer_conversions<I3EnergyLoss>();    
  I3CONVERTER_NAMESPACE(stochastics);
  I3CONVERTER_EXPORT_DEFAULT(I3EnergyLossConverter,
			     "Dumps I3EnergyLoss parameter objects");
}
