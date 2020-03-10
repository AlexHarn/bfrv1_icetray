
#include "snowstorm/UniformDistribution.h"

namespace bp = boost::python;
using namespace snowstorm;

void register_UniformDistribution()
{
    bp::class_<UniformDistribution, UniformDistributionPtr, bp::bases<Distribution>>
        ("UniformDistribution",
        bp::init<const std::vector<std::pair<double, double>>&>(bp::args("limits")))
    ;
    register_pointer_conversions<UniformDistribution>();
}
