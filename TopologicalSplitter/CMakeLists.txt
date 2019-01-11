i3_project(TopologicalSplitter
  DOCS_DIR resources/docs
  PYTHON_DIR python
)

i3_add_library(TopologicalSplitter
  private/TopologicalSplitter/TopologicalSplitter.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses phys-services
)

i3_test_executable(test 
  private/test/*.cxx
  USE_PROJECTS icetray dataclasses dataio TopologicalSplitter
)
