if(SUITESPARSE_FOUND)
	i3_project(millipede
		DOCS_DIR resources/docs
		PYTHON_DIR python
	)

	i3_add_library(millipede
		private/millipede/MillipedeBase.cxx
		private/millipede/MillipedeDOMCacheMap.cxx
		private/millipede/MillipedeLikelihood.cxx
		private/millipede/MillipedeSolver.cxx
		private/millipede/MuMillipede.cxx
		private/millipede/TauMillipede.cxx
		private/millipede/PyMillipede.cxx
		private/millipede/Monopod.cxx
		private/millipede/MillipedeFisherMatrixCalculator.cxx
		private/millipede/MillipedeDataChecker.cxx
		private/millipede/pcg.c
		private/millipede/converter/MillipedeFitParamsConverter.cxx

		USE_TOOLS boost python suitesparse blas lapack gsl
		USE_PROJECTS serialization icetray dataclasses photonics-service
		    lilliput gulliver phys-services tableio
	)

	i3_test_executable(test
		private/test/main.cxx
		private/test/MillipedeTest.cxx
		USE_TOOLS boost python suitesparse blas lapack gsl
		USE_PROJECTS millipede dataio icetray
	)
	
	if(NUMPY_FOUND)
		i3_test_scripts(
			resources/test/monopod_test.py
			resources/test/monopod_test_solver.py
			resources/test/monopod_test_binning.py
			resources/test/monopod_test_gradient.py
			resources/test/millipede_test_nugen.py
			resources/test/pymillipede_test.py
		)
        set_tests_properties(millipede::monopod_test.py
        	millipede::monopod_test_gradient.py
            millipede::monopod_test_solver.py
	        PROPERTIES LABELS RUN_LONG
	    )
	else(NUMPY_FOUND)
		colormsg(CYAN "+-- numpy *not* found, skipping at least one test in ${PROJECT_NAME}")
	endif(NUMPY_FOUND)

	i3_add_pybindings(millipede
		private/pybindings/module.cxx
		USE_TOOLS boost python
		USE_PROJECTS millipede
	)
else(SUITESPACE_FOUND)
	colormsg(CYAN "+-- SuiteSparse not found. Skipping millipede.
       If you need it you can install SuiteSparse through I3Ports.")
endif(SUITESPARSE_FOUND)
