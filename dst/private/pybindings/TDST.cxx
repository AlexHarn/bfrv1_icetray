//  pybindings for dst-extractor
//  All the blame goes to Brad Madajczyk

#include <icetray/I3FrameObject.h>
#include <dst/extractor/TDST.h>
#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

void register_TDST()
{
  {
    bp::class_<TDST, bp::bases<I3FrameObject>, TDSTPtr>("TDST")
      .def(bp::dataclass_suite<TDST>())
      .def_readwrite("localMST", &TDST::localMST)
      .def_readwrite("mjdTime", &TDST::mjdTime)
      .def_readwrite("llhAzimuth", &TDST::llhAzimuth)
      .def_readwrite("llhZenith", &TDST::llhZenith)
      .def_readwrite("lfAzimuth", &TDST::lfAzimuth)
      .def_readwrite("lfZenith", &TDST::lfZenith)
      .def_readwrite("linllhOpeningAngle", &TDST::linllhOpeningAngle)
      .def_readwrite("RA", &TDST::RA)
      .def_readwrite("Dec", &TDST::Dec)
      .def_readwrite("RAAntiS", &TDST::RAAntiS)
      .def_readwrite("DecAntiS", &TDST::DecAntiS)
      .def_readwrite("RASolar", &TDST::RASolar)
      .def_readwrite("DecSolar", &TDST::DecSolar)
      .def_readwrite("RASun", &TDST::RASun)
      .def_readwrite("DecSun", &TDST::DecSun)
      .def_readwrite("RAMoon", &TDST::RAMoon)
      .def_readwrite("DecMoon", &TDST::DecMoon)
      .def_readwrite("logMuE", &TDST::logMuE)
      .def_readwrite("rlogl", &TDST::rlogl)
      .def_readwrite("sdcog", &TDST::sdcog)
      .def_readwrite("cogx", &TDST::cogx)
      .def_readwrite("cogy", &TDST::cogy)
      .def_readwrite("cogz", &TDST::cogz)
      .def_readwrite("ldir", &TDST::ldir)
      .def_readwrite("runId", &TDST::runId)
      .def_readwrite("ndir", &TDST::ndir)
      .def_readwrite("nchan", &TDST::nchan)
      .def_readwrite("nstring", &TDST::nstring)
      .def_readwrite("subrunId", &TDST::subrunId)
      .def_readwrite("isGoodLineFit", &TDST::isGoodLineFit)
      .def_readwrite("isGoodLLH", &TDST::isGoodLLH)
      ;
  }

}

