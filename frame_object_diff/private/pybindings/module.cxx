/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file main.cxx
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#include <icetray/I3FrameObject.h>
#include <icetray/load_project.h>
#include <boost/preprocessor.hpp>

using namespace boost::python;

#define REGISTER_THESE_THINGS \
    (I3GeometryDiff)(I3CalibrationDiff)(I3DOMCalibrationDiff) \
    (I3DetectorStatusDiff) \
    (I3VectorDiff)(bitset)

#define I3_REGISTRATION_FN_DECL(r, data, t) void BOOST_PP_CAT(register_,t)();
#define I3_REGISTER(r, data, t) BOOST_PP_CAT(register_,t)();
BOOST_PP_SEQ_FOR_EACH(I3_REGISTRATION_FN_DECL, ~, REGISTER_THESE_THINGS)

BOOST_PYTHON_MODULE(frame_object_diff)
{
  load_project("frame_object_diff", false);
  BOOST_PP_SEQ_FOR_EACH(I3_REGISTER, ~, REGISTER_THESE_THINGS);
}

#undef REGISTER_THESE_THINGS
#undef I3_REGISTRATION_FN_DECL
#undef I3_REGISTER
