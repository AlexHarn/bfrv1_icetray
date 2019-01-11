#
# $Id$
#
i3_project(clast
           PYTHON_DIR python
           DOCS_DIR resources/docs)

SET(LIB_${PROJECT_NAME}_EXTRAS "")
SET(LIB_${PROJECT_NAME}_TOOLS boost python gsl)

i3_add_library(clast 
  private/clast/I3CLastModule.cxx

  USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS}
  USE_PROJECTS serialization dataclasses icetray phys-services tableio recclasses

  ${LIB_${PROJECT_NAME}_EXTRAS}
)

i3_test_executable(test
  private/test/CLastIOTest.cxx
  private/test/main.cxx
  USE_PROJECTS serialization clast dataio recclasses
)

i3_test_scripts(resources/test/*.py)
