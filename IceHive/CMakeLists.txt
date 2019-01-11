i3_project(IceHive
  DOCS_DIR resources/docs
  PYTHON_DIR python
)

LIST(APPEND LIB_${PROJECT_NAME}_HEADERFILES
  public/IceHive/OMKeyHash.h
  public/IceHive/HitSorting.h
  public/IceHive/Hive-lib.h
  public/IceHive/IceHiveHelpers.h
  public/tools/IC86Topology.h
  private/IceHive/HiveSplitter.h
  private/IceHive/HiveCleaning.h
  private/IceHive/TriggerSplitter.h
  private/IceHive/I3IceHive.h
  private/IceHive/I3HiveCleaning.h
  private/IceHive/I3HiveCluster.h
)

LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
  private/IceHive/HitSorting.cxx
  private/IceHive/Hive-lib.cxx
  private/IceHive/IceHiveHelpers.cxx
  private/IceHive/HiveSplitter.cxx
  private/IceHive/TriggerSplitter.cxx
  private/IceHive/I3IceHive.cxx
  private/IceHive/HiveCleaning.cxx
  private/IceHive/I3HiveCleaning.cxx
  private/IceHive/I3HiveCluster.cxx
  private/tools/IdealGeometry.cxx
  private/tools/IC86Topology.cxx
)

i3_add_library(IceHive
  ${LIB_${PROJECT_NAME}_HEADERFILES}
  ${LIB_${PROJECT_NAME}_SOURCEFILES}
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses phys-services simclasses
  NOT_INSPECTABLE
)

i3_add_pybindings(IceHive
  private/pybindings/IceHiveHelpers.cxx
  private/pybindings/HitSorting.cxx
  private/pybindings/module.cxx
  USE_TOOLS boost python
  USE_PROJECTS icetray dataclasses IceHive simclasses
)

i3_test_scripts(
  resources/test/i3icehiveTest.py
  resources/test/i3hiveclusterTest.py
  resources/test/i3hivecleaningTest.py
)

LIST(APPEND LIB_${PROJECT_NAME}_UNITTESTS
  private/test/main.cxx
  private/test/TestHelpers.cxx
  private/test/OMKeyHashTest.cxx
  private/test/HitSortingTest.cxx
  private/test/IceHiveHelpersTest.cxx
  private/test/Hive-libTest.cxx
  private/test/HiveSplitterTest.cxx
  private/test/HiveCleaningTest.cxx
)

i3_test_executable(test
  ${LIB_${PROJECT_NAME}_UNITTESTS}
  USE_PROJECTS icetray dataclasses dataio IceHive
)

set_tests_properties(IceHive::test PROPERTIES LABELS RUN_LONG)
