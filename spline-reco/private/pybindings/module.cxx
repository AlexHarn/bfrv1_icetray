
#include <icetray/load_project.h>

void register_ServiceWrappers();

BOOST_PYTHON_MODULE(spline_reco)
{
	load_project("spline-reco",false);
	register_ServiceWrappers();
}
