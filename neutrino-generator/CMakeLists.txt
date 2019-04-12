
i3_project(neutrino-generator
	PYTHON_DEST icecube/neutrino_generator
	PYTHON_DIR python
        DOCS_DIR resources/docs
        )

SET(LIB_${PROJECT_NAME}_SOURCEFILES
	private/neutrino-generator/Particle.cxx
	private/neutrino-generator/Steering.cxx
	private/neutrino-generator/interactions/InteractionBase.cxx
	private/neutrino-generator/interactions/InteractionCC.cxx
	private/neutrino-generator/interactions/InteractionCCDifferential.cxx
	private/neutrino-generator/interactions/InteractionNC.cxx
	private/neutrino-generator/interactions/InteractionNCDifferential.cxx
	private/neutrino-generator/interactions/InteractionGR.cxx
	private/neutrino-generator/interactions/TauDecay.cxx
	private/neutrino-generator/interactions/InteractionManagerBase.cxx
	private/neutrino-generator/interactions/NuGInteractionManager.cxx
	private/neutrino-generator/interactions/DifferentialUtils.cxx
	private/neutrino-generator/table-interface/FinalStateTableReader.cxx
	private/neutrino-generator/table-interface/CrosssectionTableReader.cxx
	private/neutrino-generator/table-interface/DecayStateTableReader.cxx
	private/neutrino-generator/table-interface/TableUtils.cxx
	private/neutrino-generator/utils/Calculator.cxx
	private/neutrino-generator/utils/Pickup.cxx
	private/neutrino-generator/utils/Utils.cxx
	private/neutrino-generator/utils/TreeUtils.cxx
	private/neutrino-generator/utils/EnumTypeDefs.cxx
	private/neutrino-generator/utils/Constants.cxx
	private/neutrino-generator/utils/ZenithSampler.cxx
	private/neutrino-generator/sources/I3NuGDiffuseSource.cxx
	private/neutrino-generator/sources/I3NuGPointSource.cxx
	private/neutrino-generator/sources/I3NuGSourceSelector.cxx
	private/neutrino-generator/sources/SourceUtils.cxx
	private/neutrino-generator/legacy/I3NeutrinoGeneratorBase.cxx
	private/neutrino-generator/legacy/I3NeutrinoGenerator.cxx
	private/neutrino-generator/legacy/I3NeutrinoPropagator.cxx
	private/neutrino-generator/legacy/I3NuGVEvent.cxx
	private/neutrino-generator/legacy/I3NuGEvent.cxx
	private/neutrino-generator/legacy/I3NuGWeightedEvent.cxx
	private/neutrino-generator/legacy/I3NuGInjector.cxx
	private/neutrino-generator/legacy/I3NuGInteractionInfo.cxx
	private/neutrino-generator/legacy/I3NuGInteractionInfoDifferential.cxx
	private/neutrino-generator/legacy/MemoryInfo.cxx
 )

SET(LIB_${PROJECT_NAME}_PROJECTS serialization icetray phys-services dataclasses simclasses sim-services earthmodel-service photospline)
SET(LIB_${PROJECT_NAME}_TOOLS python boost)
SET(LIB_${PROJECT_NAME}_EXTRA_LIBRARIES "")

## d/l cross section data to the nugen build directory
if(EXISTS /cvmfs/icecube.opensciencegrid.org/data/neutrino-generator/cross_section_data)
  colormsg(CYAN   "+-- /cvmfs/icecube.opensciencegrid.org/data/neutrino-generator/cross_section_data exists, skipping download")
elseif(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/cross_section_data)
  colormsg(CYAN   "+-- ${CMAKE_CURRENT_BINARY_DIR}/cross_section_data exists, skipping download")
else()
  colormsg(RED    "+-- ${CMAKE_CURRENT_BINARY_DIR}/cross_section_data does not exist.  Downloading...")
  colormsg(GREEN  "+-- Downloading and unpacking the crosssection data files...")
  execute_process(COMMAND 
    ${CMAKE_SOURCE_DIR}/neutrino-generator/resources/download_crosssection_data.sh ${CMAKE_CURRENT_BINARY_DIR})
  colormsg(GREEN "+-- ...download complete.")
endif()


