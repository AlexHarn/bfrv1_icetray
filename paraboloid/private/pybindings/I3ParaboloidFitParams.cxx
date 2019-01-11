#include <icetray/I3FrameObject.h>
#include <icetray/load_project.h>
#include <tableio/converter/pybindings.h>

#include "paraboloid/I3ParaboloidFitParams.h"
#include "paraboloid/I3ParaboloidFitParamsConverter.h"

namespace bp = boost::python ;

BOOST_PYTHON_MODULE(paraboloid)
{

  load_project("paraboloid", false);

  {
    bp::scope in_i3paraboloidfitparams = bp::class_<I3ParaboloidFitParams, 
      bp::bases<I3FrameObject>, boost::shared_ptr<I3ParaboloidFitParams> >("I3ParaboloidFitParams")
      .def_readwrite("pbfZen", &I3ParaboloidFitParams::pbfZen_)
      .def_readwrite("pbfAzi", &I3ParaboloidFitParams::pbfAzi_)
      .def_readwrite("pbfErr1", &I3ParaboloidFitParams::pbfErr1_)
      .def_readwrite("pbfErr2", &I3ParaboloidFitParams::pbfErr2_)
      .def_readwrite("pbfRotAng", &I3ParaboloidFitParams::pbfRotAng_)
      .def_readwrite("pbfCenterLlh", &I3ParaboloidFitParams::pbfCenterLlh_)
      .def_readwrite("pbfLlh", &I3ParaboloidFitParams::pbfLlh_)
      .def_readwrite("pbfZenOff", &I3ParaboloidFitParams::pbfZenOff_)
      .def_readwrite("pbfAziOff", &I3ParaboloidFitParams::pbfAziOff_)
      .def_readwrite("pbfCurv11", &I3ParaboloidFitParams::pbfCurv11_)
      .def_readwrite("pbfCurv12", &I3ParaboloidFitParams::pbfCurv12_)
      .def_readwrite("pbfCurv22", &I3ParaboloidFitParams::pbfCurv22_)
      .def_readwrite("pbfChi2", &I3ParaboloidFitParams::pbfChi2_)
      .def_readwrite("pbfDetCurvM", &I3ParaboloidFitParams::pbfDetCurvM_)
      .def_readwrite("pbfSigmaZen", &I3ParaboloidFitParams::pbfSigmaZen_)
      .def_readwrite("pbfSigmaAzi", &I3ParaboloidFitParams::pbfSigmaAzi_)
      .def_readwrite("pbfCovar", &I3ParaboloidFitParams::pbfCovar_)
      .def_readwrite("pbfTrOffZen", &I3ParaboloidFitParams::pbfTrOffZen_)
      .def_readwrite("pbfTrOffAzi", &I3ParaboloidFitParams::pbfTrOffAzi_)
      .def_readwrite("pbfTrZen", &I3ParaboloidFitParams::pbfTrZen_)
      .def_readwrite("pbfTrAzi", &I3ParaboloidFitParams::pbfTrAzi_)
      .def_readwrite("pbfStatus", &I3ParaboloidFitParams::pbfStatus_)
      .def_readwrite("logl", &I3ParaboloidFitParams::logl_)
      .def_readwrite("rlogl", &I3ParaboloidFitParams::rlogl_)
      .def_readwrite("ndof", &I3ParaboloidFitParams::ndof_)
      .def_readwrite("nmini", &I3ParaboloidFitParams::nmini_)
      ;

    bp::enum_<I3ParaboloidFitParams::ParaboloidFitStatus>("ParaboloidFitStatus")
      .value("PBF_UNDEFINED", I3ParaboloidFitParams::PBF_UNDEFINED)
      .value("PBF_NO_SEED", I3ParaboloidFitParams::PBF_NO_SEED)
      .value("PBF_INCOMPLETE_GRID", I3ParaboloidFitParams::PBF_INCOMPLETE_GRID)
      .value("PBF_FAILED_PARABOLOID_FIT", I3ParaboloidFitParams::PBF_FAILED_PARABOLOID_FIT)
      .value("PBF_SINGULAR_CURVATURE_MATRIX", I3ParaboloidFitParams::PBF_SINGULAR_CURVATURE_MATRIX)
      .value("PBF_SUCCESS", I3ParaboloidFitParams::PBF_SUCCESS)
      .value("PBF_NON_POSITIVE_ERRS", I3ParaboloidFitParams::PBF_NON_POSITIVE_ERRS)
      .value("PBF_NON_POSITIVE_ERR_1", I3ParaboloidFitParams::PBF_NON_POSITIVE_ERR_1)
      .value("PBF_NON_POSITIVE_ERR_2", I3ParaboloidFitParams::PBF_NON_POSITIVE_ERR_2)
      .value("PBF_TOO_SMALL_ERRS", I3ParaboloidFitParams::PBF_TOO_SMALL_ERRS)
      .export_values()
      ;
  }

  register_pointer_conversions<I3ParaboloidFitParams>();

  I3CONVERTER_NAMESPACE(paraboloid);
  I3CONVERTER_EXPORT_DEFAULT(I3ParaboloidFitParamsConverter, "Tabulator of I3ParaboloidFitParams");
}
