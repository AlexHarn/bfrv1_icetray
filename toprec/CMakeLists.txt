#
# $Id$
#
i3_project(toprec
  DOCS_DIR resources/docs
  PYTHON_DIR python
  )

set (TOPREC_TOOLS boost gsl python)

i3_add_library(toprec
  private/toprec/*.cxx
  private/toprec/laputop/*.cxx
  private/toprec/converter/*.cxx
  private/toprec/snowservices/*.cxx
  private/toprec/curvaturefitter/*.cxx

  USE_PROJECTS serialization icetray tableio dataclasses phys-services dataio gulliver recclasses

  USE_TOOLS ${TOPREC_TOOLS}
)

i3_test_executable(test
  private/test/*.cxx
  USE_PROJECTS toprec icetray dataclasses phys-services dataio interfaces lilliput recclasses
  )
set_tests_properties(toprec::test PROPERTIES LABELS RUN_LONG)


i3_add_pybindings(toprec
  private/pybindings/*.cxx
  USE_TOOLS boost python
  USE_PROJECTS toprec icetray dataclasses phys-services recclasses
  )


