#include <icetray/load_project.h>
#include <boost/python.hpp>
#include <tableio/converter/pybindings.h>
#include <gulliver-bootstrap/BootstrapParams.h>
#include <gulliver-bootstrap/BootstrapParamsConverter.h>
#include <gulliver-bootstrap/BootstrappingLikelihoodService.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(gulliver_bootstrap){
	enum_<BootstrappingLikelihoodService::BootstrapOption>("BootstrapOption")
	.value("Poisson",BootstrappingLikelihoodService::POISSON)
	.value("Multinomial",BootstrappingLikelihoodService::MULTINOMIAL)
	.export_values()
	;
	
	{
		scope params_scope =
		class_<BootstrapParams,bases<I3FrameObject>, boost::shared_ptr<BootstrapParams> >("BootstrapParams")
		.def_readwrite("status",&BootstrapParams::status)
		.def_readwrite("successfulFits",&BootstrapParams::successfulFits)
		.def_readwrite("totalFits",&BootstrapParams::totalFits)
		;
		
		enum_<BootstrapParams::ResultStatus>("ResultStatus")
		.value("OK",BootstrapParams::OK)
		.value("Underflow",BootstrapParams::Underflow)
		.value("Overflow",BootstrapParams::Overflow)
		.value("NoValidFits",BootstrapParams::NoValidFits)
		.export_values()
		;
	}
	
	register_pointer_conversions<BootstrapParams>();
	
	I3CONVERTER_NAMESPACE(gulliver_bootstrap);
	I3CONVERTER_EXPORT_DEFAULT(BootstrapParamsConverter,"");
}
