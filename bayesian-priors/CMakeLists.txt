#
# $Id$
#

i3_project(bayesian-priors
  DOCS_DIR resources/docs
  PYTHON_DIR python
  PYTHON_DEST icecube/bayesian_priors
  )


i3_add_library(bayesian-priors 
  private/bayesian-priors/*.cxx

  USE_TOOLS boost python

  USE_PROJECTS serialization icetray dataclasses phys-services gulliver
)

i3_test_scripts(resources/tests/test_bayesian.py)

