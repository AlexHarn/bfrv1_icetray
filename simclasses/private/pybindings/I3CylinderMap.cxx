#include <simclasses/I3CylinderMap.h>
#include <icetray/python/dataclass_suite.hpp>

using namespace boost::python;

void register_I3CylinderMap()
{
  class_<I3CylinderMap,
    boost::shared_ptr<I3CylinderMap>>("I3CylinderMap")
    .def(dataclass_suite<I3CylinderMap>())
    ;

  register_pointer_conversions<I3CylinderMap>();
}
