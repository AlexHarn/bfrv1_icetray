#
# $Id$
#

i3_project(cramer-rao
  PYTHON_DIR python
  DOCS_DIR resources/docs
  )

SET(LIB_${PROJECT_NAME}_EXTRAS "")
SET(LIB_${PROJECT_NAME}_TOOLS boost python gsl)

i3_add_library(cramer-rao
  private/cramer-rao/CramerRao.cxx
  USE_PROJECTS serialization icetray dataclasses phys-services tableio recclasses
  USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS}
  ${LIB_${PROJECT_NAME}_EXTRAS}
)


i3_test_executable(test 
  private/test/cr-test.cxx
  private/test/main.cxx
  USE_PROJECTS cramer-rao dataio
)

i3_test_scripts(resources/scripts/*.py)
