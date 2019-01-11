#
# $Id$
#
i3_project(core-removal
  PYTHON_DIR python
  PYTHON_DEST icecube/core_removal
  DOCS_DIR resources/docs
  )

i3_add_library(core-removal
  private/core-removal/*.cxx 

  USE_TOOLS boost python

  USE_PROJECTS serialization dataclasses icetray phys-services 

)

i3_test_executable(test
  private/test/*.cxx
  USE_PROJECTS core-removal icetray
)

# FIXME : Make these scripts work and then enable them. 
#i3_test_scripts(resources/scripts/*.py)
