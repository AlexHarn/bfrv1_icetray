#
# $Id$
#

i3_project(paraboloid	
    PYTHON_DIR python
    DOCS_DIR resources/docs)

i3_add_library(paraboloid
    private/paraboloid/I3ParaboloidFitParamsConverter.cxx
    private/paraboloid/I3ParaboloidFitParams.cxx
    private/paraboloid/I3ParaboloidFitter.cxx
    private/paraboloid/ParaboloidImpl.cxx
    USE_TOOLS boost python
    USE_PROJECTS serialization icetray dataclasses phys-services gulliver lilliput tableio
)

i3_add_pybindings(paraboloid
    private/pybindings/I3ParaboloidFitParams.cxx
    USE_TOOLS boost python
    USE_PROJECTS paraboloid 
)

i3_test_executable(test private/test/*.cxx
  USE_PROJECTS dataio paraboloid)

i3_test_scripts(resources/scripts/cmp.py
                resources/scripts/zollamt.py
		resources/test/test_paraboloid_nugen.py
		resources/test/test_failures.py
		resources/test/test_paraboloid_params_converter.py
		)
