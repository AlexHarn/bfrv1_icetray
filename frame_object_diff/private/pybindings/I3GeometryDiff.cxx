/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3GeometryDiff.cxx
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#include <boost/preprocessor/seq/for_each.hpp>

#include <frame_object_diff/geometry/I3GeometryDiff.h>
#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

// handle function overloads
namespace {
    I3GeometryPtr (I3GeometryDiff::*Unpack1)(const I3Geometry& base) const
        = &I3GeometryDiff::Unpack;
    I3GeometryPtr (I3GeometryDiff::*Unpack2)(I3GeometryConstPtr base) const
        = &I3GeometryDiff::Unpack;
}

void register_I3GeometryDiff()
{
  bp::class_<I3GeometryDiff, bp::bases<I3FrameObject>, I3GeometryDiffPtr >
      ("I3GeometryDiff", "A diff between two I3Geometry objects.")
    .def(bp::init<I3GeometryDiff>())
    .def(bp::init<const std::string, const I3Geometry&, const I3Geometry&>())
    .def(bp::init<const std::string, I3GeometryConstPtr, I3GeometryConstPtr>())
    .def("unpack", Unpack1, "Unpack a diff using a base I3Geometry.")
    .def("unpack", Unpack2, "Unpack a diff using a base I3Geometry.")
    #define RO_PROPERTIES (base_filename)(omgeo)(stationgeo) \
                          (startTime)(endTime)
    BOOST_PP_SEQ_FOR_EACH(WRAP_RO, I3GeometryDiff, RO_PROPERTIES)
    #undef RO_PROPERTIES
    .def(bp::dataclass_suite<I3GeometryDiff>())
    ;
  register_pointer_conversions<I3GeometryDiff>();
}
