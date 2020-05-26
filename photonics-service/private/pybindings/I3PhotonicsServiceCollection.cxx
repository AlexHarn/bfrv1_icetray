#include "photonics-service/I3PhotonicsServiceCollection.h"
#include "icetray/python/std_map_indexing_suite.hpp"

using namespace boost::python;

void register_I3PhotonicsServiceCollection() {

	typedef std::map<I3OMGeo::OMType, I3PhotonicsServicePtr> ServiceMap;
	class_<ServiceMap, boost::shared_ptr<ServiceMap> >("I3MapOMTypeI3PhotonicsService")
	    .def(std_map_indexing_suite<ServiceMap>())
	;

	class_<I3PhotonicsServiceCollection,
	    boost::shared_ptr<I3PhotonicsServiceCollection > ,
	    bases<I3PhotonicsService >, boost::noncopyable>("I3PhotonicsServiceCollection", no_init)
	    .def(init<ServiceMap&>(
	        args("services"),
	        "Create an I3PhotoSplineService object."))
	    ;
}

