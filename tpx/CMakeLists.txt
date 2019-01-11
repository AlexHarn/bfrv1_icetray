#
# $Id$
#
i3_project(tpx
  DOCS_DIR resources/docs
  PYTHON_DIR python
  )
	
i3_add_library(tpx 
  private/tpx/I3TopHLCPulseExtractor.cxx
  private/tpx/I3TopSLCPulseExtractor.cxx
  private/tpx/I3IceTopBaselineModule.cxx
  private/tpx/I3TopPulseInfo.cxx
  private/tpx/I3IceTopBaseline.cxx
  private/tpx/I3VEMConverter.cxx
  private/tpx/utility.cxx
  private/tpx/converter/I3TopPulseInfoConverter.cxx
  private/tpx/converter/convert_I3IceTopBaseline.cxx
  
  USE_TOOLS boost python

  USE_PROJECTS serialization icetray dataclasses tableio
)

i3_add_pybindings(tpx
  private/pybindings/I3TopPulseInfo.cxx
  private/pybindings/I3IceTopBaseline.cxx
  private/pybindings/module.cxx
  USE_TOOLS boost python
  USE_PROJECTS tpx icetray dataclasses phys-services tableio
)

i3_test_scripts(resources/tests/*.py)

SET(FILE         slc_calib_parameters.pcl)
SET(SOURCE       http://www.bartol.udel.edu/~javierg)
SET(TO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/data)


# Fetch calibration constants only if explicitly requested
OPTION(FETCH_TPX OFF)
if(FETCH_TPX)
  colormsg(GREEN "+-- Downloading and unpacking the IceTop SLC calibration data.")
  file(DOWNLOAD ${SOURCE}/${FILE} ${CMAKE_CURRENT_BINARY_DIR}/${FILE}
       #SHOW_PROGRESS
       EXPECTED_MD5 4d1bb331089c498f4b8fa638dd3d3b92)
else()
   colormsg(CYAN "+-- FETCH_TPX set to OFF. SLC calibration data may not be available.")
endif()
