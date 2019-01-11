i3_project(gulliver-bootstrap
	PYTHON_DIR python
	PYTHON_DEST icecube/gulliver_bootstrap
	DOCS_DIR resources/docs
)

i3_add_library(gulliver-bootstrap 
  public/gulliver-bootstrap/BootstrappingLikelihoodService.h
  private/gulliver-bootstrap/BootstrappingSeedService.cxx
  private/gulliver-bootstrap/BootstrappingLikelihoodService.cxx
  private/gulliver-bootstrap/BootstrapSeedTweak.cxx
  private/gulliver-bootstrap/BootstrapParams.cxx
  private/gulliver-bootstrap/BootstrapParamsConverter.cxx
  USE_TOOLS boost python
  USE_PROJECTS serialization icetray dataclasses phys-services gulliver tableio
)

i3_add_pybindings(gulliver_bootstrap
  private/pybindings/gulliver-bootstrap.cxx
  USE_TOOLS boost python
  USE_PROJECTS icetray dataclasses phys-services gulliver tableio gulliver-bootstrap
)

i3_test_scripts(resources/tests/*.py)
