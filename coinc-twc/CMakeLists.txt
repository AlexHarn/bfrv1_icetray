#
#
i3_project(coinc-twc
	DOCS_DIR resources/docs
	PYTHON_DIR python
        PYTHON_DEST icecube/coinc_twc		
	)

i3_add_library(coinc-twc
  private/coinc-twc/I3CoincTWC.cxx 
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses trigger-sim
  )

