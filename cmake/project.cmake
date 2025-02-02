# -*- tab-width: 8; indent-tabs-mode: t -*- ex: ts=8 noet: 
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
#
# rootcint() handles root dictionary generation
#
if(NOT ROOT_FOUND OR NOT USE_CINT)
  macro(ROOTCINT)
  endmacro(ROOTCINT)
else()
  macro(ROOTCINT TARGET)
    parse_arguments(ARG
      "LINKDEF;SOURCES;INCLUDE_DIRECTORIES;USE_TOOLS;USE_PROJECTS"
      ""
      ${ARGN}
      )

    get_directory_property(incdirs INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
    foreach(dir ${incdirs})
      list(APPEND ROOTCINT_INCLUDE_FLAGS -I${dir})
    endforeach(dir ${ARG_INCLUDE_DIRECTORIES})

    foreach(TOOL_ ${ARG_USE_TOOLS})
      string(TOUPPER ${TOOL_} TOOL)
      foreach(PATH_ ${${TOOL}_INCLUDE_DIR})
	list(APPEND ROOTCINT_INCLUDE_FLAGS "-I${PATH_}")
      endforeach(PATH_ ${${TOOL}_INCLUDE_DIR})
    endforeach(TOOL_ ${ARG_USE_TOOLS})

    foreach(PROJECT ${ARG_USE_PROJECTS})
      list(APPEND ROOTCINT_INCLUDE_FLAGS -I${CMAKE_SOURCE_DIR}/${PROJECT}/public)
    endforeach(PROJECT ${ARG_USE_PROJECTS})

    set(ROOTCINT_HEADERS "")
    set(ROOTINTERNAL_HEADERS "")
    foreach(header ${ARG_SOURCES})
      if(EXISTS ${ROOT_INCLUDE_DIR}/${header})
	# If this is a ROOT header, don't add to dependencies or
	# rootcint flags as root adds these automagically.
	list(APPEND ROOTINTERNAL_HEADERS ${header})
      elseif(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${header})
	# if it isn't in our project (and isn't a root header), it
	# isn't legal here, the other project needs to build the dict.
	# I guess.
	message("In ${CMAKE_CURRENT_SOURCE_DIR}:")
	message(FATAL_ERROR "Header '${header}' passed to rootcint does not exist")
      else()
	# okay, it exists in our project, add it to our commandline
	# and be dependent on it.
	list(APPEND ROOTCINT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${header})
      endif()
    endforeach(header ${ARG_SOURCES})

    add_custom_command(
      OUTPUT ${TARGET}
      DEPENDS ${ARG_LINKDEF} ${ROOTCINT_HEADERS}
      COMMAND ${CMAKE_BINARY_DIR}/env-shell.sh
      # rootcint found and ROOTSYS set in env-shell.sh path
      ARGS rootcint -f ${TARGET} -c -DI3_USE_ROOT -DI3_USE_CINT ${ROOTCINT_INCLUDE_FLAGS} -p ${I3_UBER_HEADER} ${ROOTCINT_HEADERS} ${ROOTINTERNAL_HEADERS} ${ARG_LINKDEF}
      COMMENT "Generating ${TARGET} with rootcint"
      VERBATIM
      )
  ENDMACRO(ROOTCINT)
ENDIF()

#
# use_projects() helper macro for, uh, using projects.
#
macro(use_projects THIS_TARGET)
  parse_arguments(${THIS_TARGET}_USE_PROJECTS
    "PROJECTS"
    ""
    ${ARGN}
    )
  foreach(USED_PROJECT ${${THIS_TARGET}_USE_PROJECTS_PROJECTS})
    if(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${USED_PROJECT})
      message(FATAL_ERROR "Attempt to use nonexistent project '${USED_PROJECT}'")
    endif(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${USED_PROJECT})
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${USED_PROJECT}/CMakeLists.txt)
      message(FATAL_ERROR "Attempt to use project '${USED_PROJECT}'. There is a directory but no CMakeLists.txt... don't know what to do.")
    endif(NOT EXISTS ${CMAKE_SOURCE_DIR}/${USED_PROJECT}/CMakeLists.txt)


    include_directories(${CMAKE_SOURCE_DIR}/${USED_PROJECT}/public)
    target_link_libraries(${THIS_TARGET} ${USED_PROJECT})
  endforeach(USED_PROJECT ${${THIS_TARGET}_USE_PROJECTS_PROJECTS})
endmacro(use_projects THIS_TARGET)

