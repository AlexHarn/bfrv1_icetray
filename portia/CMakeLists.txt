#
# $Id$
# Portia
i3_project(portia
  PYTHON_DIR python
  DOCS_DIR resources/docs)

i3_add_library(portia
  private/portia/*.cxx private/portia/converter/*.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray tableio dataclasses recclasses
)

i3_test_executable(test
  private/test/*.cxx
  USE_PROJECTS portia recclasses dataio WaveCalibrator
)

i3_test_scripts(resources/test/*.py)
