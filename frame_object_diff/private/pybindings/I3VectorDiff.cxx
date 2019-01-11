/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3VectorDiff.cxx 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3VectorDiff.cxx
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#include <boost/preprocessor/seq/for_each.hpp>

#include <frame_object_diff/I3VectorDiff.h>
#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

// handle function overloads
namespace {
    I3VectorOMKeyPtr (I3VectorOMKeyDiff::*Unpack1)(const I3VectorOMKey& base) const
        = &I3VectorOMKeyDiff::Unpack;
    I3VectorOMKeyPtr (I3VectorOMKeyDiff::*Unpack2)(I3VectorOMKeyConstPtr base) const
        = &I3VectorOMKeyDiff::Unpack;
    I3VectorTankKeyPtr (I3VectorTankKeyDiff::*Unpack3)(const I3VectorTankKey& base) const
        = &I3VectorTankKeyDiff::Unpack;
    I3VectorTankKeyPtr (I3VectorTankKeyDiff::*Unpack4)(I3VectorTankKeyConstPtr base) const
        = &I3VectorTankKeyDiff::Unpack;
    I3VectorIntPtr (I3VectorIntDiff::*Unpack5)(const I3VectorInt& base) const
        = &I3VectorIntDiff::Unpack;
    I3VectorIntPtr (I3VectorIntDiff::*Unpack6)(I3VectorIntConstPtr base) const
        = &I3VectorIntDiff::Unpack;
}

void register_I3VectorDiff()
{
  bp::class_<I3VectorOMKeyDiff, bp::bases<I3FrameObject>, I3VectorOMKeyDiffPtr >
      ("I3VectorOMKeyDiff", "A diff between two I3VectorOMKey objects.")
    .def(bp::init<I3VectorOMKeyDiff>())
    .def(bp::init<const std::string, const I3VectorOMKey&, const I3VectorOMKey&>())
    .def(bp::init<const std::string, I3VectorOMKeyConstPtr, I3VectorOMKeyConstPtr>())
    .def("unpack", Unpack1, "Unpack a diff using a base I3VectorOMKey.")
    .def("unpack", Unpack2)
    .add_property("base_filename",&I3VectorOMKeyDiff::GetFilename)
    .def(bp::dataclass_suite<I3VectorOMKeyDiff>())
    ;
  register_pointer_conversions<I3VectorOMKeyDiff>();
  bp::class_<I3VectorTankKeyDiff, bp::bases<I3FrameObject>, I3VectorTankKeyDiffPtr >
      ("I3VectorTankKeyDiff", "A diff between two I3VectorTankKey objects.")
    .def(bp::init<I3VectorTankKeyDiff>())
    .def(bp::init<const std::string, const I3VectorTankKey&, const I3VectorTankKey&>())
    .def(bp::init<const std::string, I3VectorTankKeyConstPtr, I3VectorTankKeyConstPtr>())
    .def("unpack", Unpack3, "Unpack a diff using a base I3VectorTankKey.")
    .def("unpack", Unpack4)
    .add_property("base_filename",&I3VectorTankKeyDiff::GetFilename)
    .def(bp::dataclass_suite<I3VectorTankKeyDiff>())
    ;
  register_pointer_conversions<I3VectorTankKeyDiff>();
  bp::class_<I3VectorIntDiff, bp::bases<I3FrameObject>, I3VectorIntDiffPtr >
      ("I3VectorIntDiff", "A diff between two I3VectorInt objects.")
    .def(bp::init<I3VectorIntDiff>())
    .def(bp::init<const std::string, const I3VectorInt&, const I3VectorInt&>())
    .def(bp::init<const std::string, I3VectorIntConstPtr, I3VectorIntConstPtr>())
    .def("unpack", Unpack5, "Unpack a diff using a base I3VectorInt.")
    .def("unpack", Unpack6)
    .add_property("base_filename",&I3VectorIntDiff::GetFilename)
    .def(bp::dataclass_suite<I3VectorIntDiff>())
    ;
  register_pointer_conversions<I3VectorIntDiff>();
}
