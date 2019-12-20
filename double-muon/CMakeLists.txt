#
# $Id$
#
i3_project(double-muon
  PYTHON_DIR python
  DOCS_DIR resources/docs)

i3_add_library(double-muon 
  private/double-muon/I3DoubleMuonLogLikelihood.cxx
  private/double-muon/I3DoubleMuonParametrization.cxx
  private/double-muon/I3DoubleMuonSeedService.cxx
  private/double-muon/I3ResponseMapSplitter.cxx

  USE_TOOLS boost gsl
  USE_PROJECTS serialization icetray dataclasses phys-services gulliver ipdf
)

#
# Plus Executables: 
# And tests test
#
i3_test_executable(test 
  private/test/main.cxx
  private/test/TestDoubleMuonLLH.cxx
  private/test/TestResponseMapSplitter.cxx
  USE_TOOLS python
  USE_PROJECTS icetray dataclasses  phys-services gulliver ipdf lilliput double-muon photonics-service DomTools icepick gulliver-modules
  USE_PYBINDINGS icetray dataclasses dataio linefit recclasses icepick gulliver gulliver-modules lilliput
)
i3_test_scripts(resources/scripts/brightest_string_sort_and_fit.py
                resources/scripts/doublefit_10par.py
                resources/scripts/doublefit_2x5par.py
                resources/scripts/time_residual_sort_and_fit.py
        )
