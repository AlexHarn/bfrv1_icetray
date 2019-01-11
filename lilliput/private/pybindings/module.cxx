/**
 *
 * Implementation of Python bindings
 *
 * (c) 2015
 * the IceCube Collaboration
 * $Id$
 *
 * @file module.cxx
 * @date $Date$
 * @author boersma
 *
 */
#include <boost/preprocessor.hpp>

#include <icetray/load_project.h>

#define REGISTER_THESE_THINGS \
  (minimizer) (parametrization) (seedservice)

#define I3_REGISTRATION_FN_DECL(r, data, t) void BOOST_PP_CAT(register_, t)();
#define I3_REGISTER(r, data, t) BOOST_PP_CAT(register_, t)();

BOOST_PP_SEQ_FOR_EACH(I3_REGISTRATION_FN_DECL, ~, REGISTER_THESE_THINGS)

BOOST_PYTHON_MODULE(lilliput)
{

  load_project("lilliput", false);

  BOOST_PP_SEQ_FOR_EACH(I3_REGISTER, ~, REGISTER_THESE_THINGS);
}
