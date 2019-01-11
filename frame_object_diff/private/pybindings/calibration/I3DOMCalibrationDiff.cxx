/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3DOMCalibrationDiff.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3DOMCalibrationDiff.cxx
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#include <vector>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/python/iterator.hpp>

#include <frame_object_diff/calibration/I3DOMCalibrationDiff.h>
#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

// handle function overloads
namespace {
    I3DOMCalibrationPtr (I3DOMCalibrationDiff::*Unpack1)(const I3DOMCalibration& base) const
        = &I3DOMCalibrationDiff::Unpack;
    I3DOMCalibrationPtr (I3DOMCalibrationDiff::*Unpack2)(I3DOMCalibrationConstPtr base) const
        = &I3DOMCalibrationDiff::Unpack;
    I3DOMCalibrationMapPtr (I3DOMCalibrationMapDiff::*Unpack3)(const I3DOMCalibrationMap& base) const
        = &I3DOMCalibrationMapDiff::Unpack;
    I3DOMCalibrationMapPtr (I3DOMCalibrationMapDiff::*Unpack4)(I3DOMCalibrationMapConstPtr base) const
        = &I3DOMCalibrationMapDiff::Unpack;

    std::vector<OMKey> plus_keys(I3DOMCalibrationMapDiff& base)
    {
        std::vector<OMKey> ret;
        for(I3DOMCalibrationMapDiff::plus_iterator i=base.begin_plus();
            i != base.end_plus();i++)
        {
            ret.push_back(i->first);
        }
        return ret;
    }

    std::vector<I3DOMCalibrationDiff> plus_values(I3DOMCalibrationMapDiff& base)
    {
        std::vector<I3DOMCalibrationDiff> ret;
        for(I3DOMCalibrationMapDiff::plus_iterator i=base.begin_plus();
            i != base.end_plus();i++)
        {
            ret.push_back(i->second);
        }
        return ret;
    }
}

void register_I3DOMCalibrationDiff()
{
  bp::class_<I3DOMCalibrationDiff, I3DOMCalibrationDiffPtr >
      ("I3DOMCalibrationDiff", "A diff between two I3DOMCalibration objects.")
    .def(bp::init<I3DOMCalibrationDiff>())
    .def(bp::init<const I3DOMCalibration&, const I3DOMCalibration&>())
    .def(bp::init<I3DOMCalibrationConstPtr, I3DOMCalibrationConstPtr>())
    .def("unpack", Unpack1,"Unpack a diff using a base I3DOMCalibration.")
    .def("unpack", Unpack2)
    #define RO_PROPERTIES (bits)
    BOOST_PP_SEQ_FOR_EACH(WRAP_RO, I3DOMCalibrationDiff, RO_PROPERTIES)
    #undef RO_PROPERTIES
    .def(bp::dataclass_suite<I3DOMCalibrationDiff>())
    ;
  bp::implicitly_convertible<boost::shared_ptr<I3DOMCalibrationDiff>,
      boost::shared_ptr<const I3DOMCalibrationDiff> >();

  bp::class_<I3DOMCalibrationMapDiff, I3DOMCalibrationMapDiffPtr >
      ("I3DOMCalibrationMapDiff", "A diff between two I3DOMCalibrationMap objects.")
    .def(bp::init<I3DOMCalibrationMapDiff>())
    .def(bp::init<const I3DOMCalibrationMap&, const I3DOMCalibrationMap&>())
    .def(bp::init<I3DOMCalibrationMapConstPtr, I3DOMCalibrationMapConstPtr>())
    .def("unpack", Unpack3,"Unpack a diff using a base I3DOMCalibrationMap.")
    .def("unpack", Unpack4)
    .def("size", &I3DOMCalibrationMapDiff::size, "Size of plus and minus diff.")
    /*.def("iter_plus", bp::range<bp::return_value_policy<bp::copy_non_const_reference> >
      (
        (I3DOMCalibrationMapDiff::plus_iterator(I3DOMCalibrationMapDiff::*)() const) &I3DOMCalibrationMapDiff::begin_plus,
        (I3DOMCalibrationMapDiff::plus_iterator(I3DOMCalibrationMapDiff::*)() const) &I3DOMCalibrationMapDiff::end_plus
      )
    )*/
    .def("plus_keys", &plus_keys)
    .def("plus_values", &plus_values)
    .def(bp::dataclass_suite<I3DOMCalibrationMapDiff>())
    ;
  bp::implicitly_convertible<boost::shared_ptr<I3DOMCalibrationMapDiff>,
      boost::shared_ptr<const I3DOMCalibrationMapDiff> >();

  bp::class_<std::vector<I3DOMCalibrationDiff> >("I3DOMCalibrationVectorDiff")
    .def(bp::dataclass_suite<std::vector<I3DOMCalibrationDiff> >())
    ;
}
