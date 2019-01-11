if (HEALPIX-CXX_FOUND)

  i3_project(NoiseEngine
    PYTHON_DIR python
    DOCS_DIR resources/docs
    )

  if(LOG4CPLUS_FOUND)
    set(log4cplus "log4cplus")
  else(LOG4CPLUS_FOUND)
    set(log4cplus "")
  endif(LOG4CPLUS_FOUND)

  i3_add_library(NoiseEngine
    private/NoiseEngine/NoiseEngine.cxx 
    private/NoiseEngine/HitPair.cxx
    public/NoiseEngine/HitPair.h
    USE_TOOLS boost python ${log4cplus} healpix-cxx cfitsio 
    USE_PROJECTS serialization icetray dataclasses
    )

  i3_test_executable(test 
    private/test/main.cxx
    private/test/HitPair.cxx
    USE_PROJECTS NoiseEngine
    )

  i3_test_scripts(resources/test/testModule.py
    resources/test/testSegment.py)

if(USE_CVMFS AND CMAKE_COMPILER_IS_CLANG)
  target_link_libraries(${PROJECT_NAME}-test "gcc_s")
endif()

if(HEALPIX_CXX_ON_FEDORA)
  add_definitions(-DHEALPIX_CXX_ON_FEDORA)
endif()

else (HEALPIX-CXX_FOUND)
  colormsg(RED "Skipping NoiseEngine (healpix-cxx not found)")
endif (HEALPIX-CXX_FOUND)
