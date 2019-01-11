#
# $Id: CMakeLists.txt $
#

if (GEANT4_CONFIG_ERROR OR NOT DEFINED GEANT4_CONFIG_ERROR)

  message(STATUS "    Geant4 not in I3_PORTS - trying system installation")

  include(FindGeant4.cmake)
  include(FindCLHEP.cmake)

else (GEANT4_CONFIG_ERROR OR NOT DEFINED GEANT4_CONFIG_ERROR)
  set(GEANT4_FOUND TRUE)
  set(CLHEP_FOUND TRUE)
endif (GEANT4_CONFIG_ERROR OR NOT DEFINED GEANT4_CONFIG_ERROR)

set (G4TANKRESPONSE_HEADERS
  private/g4-tankresponse/g4classes/G4Interface.h
  private/g4-tankresponse/g4classes/G4TankIceSD.h
  private/g4-tankresponse/g4classes/G4IceTopRunManager.h
  private/g4-tankresponse/g4classes/G4IceTopDetectorConstruction.h
  private/g4-tankresponse/g4classes/G4IceTopTank.h
  private/g4-tankresponse/g4classes/G4IceTopPhysicsList.h
  private/g4-tankresponse/g4classes/G4IceTopGeneralPhysics.h
  private/g4-tankresponse/g4classes/G4IceTopUserSteppingAction.h
  private/g4-tankresponse/g4classes/G4IceTopUserTrackingAction.h
  private/g4-tankresponse/g4classes/G4Delaunay.h
)

set (G4TANKRESPONSE_SOURCES
  private/g4-tankresponse/g4classes/G4Interface.cxx
  private/g4-tankresponse/g4classes/G4TankIceSD.cxx
  private/g4-tankresponse/g4classes/G4IceTopRunManager.cxx
  private/g4-tankresponse/g4classes/G4IceTopDetectorConstruction.cxx
  private/g4-tankresponse/g4classes/G4IceTopTank.cxx
  private/g4-tankresponse/g4classes/G4IceTopGeneralPhysics.cxx
  private/g4-tankresponse/g4classes/G4IceTopPhysicsList.cxx
  private/g4-tankresponse/g4classes/G4IceTopUserSteppingAction.cxx
  private/g4-tankresponse/g4classes/G4IceTopUserTrackingAction.cxx
  private/g4-tankresponse/g4classes/G4Delaunay.cxx
)

if (GEANT4_VERSION LESS 1000)
  list (APPEND G4TANKRESPONSE_HEADERS
    private/g4-tankresponse/g4classes/G4IceTopEMPhysics.h
    private/g4-tankresponse/g4classes/G4IceTopHadronPhysics.h
    private/g4-tankresponse/g4classes/G4IceTopIonPhysics.h
    private/g4-tankresponse/g4classes/G4IceTopMuonPhysics.h
  )
  list (APPEND G4TANKRESPONSE_SOURCES
    private/g4-tankresponse/g4classes/G4IceTopEMPhysics.cxx
    private/g4-tankresponse/g4classes/G4IceTopHadronPhysics.cxx
    private/g4-tankresponse/g4classes/G4IceTopIonPhysics.cxx
    private/g4-tankresponse/g4classes/G4IceTopMuonPhysics.cxx
  )
endif (GEANT4_VERSION LESS 1000)


if (GEANT4_FOUND AND CLHEP_FOUND)
  option(G4_TANK_USE_TRIANGLE_C "Use the older C-based triangulation algorithm" False)
  if (G4_TANK_USE_TRIANGLE_C)
    add_definitions("-DG4_TANK_USE_TRIANGLE_C")
  endif (G4_TANK_USE_TRIANGLE_C)

  i3_project(g4-tankresponse
    PYTHON_DIR python
    DOCS_DIR resources/docs
  )

  #
  # The following definitions are required by Triangle
  # for the time being, it is compiled and tested but not used in G4Delaunay
  #
  add_definitions("-DANSI_DECLARATORS")
  add_definitions("-DTRILIBRARY")
  add_definitions("-DREDUCED")
  add_definitions("-DCDT_ONLY")

  i3_add_library(g4-tankresponse
    private/g4-tankresponse/*.cxx
    private/g4-tankresponse/triangle/*.cxx
    ${G4TANKRESPONSE_HEADERS}
    ${G4TANKRESPONSE_SOURCES}
    private/g4-tankresponse/triangle/*.c
    private/g4-tankresponse/bowyer/*.h
    private/g4-tankresponse/bowyer/*.cpp

    USE_TOOLS boost python clhep geant4 gsl
    USE_PROJECTS serialization icetray dataclasses topsimulator phys-services simclasses
  )

  include(FindG4VisDrivers.cmake)

  i3_test_scripts(resources/test/test_examples.py)
  set_tests_properties(g4-tankresponse::test_examples.py PROPERTIES LABELS RUN_LONG)

  i3_test_executable(test
    private/test/main.cxx
    private/test/triangle.cxx
    private/test/g4-tankresponse.cxx
    private/test/bowyer.cxx

    USE_PROJECTS g4-tankresponse sim-services
  )

else (GEANT4_FOUND AND CLHEP_FOUND)
  message(STATUS "  - Geant4 and/or CLHEP not found - Skipping build of g4-tankresponse")
endif (GEANT4_FOUND AND CLHEP_FOUND)

if (NOT (GEANT4_VERSION LESS 1000) )
  colormsg(CYAN "+-- g4-tankresponse has not been thoroughly tested with Geant4 versions 4.10 and above")
endif (NOT (GEANT4_VERSION LESS 1000) )
