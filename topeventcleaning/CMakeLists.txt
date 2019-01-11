#
# $Id$
#
i3_project(topeventcleaning
  DOCS_DIR resources/docs
  PYTHON_DIR python
  )

i3_add_library(topeventcleaning
  private/topeventcleaning/I3TopHLCClusterCleaning.cxx
  private/topeventcleaning/I3HLCTankPulseMerger.cxx
  
  USE_TOOLS boost python

  USE_PROJECTS serialization icetray dataclasses phys-services
)
