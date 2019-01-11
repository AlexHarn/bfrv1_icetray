i3_project(frame_object_diff
  PYTHON_DIR python
  DOCS_DIR resources/docs)

i3_add_library(frame_object_diff
  private/*.cxx
  private/frame_object_diff/*.cxx
  private/frame_object_diff/*/*.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses
  )

i3_test_executable(test 
  private/test/*.cxx
  USE_PROJECTS frame_object_diff dataio
  )

i3_add_pybindings(frame_object_diff
  private/pybindings/*.cxx
  private/pybindings/*/*.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses frame_object_diff
  )
add_dependencies(frame_object_diff-pybindings dataclasses-pybindings)

set(TEST_SCRIPTS
  resources/test/test*.py
)
i3_test_scripts("${TEST_SCRIPTS}")
set_tests_properties(frame_object_diff::test_segments.py PROPERTIES LABELS RUN_LONG)
