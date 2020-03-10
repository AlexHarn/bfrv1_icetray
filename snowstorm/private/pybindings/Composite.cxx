
#include "snowstorm/Composite.h"

namespace bp = boost::python;
using namespace snowstorm;

void register_Composite()
{
    bp::class_<Composite, CompositePtr, bp::bases<Distribution>>
        ("Composite")
        .def("add", &Composite::add)
    ;
    register_pointer_conversions<Composite>();
}
