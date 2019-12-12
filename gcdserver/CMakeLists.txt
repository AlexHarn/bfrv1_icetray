
if(WORKS_AND_TESTS_PASS)

  if(${Python_VERSION} VERSION_LESS 3) 
    i3_project(gcdserver
      PYTHON_DIR python
      )
    
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import zmq" RESULT_VARIABLE ZMQ_NOT_FOUND ERROR_QUIET)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import pymongo" RESULT_VARIABLE PYMONGO_NOT_FOUND ERROR_QUIET)
    
    if (ZMQ_NOT_FOUND)
      message(STATUS "pyzmq not found.  Skipping unit tests")
    endif(ZMQ_NOT_FOUND)
    
    if (PYMONGO_NOT_FOUND)
      message(STATUS "pymongo not found.  Skipping unit tests")
    endif(PYMONGO_NOT_FOUND)
    
    # Exclude TestI3MS, as port 6012 is used on cobalt
    
    if (NOT ZMQ_NOT_FOUND)
      if (NOT PYMONGO_NOT_FOUND)
	no_dotfile_glob(tmp_SCRIPTS resources/test/TestCalibration.py)
	i3_test_scripts(resources/test/TestGeometry.py
          resources/test/TestCalibration.py
          resources/test/TestDetectorStatus.py
          resources/test/TestDB.py)
      endif(NOT PYMONGO_NOT_FOUND)
    endif(NOT ZMQ_NOT_FOUND)
  else()
    colormsg(RED "+-- gcdserver does not support Python3 yet.")
  endif()
else()
    colormsg(RED "+-- gcdserver does not work.  Not building.")
endif()
