#  $Id$
#
#  Copyright (C) 2012
#  Martin Wolf <martin.wolf@icecube.wisc.edu>
#  and the IceCube Collaboration <http://www.icecube.wisc.edu>
#
#  version: $Revision$
#  date:    $Date$
#
#  This file is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#

i3_project(STTools
    DOCS_DIR resources/docs
    PYTHON_DIR python
    PYTHON_DEST icecube/STTools
)

# Define the lists of required tools.
SET(LIB_${PROJECT_NAME}_USE_TOOLS_LIST
    boost
    python
)

# Use log4cplus tool when it is available (for backward-compatibility with older
# offline-software).
if(LOG4CPLUS_FOUND)
    list(APPEND LIB_${PROJECT_NAME}_USE_TOOLS_LIST log4cplus)
endif(LOG4CPLUS_FOUND)

i3_add_library(STTools
    private/STTools/hit_info_getters.cxx
    private/STTools/I3FourVector.cxx
    private/STTools/I3RUsageTimer.cxx
    private/STTools/utilities.cxx
    private/STTools/algorithms/seededRT/I3SeededRTConfiguration.cxx
    private/STTools/algorithms/seededRT/I3SeededRTConfigurationService.cxx
    private/STTools/algorithms/seededRT/utilities.cxx
    private/STTools/modules/I3SeededRTCleaningModule.cxx
    private/STTools/modules/I3RTVetoModule.cxx

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS serialization icetray dataclasses recclasses
)

i3_add_pybindings(STTools
    private/pybindings/algorithms/seededRT/I3SeededRTConfiguration.cxx
    private/pybindings/algorithms/seededRT/I3SeededRTSContext.cxx
    private/pybindings/algorithms/seededRT/I3SeededRTConfigurationService.cxx
    private/pybindings/algorithms/seededRT/module.cxx
    private/pybindings/I3RUsageTimer.cxx
    private/pybindings/module.cxx

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS icetray dataclasses recclasses STTools
)

i3_test_executable(test
    private/test/main.cxx
    private/test/SeededRTCleaningTest.cxx
    USE_PROJECTS icetray dataclasses interfaces phys-services STTools recclasses
)
