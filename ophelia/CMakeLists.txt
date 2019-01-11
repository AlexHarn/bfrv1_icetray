#
# ophelia
#
option(OPHELIA_ICEREC_MODE "Build only IceRec modules for ophelia" ON)

	i3_project(ophelia
	PYTHON_DIR python
	DOCS_DIR resources/docs
	)

	if(OPHELIA_ICEREC_MODE)	
	  i3_add_library(ophelia
	    private/ophelia/module/*.cxx
	    private/ophelia/util/EHECriteria.cxx
	    private/ophelia/util/I3Ophelia*.cxx
	    private/ophelia/util/I3EHEEventSelector.cxx

	    USE_TOOLS python boost

	    USE_PROJECTS serialization icetray dataclasses dataio phys-services portia tableio recclasses
	  )
	else(OPHELIA_ICEREC_MODE)
	  i3_add_library(ophelia
	  private/ophelia/*/*.cxx

	  USE_TOOLS python boost

	  USE_PROJECTS serialization icetray dataclasses dataio phys-services juliet-interface portia simclasses tableio recclasses
	  )
	endif(OPHELIA_ICEREC_MODE)

	i3_test_executable(test
	  private/test/*.cxx
	  USE_TOOLS
	  USE_PROJECTS serialization ophelia portia phys-services dataio icepick recclasses
	)

	i3_test_scripts(resources/scripts/Test*.py)
