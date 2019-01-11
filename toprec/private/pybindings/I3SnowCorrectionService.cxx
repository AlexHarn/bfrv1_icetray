#include "toprec/I3SnowCorrectionService.h"
#include <icetray/python/dataclass_suite.hpp>
#include <boost/python.hpp>

void register_I3SnowCorrectionService() {
  using namespace boost::python;

  class_<
    I3SnowCorrectionServiceBase,
    I3SnowCorrectionServiceBasePtr,
    boost::noncopyable
  >("I3SnowCorrectionService", init<std::string>(arg("name")))
    .def("attenuation_factor",
         &I3SnowCorrectionServiceBase::AttenuationFactor,
         "Return the attenuation factor for snow, divide by this "
         "to approximately correct for the snow effect",
         args("pos", "snow_depth", "shower_particle", "shower_params"))
    ;

  register_ptr_to_python< I3SnowCorrectionServiceBasePtr >();

  class_<
    I3SimpleSnowCorrectionService,
    I3SimpleSnowCorrectionServicePtr,
    bases<I3SnowCorrectionServiceBase>,
    boost::noncopyable
  >("I3SimpleSnowCorrectionService",
    init<std::string, double>((arg("name"), arg("lambda"))))
    .add_property("lambda",
                  &I3SimpleSnowCorrectionService::GetLambda,
                  &I3SimpleSnowCorrectionService::ResetLambda)
    ;

  register_ptr_to_python< I3SimpleSnowCorrectionServicePtr >();

  class_<
    I3BORSSnowCorrectionService,
    I3BORSSnowCorrectionServicePtr,
    bases<I3SnowCorrectionServiceBase>,
    boost::noncopyable
  >("I3BORSSnowCorrectionService",
    init<std::string, bool>((arg("name"), arg("em_only")=false)))
    .def("fraction_em", &I3BORSSnowCorrectionService::FractionEM)
    .add_property("t_stage",
                  &I3BORSSnowCorrectionService::GetTStage,
                  &I3BORSSnowCorrectionService::SetTStage)
    .def("unset_t_stage", &I3BORSSnowCorrectionService::UnsetTStage)
    ;

  register_ptr_to_python< I3BORSSnowCorrectionServicePtr >();
}