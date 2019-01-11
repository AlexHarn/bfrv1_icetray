
if (HEALPIX-CXX_FOUND)

    SET(DST_TOOLS
       boost 
       cfitsio 
       python 
       healpix-cxx
       gsl
    )

   i3_project(dst
	 PYTHON_DIR python
	 DOCS_DIR resources/docs)
	 
   SET(DST_PROJECTS
      tableio serialization phys-services icetray dataclasses
      recclasses interfaces gulliver CommonVariables
      )

   SET(DST_FILES	
       private/dst/DSTCoordinates.cxx  
       private/dst/HealPixCoordinates.cxx  
       private/dst13/I3DSTDAQModule13.cxx
       private/dst13/I3DSTModule13.cxx
       private/dst16/I3DSTModule16.cxx
       private/extractor/TDST.cxx
       private/test-support/DSTTestGenerator.cxx
       )

   IF(SLA_FOUND OR PAL_FOUND)
     colormsg(CYAN "+-- Using I3DSTModulexx (astro found)")			
     LIST(APPEND DST_FILES
       private/extractor/I3DSTExtractor13.cxx
       private/extractor/I3DSTExtractor16.cxx
       )
     LIST(APPEND DST_PROJECTS astro)
   ELSE(SLA_FOUND OR PAL_FOUND)
     colormsg(CYAN "--- Skipping I3DSTModulexx (astro not found)")
   ENDIF(SLA_FOUND OR PAL_FOUND)

   colormsg(CYAN "+-- linking to ${DST_TOOLS}")	
   i3_add_library(dst
       ${DST_FILES}
       USE_TOOLS ${DST_TOOLS}
       USE_PROJECTS ${DST_PROJECTS}
   )

   i3_add_pybindings(dst
      private/pybindings/module.cxx
      private/pybindings/DSTCoordinate.cxx
      private/pybindings/TDST.cxx
      private/pybindings/HealPixCoordinate.cxx
      USE_TOOLS ${DST_TOOLS}
      USE_PROJECTS icetray recclasses dst
    )

   i3_test_executable(test 
      private/test/*.cxx
      USE_PROJECTS dst phys-services icetray dataclasses recclasses dataio gulliver 
    )

   if(ROOT_FOUND AND USE_ROOT)
     #these test scripts require rootwriter to work
     i3_test_scripts(resources/test/*.py)
   endif()

   set_tests_properties(dst::test PROPERTIES LABELS RUN_LONG)

   if(HEALPIX_CXX_ON_FEDORA)
     add_definitions(-DHEALPIX_CXX_ON_FEDORA)
   endif()

else (HEALPIX-CXX_FOUND)
   colormsg(CYAN "+-- Skipping dst (healpix-cxx not found)")
endif (HEALPIX-CXX_FOUND)
