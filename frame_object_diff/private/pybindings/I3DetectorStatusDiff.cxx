/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3DetectorStatusDiff.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3DetectorStatusDiff.cxx
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#include <boost/preprocessor/seq/for_each.hpp>

#include <frame_object_diff/status/I3DetectorStatusDiff.h>
#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

// handle function overloads
namespace {
    I3DetectorStatusPtr (I3DetectorStatusDiff::*Unpack1)(const I3DetectorStatus& base) const
        = &I3DetectorStatusDiff::Unpack;
    I3DetectorStatusPtr (I3DetectorStatusDiff::*Unpack2)(I3DetectorStatusConstPtr base) const
        = &I3DetectorStatusDiff::Unpack;
}

void register_I3DetectorStatusDiff()
{
  bp::class_<I3DetectorStatusDiff, bp::bases<I3FrameObject>, I3DetectorStatusDiffPtr >
      ("I3DetectorStatusDiff", "A diff between two I3DetectorStatus objects.")
    .def(bp::init<I3DetectorStatusDiff>())
    .def(bp::init<const std::string, const I3DetectorStatus&, const I3DetectorStatus&>())
    .def(bp::init<const std::string, I3DetectorStatusConstPtr, I3DetectorStatusConstPtr>())
    .def("unpack", Unpack1, "Unpack a diff using a base I3DetectorStatus.")
    .def("unpack", Unpack2)
    #define RO_PROPERTIES (base_filename)(startTime)(endTime) \
                          (domStatus)(triggerStatus) \
                          (daqConfigurationName)
    BOOST_PP_SEQ_FOR_EACH(WRAP_RO, I3DetectorStatusDiff, RO_PROPERTIES)
    #undef RO_PROPERTIES
    .def(bp::dataclass_suite<I3DetectorStatusDiff>())
    ;
  register_pointer_conversions<I3DetectorStatusDiff>();
}
