#include <simclasses/I3CylinderMap.h>
#include <icetray/python/dataclass_suite.hpp>

using boost::python::class_;
using boost::python::dataclass_suite;

void register_I3CylinderMap()
{
  class_< I3CylinderMap , boost::shared_ptr<I3CylinderMap> >("I3CylinderMap")
    .def(dataclass_suite<I3CylinderMap>())
    ;
  bp::implicitly_convertible<boost::shared_ptr<I3CylinderMap>, boost::shared_ptr<const I3CylinderMap> >();
}
