#
#

i3_project(stochastics
	DOCS_DIR resources/docs
	PYTHON_DIR python)

i3_add_library(stochastics
		private/stochastics/*.cxx 
                private/stochastics/converter/*.cxx 		  
  USE_TOOLS boost python 
  USE_PROJECTS serialization icetray dataclasses tableio lilliput
)

i3_test_scripts(resources/scripts/*.py)

add_subdirectory(private/pybindings)

