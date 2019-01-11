/**
 * \file HitSortingTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * Unit test to test the robustness of HitSorting as it is implemented in HiveSplitter/HitSorting.h
 */

#include <boost/python.hpp>

#include "IceHive/IceHiveHelpers.h"
#include "tools/IC86Topology.h"

//specialize for these Hit/Pulse-like classes, which come in the flavours of I3Map<OMKey, vector<Hit> >
#include "dataclasses/physics/I3RecoPulse.h"

namespace bp = boost::python;


I3RecoPulseSeriesMap Clip_I3RecoPulses (const I3RecoPulseSeriesMap &pulses,
                                        const I3TimeWindow &tw) {
  return IceHiveHelpers::GetPulsesInTimeRange<I3RecoPulse> (pulses, tw);
};

I3TriggerHierarchy Clip_TriggerHierarchy (const I3TriggerHierarchy &trigHier,
                                          const I3TimeWindow& twindow,
                                          const bp::list& l) {
  std::vector<int> trig_ids;
  for (int i=0; i<bp::len(l); i++)
    trig_ids.push_back(bp::extract<int>(l[i]));
  return IceHiveHelpers::ClipTriggerHierarchy (trigHier,twindow,trig_ids);
};


void register_IceHiveHelpers() {
  bp::def("clip_TriggerHierarchy", &Clip_TriggerHierarchy
    , (bp::arg("trigHier"), bp::arg("twindow"), bp::arg("configIDs"))
    , "Clips a TriggerHierarchy based on config IDs and time window."
  );

  bp::def("clip_I3RecoPulses", &Clip_I3RecoPulses
    , (bp::arg("pulses"), bp::arg("tw"))
    , "Clips a I3RecoPulseSeriesMap based on time window."
  );
  
  bp::def("is_ic_dom", &IC86Topology::IsICDOM
    , (bp::arg("omkey"))
    , "Is this a DOM in regular IceCube?"
  );
  
  bp::def("is_dc_dom", &IC86Topology::IsDCDOM
    , (bp::arg("omkey"))
    , "Is this a DOM in DeepCore?"
  );
      
  bp::def("is_vetocap_dom", &IC86Topology::IsVetoCapDOM
    , (bp::arg("omkey"))
    , "Is this DOM in the DeepCore Veto layer?"
  );
  
  bp::def("is_dcfid_dom", &IC86Topology::IsDCFidDOM
    , (bp::arg("omkey"))
    , "Is this DOM in the denser populated DeepCore region?"
  );

  bp::def("is_vetocapfid_dom", &IC86Topology::IsVetoCapFidDOM
    , (bp::arg("omkey"))
    , "Is this DOM in the denser populated DeepCore-Veto region?"
  );
    
  bp::def("is_dcvetocapfid_dom", &IC86Topology::IsDCVetoCapFidDOM
    , (bp::arg("omkey"))
    , "Is this DOM in the any dense populated region?"
  );

  bp::def("is_icetop_dom", &IC86Topology::IsIceTopDOM
    , (bp::arg("omkey"))
    , "Is this DOM in IceTop?"
  );
  
  bp::def("is_ic_string", &IC86Topology::IsICString
    , (bp::arg("omkey"))
    , "Is this string a in the regular IceCube array?"
  );
    
  bp::def("is_dc_dom", &IC86Topology::IsDCString
    , (bp::arg("omkey"))
    , "Is this string in the DeepCore array?"
  );
    
  bp::def("is_pingu_dom", &IC86Topology::IsPinguString
    , (bp::arg("omkey"))
    , "Is this string in the Pingu (DeepCore infill) array?"
  );
  
  //LimitPairs
  //Ring Limits
  bp::class_<Limits::LimitPair, boost::shared_ptr<Limits::LimitPair> >("LimitPair", bp::init<double, double>((bp::arg("minus"), bp::arg("plus"))))
    .add_property("minus", &Limits::LimitPair::minus_)
    .add_property("plus", &Limits::LimitPair::plus_)
  ;
  
  bp::class_<Limits::RingLimits, boost::shared_ptr<Limits::RingLimits> >("RingLimits")
    .add_property("limits_pairs", &Limits::RingLimits::limitPairs_)
    .def("AddLimitPair", &Limits::RingLimits::AddLimitPair)
    .def("NRings", &Limits::RingLimits::NRings)
  ;
  
};

