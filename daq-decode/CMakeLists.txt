#
# $Id$
#
i3_project(daq-decode
  DOCS_DIR resources/docs
  PYTHON_DEST icecube/daq_decode
  PYTHON_DIR python)

i3_add_library(daq-decode 
  private/I3DAQDecoderTestStandAlone/*.cxx private/daq-decode/*.cxx private/daq-decode/*/*.cxx
  USE_TOOLS boost
  USE_PROJECTS serialization icetray dataclasses phys-services interfaces 
)

i3_add_pybindings(daq_decode
  private/pybindings/*.cxx
  USE_TOOLS boost python
  USE_PROJECTS icetray dataclasses daq-decode
)

i3_test_executable(test private/test/*.cxx
  USE_PROJECTS icetray daq-decode
  USE_TOOLS python)

i3_test_scripts(resources/test/*.py)
