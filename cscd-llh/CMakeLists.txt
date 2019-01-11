#
# $Id$
#
i3_project(cscd-llh
        DOCS_DIR resources/docs           
        PYTHON_DIR python
        )

i3_add_library(cscd-llh
  private/cscd-llh/I3CscdLlhFitter.cxx
  private/cscd-llh/I3CscdLlhModule.cxx
  private/minimizer/I3CscdLlhBrent.cxx
  private/minimizer/I3CscdLlhMinimizer.cxx
  private/minimizer/I3CscdLlhPowell.cxx
  private/minimizer/I3CscdLlhSimplex.cxx
  private/parser/I3CscdLlhGeneralParser.cxx
  private/parser/I3CscdLlhHitNoHitParser.cxx
  private/parser/I3CscdLlhHnhDirParser.cxx
  private/parser/I3CscdLlhPndlHnhParser.cxx
  private/parser/I3CscdLlhUPandelParser.cxx
  private/pdf/I3CscdLlhHitNoHit.cxx
  private/pdf/I3CscdLlhHitNoHitMpe.cxx
  private/pdf/I3CscdLlhHnhDir.cxx
  private/pdf/I3CscdLlhPndlHnh.cxx
  private/pdf/I3CscdLlhUPandel.cxx
  private/pdf/I3CscdLlhUPandelMpe.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization dataclasses icetray tableio recclasses
)

i3_test_scripts(
  resources/test/test_cscd_llh_1.py
  resources/test/test_cscd_llh_2.py
  resources/test/run_examples-cscd-llh-1.py
  resources/test/run_examples-cscd-llh-2.py
  resources/test/run_examples-cscd-llh-7.py
  resources/test/run_examples-cscd-llh2.py 
)

