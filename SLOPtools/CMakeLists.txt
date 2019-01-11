#
# $Id$
#
i3_project(SLOPtools
  DOCS_DIR resources/docs
  PYTHON_DIR python
  )

i3_test_scripts(resources/test/slop_tools_test.py)
#set_tests_properties(SLOPTools::slop_tools_test.py PROPERTIES LABELS RUN_LONG)
