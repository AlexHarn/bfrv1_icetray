#
# $Id$
#
i3_project(gulliver
  PYTHON_DIR python
  DOCS_DIR resources/docs)

i3_add_library(gulliver 
  private/gulliver/*.cxx private/gulliver/*/*.cxx

  USE_TOOLS boost

  USE_PROJECTS serialization icetray dataclasses tableio interfaces phys-services
)

i3_test_executable(test 
  private/test/main.cxx
  private/test/*.cxx
  USE_PROJECTS icetray gulliver
)
i3_test_scripts(resources/tests/*.py)

i3_add_pybindings(gulliver
  private/pybindings/I3LogLikelihoodFitParams.cxx
  private/pybindings/I3EventHypothesis.cxx
  private/pybindings/I3Gulliver.cxx
  private/pybindings/I3FitParameterInitSpecs.cxx
  private/pybindings/I3MinimizerResult.cxx
  private/pybindings/ServiceWrappers.cxx
  private/pybindings/module.cxx
  USE_TOOLS boost python 
  USE_PROJECTS gulliver icetray dataclasses phys-services serialization 
  )

