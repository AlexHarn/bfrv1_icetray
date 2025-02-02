/**
 *  $Id$
 *  
 *  Copyright (C) 2008
 *  Fabian Kislat  <fabian.kislat@desy.de>
 *  and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *  
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *  
 */

#include <icetray/load_project.h>
#include <boost/preprocessor.hpp>
#include <boost/python.hpp>

/* CLASSES_TO_REGISTER is generated by cmake and passed via -D
 *
 * If you add a new wrapper, make sure it is called
 * <name>.cxx and contains a function "void register_<name>()"
 */
#define I3_REGISTRATION_FN_DECL(r, data, t) void BOOST_PP_CAT(register_,t)();
#define I3_REGISTER(r, data, t) BOOST_PP_CAT(register_,t)();
BOOST_PP_SEQ_FOR_EACH(I3_REGISTRATION_FN_DECL, ~, CLASSES_TO_REGISTER)

BOOST_PYTHON_MODULE(recclasses)
{
  load_project("recclasses", false);

  BOOST_PP_SEQ_FOR_EACH(I3_REGISTER, ~, CLASSES_TO_REGISTER);
}
