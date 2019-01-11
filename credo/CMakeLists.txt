i3_project(credo 
    PYTHON_DIR python
    DOCS_DIR resources/docs)

i3_add_library(credo
  private/credo/*.cxx

  USE_TOOLS gsl boost python
  USE_PROJECTS serialization icetray phys-services gulliver dataclasses photonics-service 
)

i3_add_pybindings(credo
  private/pybindings/module.cxx
  USE_TOOLS boost python
  USE_PROJECTS icetray gulliver credo photonics-service
  )

i3_test_executable(test 
  private/test/*.cxx 
  USE_PROJECTS credo
)

i3_test_scripts(resources/scripts/*.py
  resources/tests/*.py
  )
