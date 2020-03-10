
#include <icetray/serialization.h>
#include <dataclasses/I3Vector.h>
#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

template <typename T, typename U>
std::pair< T, U >
py_make_pair( T t, U u)
{
    return std::make_pair(t,u);
}

template <typename T, typename U>
void
register_std_pair(const char* s)
{
  typedef std::pair<T, U> type_t;

  bp::class_<type_t>(s)
    .def_readwrite("first", &type_t::first)
    .def_readwrite("second", &type_t::second)
    .def(bp::dataclass_suite<type_t>())
    ;
  bp::def("make_pair", &py_make_pair<T, U>);
}

void register_vectors()
{
    register_std_pair<size_t,size_t>("Range");
    bp::class_<I3Vector<std::pair<size_t,size_t>>, boost::shared_ptr<I3Vector<std::pair<size_t,size_t>>>,
        bp::bases<I3FrameObject>>("RangeVector")
        .def(bp::dataclass_suite<I3Vector<std::pair<size_t,size_t>>>())
    ;
    register_pointer_conversions<I3Vector<std::pair<size_t,size_t>>>();
}

typedef I3Vector<std::pair<size_t,size_t>> RangeVector;
I3_SERIALIZABLE(RangeVector);

