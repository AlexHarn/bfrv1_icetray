i3_project(finiteReco 
  PYTHON_DIR python
  DOCS_DIR resources/docs
)

i3_add_library(finiteReco
  private/finiteReco/*.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray phys-services gulliver dataclasses photonics-service recclasses
)

i3_test_executable(test 
  private/test/*.cxx 
  USE_PROJECTS finiteReco recclasses
)

i3_test_scripts(resources/scripts/*.py)
