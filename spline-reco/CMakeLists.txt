#
# $Id$
#

i3_project(spline-reco
    PYTHON_DIR python
    DOCS_DIR resources/docs
)

i3_add_library(spline-reco
    private/spline-reco/I3SplineRecoLikelihood.cxx
    private/spline-reco/I3SplineRecoLikelihoodFactory.cxx
    private/spline-reco/gaussianiir1d.cxx

    USE_TOOLS boost python
    USE_PROJECTS serialization icetray dataclasses photonics-service
            lilliput gulliver phys-services
)

i3_test_executable(test 
	private/test/spline-reco-test.cxx
	private/test/main.cxx
	USE_PROJECTS icetray dataclasses gulliver-modules spline-reco
)

i3_test_scripts(
	resources/test/spline_reco_test.py
)

i3_add_pybindings(spline_reco
	private/pybindings/module.cxx
	private/pybindings/ServiceWrappers.cxx
	USE_TOOLS boost python 
	USE_PROJECTS spline-reco icetray dataclasses 
) 
