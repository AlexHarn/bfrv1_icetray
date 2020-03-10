
#include "snowstorm/MultivariateNormal.h"

namespace bp = boost::python;
using namespace snowstorm;

void register_MultivariateNormal()
{
    bp::class_<MultivariateNormal, MultivariateNormalPtr, bp::bases<Distribution>>
        ("MultivariateNormal",
        bp::init<const I3Matrix&, const std::vector<double>&>((bp::args("covariance"), ("mean"))))
    ;
    register_pointer_conversions<MultivariateNormal>();
}
