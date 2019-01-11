if(OPENCL_FOUND)
  i3_project(ppc
    PYTHON_DIR python
    DOCS_DIR resources/docs
    )

  colormsg(GREEN "+-- OpenCL SDK found.  Building ppc.")
  i3_add_library(ppc
    private/ppc/i3ppc.cxx
    USE_TOOLS boost python
    USE_PROJECTS serialization icetray dataclasses simclasses phys-services interfaces clsim
    )
  i3_add_library(xppc
    private/ppc/ocl/ppc.cxx
    USE_TOOLS opencl
    COMPILE_FLAGS "-fPIC -DXLIB -std=gnu++98"
    )
  target_link_libraries(ppc xppc)
else()
  message(WARNING "OpenCL was not found.  Not building ppc.")
  find_package(OpenCL) # find non-quiet to let the user know what went wrong.
endif()