if(ROOT_FOUND AND USE_ROOT AND GENIE_FOUND AND EXISTS ${CMAKE_SOURCE_DIR}/genie-icetray)
	colormsg(GREEN "+-- genie-icetray found. building with GENIE support.")

        if (NOT LOG4CPP_FOUND)
          colormsg(YELLOW "+-- log4cpp not installed.")
        endif (NOT LOG4CPP_FOUND)
        if (NOT XML2_FOUND)
          colormsg(YELLOW "+-- libxml2 not installed.")
        endif (NOT XML2_FOUND)
        if (NOT ROOT_FOUND)
          colormsg(YELLOW "+-- ROOT not installed.")
        endif (NOT ROOT_FOUND)
        if (NOT GENIE_FOUND)
          colormsg(YELLOW "+-- genie not installed.")
        endif (NOT GENIE_FOUND)

	# so that c++ code can use "#idef HAS_GENIE" if needed
	# (do not use in public header files!)
	add_definitions(-DHAS_GENIE)

	# we need three extra ROOT libs
	find_library(ROOT_GEOM_LIB Geom PATHS ${ROOTSYS}/lib NO_DEFAULT_PATH)
	find_library(ROOT_EG_LIB EG PATHS ${ROOTSYS}/lib NO_DEFAULT_PATH)
	find_library(ROOT_EGPYTHIA6_LIB EGPythia6 PATHS ${ROOTSYS}/lib NO_DEFAULT_PATH)
	find_library(LHAPDF_LIB LHAPDF)
	find_library(PYTHIA_SO_LIB Pythia6)

	# let the code know where GENIE is located in order to be able to read some data files
	add_definitions(-DGENIE_PATH="${GENIE_PATH}" -DGENIE_GVERSION_H="${GENIE_GVERSION_H}")

	LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
		private/neutrino-generator/I3NuGGenie.cxx
	)

	LIST(APPEND LIB_${PROJECT_NAME}_PROJECTS
		genie-icetray
	)

	LIST(APPEND LIB_${PROJECT_NAME}_TOOLS
		root log4cpp xml2 genie
	)

	LIST(APPEND LIB_${PROJECT_NAME}_EXTRA_LIBRARIES
		${PYTHIA_SO_LIB} ${ROOT_GEOM_LIB} ${ROOT_EG_LIB} ${ROOT_EGPYTHIA6_LIB} ${LHAPDF_LIB}
	)
else(ROOT_FOUND AND USE_ROOT AND GENIE_FOUND AND EXISTS ${CMAKE_SOURCE_DIR}/genie-icetray)
	colormsg(YELLOW "+-- genie-icetray NOT found. building without GENIE support.")
endif(ROOT_FOUND AND USE_ROOT AND GENIE_FOUND AND EXISTS ${CMAKE_SOURCE_DIR}/genie-icetray)

i3_add_library(${PROJECT_NAME}
	${LIB_${PROJECT_NAME}_SOURCEFILES}
	USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS} 
	USE_PROJECTS ${LIB_${PROJECT_NAME}_PROJECTS}

	LINK_LIBRARIES ${LIB_${PROJECT_NAME}_EXTRA_LIBRARIES}
)

i3_add_pybindings(neutrino_generator
  private/pybindings/TypeDefs.cxx
  private/pybindings/Steering.cxx
  private/pybindings/Interaction.cxx
  private/pybindings/legacy.cxx
  private/pybindings/ZenithSampler.cxx
  private/pybindings/module.cxx
  USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS}
  USE_PROJECTS ${PROJECT_NAME} ${LIB_${PROJECT_NAME}_PROJECTS}
)

i3_test_executable(test
        private/test/I3NuGenTestModule.cxx
        private/test/I3NeutrinoGeneratorTest.cxx
	private/test/NuGInjectorTest.cxx
	private/test/NuGInteractionTest.cxx
	private/test/ParticleTest.cxx
	private/test/TestUtils.cxx
	private/test/UtilsTest.cxx
	private/test/CalculatorTest.cxx
	private/test/NuGInteractionInfoTest.cxx
	#private/test/ZenithTest.cxx
	private/test/main.cxx
	USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS} gsl 
	USE_PROJECTS ${LIB_${PROJECT_NAME}_PROJECTS} neutrino-generator

	LINK_LIBRARIES ${LIB_${PROJECT_NAME}_EXTRA_LIBRARIES}
)

i3_test_scripts(
       resources/scripts/load_test.py
)

set_tests_properties(neutrino-generator::test PROPERTIES LABELS RUN_LONG)
