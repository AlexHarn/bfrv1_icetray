/**
 * Copyright (C) 2013 - 2014
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/cpp/streamToString.hpp
 * @date $Date$
 * @brief This file contains the definition of the streamToString
 *        template function that converts a given class instance into a
 *        std::string by using the operator<< function.
 *
 *        Note: The operator<< function for the particular class of the form
 *              std::ostream& operator<<(std::ostream& os, const T& rhs);
 *              must be defined BEFORE this header is included by the first
 *              time! Otherwise the clang compiler will fail with an error not
 *              finding this function.
 *
 *              Basically, this means, that one should include this header file
 *              at the very last.
 *
 *        ----------------------------------------------------------------------
 *        This file is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation; either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef STTOOLS_CPP_STREAMTOSTRING_HPP_INCLUDED
#define STTOOLS_CPP_STREAMTOSTRING_HPP_INCLUDED

#include <string>
#include <sstream>

namespace sttools {

template <class T>
std::string
streamToString(const T& cls)
{
    std::stringstream ss;
    ss << cls;
    return ss.str();
}

}// namespace sttools


#endif//STTOOLS_CPP_STREAMTOSTRING_HPP_INCLUDED
