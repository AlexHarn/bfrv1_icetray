#
# $Id$
#

i3_project(DeepCore_Filter
	PYTHON_DIR python
	DOCS_DIR resources/docs)

i3_add_library(DeepCore_Filter
	private/DeepCore_Filter/*.cxx
	USE_TOOLS boost python
	USE_PROJECTS serialization icetray dataclasses
)

i3_test_executable(test
	private/test/main.cxx
	private/test/VetoWindow.cxx
	private/test/ReduceHits.cxx
	private/test/CoG.cxx
	USE_PROJECTS icetray dataclasses DeepCore_Filter
	)

i3_test_scripts(resources/test/DCVetoTest.py
                resources/test/DCVetoLaunchesTest.py)
