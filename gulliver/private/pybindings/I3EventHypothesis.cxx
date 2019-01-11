#include <gulliver/I3EventHypothesis.h>

namespace bp = boost::python;

void register_I3EventHypothesis()
{
    bp::class_<I3EventHypothesis, boost::shared_ptr<I3EventHypothesis> >("I3EventHypothesis", bp::init<const I3Particle&>())
        .def_readwrite("particle", &I3EventHypothesis::particle)
        .def_readwrite("nonstd", &I3EventHypothesis::nonstd)
    ;
}
