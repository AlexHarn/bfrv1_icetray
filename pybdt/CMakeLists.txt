if (PYTHON_NUMERIC_VERSION LESS 20600)
    colormsg (YELLOW "*** python v2.6 or greater is required... skipping")
else (PYTHON_NUMERIC_VERSION LESS 20600)

if (NUMPY_FOUND)

i3_project (pybdt
    DOCS_DIR resources/docs
)

i3_add_library (pybdt
    private/pybdt/bdtlearner.cpp
    private/pybdt/bdtmodel.cpp
    private/pybdt/dataset.cpp
    private/pybdt/dtlearner.cpp
    private/pybdt/dtmodel.cpp
    private/pybdt/histogram.cpp
    private/pybdt/learner.cpp
    private/pybdt/linear_histogram.cpp
    private/pybdt/model.cpp
    private/pybdt/nonlinear_histogram.cpp
    private/pybdt/pruner.cpp
    private/pybdt/pybdt.cpp
    private/pybdt/random_sampler.cpp
    private/pybdt/vinelearner.cpp
    private/pybdt/vinemodel.cpp
    USE_TOOLS boost gsl python numpy
    USE_PROJECTS icetray dataclasses
    NOT_INSPECTABLE
    WITHOUT_I3_HEADERS
    MODULE
    NO_DOXYGEN
    INSTALL_DESTINATION lib/pybdt
)

i3_test_scripts(
    resources/test/test_ml.py
)
#set_tests_properties(pybdt::run_all_examples.sh
#    pybdt::test_pybdt_api.py
#    PROPERTIES LABELS RUN_LONG
#)

set_target_properties (pybdt PROPERTIES PREFIX "pybdt/_")

file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/lib/pybdt")

add_custom_target (pybdt_python
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/python/__init__.py ${CMAKE_BINARY_DIR}/lib/pybdt/__init__.py
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/python/histlite.py ${CMAKE_BINARY_DIR}/lib/pybdt/histlite.py
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/python/ml.py ${CMAKE_BINARY_DIR}/lib/pybdt/ml.py
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/python/util.py ${CMAKE_BINARY_DIR}/lib/pybdt/util.py
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/python/validate.py ${CMAKE_BINARY_DIR}/lib/pybdt/validate.py
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/python/viz.py ${CMAKE_BINARY_DIR}/lib/pybdt/viz.py

    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/python/pybdtmodule.py ${CMAKE_BINARY_DIR}/lib/icecube/pybdtmodule.py
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/python/convertdsmodule.py ${CMAKE_BINARY_DIR}/lib/pybdt/convertdsmodule.py
    )

add_dependencies (pybdt_python ${CMAKE_CURRENT_SOURCE_DIR}/python/__init__.py)
add_dependencies (pybdt_python ${CMAKE_CURRENT_SOURCE_DIR}/python/histlite.py)
add_dependencies (pybdt_python ${CMAKE_CURRENT_SOURCE_DIR}/python/util.py)
add_dependencies (pybdt_python ${CMAKE_CURRENT_SOURCE_DIR}/python/validate.py)
add_dependencies (pybdt_python ${CMAKE_CURRENT_SOURCE_DIR}/python/viz.py)
add_dependencies (pybdt pybdt_python)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/__init__.py
              ${CMAKE_CURRENT_SOURCE_DIR}/python/histlite.py
              ${CMAKE_CURRENT_SOURCE_DIR}/python/ml.py
              ${CMAKE_CURRENT_SOURCE_DIR}/python/util.py
              ${CMAKE_CURRENT_SOURCE_DIR}/python/validate.py
              ${CMAKE_CURRENT_SOURCE_DIR}/python/viz.py
        DESTINATION lib/pybdt
        OPTIONAL)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/pybdtmodule.py
        DESTINATION lib/icecube
        OPTIONAL)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/convertdsmodule.py
        DESTINATION lib/icecube
        OPTIONAL)

endif (NUMPY_FOUND)

endif (PYTHON_NUMERIC_VERSION LESS 20600)

# vim: nospell tw=0
