
i3_project(CascadeVariables PYTHON_DIR python DOCS_DIR resources/docs) 

i3_add_library(CascadeVariables
  private/CascadeVariables/*.cxx

  USE_TOOLS boost python 

  USE_PROJECTS serialization icetray dataclasses phys-services recclasses
)
target_link_libraries(CascadeVariables)

#i3_test_executable(test 
#    private/test/*.cxx
#    USE_PROJECTS veto
#    )

#
# Plus Executables: 
# And tests test
#
#i3_test_scripts(resources/scripts/*.py)

