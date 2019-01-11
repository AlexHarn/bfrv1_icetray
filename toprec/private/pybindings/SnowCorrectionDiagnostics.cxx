/**
 *  $Id$
 *  
 *  Copyright (C) 2014
 *  Katherine Rawlins  <krawlins@uaa.alaska.edu>
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

#include <tableio/converter/pybindings.h>
#include "../toprec/converter/SnowCorrectionDiagnosticsConverter.h"

void register_SnowCorrectionDiagnostics()
{
  using namespace boost::python;

  class_<SnowCorrectionDiagnostics, bases<I3FrameObject>, SnowCorrectionDiagnosticsPtr>("SnowCorrectionDiagnostics")
    // The stage of shower evolution                                                                               
    .def_readwrite("tstage", &SnowCorrectionDiagnostics::tstage)
    .def_readwrite("tstage_restricted", &SnowCorrectionDiagnostics::tstage_restricted)
    
    // Fraction of signal which is EM                                                                              
    .def_readwrite("fEM_30m", &SnowCorrectionDiagnostics::fEM_30m)
    .def_readwrite("fEM_50m", &SnowCorrectionDiagnostics::fEM_50m)
    .def_readwrite("fEM_80m", &SnowCorrectionDiagnostics::fEM_80m)
    .def_readwrite("fEM_100m", &SnowCorrectionDiagnostics::fEM_100m)
    .def_readwrite("fEM_125m", &SnowCorrectionDiagnostics::fEM_125m)
    .def_readwrite("fEM_150m", &SnowCorrectionDiagnostics::fEM_150m)
    .def_readwrite("fEM_200m", &SnowCorrectionDiagnostics::fEM_200m)
    .def_readwrite("fEM_300m", &SnowCorrectionDiagnostics::fEM_300m)
    .def_readwrite("fEM_500m", &SnowCorrectionDiagnostics::fEM_500m)
    .def_readwrite("fEM_1000m", &SnowCorrectionDiagnostics::fEM_1000m)

    // Effective lambda's: EM only                                                                                 
    .def_readwrite("lambda_EM_30m", &SnowCorrectionDiagnostics::lambda_EM_30m)
    .def_readwrite("lambda_EM_50m", &SnowCorrectionDiagnostics::lambda_EM_50m)
    .def_readwrite("lambda_EM_80m", &SnowCorrectionDiagnostics::lambda_EM_80m)
    .def_readwrite("lambda_EM_100m", &SnowCorrectionDiagnostics::lambda_EM_100m)
    .def_readwrite("lambda_EM_125m", &SnowCorrectionDiagnostics::lambda_EM_125m)
    .def_readwrite("lambda_EM_150m", &SnowCorrectionDiagnostics::lambda_EM_150m)
    .def_readwrite("lambda_EM_200m", &SnowCorrectionDiagnostics::lambda_EM_200m)
    .def_readwrite("lambda_EM_300m", &SnowCorrectionDiagnostics::lambda_EM_300m)
    .def_readwrite("lambda_EM_500m", &SnowCorrectionDiagnostics::lambda_EM_500m)
    .def_readwrite("lambda_EM_1000m", &SnowCorrectionDiagnostics::lambda_EM_1000m)
    .def_readwrite("lambda_EM_30m_restricted", &SnowCorrectionDiagnostics::lambda_EM_30m_restricted)
    .def_readwrite("lambda_EM_50m_restricted", &SnowCorrectionDiagnostics::lambda_EM_50m_restricted)
    .def_readwrite("lambda_EM_80m_restricted", &SnowCorrectionDiagnostics::lambda_EM_80m_restricted)
    .def_readwrite("lambda_EM_100m_restricted", &SnowCorrectionDiagnostics::lambda_EM_100m_restricted)
    .def_readwrite("lambda_EM_125m_restricted", &SnowCorrectionDiagnostics::lambda_EM_125m_restricted)
    .def_readwrite("lambda_EM_150m_restricted", &SnowCorrectionDiagnostics::lambda_EM_150m_restricted)
    .def_readwrite("lambda_EM_200m_restricted", &SnowCorrectionDiagnostics::lambda_EM_200m_restricted)
    .def_readwrite("lambda_EM_300m_restricted", &SnowCorrectionDiagnostics::lambda_EM_300m_restricted)
    .def_readwrite("lambda_EM_500m_restricted", &SnowCorrectionDiagnostics::lambda_EM_500m_restricted)
    .def_readwrite("lambda_EM_1000m_restricted", &SnowCorrectionDiagnostics::lambda_EM_1000m_restricted)
    
  /* not yet
    .def_readwrite("lambda_eff_30m", &SnowCorrectionDiagnostics::lambda_eff_30m)
    .def_readwrite("lambda_eff_50m", &SnowCorrectionDiagnostics::lambda_eff_50m)
    .def_readwrite("lambda_eff_80m", &SnowCorrectionDiagnostics::lambda_eff_80m)
    .def_readwrite("lambda_eff_100m", &SnowCorrectionDiagnostics::lambda_eff_100m)
    .def_readwrite("lambda_eff_125m", &SnowCorrectionDiagnostics::lambda_eff_125m)
    .def_readwrite("lambda_eff_150m", &SnowCorrectionDiagnostics::lambda_eff_150m)
    .def_readwrite("lambda_eff_200m", &SnowCorrectionDiagnostics::lambda_eff_200m)
    .def_readwrite("lambda_eff_300m", &SnowCorrectionDiagnostics::lambda_eff_300m)
    .def_readwrite("lambda_eff_500m", &SnowCorrectionDiagnostics::lambda_eff_500m)
    .def_readwrite("lambda_eff_1000m", &SnowCorrectionDiagnostics::lambda_eff_1000m);
  */

  // Snow depths over key tanks:
  .def_readwrite("snowdepth_39B", &SnowCorrectionDiagnostics::snowdepth_39B)
  .def_readwrite("snowdepth_44A", &SnowCorrectionDiagnostics::snowdepth_44A)
  .def_readwrite("snowdepth_59A", &SnowCorrectionDiagnostics::snowdepth_59A)
  .def_readwrite("snowdepth_74A", &SnowCorrectionDiagnostics::snowdepth_74A);

  register_pointer_conversions<SnowCorrectionDiagnostics>();
  
  I3CONVERTER_NAMESPACE(toprec);
  I3CONVERTER_EXPORT_DEFAULT(SnowCorrectionDiagnosticsConverter, "Dumps SnowCorrectionDiagnostics parameter objects");
}
