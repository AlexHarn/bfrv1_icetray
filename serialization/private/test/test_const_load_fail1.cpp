/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// test_const.cpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// compile only

#include <archive/text_iarchive.hpp>

using namespace icecube::archive;

struct A {
    template<class Archive>
    void serialize(Archive & ar, unsigned int version) {
    }
};

void f1(text_iarchive & ia, const A & a){
    ia >> a;
}
