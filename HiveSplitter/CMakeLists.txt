i3_project(HiveSplitter
  DOCS_DIR resources/docs
  PYTHON_DIR python
)

LIST(APPEND LIB_${PROJECT_NAME}_HEADERFILES
  public/HiveSplitter/OMKeyHash.h
  public/HiveSplitter/HitSorting.h
  public/HiveSplitter/Hive-lib.h
  public/HiveSplitter/HiveSplitter.h
  public/HiveSplitter/I3HiveSplitter.h
)

LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
  private/HiveSplitter/HitSorting.cxx
  private/HiveSplitter/Hive-lib.cxx
  private/HiveSplitter/HiveSplitter.cxx
  private/HiveSplitter/I3HiveSplitter.cxx
)

i3_add_library(HiveSplitter
  ${LIB_${PROJECT_NAME}_HEADERFILES}
  ${LIB_${PROJECT_NAME}_SOURCEFILES}
  USE_TOOLS boost python #log4cplus :uncomment for backwards compatibility 
  USE_PROJECTS serialization icetray dataclasses phys-services
)

LIST(APPEND LIB_${PROJECT_NAME}_UNITTESTS
  private/test/main.cxx
  private/test/OMKeyHashTest.cxx
  private/test/HitSortingTest.cxx
  private/test/Hive-libTest.cxx
  private/test/SplittingTest.cxx
  private/test/HiveSplitterTest.cxx
)

i3_test_executable(test
  ${LIB_${PROJECT_NAME}_UNITTESTS}
  USE_PROJECTS icetray dataclasses dataio HiveSplitter
)
#add_definitions(-g) #enable for debug stop-marks

set_tests_properties(HiveSplitter::test PROPERTIES LABELS RUN_LONG)
