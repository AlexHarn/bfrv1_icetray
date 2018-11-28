#
#  $Id$
#  
#  Copyright (C) 2007   Troy D. Straszheim  <troy@icecube.umd.edu>
#  and the IceCube Collaboration <http://www.icecube.wisc.edu>
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

if(NOT SYSTEM_PACKAGES)
endif(NOT SYSTEM_PACKAGES)

colormsg("")
colormsg(HICYAN "Boost")
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
if (SYSTEM_PACKAGES)
  set(BASE_COMPONENTS system signals thread date_time filesystem program_options regex iostreams)
  # From the cmake docs:
  # Boost Python components require a Python version suffix (Boost 1.67 and later),
  # e.g. python36 or python27 for the versions built against Python 3.6 and 2.7, respectively.
  #
  # Make a first pass at finding boost, with no components specified, just to figure out
  # what version of boost we're dealing with.  We'll add the components later.
  find_package(Boost 1.38.0) # NB: 1.38.0 is the minimum version required.

  # If the first find failed, there's little hope of either of the next three searches
  # successfully finding boost, especially in the cases where Boost_VERSION* isn't set.
  # In that case the last find_package fails with a cryptic error message, since the
  # boost version you're passing is effectively '.' ...not helpful.
  # So, there's no point in continuing if boost isn't found.
  if (Boost_FOUND) 
    # Now that boost was found and we know which version, we can choose the correct
    # boost::python libraries that match the python version we're building against.
    if ((Boost_VERSION GREATER 106700) OR (Boost_VERSION EQUAL 106700))
      find_package(Boost ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION} EXACT COMPONENTS
                   python${PYTHON_STRIPPED_MAJOR_MINOR_VERSION} ${BASE_COMPONENTS})
    else()
      # Old boost, so find using the old method
      find_package(Boost ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION} COMPONENTS python ${BASE_COMPONENTS})
    endif()
  endif()
  
  if(NOT Boost_FOUND)
    colormsg(RED ${Boost_ERROR_REASON})
  endif ()
endif ()

if((NOT SYSTEM_PACKAGES) OR (NOT Boost_FOUND))
  set(BOOST_PORTSVERSION "1.38.0" CACHE PATH "The boost version.")
  set(BOOST_INCLUDEDIR ${I3_PORTS}/include/boost-${BOOST_PORTSVERSION})
  set(BOOST_LIBRARYDIR ${I3_PORTS}/lib/boost-${BOOST_PORTSVERSION})
  set(Boost_NO_SYSTEM_PATHS TRUE)
  find_package(Boost ${BOOST_PORTSVERSION} EXACT REQUIRED COMPONENTS python system signals thread date_time filesystem program_options regex iostreams)
endif((NOT SYSTEM_PACKAGES) OR (NOT Boost_FOUND))

if(Boost_FOUND)
  set(BOOST_FOUND TRUE CACHE BOOL "Boost found successfully")
  set(BOOST_INCLUDE_DIR ${Boost_INCLUDE_DIR} CACHE PATH "Path to the boost include directories.")
  set(BOOST_LIBRARIES ${Boost_LIBRARIES} CACHE PATH "Boost libraries")
  if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/cmake/tool-patches/boost-${BOOST_PORTSVERSION})
    include_directories(${CMAKE_SOURCE_DIR}/cmake/tool-patches/boost-${BOOST_PORTSVERSION})
  else()
    include_directories(${CMAKE_SOURCE_DIR}/cmake/tool-patches/boost-new)
  endif()

  if(${BUILDNAME} MATCHES "ARCH")
  foreach(lib ${BOOST_LIBRARIES})
    if(${PYTHON_LIBRARIES} MATCHES "libpython3.*\\.so" AND ${lib} MATCHES "(.*libboost_python)\\.so")
      list(FIND BOOST_LIBRARIES "${lib}" i)
      list(REMOVE_AT BOOST_LIBRARIES ${i})
      list(INSERT BOOST_LIBRARIES ${i} "${CMAKE_MATCH_1}3.so")
    endif()
  endforeach(lib ${BOOST_LIBRARIES})
  endif()
  foreach(lib ${BOOST_LIBRARIES})
    if(NOT ${lib} STREQUAL "optimized" AND NOT ${lib} STREQUAL "debug")
      add_custom_command(TARGET install_tool_libs
        PRE_BUILD
        COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib/tools
        COMMAND ${CMAKE_SOURCE_DIR}/cmake/install_shlib.py ${lib} ${CMAKE_INSTALL_PREFIX}/lib/tools) 
    endif(NOT ${lib} STREQUAL "optimized" AND NOT ${lib} STREQUAL "debug")
  endforeach(lib ${BOOST_LIBRARIES})
endif(Boost_FOUND)

