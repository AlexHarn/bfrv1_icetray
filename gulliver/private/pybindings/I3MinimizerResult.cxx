#include <gulliver/I3MinimizerResult.h>

namespace bp = boost::python;

void register_I3MinimizerResult()
{
    bp::class_<I3MinimizerResult>("I3MinimizerResult", bp::init<int>())
        .def_readwrite("converged", &I3MinimizerResult::converged_)
        .def_readwrite("minval", &I3MinimizerResult::minval_)
        .def_readwrite("par", &I3MinimizerResult::par_)
        .def_readwrite("err", &I3MinimizerResult::err_)
        .def_readwrite("diagnostics", &I3MinimizerResult::diagnostics_)
    ;
}
