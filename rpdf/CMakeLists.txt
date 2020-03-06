#
# $Id: CMakeLists.txt 158138 2017-09-14 11:11:53Z kjmeagher $
#

i3_project(rpdf
    PYTHON_DIR python
    DOCS_DIR resources/docs
    )

i3_add_library(rpdf
    private/rpdf/geometry.cxx
    private/rpdf/pandel.cxx
    private/rpdf/I3RecoLLH.cxx
    private/rpdf/I3RecoLLHFactory.cxx
    USE_TOOLS boost gsl
    USE_PROJECTS serialization icetray dataclasses gulliver phys-services
    )

i3_add_pybindings(rpdf
    private/pybindings/rpdf.cxx
    USE_TOOLS boost python gsl
    USE_PROJECTS rpdf
    )

i3_test_scripts(
    resources/tests/compare_with_ipdf.py
    resources/tests/test_muon_geometry.py
    resources/tests/test_pandel_convoluted.py
    resources/tests/test_pandel_integral.py
    resources/tests/test_pandel_regions.py
    resources/tests/test_pandel_sample.py
    resources/tests/test_pandel_values.py
    )