#
# use_pybindings() helper macro for, uh, using pybindings.
#
macro(use_pybindings THIS_TARGET)
  parse_arguments(${THIS_TARGET}_USE_PROJECTS
    "PROJECTS"
    ""
    ${ARGN}
    )
  foreach(USED_PROJECT ${${THIS_TARGET}_USE_PROJECTS_PROJECTS})
    if(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${USED_PROJECT})
      message(FATAL_ERROR "Attempt to use nonexistent project '${USED_PROJECT}'")
    endif(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${USED_PROJECT})
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${USED_PROJECT}/CMakeLists.txt)
      message(FATAL_ERROR "Attempt to use project '${USED_PROJECT}'. There is a directory but no CMakeLists.txt... don't know what to do.")
    endif(NOT EXISTS ${CMAKE_SOURCE_DIR}/${USED_PROJECT}/CMakeLists.txt)

    string(REPLACE "-" "_" PYBINDING_TARGET ${USED_PROJECT})
    string(CONCAT PYBINDING_TARGET ${PYBINDING_TARGET} "-pybindings")
    add_dependencies(${THIS_TARGET} ${PYBINDING_TARGET})
  endforeach(USED_PROJECT ${${THIS_TARGET}_USE_PROJECTS_PROJECTS})
endmacro(use_pybindings THIS_TARGET)

