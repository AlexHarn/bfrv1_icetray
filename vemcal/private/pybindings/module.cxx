/**
 *  $Id: module.cxx 142140 2016-02-19 20:46:14Z hdembinski $
 */

#include "vemcal/I3VEMCalData.h"
#include <icetray/load_project.h>
#include <icetray/python/dataclass_suite.hpp>
#include <icetray/I3FrameObject.h>
#include <boost/python.hpp>
#include <boost/foreach.hpp>

namespace boost { namespace python {
  object Get_minBiasHits(const I3VEMCalData& x)
  {
    list lst;
    BOOST_FOREACH(const I3VEMCalData::MinBiasHit& hit, x.minBiasHits)
      lst.append(hit);
    return tuple(lst); // tuple should indicate that these are not mutable
  }

  void Set_minBiasHits(I3VEMCalData& x, object obj)
  {
    x.minBiasHits.clear();
    x.minBiasHits.reserve(len(obj));
    for (unsigned i = 0, n = len(obj); i < n; ++i)
      x.minBiasHits.push_back(extract<I3VEMCalData::MinBiasHit>(obj[i]));
  }

  object Get_hglgHits(const I3VEMCalData& x)
  {
    list lst;
    BOOST_FOREACH(const I3VEMCalData::HGLGhit& hit, x.hglgHits)
      lst.append(hit);
    return tuple(lst); // tuple should indicate that these are not mutable
  }

  void Set_hglgHits(I3VEMCalData& x, object obj)
  {
    x.hglgHits.clear();
    x.hglgHits.reserve(len(obj));
    for (unsigned i = 0, n = len(obj); i < n; ++i)
      x.hglgHits.push_back(extract<I3VEMCalData::HGLGhit>(obj[i]));
  }
}}

BOOST_PYTHON_MODULE(vemcal)
{
  load_project("vemcal", false);

  using namespace boost::python;

  scope I3VEMCalData_scope =
  class_<
    I3VEMCalData
    , bases<I3FrameObject>
    , I3VEMCalDataPtr
  >("I3VEMCalData")
    .def(dataclass_suite<I3VEMCalData>())
    .def_readwrite("runID", &I3VEMCalData::runID)
    .add_property("minBiasHits", Get_minBiasHits, Set_minBiasHits)
    .add_property("hglgHits", Get_hglgHits, Set_hglgHits)
  ;

  class_<I3VEMCalData::MinBiasHit>("MinBiasHit")
    .def_readwrite("str", &I3VEMCalData::MinBiasHit::str)
    .def_readwrite("om", &I3VEMCalData::MinBiasHit::om)
    .def_readwrite("chip", &I3VEMCalData::MinBiasHit::chip)
    .def_readwrite("channel", &I3VEMCalData::MinBiasHit::channel)
    .def_readwrite("charge_dpe", &I3VEMCalData::MinBiasHit::charge_dpe)
  ;

  class_<I3VEMCalData::HGLGhit>("HGLGhit")
    .def_readwrite("str", &I3VEMCalData::HGLGhit::str)
    .def_readwrite("hg_om", &I3VEMCalData::HGLGhit::hg_om)
    .def_readwrite("hg_chip", &I3VEMCalData::HGLGhit::hg_chip)
    .def_readwrite("hg_channel", &I3VEMCalData::HGLGhit::hg_channel)
    .def_readwrite("hg_charge_pe", &I3VEMCalData::HGLGhit::hg_charge_pe)
    .def_readwrite("lg_om", &I3VEMCalData::HGLGhit::lg_om)
    .def_readwrite("lg_chip", &I3VEMCalData::HGLGhit::lg_chip)
    .def_readwrite("lg_channel", &I3VEMCalData::HGLGhit::lg_channel)
    .def_readwrite("lg_charge_pe", &I3VEMCalData::HGLGhit::lg_charge_pe)
    .def_readwrite("deltat_2ns", &I3VEMCalData::HGLGhit::deltat_2ns)
  ;

  register_pointer_conversions<I3VEMCalData>();
}
