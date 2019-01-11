
#include <daq-decode/I3DAQDecoderUtil.h>

using namespace boost::python;

void register_I3DAQDecoderUtil()
{
  class_<I3DAQDecoderUtil, boost::shared_ptr<I3DAQDecoderUtil> >("I3DAQDecoderUtil")
    .def("fill_trigger", &I3DAQDecoderUtil::FillTrigger)
    .staticmethod("fill_trigger")
    .def("create_trigger_key", &I3DAQDecoderUtil::CreateTriggerKey)
    .staticmethod("create_trigger_key")
    .def("add_trigger", &I3DAQDecoderUtil::AddTrigger)
    .staticmethod("add_trigger")
    ;
}
