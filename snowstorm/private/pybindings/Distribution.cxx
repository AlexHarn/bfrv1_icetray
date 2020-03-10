
#include <phys-services/I3RandomService.h>
#include <icetray/python/dataclass_suite.hpp>
#include "snowstorm/Distribution.h"

namespace bp = boost::python;
using namespace snowstorm;

void register_Distribution()
{
    bp::class_<Distribution, DistributionPtr, bp::bases<I3FrameObject>, boost::noncopyable>("Distribution", bp::no_init)
        .def("Sample", bp::pure_virtual(&Distribution::Sample), (bp::args("randomService")))
        .def("LogPDF", bp::pure_virtual(&Distribution::LogPDF), (bp::args("x")))
        .def("size", bp::pure_virtual(&Distribution::size))
    ;
    register_pointer_conversions<Distribution>();
}
