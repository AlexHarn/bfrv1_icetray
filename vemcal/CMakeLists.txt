#
# $Id: CMakeLists.txt $
#
i3_project(vemcal
  PYTHON_DIR python
  DOCS_DIR resources/docs
  )

SET(LIB_${PROJECT_NAME}_SOURCEFILES
  private/vemcal/I3HGLGPairSelector.cxx
  private/vemcal/I3VEMCalData.cxx
  private/vemcal/I3VEMCalExtractor.cxx
)

SET(LIB_${PROJECT_NAME}_TOOLS
  boost python
)

if (ROOT_FOUND)
  LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
    private/vemcal/I3VEMCalHistWriter.cxx
    private/vemcal/I3VEMCalTreeWriter.cxx
  )
  LIST(APPEND LIB_${PROJECT_NAME}_TOOLS
    root
  )
# Commented out.  Causing nasty segfaults in Pnf (EKB, Mar 23, 2017)  
#  if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/I3Db)
#    # VEMCalAnalyzer needs I3Db
#    LIST(APPEND LIB_${PROJECT_NAME}_SOURCEFILES
#      private/vemcal/VEMCalAnalyzer.cxx
#    )
#    LIST(APPEND LIB_${PROJECT_NAME}_TOOLS
#      mysql
#    )
#  endif(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/I3Db)
endif (ROOT_FOUND)

i3_add_library(vemcal
  ${LIB_${PROJECT_NAME}_SOURCEFILES}
  USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS}
  USE_PROJECTS serialization icetray dataclasses tpx
  )

i3_add_pybindings(vemcal
  private/pybindings/module.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses vemcal
  )

if (ROOT_FOUND)
  i3_executable(plot 
    private/plot/*.cxx
    USE_TOOLS root
    ) 

#  if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/I3Db)
#    # analyze, history and history-plot need I3Db
#    i3_executable(analyze 
#      private/analyze/*.cxx
#      USE_PROJECTS vemcal I3Db
#      USE_TOOLS root mysql
#      )	       
#
#    i3_executable(history 
#      private/history/*.cxx
#      USE_PROJECTS icetray dataclasses
#      USE_TOOLS root
#      )
#
#    i3_executable(history-plot 
#      private/history-plot/*.cxx
#      USE_PROJECTS icetray dataclasses I3Db
#      USE_TOOLS root mysql
#      )
#  endif(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/I3Db)
endif (ROOT_FOUND)

# TODO: no idea if these run in a standard icetray installation or if they needed
# work. -ck
# i3_test_scripts(resources/scripts/*.py)

i3_test_executable(test
  private/test/main.cxx
  private/test/I3VEMCalData.cxx
  
  USE_PROJECTS icetray dataio vemcal
  )
