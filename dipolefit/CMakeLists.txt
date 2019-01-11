i3_project(dipolefit
  PYTHON_DIR python
  DOCS_DIR resources/docs
)

i3_add_library(dipolefit 
  private/dipolefit/*.cxx 

  USE_TOOLS boost python

  USE_PROJECTS serialization icetray dataclasses tableio phys-services recclasses
)


i3_test_scripts(resources/tests/*.py)

