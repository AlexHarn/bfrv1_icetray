#
# $Id$
#
i3_project(smallshower-filter
  DOCS_DIR resources/docs)


i3_add_library(smallshower-filter private/smallshower-filter/*.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray interfaces dataclasses
  )

i3_test_executable(test
  private/test/*.cxx
  USE_PROJECTS icetray dataclasses dataio phys-services smallshower-filter 
)
