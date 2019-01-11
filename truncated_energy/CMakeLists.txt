i3_project(truncated_energy
           PYTHON_DIR python
           DOCS_DIR resources/docs)

i3_add_library(truncated_energy
  private/truncated_energy/I3TruncatedEnergy.cxx

  USE_TOOLS boost python

  USE_PROJECTS serialization photonics-service icetray gulliver phys-services dataclasses
)

i3_test_scripts(resources/test/test_truncated.py)
