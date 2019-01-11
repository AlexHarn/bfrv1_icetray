#
# $Id$
#

i3_project(CoincSuite
  DOCS_DIR resources/docs
  PYTHON_DIR python
  PYTHON_DEST icecube/CoincSuite
)

#Header Files
LIST(APPEND LIB_${PROJECT_NAME}_HEADERFILES
  #LIBRARIES
  public/CoincSuite/lib/OMKeyHash.h
  public/CoincSuite/lib/HitSorting.h
  public/CoincSuite/lib/PartialCOG.h
  public/CoincSuite/lib/I3FakeCalculator.h
  public/CoincSuite/lib/CoincSuiteHelpers.h
  #BASE-CLASSES
  public/CoincSuite/Modules/FrameCombiner.h
  public/CoincSuite/Testers/TesterModule.h
)

#CXX Files
LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
  #LIBRARIES
  private/CoincSuite/lib/HitSorting.cxx
  private/CoincSuite/lib/PartialCOG.cxx
  private/CoincSuite/lib/I3FakeCalculator.cxx
  private/CoincSuite/lib/CoincSuiteHelpers.cxx
  #BASE-CLASSES
  private/CoincSuite/Modules/FrameCombiner.cxx
  private/CoincSuite/Testers/TesterModule.cxx
  #DERIVED MODULES: General Purpose
  private/CoincSuite/Modules/HypoFrame.cxx
  #DERIVED MODULES: TesterModules
  private/CoincSuite/Testers/SpeedTester.cxx
  private/CoincSuite/Testers/CausalConnectTester.cxx #DEPRICATED
  private/CoincSuite/Testers/impCausalConnectTester.cxx
  private/CoincSuite/Testers/cogCausalConnectTester.cxx
  private/CoincSuite/Testers/TrackLikelihoodTester.cxx #DEPRICATED
  private/CoincSuite/Testers/ReducingLikelihoodTester.cxx
  private/CoincSuite/Testers/TrackSystemTester.cxx
  private/CoincSuite/Testers/CylinderPulsesTester.cxx #DEPRICATED 
  private/CoincSuite/Testers/AlignmentTester.cxx
  private/CoincSuite/Testers/AfterpulseTester.cxx
  #DERIVED MODULES: DiscardClasses
  private/CoincSuite/Modules/DecisionMaker.cxx
  private/CoincSuite/Modules/AfterpulseDiscard.cxx
  #private/CoincSuite/Modules/EmptyFrameDiscard.cxx DEPRICATED
  private/CoincSuite/Modules/NoiseClusterDiscard.cxx
  #private/CoincSuite/Modules/FrameSentinel.cxx DEPRICATED
)

i3_add_library(CoincSuite
  ${LIB_${PROJECT_NAME}_HEADERFILES}
  ${LIB_${PROJECT_NAME}_SOURCEFILES}

  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses phys-services lilliput gulliver
)

i3_add_pybindings(CoincSuite
  private/pybindings/CoincSuiteHelpers.cxx
  private/pybindings/module.cxx
  USE_TOOLS boost python
  USE_PROJECTS icetray dataclasses CoincSuite
)

i3_test_scripts(
  resources/test/helpersTest.py
  resources/test/recursionTest.py
  resources/test/HypoFrameTest.py
  resources/test/AfterpulseTest.py
  resources/test/AlignmentTest.py
  #resources/test/CylinderPulsesTest.py #DEPRICATED MODULE
  resources/test/DecisionMakerTest.py
  resources/test/impCausalConnectTest.py
  resources/test/SpeedTest.py
  #resources/test/CausalConnectTest.py #DEPRICATED MODULE
  #resources/test/TrackLikelihoodTest.py #DEPRICATED MODULE
  resources/test/cogCausalConnectTest.py
  resources/test/TrackSystemTest.py
  resources/test/ReducingLikelihoodTest.py
)

LIST(APPEND LIB_${PROJECT_NAME}_UNITTESTS
  private/test/main.cxx
  private/test/HitSortingTest.cxx
  private/test/OMKeyHashTest.cxx
  private/test/FrameCombinerTest.cxx
  private/test/CoincSuiteHelpersTest.cxx
  private/test/PartialCOGTest.cxx
)

i3_test_executable(test
  ${LIB_${PROJECT_NAME}_UNITTESTS}
  USE_PROJECTS serialization icetray dataclasses dataio CoincSuite
)

#add_definitions(-g) #enable for debug stop-marks
