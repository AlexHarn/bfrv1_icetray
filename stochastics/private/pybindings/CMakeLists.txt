if (BUILD_PYBINDINGS)
  i3_add_pybindings(stochastics
    I3EnergyLoss.cxx
    USE_TOOLS boost python
    USE_PROJECTS stochastics icetray dataclasses phys-services  
    )
endif(BUILD_PYBINDINGS)
