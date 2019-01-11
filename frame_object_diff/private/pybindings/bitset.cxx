/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3VectorDiff.cxx 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3VectorDiff.cxx
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#include <boost/preprocessor/seq/for_each.hpp>

#include <bitset>
#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

#define SEQ (0)(1)(2)(3)(4)(5)(6)(7)(8)(9)(10) \
    (11)(12)(13)(14)(15)(16)(17)(18)(19)(20) \
    (21)(22)(23)(24)(25)(26)(27)(28)(29)(30)

// handle function overloads
namespace {
#define OVERLOADS(T, data, elem) \
  void set_all_bits##T (std::bitset<T>& b){b.set();} \
  void unset_all_bits##T (std::bitset<T>& b){b.reset();} \
  bool get_bit##T (std::bitset<T>& b, int i){ \
    if (i <= -1*T || i >= T) { \
      PyErr_SetString(PyExc_IndexError, "out of range"); \
      bp::throw_error_already_set(); \
      return false; \
    } else if (i < 0) \
      return b[T-i]; \
    else \
      return b[i]; \
  } \
  void set_bit##T (std::bitset<T>& b, int i, bool v){ \
    if (i <= -1*T || i >= T) { \
      PyErr_SetString(PyExc_IndexError, "out of range"); \
      bp::throw_error_already_set(); \
    } else if (i < 0) \
      b[T-i] = v; \
    else \
      b[i] = v; \
  }
BOOST_PP_SEQ_FOR_EACH(OVERLOADS, _, SEQ)
}

void register_bitset()
{
#define BITSET(T, data, elem) \
  bp::class_<std::bitset<T> >("bitset" #T ,"A bitset of " #T " bits.") \
    .add_property("size",&std::bitset<T>::size) \
    .add_property("count",&std::bitset<T>::count) \
    .def("set",set_all_bits##T, "set all bits to true") \
    .def("reset",unset_all_bits##T, "set all bits to false") \
    .def("__getitem__",get_bit##T, "get bit value at position") \
    .def("__setitem__",set_bit##T, "set bit value at position") \
    .def("__str__", &stream_to_string<std::bitset<T> >) \
    ;

BOOST_PP_SEQ_FOR_EACH(BITSET, _, SEQ)

}
