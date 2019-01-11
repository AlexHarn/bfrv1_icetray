#include <gulliver/I3FitParameterInitSpecs.h>
#include <icetray/python/list_indexing_suite.hpp>

namespace bp = boost::python;

void register_I3FitParameterInitSpecs()
{
    bp::class_<I3FitParameterInitSpecs>("I3FitParameterInitSpecs", bp::init<std::string>())
        .def_readwrite("name", &I3FitParameterInitSpecs::name_)
        .def_readwrite("initval", &I3FitParameterInitSpecs::initval_)
        .def_readwrite("stepsize", &I3FitParameterInitSpecs::stepsize_)
        .def_readwrite("minval", &I3FitParameterInitSpecs::minval_)
        .def_readwrite("maxval", &I3FitParameterInitSpecs::maxval_)
    ;

    bp::class_<std::vector<I3FitParameterInitSpecs> >("I3FitParameterInitSpecsSeries")
        .def(bp::list_indexing_suite<std::vector<I3FitParameterInitSpecs> >())
    ;
}
