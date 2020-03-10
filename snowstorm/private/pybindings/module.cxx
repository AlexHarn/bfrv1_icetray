
#include <icetray/load_project.h>

void register_vectors();
void register_Distribution();
void register_Composite();
void register_MultivariateNormal();
void register_DeltaDistribution();
void register_UniformDistribution();

BOOST_PYTHON_MODULE(snowstorm)
{
  boost::python::import("icecube.icetray");
  load_project("snowstorm", false);
  register_vectors();
  register_Distribution();
  register_Composite();
  register_MultivariateNormal();
  register_DeltaDistribution();
  register_UniformDistribution();
}
