#
# $Id$
#
i3_project(gulliver-modules
    PYTHON_DIR python
    PYTHON_DEST icecube/gulliver_modules
    DOCS_DIR resources/docs)

i3_add_library(gulliver-modules
    private/gulliver-modules/*.cxx
    private/test/*.cxx
    private/test/*/*.cxx

    USE_TOOLS gsl boost python
    USE_PROJECTS serialization icetray dataclasses phys-services icetray gulliver)

# i3_test_executable(test
#    private/test/*.cxx
#    private/test/*.c)

i3_test_scripts(resources/test/addllh.py
                resources/test/fancyfit.py
                resources/test/iterativefit.py
                resources/test/layered_pandel_fit.py
                resources/test/mpefit.py
                resources/test/reversefit.py
                resources/test/simple_cascade.py
                resources/test/simplefit.py)
