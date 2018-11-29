#include <simclasses/I3CylinderMap.h>
#include <icetray/python/dataclass_suite.hpp>

using boost::python::class_;
using boost::python::dataclass_suite;
using boost::python::bases;

void register_I3CylinderMap()
{
  class_<I3CylinderMap,  I3CylinderMapPtr, bases<I3FrameObject> >("I3CylinderMap")
    .def(dataclass_suite<I3CylinderMap >())
    ;
  register_pointer_conversions<I3CylinderMap>();
}
