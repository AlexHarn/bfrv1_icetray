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
    private/vemcal/VEMCalAnalyzer.cxx
    private/vemcal/I3VEMCalHistWriter.cxx
    private/vemcal/I3VEMCalTreeWriter.cxx
  )
  LIST(APPEND LIB_${PROJECT_NAME}_TOOLS
    root
  )
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
  USE_PROJECTS vemcal 
  USE_TOOLS root
  ) 

  i3_executable(analyze 
  private/analyze/*.cxx
  USE_PROJECTS vemcal 
  USE_TOOLS root 
  )	       
endif (ROOT_FOUND)

i3_test_executable(test
  private/test/main.cxx
  private/test/I3VEMCalData.cxx
  USE_PROJECTS icetray dataio vemcal
)
