
i3_project(snowstorm
  PYTHON_DIR python
  # DOCS_DIR resources/docs
)

i3_add_library(snowstorm
  private/snowstorm/Distribution.cxx
  private/snowstorm/DeltaDistribution.cxx
  private/snowstorm/MultivariateNormal.cxx
  private/snowstorm/UniformDistribution.cxx
  private/snowstorm/Composite.cxx
  USE_TOOLS boost gsl
  USE_PROJECTS icetray serialization dataclasses phys-services
)

i3_add_pybindings(snowstorm
  private/pybindings/vectors.cxx
  private/pybindings/Distribution.cxx
  private/pybindings/DeltaDistribution.cxx
  private/pybindings/MultivariateNormal.cxx
  private/pybindings/UniformDistribution.cxx
  private/pybindings/Composite.cxx
  private/pybindings/module.cxx
  USE_TOOLS boost gsl python
  USE_PROJECTS snowstorm icetray serialization dataclasses phys-services
)

i3_test_scripts(resources/tests/*.py)
