#include "wrappers.h"

using namespace boost::python;

void register_I3PhotoSplineService() {
#ifdef USE_PHOTOSPLINE
	class_<I3PhotoSplineTable,
	    boost::shared_ptr<I3PhotoSplineTable > >("I3PhotoSplineTable")
	    .def("SetupTable", &I3PhotoSplineTable::SetupTable)
	    .def("Eval", &splinetableeval)
#ifdef USE_NUMPY
	    .def("EvalHessian", &splinetableeval_hessian,
	        args("self", "coordinates"))
	    .def("EvalGradients", &splinetableeval_gradients,
	        args("self", "coordinates"))
#endif
            #define PROPS (Geometry)(Parity)(NGroupTable)
            BOOST_PP_SEQ_FOR_EACH(WRAP_PROP_RO, I3PhotoSplineTable, PROPS)
            #undef PROPS
	    ;

	class_<I3PhotoSplineService,
	    boost::shared_ptr<I3PhotoSplineService > ,
	    bases<I3PhotonicsService >, boost::noncopyable>("I3PhotoSplineService")
	    .def(init<const std::string &, const std::string &, double, const std::string &, double>(
		(args("amplitudetable", "timingtable"), args("timingSigma")=0.0, args("tiltTableDir")="", args("maxRadius")=std::numeric_limits<double>::infinity()),
	        "Create an I3PhotoSplineService object."))
	    .def("LoadSplineTables", &I3PhotoSplineService::LoadSplineTables)
#ifdef USE_NUMPY
	    .def("GetKnotVector", &GetKnotVector,
	        args("self", "dimension"),
	        "Returns knot vector along dimension n.")
	   	.def("GetTimeSliceCoefficients", &GetTimeSliceCoefficients,
	        args("self", "time_dimension", "derivative", "area_norm"),
	        "Returns knot vector along dimension n.")
#endif
	   	.def("GetAxisOrder", &GetAxisOrder,
	        args("self", "dimension"),
	        "Returns order of dimension n.")
	    #define PHOTOSPLINE_PROPS (Geometry)(Parity)
	    BOOST_PP_SEQ_FOR_EACH(WRAP_PROP_RO, I3PhotoSplineService, PHOTOSPLINE_PROPS)
	    ;
#endif	
}

