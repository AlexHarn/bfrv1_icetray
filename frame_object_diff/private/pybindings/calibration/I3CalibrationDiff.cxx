/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3CalibrationDiff.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3CalibrationDiff.cxx
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#include <boost/preprocessor/seq/for_each.hpp>

#include <frame_object_diff/calibration/I3CalibrationDiff.h>
#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

// handle function overloads
namespace {
    I3CalibrationPtr (I3CalibrationDiff::*Unpack1)(const I3Calibration& base) const 
        = &I3CalibrationDiff::Unpack;
    I3CalibrationPtr (I3CalibrationDiff::*Unpack2)(I3CalibrationConstPtr base) const 
        = &I3CalibrationDiff::Unpack;
}

void register_I3CalibrationDiff()
{
  bp::class_<I3CalibrationDiff, bp::bases<I3FrameObject>, I3CalibrationDiffPtr >
      ("I3CalibrationDiff", "A diff between two I3Calibration objects.")
    .def(bp::init<I3CalibrationDiff>())
    .def(bp::init<const std::string, const I3Calibration&, const I3Calibration&>())
    .def(bp::init<const std::string, I3CalibrationConstPtr, I3CalibrationConstPtr>())
    .def("unpack", Unpack1,"Unpack a diff using a base I3Calibration.")
    .def("unpack", Unpack2)
    #define RO_PROPERTIES (base_filename)(startTime)(endTime) \
                          (domCal)(vemCal)
    BOOST_PP_SEQ_FOR_EACH(WRAP_RO, I3CalibrationDiff, RO_PROPERTIES)
    #undef RO_PROPERTIES
    .def(bp::dataclass_suite<I3CalibrationDiff>())
    ;
  register_pointer_conversions<I3CalibrationDiff>();
}
