#include <vector>

#include <icetray/python/dataclass_suite.hpp>
#include <tableio/converter/pybindings.h>
#include <tableio/converter/I3MapConverter.h>
#include <tpx/I3TopPulseInfo.h>
#include <tpx/converter/I3TopPulseInfoConverter.h>

using namespace boost::python;

void register_I3TopPulseInfo()
{
  enum_<I3TopPulseInfo::Status>("Status")
    .value("OK", I3TopPulseInfo::OK)
    .value("Saturated", I3TopPulseInfo::Saturated)
    .value("BadCharge", I3TopPulseInfo::BadCharge)
    .value("BadTime", I3TopPulseInfo::BadTime)
    .export_values()
    ;


  class_<I3TopPulseInfo, boost::shared_ptr<I3TopPulseInfo> >("I3TopPulseInfo")
    .def_readwrite("amplitude", &I3TopPulseInfo::amplitude)
    .def_readwrite("risetime", &I3TopPulseInfo::risetime)
    .def_readwrite("trailingEdge", &I3TopPulseInfo::trailingEdge)
    .def_readwrite("status", &I3TopPulseInfo::status)
    .def_readwrite("channel", &I3TopPulseInfo::channel)
    .def_readwrite("source_id", &I3TopPulseInfo::sourceID)
    .def(self == self)
    .def(dataclass_suite<I3TopPulseInfo>())
    ;
  
  class_<std::vector<I3TopPulseInfo> >("vector_I3TopPulseInfo")
    .def(dataclass_suite<std::vector<I3TopPulseInfo> >())
    ;
  
  class_<I3TopPulseInfoSeriesMap, bases<I3FrameObject>, I3TopPulseInfoSeriesMapPtr>("I3TopPulseInfoSeriesMap")
    .def(dataclass_suite<I3TopPulseInfoSeriesMap>())
    ;
  register_pointer_conversions<I3TopPulseInfoSeriesMap>();

  I3CONVERTER_NAMESPACE(tpx);
  typedef I3MapOMKeyVectorConverter<convert::I3TopPulseInfo> I3TopPulseInfoSeriesMapConverter;
  I3_MAP_CONVERTER_EXPORT_DEFAULT(I3TopPulseInfoSeriesMapConverter, "Dumps I3TopPulseInfoSeriesMap objects");
}

