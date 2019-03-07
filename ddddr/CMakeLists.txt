#
# $Id: CMakeLists.txt 69971 2011-01-11 15:37:38Z kislat $
#
i3_project(ddddr
  PYTHON_DIR python
  DOCS_DIR resources/docs
  )

SET(LIB_${PROJECT_NAME}_SOURCEFILES
  private/ddddr/I3MuonEnergy.cxx
  private/ddddr/I3MuonEnergyParams.cxx
  private/ddddr/I3MuonEnergyCascadeParams.cxx
  private/ddddr/I3MuonEnergyProfile.cxx
  private/ddddr/I3TrueMuonEnergy.cxx
  private/ddddr/MuonEnergyFunctions.cxx
  private/ddddr/MuonGunTrack.cxx
)
SET(LIB_${PROJECT_NAME}_TEST_SOURCEFILES
  private/test/I3MuonEnergyProfileTest.cxx
  private/test/main.cxx
)
SET(LIB_${PROJECT_NAME}_TOOLS boost gsl python)
SET(LIB_${PROJECT_NAME}_PROJECTS serialization icetray dataclasses phys-services dataio simclasses)

if(USE_ROOT AND ROOT_FOUND)
  LIST(APPEND LIB_${PROJECT_NAME}_TOOLS root)
  colormsg(GREEN "+-- Using Minuit2 from ROOT.")
endif()

## during detection, we prefer ROOT's Minuit2 to the standalone version
if(MINUIT2_FOUND)
  LIST(APPEND LIB_${PROJECT_NAME}_TOOLS minuit2)
  colormsg(GREEN "+-- Found Minuit2, fitting enabled.")
  ADD_DEFINITIONS(-DUSE_MINUIT2)

  LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
    private/ddddr/MuonEnergyMinuit2.cxx
    private/ddddr/TomFFcn.cxx
    private/ddddr/ExpoFcn.cxx
    )
else()
  colormsg(CYAN "+-- Either ROOT not found, disabled or Minuit2 not available.")
  colormsg(CYAN "    DDDDR will still be built and can be run with disabled fitting.")
endif()


i3_add_library(ddddr
  ${LIB_${PROJECT_NAME}_SOURCEFILES}
  USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS}
  USE_PROJECTS ${LIB_${PROJECT_NAME}_PROJECTS}
  )

i3_test_executable(test
  ${LIB_${PROJECT_NAME}_TEST_SOURCEFILES}
  USE_PROJECTS ddddr
  )

i3_add_pybindings(ddddr
  private/pybindings/module.cxx
  private/pybindings/I3MuonEnergyParams.cxx
  private/pybindings/I3MuonEnergyProfile.cxx
  private/pybindings/I3MuonEnergyCascadeParams.cxx
  USE_TOOLS boost python 
  USE_PROJECTS ddddr icetray serialization dataclasses phys-services simclasses
  )

i3_test_scripts(resources/scripts/*.py)
