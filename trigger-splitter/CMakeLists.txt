i3_project(trigger-splitter
  PYTHON_DEST icecube/trigger_splitter
  PYTHON_DIR python
  DOCS_DIR resources/docs
)

i3_add_library(trigger-splitter
	private/trigger-splitter/I3TriggerSplitter.cxx
	
	USE_TOOLS python boost 
	USE_PROJECTS serialization icetray dataclasses phys-services
)

