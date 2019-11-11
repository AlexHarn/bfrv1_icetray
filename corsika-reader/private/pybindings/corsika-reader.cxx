/**
 * @brief Python bindings for corsika-reader
 *
 * @copyright (C) 2018 The Icecube Collaboration
 * 
 * @file corsika-reader.cxx
 * @author Kevin Meagher
 * @date June 2018
 *
 */

#include "icetray/load_project.h"
#include "corsika-reader/I3CorsikaWeight.h"

namespace bp=boost::python;

#ifdef USE_CORSIKA_CLIENT
#include "corsika-reader/I3CORSIKAService.h"

void add_bias(I3Particle &p, float mu_bias, I3Frame& frame){
  auto biases = boost::make_shared<ShowerBiasMap>();
  biases->emplace(p.GetID(), ShowerBiasMap::mapped_type(CorsikaClient::Mu,mu_bias,1));
  frame.Put("ShowerBias", biases);
}
#endif //USE_CORSIKA_CLIENT

BOOST_PYTHON_MODULE(corsika_reader)
{
  bp::import("icecube.icetray");
  bp::import("icecube.sim_services");



  bp::class_<I3CorsikaWeight,
             bp::bases<I3FrameObject>,
             boost::shared_ptr<I3CorsikaWeight> >
    ("I3CorsikaWeight")
    .def_readonly("primary",    &I3CorsikaWeight::primary)
    .def_readonly("bias_factor",&I3CorsikaWeight::bias_factor)
    .def_readonly("bias_target",&I3CorsikaWeight::bias_target)
    .def_readonly("weight",     &I3CorsikaWeight::weight)
    .def_readonly("max_x",      &I3CorsikaWeight::max_x)    
    ;

#ifdef USE_CORSIKA_CLIENT 
  bp::def("add_bias",&add_bias);
  bp::class_<CorsikaService,
             boost::shared_ptr<CorsikaService>,
             bp::bases<I3IncrementalEventGeneratorService>,
             boost::noncopyable>
    ("CorsikaService",
     bp::init<std::string>()
     )
    .def("StartShower",&CorsikaService::StartShower)
    .def("NextParticle",&CorsikaService::NextParticle)
    .def("EndEvent",&CorsikaService::EndEvent)
    ;
#endif //USE_CORSIKA_CLIENT
}
    