#
#  i3_add_library
#
macro(i3_add_library THIS_LIB_NAME)
  if (BUILD_${I3_PROJECT})
    #
    # Grrr...  this *_ARGS variable has to be unique to the project, otherwise
    # if you have two instances of i3_add_library the second will include the
    # parsed arg values from the first.
    #
    parse_arguments(${THIS_LIB_NAME}_ARGS
      "USE_TOOLS;USE_PROJECTS;ROOTCINT;INSTALL_DESTINATION;LINK_LIBRARIES;COMPILE_FLAGS"
      "NOT_INSPECTABLE;MODULE;EXCLUDE_FROM_ALL;WITHOUT_I3_HEADERS;NO_DOXYGEN;IWYU;PYBIND11"
      ${ARGN}
      )

    include_directories(
      ${PROJECT_SOURCE_DIR}/public
      ${PROJECT_SOURCE_DIR}/private
      )

    no_dotfile_glob(${THIS_LIB_NAME}_ARGS_SOURCES ${${THIS_LIB_NAME}_ARGS_DEFAULT_ARGS})

    if(${THIS_LIB_NAME}_ARGS_ROOTCINT)
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h)
	set(LINKDEF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h)
      else()
	set(LINKDEF_FILE "NOTFOUND")
      endif()
    endif(${THIS_LIB_NAME}_ARGS_ROOTCINT)

    if (LINKDEF_FILE AND ${THIS_LIB_NAME}_ARGS_ROOTCINT AND USE_CINT)

      set (DICTIONARY_SOURCEFILE
	${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${THIS_LIB_NAME}Dict.cxx)

      set (${THIS_LIB_NAME}_ARGS_SOURCES ${${THIS_LIB_NAME}_ARGS_SOURCES} ${DICTIONARY_SOURCEFILE})

      rootcint(${DICTIONARY_SOURCEFILE}
	LINKDEF ${LINKDEF_FILE}
	SOURCES ${${THIS_LIB_NAME}_ARGS_ROOTCINT}
	USE_PROJECTS ${${THIS_LIB_NAME}_ARGS_USE_PROJECTS} ${THIS_LIB_NAME}
	USE_TOOLS    ${${THIS_LIB_NAME}_ARGS_USE_TOOLS}
	)

    endif (LINKDEF_FILE AND ${THIS_LIB_NAME}_ARGS_ROOTCINT AND USE_CINT)

    set (ARGS)
    if (${THIS_LIB_NAME}_ARGS_EXCLUDE_FROM_ALL)
       set(ARGS EXCLUDE_FROM_ALL)
    endif (${THIS_LIB_NAME}_ARGS_EXCLUDE_FROM_ALL)
    if (${THIS_LIB_NAME}_ARGS_MODULE)
       set(ARGS ${ARGS} MODULE)
    endif (${THIS_LIB_NAME}_ARGS_MODULE)


    add_library(${THIS_LIB_NAME} ${ARGS} ${${THIS_LIB_NAME}_ARGS_SOURCES})
    add_dependencies(${THIS_LIB_NAME} env-check)

    set_target_properties(${THIS_LIB_NAME}
      PROPERTIES
      COMPILE_DEFINITIONS PROJECT=${PROJECT_NAME}
      )

    if(${THIS_LIB_NAME}_ARGS_IWYU AND USE_IWYU)
      set_target_properties(${THIS_LIB_NAME}
        PROPERTIES
        CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PROGRAM}
        )
    endif()

    add_custom_command(TARGET ${THIS_LIB_NAME}
      PRE_LINK
      COMMAND mkdir -p ${LIBRARY_OUTPUT_PATH}
      )

    # Disabled all special linker flags for APPLE:
    #  - single_module: this is the default anyway
    #  - undefined dynamic_lookup: it seems not to hurt letting the
    #      linker throw an error for undefined symbols.
    #  - flat_namespace: not using the two-level namespace (library+symbol name)
    #      seems to introduce bugs in exception handling with boost::python.
    #
    #if(APPLE)
    #  set_target_properties(${THIS_LIB_NAME}
    #  PROPERTIES
    #    LINK_FLAGS "-single_module -undefined dynamic_lookup -flat_namespace"
    #    )
    #endif(APPLE)

    if(NOT ${THIS_LIB_NAME}_ARGS_WITHOUT_I3_HEADERS)
      set_target_properties(${THIS_LIB_NAME}
	PROPERTIES
	COMPILE_FLAGS "-include ${I3_UBER_HEADER}"
	)
    endif(NOT ${THIS_LIB_NAME}_ARGS_WITHOUT_I3_HEADERS)
    if(${THIS_LIB_NAME}_ARGS_COMPILE_FLAGS)
      set_target_properties(${THIS_LIB_NAME}
	PROPERTIES
	COMPILE_FLAGS ${${THIS_LIB_NAME}_ARGS_COMPILE_FLAGS}
	)
    endif(${THIS_LIB_NAME}_ARGS_COMPILE_FLAGS)

    if (${THIS_LIB_NAME}_ARGS_LINK_LIBRARIES)
      target_link_libraries(${THIS_LIB_NAME} ${${THIS_LIB_NAME}_ARGS_LINK_LIBRARIES})
    endif (${THIS_LIB_NAME}_ARGS_LINK_LIBRARIES)

    #
    # set "inspectable" flag for use by icetray-inspect docs generator
    #
    if(${THIS_LIB_NAME}_ARGS_NOT_INSPECTABLE)
      set_target_properties(${THIS_LIB_NAME}
	PROPERTIES
	INSPECTABLE FALSE)
    else(${THIS_LIB_NAME}_ARGS_NOT_INSPECTABLE)
      set_target_properties(${THIS_LIB_NAME}
	PROPERTIES
	INSPECTABLE TRUE)
    endif(${THIS_LIB_NAME}_ARGS_NOT_INSPECTABLE)

    use_projects(${THIS_LIB_NAME}
      PROJECTS "${${THIS_LIB_NAME}_ARGS_USE_PROJECTS}"
      )
    set(${THIS_LIB_NAME}_PROJECT_DEPENDS ${${THIS_LIB_NAME}_ARGS_USE_PROJECTS} CACHE INTERNAL "Projects needed by library ${THIS_LIB_NAME}")

    use_tools(${THIS_LIB_NAME}
      TOOLS "${${THIS_LIB_NAME}_ARGS_USE_TOOLS}"
      )

    if(NOT ${THIS_LIB_NAME}_ARGS_EXCLUDE_FROM_ALL)
      if(${THIS_LIB_NAME}_ARGS_INSTALL_DESTINATION)
        install(TARGETS ${THIS_LIB_NAME} DESTINATION ${${THIS_LIB_NAME}_ARGS_INSTALL_DESTINATION})
      else(${THIS_LIB_NAME}_ARGS_INSTALL_DESTINATION)
        install(TARGETS ${THIS_LIB_NAME} DESTINATION lib)
      endif(${THIS_LIB_NAME}_ARGS_INSTALL_DESTINATION)
    endif(NOT ${THIS_LIB_NAME}_ARGS_EXCLUDE_FROM_ALL)

    configure_file(
      ${CMAKE_SOURCE_DIR}/cmake/doxyfile.in
      ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/doxyfile
      @ONLY
      )

    if(NOT ${THIS_LIB_NAME}_ARGS_NO_DOXYGEN AND DOXYGEN_FOUND)

      add_custom_target(${PROJECT_NAME}-${THIS_LIB_NAME}-doxygen
	COMMAND mkdir -p ${DOXYGEN_OUTPUT_PATH}/${PROJECT_NAME}
	COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/doxyfile
	)
      add_dependencies(${PROJECT_NAME}-doxygen
	${PROJECT_NAME}-${THIS_LIB_NAME}-doxygen
	)
      add_dependencies(doxygen ${PROJECT_NAME}-doxygen)
      foreach(l ${${THIS_LIB_NAME}_PROJECT_DEPENDS})
	file(APPEND ${DOXYGEN_OUTPUT_PATH}/.tagfiles/${PROJECT_NAME}.include "${DOXYGEN_OUTPUT_PATH}/.tagfiles/${l}.tag=../${l}\n")
	add_dependencies(${PROJECT_NAME}-${THIS_LIB_NAME}-doxygen ${l}-doxygen)
      endforeach()

    endif(NOT ${THIS_LIB_NAME}_ARGS_NO_DOXYGEN AND DOXYGEN_FOUND)

    if(XSLTPROC_BIN)
      
      if(${THIS_LIB_NAME}_ARGS_NOT_INSPECTABLE)

        file(WRITE ${CMAKE_BINARY_DIR}/docs/no_inspect/${THIS_LIB_NAME} "")
	
      else(${THIS_LIB_NAME}_ARGS_NOT_INSPECTABLE)
    
        set(XML_TMP ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${THIS_LIB_NAME}-inspection.xml)
        set(HTML_OUTPUT ${CMAKE_BINARY_DIR}/docs/inspect/${THIS_LIB_NAME}.html)
        set(RST_OUTPUT ${SPHINX_DIR}/source/inspect/${THIS_LIB_NAME}.rst)
        add_custom_target(${PROJECT_NAME}-${THIS_LIB_NAME}-inspect
        	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/docs/inspect
    	  	COMMAND ${CMAKE_BINARY_DIR}/env-shell.sh ${EXECUTABLE_OUTPUT_PATH}/icetray-inspect ${THIS_LIB_NAME} --xml -o ${XML_TMP}
		COMMAND ${XSLTPROC_BIN} ${CMAKE_SOURCE_DIR}/icetray/resources/inspect2html.xsl ${XML_TMP} > ${HTML_OUTPUT}
	
		COMMAND mkdir -p ${SPHINX_DIR}/source/inspect/
		COMMAND ${CMAKE_BINARY_DIR}/env-shell.sh
		${EXECUTABLE_OUTPUT_PATH}/icetray-inspect ${THIS_LIB_NAME}
		--sphinx --subsection-headers --sphinx-functions
		--verbose-docs
		#--expand-segments
		--title=""
		-o ${RST_OUTPUT}
		COMMENT "Generating rst from icetray-inspect of ${THIS_LIB_NAME}"
		)

        add_dependencies(inspect
	  ${PROJECT_NAME}-${THIS_LIB_NAME}-inspect
	  )
	  
      endif(${THIS_LIB_NAME}_ARGS_NOT_INSPECTABLE)

    endif(XSLTPROC_BIN)	



    if(DPKG_INSTALL_PREFIX)
      set_target_properties(${THIS_LIB_NAME}
	PROPERTIES
	INSTALL_RPATH_USE_LINK_PATH TRUE
	)

    endif(DPKG_INSTALL_PREFIX)

  endif(BUILD_${I3_PROJECT})
