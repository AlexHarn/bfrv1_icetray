
#include "snowstorm/DeltaDistribution.h"

namespace bp = boost::python;
using namespace snowstorm;

void register_DeltaDistribution()
{
    bp::class_<DeltaDistribution, DeltaDistributionPtr, bp::bases<Distribution>>
        ("DeltaDistribution",
        bp::init<const std::vector<double>&>(bp::args("x0")))
    ;
    register_pointer_conversions<DeltaDistribution>();
}
