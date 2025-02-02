/**
 *  $Id$
 *  
 *  Copyright (C) 2008
 *  Fabian Kislat  <fabian.kislat@desy.de>
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

void register_I3SnowCorrectionService();
void register_SnowCorrectionDiagnostics();

BOOST_PYTHON_MODULE(toprec)
{
  load_project("toprec", false);

  register_SnowCorrectionDiagnostics();
  register_I3SnowCorrectionService();
}