endmacro(i3_add_library)

#
# i3_project(ARG)
#
# Sets the name of the project to ARG, and adds BUILD_<ARG>
#
macro(i3_project PROJECT_NAME)
  project(${PROJECT_NAME})
  parse_arguments(ARG
    "PYTHON_DIR;PYTHON_DEST;DOCS_DIR"
    "USE_SETUPTOOLS"
    ${ARGN}
    )

  string(TOUPPER "${PROJECT_NAME}" I3_PROJECT)
  string(TOUPPER "BUILD_${I3_PROJECT}" BUILD_PROJECT_OPTION)
  option(${BUILD_PROJECT_OPTION}
    "Build project I3.${PROJECT_NAME} (prefer using make targets, not this, for building individual libs)"
    ON)

  if(BUILD_${I3_PROJECT})

    add_custom_target(${PROJECT_NAME}-doxygen)

    #i3_add_testing_targets()

    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources)
      install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/
	DESTINATION ${PROJECT_NAME}/resources/
	PATTERN ".svn" EXCLUDE
	PATTERN "*.py"
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
    endif (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources)

    if(ARG_DOCS_DIR)
      file(MAKE_DIRECTORY "${SPHINX_DIR}/source/projects")
      execute_process(COMMAND ln -fsn
      	${CMAKE_CURRENT_SOURCE_DIR}/${ARG_DOCS_DIR}
        ${SPHINX_DIR}/source/projects/${PROJECT_NAME})
    endif(ARG_DOCS_DIR)

    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/public/${PROJECT_NAME} AND INSTALL_HEADERS)
      install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/public/${PROJECT_NAME}
	DESTINATION include
	PATTERN ".svn" EXCLUDE)
    endif (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/public/${PROJECT_NAME} AND INSTALL_HEADERS)

    if(ARG_PYTHON_DIR AND IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PYTHON_DIR})
      if(ARG_USE_SETUPTOOLS)
	colormsg(GREEN "+-- python [setuptools]")

	#
	# do the 'setup.py develop'
	#
	execute_process(COMMAND
	  /usr/bin/env PYTHONPATH=${LIBRARY_OUTPUT_PATH}:$ENV{PYTHONPATH}
	  ${PYTHON_EXECUTABLE} setup.py -q develop
	  --install-dir ${LIBRARY_OUTPUT_PATH}
	  --script-dir  ${EXECUTABLE_OUTPUT_PATH}
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	  )

	#
	# Install targets
	#
	add_custom_target(${PROJECT_NAME}-install-to-tarball
	  /usr/bin/env PYTHONPATH=${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/lib:$ENV{PYTHONPATH}
	  ${PYTHON_EXECUTABLE} setup.py install
	  --install-lib ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/lib
	  --install-scripts  ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/bin
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	  )
	add_dependencies(${PROJECT_NAME}-install-to-tarball
	  tarball-install)
	add_dependencies(tarball-finish
	  ${PROJECT_NAME}-install-to-tarball)

      else(ARG_USE_SETUPTOOLS)
	if (COPY_PYTHON_DIR)
	  colormsg(GREEN "+-- python [directory copy]")
	else (COPY_PYTHON_DIR)
	  colormsg(GREEN "+-- python [symlinks]")
	endif (COPY_PYTHON_DIR)

	if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PYTHON_DIR}/__init__.py)
	  message(FATAL_ERROR
	    "Project ${PROJECT_NAME} has PYTHON_DIR specified, but the directory contains no file '__init__.py' and will be useless")
	endif (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PYTHON_DIR}/__init__.py)

	if(NOT ARG_PYTHON_DEST)
	  set(ARG_PYTHON_DEST icecube/${PROJECT_NAME})
	  string(REPLACE "-" "_" ARG_PYTHON_DEST "icecube/${PROJECT_NAME}")
	endif(NOT ARG_PYTHON_DEST)

	#
	#  Just bare python, no setuptools
	#
	if (COPY_PYTHON_DIR)
	  file(GLOB_RECURSE python_components RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_PYTHON_DIR}/*.py)
	  foreach(file ${python_components})
            string(REPLACE ${ARG_PYTHON_DIR}/ "" file ${file})
            configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PYTHON_DIR}/${file} ${CMAKE_BINARY_DIR}/lib/${ARG_PYTHON_DEST}/${file} COPYONLY)
          endforeach()
        else (COPY_PYTHON_DIR)
	  execute_process(COMMAND ln -fsn ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PYTHON_DIR} ${CMAKE_BINARY_DIR}/lib/${ARG_PYTHON_DEST})
	  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PYTHON_DIR}/
	    DESTINATION lib/${ARG_PYTHON_DEST}
	    PATTERN ".svn" EXCLUDE)
	  execute_process(COMMAND python -m compileall -fq ${CMAKE_BINARY_DIR}/lib/${ARG_PYTHON_DEST} OUTPUT_QUIET)
	endif (COPY_PYTHON_DIR)
      endif(ARG_USE_SETUPTOOLS)

    endif(ARG_PYTHON_DIR AND IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PYTHON_DIR})

  endif(BUILD_${I3_PROJECT})
endmacro(i3_project PROJECT_NAME)

macro(i3_executable_script THIS_EXECUTABLE_NAME THIS_SCRIPT_NAME)
    if(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_NO_PREFIX)
      set(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME
	${THIS_EXECUTABLE_NAME})
    else(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_NO_PREFIX)
      set(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME
	${PROJECT_NAME}-${THIS_EXECUTABLE_NAME})
    endif(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_NO_PREFIX)

    # copy it for local use
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${THIS_SCRIPT_NAME}
        ${EXECUTABLE_OUTPUT_PATH}/${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
        COPYONLY)
    # and install it in the tarball when the time comes
    install(PROGRAMS ${THIS_SCRIPT_NAME} DESTINATION bin
        RENAME ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME})
endmacro(i3_executable_script THIS_EXECUTABLE_NAME THIS_SCRIPT_NAME)

macro(i3_executable THIS_EXECUTABLE_NAME)
  if(BUILD_${I3_PROJECT})
    parse_arguments(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}
      "USE_TOOLS;USE_PROJECTS;LINK_LIBRARIES"
      "NO_PREFIX;WITHOUT_I3_HEADERS;IWYU"
      ${ARGN}
      )
    no_dotfile_glob(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_SOURCES
      ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_DEFAULT_ARGS})

    if(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_NO_PREFIX)
      set(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME
	${THIS_EXECUTABLE_NAME})
    else(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_NO_PREFIX)
      set(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME
	${PROJECT_NAME}-${THIS_EXECUTABLE_NAME})
    endif(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_NO_PREFIX)

    add_executable(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
      ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_SOURCES})

    if(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_WITHOUT_I3_HEADERS)
      message(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_WITHOUT_I3_HEADERS)
      set(THIS_I3H_FLAGS "")
    else()
      set(THIS_I3H_FLAGS "-include ${I3_UBER_HEADER}")
    endif()

    add_dependencies(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME} env-check)

    use_projects(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
      PROJECTS ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_USE_PROJECTS})
    use_tools(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
      TOOLS "${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_USE_TOOLS}"
      )

    if(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_LINK_LIBRARIES)
    target_link_libraries(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
      ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_LINK_LIBRARIES})
    endif(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_LINK_LIBRARIES)

    ## FIXME: temporarily force pthread linking
    target_compile_options(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
      BEFORE PUBLIC "-pthread")
    set_target_properties(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
      PROPERTIES LINK_FLAGS "-pthread")
    ## :FIXME

    set_target_properties(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
      PROPERTIES
      COMPILE_FLAGS "${THIS_I3H_FLAGS} -DPROJECT=${PROJECT_NAME}"
      )

    if(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_IWYU AND USE_IWYU)
      set_target_properties(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
        PROPERTIES
        CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PROGRAM}
        )
    endif()

    if(APPLE)
      set_target_properties(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
	PROPERTIES
	LINK_FLAGS "-bind_at_load -multiply_defined suppress"
	)
    endif(APPLE)
    install(TARGETS ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME} RUNTIME DESTINATION bin)

    if(DPKG_INSTALL_PREFIX)
      set_target_properties(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_TARGET_NAME}
	PROPERTIES
	INSTALL_RPATH_USE_LINK_PATH TRUE
	)
    endif(DPKG_INSTALL_PREFIX)

  endif(BUILD_${I3_PROJECT})
endmacro(i3_executable THIS_EXECUTABLE_NAME)

macro(i3_test_executable THIS_EXECUTABLE_NAME)
  if (BUILD_${I3_PROJECT})
    add_test(NAME "${PROJECT_NAME}::${THIS_EXECUTABLE_NAME}" #::${testable_file}/${unittest}"
             WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
             COMMAND ${PROJECT_NAME}-${THIS_EXECUTABLE_NAME} -t 1100 -saf)

    parse_arguments(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}
      "USE_TOOLS;USE_PROJECTS;USE_PYBINDINGS;LINK_LIBRARIES"
      ""
      ${ARGN}
      )
    no_dotfile_glob(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_SOURCES
      ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_DEFAULT_ARGS}
      )

    add_executable(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
      EXCLUDE_FROM_ALL
      ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_SOURCES}
      )

    add_dependencies(test-bins ${PROJECT_NAME}-${THIS_EXECUTABLE_NAME})
    use_pybindings(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME})

    ## FIXME: temporarily force pthread linking
    target_compile_options(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
      BEFORE PUBLIC "-pthread")
    set_target_properties(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
      PROPERTIES LINK_FLAGS "-pthread")
    ## :FIXME

    set_target_properties(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
      PROPERTIES
      COMPILE_DEFINITIONS PROJECT=${PROJECT_NAME}
      )
    if(APPLE)
      set_target_properties(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
	PROPERTIES
	LINK_FLAGS "-bind_at_load -multiply_defined suppress"
	)
    endif(APPLE)

    use_projects(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
      PROJECTS ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_USE_PROJECTS})

    use_pybindings(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
      PROJECTS ${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_USE_PYBINDINGS})

    use_tools(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
      TOOLS "${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_USE_TOOLS}")

    set_source_files_properties(${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_SOURCES}
      PROPERTIES
      COMPILE_FLAGS "-include ${I3_UBER_HEADER}"
      )

    if(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_LINK_LIBRARIES)
      target_link_libraries(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
	${${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_LINK_LIBRARIES})
    endif(${PROJECT_NAME}_${THIS_EXECUTABLE_NAME}_LINK_LIBRARIES)

    if(DPKG_INSTALL_PREFIX)
      set_target_properties(${PROJECT_NAME}-${THIS_EXECUTABLE_NAME}
	PROPERTIES
	INSTALL_RPATH_USE_LINK_PATH TRUE
	)
    endif(DPKG_INSTALL_PREFIX)
  endif ()
endmacro(i3_test_executable THIS_EXECUTABLE_NAME)

#
#  Python bindings macro
#
option(BUILD_PYBINDINGS "Build python bindings" ON)

#
#  Magic __init__.py needed by everybody
#
configure_file(${CMAKE_SOURCE_DIR}/cmake/__init__.py.in
  ${LIBRARY_OUTPUT_PATH}/icecube/__init__.py
  COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/load_pybindings.py.in
  ${LIBRARY_OUTPUT_PATH}/icecube/load_pybindings.py
  @ONLY)
install(FILES ${LIBRARY_OUTPUT_PATH}/icecube/__init__.py
  ${LIBRARY_OUTPUT_PATH}/icecube/load_pybindings.py
  DESTINATION lib/icecube
  )

macro(i3_add_pybindings MODULENAME)
  if (BUILD_${I3_PROJECT})
    #
    # In many places these if() guards are *outside* the call to i3_add_pybindings.
    # this is so you can use these projects with older i3-cmakes that do not yet
    # have this macro
    #

    parse_arguments(${MODULENAME}_ARGS
        "USE_PROJECTS;USE_TOOLS;LINK_LIBRARIES"
        "IWYU;PYBIND11"
      ${ARGN}
      )

    #
    # NO_DOXYGEN is added here, because otherwise, upper level doxygen gets clobbered
    #
    if (${MODULENAME}_ARGS_PYBIND11)
      colormsg(GREEN "+-- ${MODULENAME}-pybindings w/ pybind11")

      i3_add_library(${MODULENAME}-pybindings ${ARGN}
        USE_TOOLS python
        INSTALL_DESTINATION lib/icecube
        NOT_INSPECTABLE NO_DOXYGEN
        MODULE
        )
      include_directories(${CMAKE_BINARY_DIR}/pybind11/include)

      if(CMAKE_VERSION VERSION_LESS 3.1.0)
	colormsg(YELLOW "+-- You need to download pybind11 from GitHub with")
	colormsg(YELLOW "+--   wget https://github.com/pybind/pybind11/archive/master.zip")
	colormsg(YELLOW "+-- or")
	colormsg(YELLOW "+--   curl -OL https://github.com/pybind/pybind11/archive/master.zip")
	colormsg(YELLOW "+-- and unpack it in ${CMAKE_BINARY_DIR} before running 'make' with:")
	colormsg(YELLOW "+--   unzip master.zip && mv pybind11-master pybind11")
      else()
	if(NOT TARGET pybind11)
	  ## if it's not already there, download pybind11 from github.
	  ## this shouldn't be necessary but cmake in CVMFS doesn't have
	  ## SSL support. not sure if it's worth adding for py2-v2.
	  if(NOT EXISTS ${CMAKE_BINARY_DIR}/master.zip)
	    find_program(WGET_EXECUTABLE wget)
	    find_program(CURL_EXECUTABLE curl)
	    if(WGET_EXECUTABLE)
	      execute_process(COMMAND ${WGET_EXECUTABLE} https://github.com/pybind/pybind11/archive/master.zip)
	    elseif(CURL_EXECUTABLE)
	      execute_process(COMMAND ${CURL_EXECUTABLE} -OL https://github.com/pybind/pybind11/archive/master.zip)
	    else()
	      colormsg(YELLOW "+-- You need to download pybind11 from GitHub from https://github.com/pybind/pybind11/archive/master.zip")
	      colormsg(YELLOW "+-- and place it in ${CMAKE_BINARY_DIR} before running 'make'")
	    endif()
	  endif()
	  include(ExternalProject)
	  ExternalProject_Add(pybind11 EXCLUDE_FROM_ALL 1
	    URL ${CMAKE_BINARY_DIR}/master.zip
	    DOWNLOAD_NO_PROGRESS 1
	    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
	    SOURCE_DIR ${CMAKE_BINARY_DIR}/pybind11
	    PREFIX ${CMAKE_BINARY_DIR}
	    CONFIGURE_COMMAND ""
	    PATCH_COMMAND ""
	    BUILD_COMMAND ""
	    INSTALL_COMMAND ""
	    #GIT_REPOSITORY https://github.com/pybind/pybind11.git
	    )
	endif()
	add_dependencies(${MODULENAME}-pybindings pybind11)
	## this is the wrong place to set C++11 support, but works for now 
	## until pybind11 is a first-class tool.
	set_target_properties(${MODULENAME}-pybindings PROPERTIES COMPILE_FLAGS "-std=gnu++11")
      endif()

      set_target_properties(${MODULENAME}-pybindings
        PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${MODULENAME}
        LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH}/icecube
        )
    else()
      colormsg(GREEN "+-- ${MODULENAME}-pybindings")

      i3_add_library(${MODULENAME}-pybindings ${ARGN}
        LINK_LIBRARIES ${BOOST_PYTHON}
        INSTALL_DESTINATION lib/icecube
        NOT_INSPECTABLE NO_DOXYGEN
        MODULE
        )

      set_target_properties(${MODULENAME}-pybindings
        PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${MODULENAME}
        DEFINE_SYMBOL I3_PYBINDINGS_MODULE
        COMPILE_FLAGS "-include ${I3_UBER_HEADER}"
        LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH}/icecube
        )
    endif()

    add_custom_command(TARGET ${MODULENAME}-pybindings
      PRE_LINK
      COMMAND mkdir -p ${CMAKE_BINARY_DIR}/lib/icecube
      )

    if(${MODULENAME}_ARGS_IWYU AND USE_IWYU)
      set_target_properties(${MODULENAME}
        PROPERTIES
        CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PROGRAM}
        )
    endif()

    add_dependencies(pybindings ${MODULENAME}-pybindings)
    use_pybindings("${MODULENAME}-pybindings"
      PROJECTS "${${MODULENAME}_ARGS_USE_PROJECTS}"
      )
    
    # Disabled special linker flags for APPLE:
    #  - undefined dynamic_lookup: it seems not to hurt letting the
    #      linker throw an error for undefined symbols.
    #  - flat_namespace: not using the two-level namespace (library+symbol name)
    #      seems to introduce bugs in exception handling with boost::python.
    if(APPLE)
      set_target_properties(${MODULENAME}-pybindings
        PROPERTIES
        LINK_FLAGS "-bundle"
        #used to be here: -flat_namespace -undefined dynamic_lookup -multiply_defined suppress
        )
    endif(APPLE)

  endif ()
endmacro(i3_add_pybindings)


#
#  Generates testing targets for scripts
#
macro(i3_test_scripts)
  no_dotfile_glob(${PROJECT_NAME}_SCRIPTS ${ARGN})
  sort(${PROJECT_NAME}_SCRIPTS_ALPHABETICAL "${${PROJECT_NAME}_SCRIPTS}")
  foreach(script ${${PROJECT_NAME}_SCRIPTS_ALPHABETICAL})
    get_filename_component(script_basename ${script} NAME)
    if(${script} MATCHES "\\.py$")
      add_test(NAME ${PROJECT_NAME}::${script_basename}
               WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
               COMMAND ${PYTHON_EXECUTABLE} ${script})
    else()
      add_test(NAME ${PROJECT_NAME}::${script_basename}
               WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
               COMMAND ${script})
    endif()
  endforeach()
endmacro(i3_test_scripts)

macro(project_moved_to NEWLOCATION)
  get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)
  message(STATUS "************************************************************")
  message(STATUS "***")
  message(STATUS "***   Subversion for project ${PROJECT_NAME} has moved:")
  message(STATUS "***")
  message(STATUS "***   The new location is ")
  message(STATUS "***   ${NEWLOCATION}")
  message(STATUS "***")
  message(STATUS "***   Please update your svn:externals accordingly.")
  message(STATUS "***")
  message(STATUS "************************************************************")
  message(FATAL_ERROR "Repository moved.")
endmacro(project_moved_to PROJECT NEWLOCATION)
