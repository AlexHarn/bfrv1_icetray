#
# $Id$
#

i3_project(fill-ratio
  PYTHON_DIR python
  PYTHON_DEST icecube/fill_ratio
  DOCS_DIR resources/docs
)

i3_add_library(fill-ratio 
  private/fill-ratio/*.cxx 
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses tableio dataio phys-services recclasses
)

i3_test_executable(test
  private/test/*.cxx
  USE_PROJECTS fill-ratio dataio
)
#
# Plus Executables: 
# And tests test
#
i3_test_scripts(resources/tests/*.py)

