#
# $Id$
#

i3_project(tensor-of-inertia
  PYTHON_DEST icecube/tensor_of_inertia
  PYTHON_DIR python
  DOCS_DIR resources/docs)

i3_add_library(tensor-of-inertia 
  private/tensor-of-inertia/I3TensorOfInertia.cxx
  private/tensor-of-inertia/I3TensorOfInertiaCalculator.cxx

  USE_TOOLS boost python gsl

  USE_PROJECTS serialization dataclasses tableio icetray phys-services dataio recclasses
)

i3_test_executable(test 
  private/test/*.cxx
  USE_PROJECTS dataclasses phys-services tensor-of-inertia
  )

i3_test_scripts(resources/test/*.py)

