#
# $Id$
#
i3_project(linefit
  PYTHON_DIR python
  DOCS_DIR resources/docs
  )

i3_add_library(linefit 
  private/linefit/I3LineFit.cxx
  private/Debiasing/Debiasing.cxx
  private/Debiasing/DebiasingEHE.cxx
  private/DelayCleaning/DelayCleaning.cxx
  private/DelayCleaning/DelayCleaningEHE.cxx
  private/HuberFit/HuberFit.cxx
  private/HuberFit/HuberFitEHE.cxx
  private/HuberFit/HuberFitPhysics.cxx
  USE_TOOLS boost python gsl
  USE_PROJECTS serialization icetray tableio dataclasses phys-services recclasses
)
#
# Plus Executables: 
# And tests test
#
i3_test_executable(test 
  private/test/main.cxx
  private/test/LineFitIOTest.cxx
  private/test/improvedLinefitTest.cxx
  USE_PROJECTS linefit dataio recclasses
)

i3_test_scripts(resources/scripts/*.py)
