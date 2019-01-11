#include <gulliver/I3EventLogLikelihoodBase.h>
#include <spline-reco/I3SplineRecoLikelihood.h>
#include <boost/python.hpp>
#include <boost/python/args.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <icetray/python/context_suite.hpp>

#define ENUM_DEF(r,data,T) .value(BOOST_PP_STRINGIZE(T), data::T)
#define WRAP_CONTEXT(base) .def(icetray::python::context_suite<base>())

namespace bp = boost::python;

void register_ServiceWrappers()
{
	bp::class_<I3SplineRecoLikelihood,bp::bases<I3EventLogLikelihoodBase>,
		boost::shared_ptr<I3SplineRecoLikelihood>,boost::noncopyable>
		spline_llh = bp::class_<I3SplineRecoLikelihood,
		bp::bases<I3EventLogLikelihoodBase>,
		boost::shared_ptr<I3SplineRecoLikelihood> ,boost::noncopyable>
		("I3SplineRecoLikelihood","Spline LLH");
		
	{
        bp::scope spline_llh_scope(spline_llh);

        bp::enum_<I3SplineRecoLikelihood::LlhChoice>("LlhChoice")
            BOOST_PP_SEQ_FOR_EACH(ENUM_DEF,I3SplineRecoLikelihood, I3SPLINERECOLIKELIHOOD_LlhChoice )
            .export_values();
       
    }
    
	spline_llh
	  .def_readwrite("PhotonicsService",&I3SplineRecoLikelihood::ps)
      .def_readwrite("PhotonicsServiceRandomNoise",&I3SplineRecoLikelihood::random_noise_ps)
      .def_readwrite("PhotonicsServiceStochastics",&I3SplineRecoLikelihood::stochastics_ps)
	  .def_readwrite ("Pulses", &I3SplineRecoLikelihood::pulses_name)
      .def_readwrite ("Likelihood", &I3SplineRecoLikelihood::llhChoice)
      .def_readwrite ("NoiseModel", &I3SplineRecoLikelihood::noiseModel)
      .def_readwrite ("FloorWeight", &I3SplineRecoLikelihood::floorWeight)
      .def_readwrite ("ModelStochastics", &I3SplineRecoLikelihood::modelStochastics)
      .def_readwrite ("NoiseRate", &I3SplineRecoLikelihood::noiseRate)
      .def_readwrite ("E_Estimators", &I3SplineRecoLikelihood::E_estimator_names)
      .def_readwrite ("ChargeWeightedLLH", &I3SplineRecoLikelihood::chargeWeight)
      .def_readwrite ("PreJitter", &I3SplineRecoLikelihood::preJitter)
      .def_readwrite ("PostJitter", &I3SplineRecoLikelihood::postJitter)
      .def_readwrite ("KSConfidenceLevel", &I3SplineRecoLikelihood::confidence_level)
      .def_readwrite ("ChargeCalcStep", &I3SplineRecoLikelihood::chargeCalcStep)
      .def_readwrite ("CutMode", &I3SplineRecoLikelihood::cutMode)
      .def_readwrite ("EnergyDependentMPE", &I3SplineRecoLikelihood::EnergyDependentMPE)
       WRAP_CONTEXT(I3SplineRecoLikelihood);
     
  
	
}
