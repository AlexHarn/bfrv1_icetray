/*
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de> Last changed by: $LastChangedBy$
 */

#include <vector>

#include <icetray/python/dataclass_suite.hpp>
#include <tableio/converter/pybindings.h>
#include <tableio/converter/I3MapConverter.h>
#include <tpx/I3IceTopBaseline.h>
#include <tpx/converter/convert_I3IceTopBaseline.h>


using namespace boost::python;


void register_I3IceTopBaseline()
{
  class_<I3IceTopBaseline, boost::shared_ptr<I3IceTopBaseline> >("I3IceTopBaseline")
    .def(init<I3Waveform::Source, uint8_t, uint8_t>("Arguments: Source type, channel, source ID"))
    .def(init<I3Waveform::Source, uint8_t, uint8_t, float, float, float>("Arguments: Source type, channel, source ID, baseline, slope, rms"))
    .def_readwrite("source", &I3IceTopBaseline::source)
    .def_readwrite("channel", &I3IceTopBaseline::channel)
    .def_readwrite("source_id", &I3IceTopBaseline::sourceID)
    .def_readwrite("baseline", &I3IceTopBaseline::baseline)
    .def_readwrite("slope", &I3IceTopBaseline::slope)
    .def_readwrite("rms", &I3IceTopBaseline::rms)
    .def(self == self)
    .def(dataclass_suite<I3IceTopBaseline>())
    ;
  
  class_<std::vector<I3IceTopBaseline> >("vector_I3IceTopBaseline")
    .def(dataclass_suite<std::vector<I3IceTopBaseline> >())
    ;
  
  class_<I3IceTopBaselineSeriesMap, bases<I3FrameObject>, I3IceTopBaselineSeriesMapPtr>("I3IceTopBaselineSeriesMap")
    .def(dataclass_suite<I3IceTopBaselineSeriesMap>())
    ;
  register_pointer_conversions<I3IceTopBaselineSeriesMap>();

  I3CONVERTER_NAMESPACE(tpx);
  typedef I3MapOMKeyVectorConverter< convert::I3IceTopBaseline > I3IceTopBaselineSeriesMapConverter;
  I3_MAP_CONVERTER_EXPORT_DEFAULT(I3IceTopBaselineSeriesMapConverter, "Dumps I3IceTopBaselineSeriesMap objects");
}
