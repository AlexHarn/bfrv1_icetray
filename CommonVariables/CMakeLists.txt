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
i3_project(CommonVariables
    PYTHON_DIR  python
    PYTHON_DEST icecube/common_variables
    DOCS_DIR    resources/docs
)

# Define the lists of core dependecies.
SET(LIB_${PROJECT_NAME}_USE_TOOLS_LIST
    boost
    python
)

# Use log4cplus tool when it is available (for backward-compatibility with older
# offline-software).
if(LOG4CPLUS_FOUND)
    list(APPEND LIB_${PROJECT_NAME}_USE_TOOLS_LIST log4cplus)
endif(LOG4CPLUS_FOUND)

SET(LIB_${PROJECT_NAME}_USE_PROJECTS_LIST
    serialization
    icetray
    dataclasses
    recclasses
    phys-services
    tableio
)

MESSAGE(STATUS "+-- Include \"direct_hits\" sub project.")
MESSAGE(STATUS "+-- Include \"hit_multiplicity\" sub project.")
MESSAGE(STATUS "+-- Include \"hit_statistics\" sub project.")
MESSAGE(STATUS "+-- Include \"track_characteristics\" sub project.")
MESSAGE(STATUS "+-- Include \"time_characteristics\" sub project.")


## Assemble the list of source files for the i3 library shared object file,
## depending on what projects are present.
#IF(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/gulliver)
#    LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
#        private/CommonVariables/reconstruction/calculator.cxx
#    )
#    LIST(APPEND LIB_${PROJECT_NAME}_USE_PROJECTS_LIST gulliver)
#    SET(BUILD_${PROJECT_NAME}__RECONSTRUCTION 1)
#    MESSAGE(STATUS "+-- Include \"reconstruction\" sub project.")
#ENDIF(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/gulliver)

i3_add_library(CommonVariables
    private/CommonVariables/direct_hits/calculator.cxx
    private/CommonVariables/direct_hits/I3DirectHitsDefinition.cxx
    private/CommonVariables/direct_hits/I3DirectHitsTimeWindow.cxx
    private/CommonVariables/hit_multiplicity/calculator.cxx
    private/CommonVariables/hit_statistics/calculator.cxx
    private/CommonVariables/track_characteristics/calculator.cxx
    private/CommonVariables/time_characteristics/calculator.cxx
    ${LIB_${PROJECT_NAME}_SOURCEFILES}

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS ${LIB_${PROJECT_NAME}_USE_PROJECTS_LIST}
)

# Add the python bindings.
i3_add_pybindings(common_variables
    private/pybindings/module.cxx

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS icetray CommonVariables
)
i3_add_pybindings(common_variables__hit_multiplicity
    private/pybindings/hit_multiplicity/module.cxx

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS icetray CommonVariables
)
i3_add_pybindings(common_variables__track_characteristics
    private/pybindings/track_characteristics/module.cxx

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS icetray CommonVariables
)
i3_add_pybindings(common_variables__hit_statistics
    private/pybindings/hit_statistics/module.cxx

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS icetray CommonVariables
)
i3_add_pybindings(common_variables__direct_hits
    private/pybindings/direct_hits/module.cxx

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS icetray CommonVariables
)
i3_add_pybindings(common_variables__time_characteristics
    private/pybindings/time_characteristics/module.cxx

    USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
    USE_PROJECTS icetray CommonVariables
)

i3_test_scripts(resources/tests/*.py)
