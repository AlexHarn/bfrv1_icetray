#include <icetray/load_project.h>

void register_I3Gulliver();
void register_I3LogLikelihoodFitParams();
void register_I3EventHypothesis();
void register_I3FitParameterInitSpecs();
void register_I3MinimizerResult();
void register_ServiceWrappers();

BOOST_PYTHON_MODULE(gulliver)
{
    load_project("gulliver", false);

    register_I3Gulliver();
    register_I3LogLikelihoodFitParams();
    register_I3EventHypothesis();
    register_I3FitParameterInitSpecs();
    register_I3MinimizerResult();
    register_ServiceWrappers();
}
