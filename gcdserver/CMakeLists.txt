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

if (NOT ZMQ_NOT_FOUND)
  if (NOT PYMONGO_NOT_FOUND)
    no_dotfile_glob(tmp_SCRIPTS resources/test/Test*.py)
    i3_test_scripts(${tmp_SCRIPTS})
  endif(NOT PYMONGO_NOT_FOUND)
endif(NOT ZMQ_NOT_FOUND)
