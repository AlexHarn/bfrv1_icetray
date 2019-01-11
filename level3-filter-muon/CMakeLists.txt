i3_project(level3-filter-muon
	PYTHON_DIR python
	PYTHON_DEST icecube/level3_filter_muon)

SET(LIB_${PROJECT_NAME}_SOURCEFILES
  private/level3-filter-muon/SplitTimeWindowCalculator.cxx
  private/level3-filter-muon/StaticDOMTimeWindowCleaning.cxx
  #private/level3-filter-muon/RecoPulseTruncationFixer.cxx
  private/level3-filter-muon/MuonL3_IC86_Utils.cxx
)

i3_add_library(level3-filter-muon
  ${LIB_${PROJECT_NAME}_SOURCEFILES}
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses
)

if(BUILD_PYBINDINGS)
  i3_add_pybindings(level3_filter_muon
    private/pybindings/level3-filter-muon.cxx
    USE_TOOLS boost python
    USE_PROJECTS icetray dataclasses level3-filter-muon
  )
endif(BUILD_PYBINDINGS)
