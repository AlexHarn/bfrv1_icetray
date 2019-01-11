#include "phys-services/I3RandomService.h"
#include "neutrino-generator/legacy/I3NeutrinoPropagator.h"
#include "neutrino-generator/legacy/I3NuGInteractionInfo.h"
#include "neutrino-generator/Steering.h"


namespace bp = boost::python;

void
register_I3NuGInteractionInfo()
{
    bp::class_<I3NuGInteractionInfo, I3NuGInteractionInfoPtr, boost::noncopyable>
        ("I3NuGInteractionInfo",
        bp::init<I3RandomServicePtr, nugen::SteeringPtr, const std::string &>())
        .def("initialize", &I3NuGInteractionInfo::Initialize)
        .def("view_interaction_info", &I3NuGInteractionInfo::ViewInteractionInfo)
        .def("get_xsec_cgs", &I3NuGInteractionInfo::GetXsecCGS)
    ;
}

void register_I3NeutrinoGenerator()
{
    bp::class_<I3NeutrinoGeneratorBase, boost::shared_ptr<I3NeutrinoGeneratorBase>, boost::noncopyable>
        ("I3NeutrinoGeneratorBase", bp::no_init) 
        .add_property("prop_mode", &I3NeutrinoGeneratorBase::GetPropagationMode, &I3NeutrinoGeneratorBase::SetPropagationMode)
        .add_property("crosssectionxcolumndepth_option", &I3NeutrinoGeneratorBase::GetCrosssectionxColumndepthOption, &I3NeutrinoGeneratorBase::SetCrosssectionxColumndepthOption)
    ;

    bp::class_<I3NeutrinoPropagator, boost::shared_ptr<I3NeutrinoPropagator>, 
        bp::bases<I3PropagatorService, I3NeutrinoGeneratorBase>, boost::noncopyable> 
        ("I3NeutrinoPropagator", bp::init<I3RandomServicePtr, nugen::SteeringPtr, I3NuGInteractionInfoPtr>())
        .def(bp::init<I3RandomServicePtr, nugen::SteeringPtr, I3NuGInteractionInfoPtr, nugen::PropagationMode, int, int, int, int>())
        .def("propagate_in_earth", 
           &I3NeutrinoPropagator::PropagateInEarthWrapper,
           (bp::arg("particle"), bp::arg("frame")), "propagate")
    ;
}


