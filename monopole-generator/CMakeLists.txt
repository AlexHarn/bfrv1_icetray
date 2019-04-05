#
# $Id$
#

i3_project(monopole-generator
    DOCS_DIR resources/docs
    PYTHON_DIR python
)

# Define the lists of required tools.
SET(LIB_${PROJECT_NAME}_USE_TOOLS_LIST
    boost
    python
    gsl
)

# Use log4cplus tool when it is available
#(for backward-compatibility with older offline-software).
if(LOG4CPLUS_FOUND)
    list(APPEND LIB_${PROJECT_NAME}_USE_TOOLS_LIST log4cplus)
endif(LOG4CPLUS_FOUND)

i3_add_library(monopole-generator
  private/monopole-generator/I3MonopoleGenerator.cxx
  private/monopole-generator/I3MonopoleGeneratorUtils.cxx
  USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
  USE_PROJECTS serialization icetray dataclasses phys-services
)


i3_add_library(monopole-propagator
  private/monopole-generator/I3MonopolePropagator.cxx
  private/monopole-generator/I3MonopoleRelativisticUtils.cxx
  private/monopole-generator/I3MonopoleSlowUtils.cxx
  USE_TOOLS ${LIB_${PROJECT_NAME}_USE_TOOLS_LIST}
  USE_PROJECTS serialization icetray dataclasses phys-services
)

i3_test_executable(test
  private/test/*.cxx
  USE_PROJECTS serialization icetray dataclasses phys-services dataio monopole-generator
)


i3_test_scripts(resources/examples/*.py)
