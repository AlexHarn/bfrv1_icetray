#
# $Id$
#
i3_project(lilliput
	PYTHON_DIR python
	DOCS_DIR resources/docs)

ADD_DEFINITIONS(-DIPDF_I3MODULES)
# if you want all 5 hole ice models, then also add: -DALL_HOLE_ICE_MODELS
# note that compiling and linking then requires a lot of RAM (several GiB)

# define the lists used to build the basic library
SET(LIB_${PROJECT_NAME}_SOURCEFILES
	private/minimizer/I3MinimizerUtils.cxx
	private/minimizer/I3GSLSimplex.cxx
	private/minimizer/I3GSLMultiMin.cxx
	private/minimizer/DSimulatedAnnealing.cxx
	private/minimizer/I3GulliverAnnealing.cxx

	private/parametrization/I3SimpleParametrization.cxx
	private/parametrization/I3HalfSphereParametrization.cxx

	private/seedservice/I3BasicSeedService.cxx
	private/seedservice/I3BasicSeedServiceFactory.cxx
)
SET(LIB_${PROJECT_NAME}_TEST_SOURCEFILES
	private/test/main.cxx
	private/test/ParametrizationTest.cxx
	private/test/HalfSphereTest.cxx
	private/test/SeedTest.cxx
	private/test/MinimizerTest.cxx
	private/test/BoundedParameterTest.cxx
)
SET(LIB_${PROJECT_NAME}_TEST_SCRIPTS
	resources/test/test_segments.py
	resources/test/test_scipymin.py
)
SET(LIB_${PROJECT_NAME}_PROJECTS serialization icetray phys-services gulliver dataclasses)
SET(LIB_${PROJECT_NAME}_TOOLS gsl boost)
SET(LIB_${PROJECT_NAME}_LINK_LIBRARIES)

# add likelihoods if ipdf is installed
if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/ipdf)
	LIST(APPEND LIB_${PROJECT_NAME}_PROJECTS ipdf photonics-service)
	LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
		private/likelihood/I3GulliverIPDFPandelFactory.cxx
	)
	LIST(APPEND LIB_${PROJECT_NAME}_TEST_SOURCEFILES
		private/test/PhotoSplineTest.cxx
		private/test/PSSTestModule.cxx
		private/test/TestFrame.cxx
		private/test/PandelTest.cxx
		private/test/pandel_track.c
	)
	MESSAGE(STATUS "     + ipdf support")
else(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/ipdf)
	MESSAGE(STATUS "     - ipdf support: not found")
endif(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/ipdf)

# add Minuit minimization algorithm (local copy, less noisy)
LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
	private/minimizer/I3GulliverMinuit.cxx
	private/minimizer/minuit/TMinuit.cxx
)

# add Minuit2 minimization algorithm
if(MINUIT2_FOUND)
	LIST(APPEND LIB_${PROJECT_NAME}_TOOLS minuit2)

	if(ROOT_FOUND)
		LIST(APPEND LIB_${PROJECT_NAME}_TOOLS root)
	endif(ROOT_FOUND)

	LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
		private/minimizer/I3GulliverMinuit2.cxx
	)
	LIST(APPEND LIB_${PROJECT_NAME}_TEST_SCRIPTS
		resources/test/test_minuit2_migrad.py
	)
	ADD_DEFINITIONS(-DUSE_MINUIT2)
	MESSAGE(STATUS "     + Minuit2 support")
else(MINUIT2_FOUND)
	MESSAGE(STATUS "     - Minuit2 support...not found")
endif(MINUIT2_FOUND)

# add L-BFGS-B if we're feeling masochistic
if(LAPACK_FOUND)
	LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
		private/minimizer/lbfgsb/lbfgsb.c
		private/minimizer/lbfgsb/linpack.c
		private/minimizer/I3GulliverLBFGSB.cxx
	)
	ADD_DEFINITIONS(-DUSE_LBFGSB)
endif(LAPACK_FOUND)

# add MultiNest minimization algorithm from Fortran (nothing to do with the reco. "MultiNest")
if(MULTINEST_FOUND)
	MESSAGE(STATUS "     + multinest support")
	LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
		private/minimizer/I3GulliverMN.cxx
	)
	LIST(APPEND LIB_${PROJECT_NAME}_TOOLS
		multinest
	)
	ADD_DEFINITIONS(-DUSE_MULTINEST)
else(MULTINEST_FOUND)
	MESSAGE(STATUS "     - multinest support...not found")
endif(MULTINEST_FOUND)

if(NLOPT_FOUND)
	MESSAGE(STATUS "     + NLopt support")
	LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
		private/minimizer/I3GulliverNLopt.cxx
	)
	LIST(APPEND LIB_${PROJECT_NAME}_TOOLS
		nlopt
	)
	ADD_DEFINITIONS(-DUSE_NLOPT)
else(NLOPT_FOUND)
	MESSAGE(STATUS "     - NLopt support...not found")
endif(NLOPT_FOUND)

i3_add_library(lilliput
	${LIB_${PROJECT_NAME}_SOURCEFILES}

	USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS}
	USE_PROJECTS ${LIB_${PROJECT_NAME}_PROJECTS}
	LINK_LIBRARIES ${LIB_${PROJECT_NAME}_LINK_LIBRARIES}
)

i3_add_pybindings(lilliput
	private/pybindings/parametrization.cxx
	private/pybindings/minimizer.cxx
	private/pybindings/seedservice.cxx
	private/pybindings/module.cxx

	USE_TOOLS boost python
	USE_PROJECTS icetray dataclasses gulliver lilliput
)

i3_test_executable(test
	${LIB_${PROJECT_NAME}_TEST_SOURCEFILES}
	USE_PROJECTS lilliput
)

i3_test_scripts(${LIB_${PROJECT_NAME}_TEST_SCRIPTS})
