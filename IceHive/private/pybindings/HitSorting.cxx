/**
 * copyright  (C) 2012
 * the IceCube Collaboration
 * $Id: $
 *
 * @file HitSortingPybindings.h
 * @date $Date: 2012-12-20$
 * @author mzoll <marcel.zoll@fysik.su.se>
 */

#include <string>

#include "icetray/python/stream_to_string.hpp"
#include "dataclasses/ostream_overloads.hpp"
#include "IceHive/HitSorting.h"

using namespace HitSorting;
namespace bp = boost::python;


#include <boost/python/operators.hpp>
#include <boost/operators.hpp>
#include <boost/python/list.hpp>
#include <boost/foreach.hpp>
#include "icetray/python/dataclass_suite.hpp"


///namespace containing specializations and casting-functions for pybindings
namespace pyHitSorting {
  
  //=== I3RecoPulse ===
  bp::list I3RecoPulse_GetHits_pyList(I3RecoPulseSeriesMap_HitFacility &hf, const std::string& key) {
    bp::list l;
    const HitDeque hits = hf.GetHits<HitDeque>(key);
    BOOST_FOREACH (const Hit &h, hits)
      l.append(h);
    return l;
  }
  
  I3RecoPulseSeriesMap I3RecoPulseSeriesMapFromHits_pyList(const I3RecoPulseSeriesMap_HitFacility &hf, const bp::list& l) {
    HitSorting::HitDeque hits_vec;
    for (int i=0; i<bp::len(l); i++)
      hits_vec.push_back(bp::extract<Hit>(l[i]));
    return hf.MapFromHits(hits_vec);
  }
  
  I3RecoPulseSeriesMapMask I3RecoPulseSeriesMapMaskFromHits_pyList(const I3RecoPulseSeriesMap_HitFacility &hf, const bp::list& l) {
   HitSorting::HitDeque hits_vec;
    for (int i=0; i<bp::len(l); i++)
      hits_vec.push_back(bp::extract<Hit>(l[i]));
    return hf.MaskFromHits(hits_vec);
  }
  
  I3RecoPulse_HitObject I3RecoPulse_HitObjectFromHitFacility(const I3RecoPulseSeriesMap_HitFacility &hf, const Hit &h) {
    return I3RecoPulse_HitObject(hf.GetHitObject(h));
  }
  
  I3RecoPulse_HitObject I3RecoPulse_HitObjectFromHit(const Hit &h) {
    return I3RecoPulse_HitObject(h.GetAssociatedHitObject<I3RecoPulse>());
  }
  
  bp::list I3RecoPulse_GetHitObjects_pyList(const I3RecoPulseSeriesMap &m) {
    bp::list l;
    const std::deque<HitObject<I3RecoPulse> > hitObjs = OMKeyMap_To_HitObjects<I3RecoPulse, std::deque<HitObject<I3RecoPulse> > >(m);
    BOOST_FOREACH (const HitObject<I3RecoPulse> &ho, hitObjs)
      l.append(ho);
    return l;
  }
  
  I3RecoPulseSeriesMap I3RecoPulseSeriesMapFromHitObjects_pyList(const bp::list& l) {
    std::deque<HitObject<I3RecoPulse> > hitObjs_vec;
    for (int i=0; i<bp::len(l); i++)
      hitObjs_vec.push_back(bp::extract<HitObject<I3RecoPulse> >(l[i]));
    return HitObjects_To_OMKeyMap<I3RecoPulse>(hitObjs_vec);
  }
  

  //=== I3DOMLaunch ===
  bp::list I3DOMLaunch_GetHits_pyList(I3DOMLaunchSeriesMap_HitFacility &hf) {
    bp::list l;
    const HitDeque hits = hf.GetHits<HitDeque>();
    BOOST_FOREACH (const Hit &h, hits)
      l.append(h);
    return l;
  }
  
  I3DOMLaunchSeriesMap I3DOMLaunchSeriesMapFromHits_pyList(const I3DOMLaunchSeriesMap_HitFacility &hf, const bp::list& l) {
    HitSorting::HitDeque hits_vec;
    for (int i=0; i<bp::len(l); i++)
      hits_vec.push_back(bp::extract<Hit>(l[i]));
    return hf.MapFromHits(hits_vec);
  }
  
  bp::list I3DOMLaunch_GetHitObjects_pyList(const I3DOMLaunchSeriesMap &m) {
    bp::list l;
    const std::deque<HitObject<I3DOMLaunch> > hitObjs = OMKeyMap_To_HitObjects<I3DOMLaunch,std::deque<HitObject<I3DOMLaunch> > >(m);
    BOOST_FOREACH (const HitObject<I3DOMLaunch> &ho, hitObjs)
      l.append(ho);
    return l;
  }
  
  I3DOMLaunchSeriesMap I3DOMLaunchSeriesMapFromHitObjects_pyList(const bp::list& l) {
    std::deque<HitObject<I3DOMLaunch> > hitObjs_vec;
    for (int i=0; i<bp::len(l); i++)
      hitObjs_vec.push_back(bp::extract<HitObject<I3DOMLaunch> >(l[i]));
    return HitObjects_To_OMKeyMap<I3DOMLaunch>(hitObjs_vec);
  }
  
  I3DOMLaunch_HitObject I3DOMLaunch_HitObjectFromHitFacility(const I3DOMLaunchSeriesMap_HitFacility &hf, const Hit &h) {
    return I3DOMLaunch_HitObject(hf.GetHitObject(h));
  }
  
  I3DOMLaunch_HitObject I3DOMLaunch_HitObjectFromHit(const Hit &h) {
    return I3DOMLaunch_HitObject(h.GetAssociatedHitObject<I3DOMLaunch>());
  }
  
  //=== I3MCHit ===
  bp::list I3MCHit_GetHits_pyList(I3MCHitSeriesMap_HitFacility &hf) {
    bp::list l;
    const HitDeque hits = hf.GetHits<HitDeque>();
    BOOST_FOREACH (const Hit &h, hits)
      l.append(h);
    return l;
  }
  
  I3MCHitSeriesMap I3MCHitSeriesMapFromHits_pyList(const I3MCHitSeriesMap_HitFacility &hf, const bp::list& l) {
    HitSorting::HitDeque hits_vec;
    for (int i=0; i<bp::len(l); i++)
      hits_vec.push_back(bp::extract<Hit>(l[i]));
    return hf.MapFromHits(hits_vec);
  }
  
  bp::list I3MCHit_GetHitObjects_pyList(const I3MCHitSeriesMap &m) {
    bp::list l;
    const std::deque<HitObject<I3MCHit> > hitObjs = OMKeyMap_To_HitObjects<I3MCHit, std::deque<HitObject<I3MCHit> > >(m);
    BOOST_FOREACH (const HitObject<I3MCHit> &ho, hitObjs)
      l.append(ho);
    return l;
  }
  
  I3MCHitSeriesMap I3MCHitSeriesMapFromHitObjects_pyList(const bp::list& l) {
    std::deque<HitObject<I3MCHit> > hitObjs_vec;
    for (int i=0; i<bp::len(l); i++)
      hitObjs_vec.push_back(bp::extract<HitObject<I3MCHit> >(l[i]));
    return HitObjects_To_OMKeyMap<I3MCHit>(hitObjs_vec);
  }
  
  I3MCHit_HitObject I3MCHit_HitObjectFromHitFacility(const I3MCHitSeriesMap_HitFacility &hf, const Hit &h) {
    return I3MCHit_HitObject(hf.GetHitObject(h));
  }
  
  I3MCHit_HitObject I3MCHit_HitObjectFromHit(const Hit &h) {
    return I3MCHit_HitObject(h.GetAssociatedHitObject<I3MCHit>());
  }
  
  
  //=== I3MCHit ===
  bp::list I3MCPulse_GetHits_pyList(I3MCPulseSeriesMap_HitFacility &hf) {
    bp::list l;
    const HitDeque hits = hf.GetHits<HitDeque>();
    BOOST_FOREACH (const Hit &h, hits)
      l.append(h);
    return l;
  }
  
  I3MCPulseSeriesMap I3MCPulseSeriesMapFromHits_pyList(const I3MCPulseSeriesMap_HitFacility &hf, const bp::list& l) {
    HitSorting::HitDeque hits_vec;
    for (int i=0; i<bp::len(l); i++)
      hits_vec.push_back(bp::extract<Hit>(l[i]));
    return hf.MapFromHits(hits_vec);
  }
  
  bp::list I3MCPulse_GetHitObjects_pyList(const I3MCPulseSeriesMap &m) {
    bp::list l;
    const std::deque<HitObject<I3MCPulse> > hitObjs = OMKeyMap_To_HitObjects<I3MCPulse,std::deque<HitObject<I3MCPulse> > >(m);
    BOOST_FOREACH (const HitObject<I3MCPulse> &ho, hitObjs)
      l.append(ho);
    return l;
  }
  
  I3MCPulseSeriesMap I3MCPulseSeriesMapFromHitObjects_pyList(const bp::list& l) {
    std::deque<HitObject<I3MCPulse> > hitObjs_vec;
    for (int i=0; i<bp::len(l); i++)
      hitObjs_vec.push_back(bp::extract<HitObject<I3MCPulse> >(l[i]));
    return HitObjects_To_OMKeyMap<I3MCPulse>(hitObjs_vec);
  }
  
  I3MCPulse_HitObject I3MCPulse_HitObjectFromHitFacility(const I3MCPulseSeriesMap_HitFacility &hf, const Hit &h) {
    return I3MCPulse_HitObject(hf.GetHitObject(h));
  }
  
  I3MCPulse_HitObject I3MCPulse_HitObjectFromHit(const Hit &h) {
    return I3MCPulse_HitObject(h.GetAssociatedHitObject<I3MCPulse>());
  }
}


///namespace containin pybinings for HitSorting library
void register_HitSorting()
{

  bp::class_<I3RecoPulse_HitObject>("I3RecoPulse_HitObject", bp::init<OMKey, I3RecoPulse>(bp::args("omkey","recoPulse"),"Description of a simplyfied Hit" ))
    .def_readonly("omkey", 
         &HitSorting::I3RecoPulse_HitObject::GetOMKey)
    .def_readonly("time", 
         &HitSorting::I3RecoPulse_HitObject::GetTime)
    .def("response", 
         &HitSorting::I3RecoPulse_HitObject::GetResponseObj, 
         (bp::arg("hit")))
    .def("CreateHit",
         &HitSorting::I3DOMLaunch_HitObject::CreateAssociatedHit)
    .def(bp::self < bp::self)
    .def("__str__", 
         &stream_to_string<HitSorting::HitObject<I3RecoPulse> >); 

  bp::class_<I3DOMLaunch_HitObject>("I3DOMLaunch_HitObject", bp::init<OMKey, I3DOMLaunch>(bp::args("omkey","domLaunch"),"Description of a simplyfied Hit" ))
    .def_readonly("omkey", 
         &HitSorting::I3DOMLaunch_HitObject::GetOMKey)
    .def_readonly("time", 
         &HitSorting::I3DOMLaunch_HitObject::GetTime)
    .def("response", 
         &HitSorting::I3DOMLaunch_HitObject::GetResponseObj, 
         (bp::arg("hit")))
    .def("CreateHit",
         &HitSorting::I3DOMLaunch_HitObject::CreateAssociatedHit)
    .def(bp::self < bp::self)
    .def("__str__", 
         &stream_to_string<HitSorting::HitObject<I3DOMLaunch> >); 

  bp::class_<I3MCHit_HitObject>("I3MCHit_HitObject", bp::init<OMKey, I3MCHit>(bp::args("omkey","mcHit"),"Description of a simplyfied Hit" ))
    .def_readonly("omkey", 
         &HitSorting::I3MCHit_HitObject::GetOMKey)
    .def_readonly("time", 
         &HitSorting::I3MCHit_HitObject::GetTime)
    .def("response", 
         &HitSorting::I3MCHit_HitObject::GetResponseObj, 
         (bp::arg("hits")))
    .def("CreateHit",
         &HitSorting::I3DOMLaunch_HitObject::CreateAssociatedHit)
    .def(bp::self < bp::self)
    .def("__str__", 
         &stream_to_string<HitSorting::HitObject<I3MCHit> >); 
  
  bp::class_<I3MCPulse_HitObject>("I3MCPulse_HitObject", bp::init<OMKey, I3MCPulse>(bp::args("omkey","mcPulse"),"Description of a simplyfied Hit" ))
    .def_readonly("omkey", 
         &HitSorting::I3MCPulse_HitObject::GetOMKey)
    .def_readonly("time", 
         &HitSorting::I3MCPulse_HitObject::GetTime)
    .def("response", 
         &HitSorting::I3MCPulse_HitObject::GetResponseObj, 
         (bp::arg("hits")))
    .def("CreateHit",
         &HitSorting::I3DOMLaunch_HitObject::CreateAssociatedHit)
    .def(bp::self < bp::self)
    .def("__str__", 
         &stream_to_string<HitSorting::HitObject<I3MCPulse> >);
    
    
  bp::def("I3RecoPulseSeriesMap_To_HitObjects", &pyHitSorting::I3RecoPulse_GetHitObjects_pyList,
          bp::args("map"),
          "Convert OMKeyMap to Series of HitObjects");
  bp::def("I3RecoPulseHitObjects_To_OMKeyMap", &pyHitSorting::I3RecoPulseSeriesMapFromHitObjects_pyList,
        bp::args("list"),
        "Convert HitObjects back to a OMkeyMap");
  bp::def("I3DOMLaunchSeriesMap_To_HitObjects", &pyHitSorting::I3DOMLaunch_GetHitObjects_pyList,
          bp::args("map"),
          "Convert OMKeyMap to Series of HitObjects");
  bp::def("I3DOMLaunchHitObjects_To_OMKeyMap", &pyHitSorting::I3DOMLaunchSeriesMapFromHitObjects_pyList,
          bp::args("list"),
          "Convert HitObjects back to a OMkeyMap");
  bp::def("I3MCHitSeriesMap_To_HitObjects", &pyHitSorting::I3MCHit_GetHitObjects_pyList,
          bp::args("map"),
          "Convert OMKeyMap to Series of HitObjects");
  bp::def("I3MCHitHitObjects_To_OMKeyMap", &pyHitSorting::I3MCHitSeriesMapFromHitObjects_pyList,
          bp::args("list"),
          "Convert HitObjects back to a OMkeyMap");  
  bp::def("I3MCPulseSeriesMap_To_HitObjects", &pyHitSorting::I3MCPulse_GetHitObjects_pyList,
          bp::args("map"),
          "Convert OMKeyMap to Series of HitObjects");
  bp::def("I3MCPulseHitObjects_To_OMKeyMap", &pyHitSorting::I3MCPulseSeriesMapFromHitObjects_pyList,
          bp::args("list"),
          "Convert HitObjects back to a OMkeyMap");
  
    
  //===== class Hit
  bp::class_<HitSorting::Hit, boost::shared_ptr<Hit> >("Hit", bp::no_init)
    .def_readonly("omkey", 
         &HitSorting::Hit::GetOMKey)
    .def_readonly("time", 
         &HitSorting::Hit::GetTime)
    .def(bp::self == bp::self)
    .def(bp::self < bp::self)
    .def("__str__", 
         &stream_to_string<HitSorting::Hit>);
  
  bp::def("GetAssociatedI3RecoPulse_HitObject",  &pyHitSorting::I3RecoPulse_HitObjectFromHit, //&HitSorting::Hit::GetAssociatedHitObject<I3RecoPulse>, bp::return_value_policy<bp::return_internal_reference>,
          bp::args("hit"),
          "Get the associated I3RecoPulse_HitObject of this hit");
  bp::def("GetAssociatedI3DOMLaunch_HitObject", &pyHitSorting::I3DOMLaunch_HitObjectFromHit,
          bp::args("hit"),
          "Get the associated I3DOMLaunch_HitObject of this hit");
  bp::def("GetAssociatedI3MCPulse_HitObject", &pyHitSorting::I3MCHit_HitObjectFromHit,
          bp::args("hit"),
          "Get the associated I3MCHit_HitObject of this hit");
  bp::def("GetAssociatedI3MCPulse_HitObject", &pyHitSorting::I3MCPulse_HitObjectFromHit,
          bp::args("hit"),
          "Get the associated I3MCPulse_HitObject of this hit");

  //=== class OMKeyMap_HitFacility
  bp::class_<I3DOMLaunchSeriesMap_HitFacility>("I3DOMLaunchSeriesMap_HitFacility",
                                               bp::init<I3FramePtr,
                                                        const std::string &>(bp::args("frame","key"),
                                               "Facilitate Hit extraction"))
    .def("GetHits", 
         &pyHitSorting::I3DOMLaunch_GetHits_pyList,
         "Extract hits from map at this key")
    .def("MapFromHits", 
         &pyHitSorting::I3DOMLaunchSeriesMapFromHits_pyList, 
         (bp::arg("hits")), 
         "Convert Hits extracted from this HitFacility back to a regular I3Map")
    .def("GetHitObject", 
         &pyHitSorting::I3DOMLaunch_HitObjectFromHitFacility,
         (bp::arg("hit"),
         "Retrieve this hit back to a HitObject"));  

    bp::class_<I3MCHitSeriesMap_HitFacility>("I3MCHitSeriesMap_HitFacility",
                                           bp::init<I3FramePtr,
                                                    const std::string &>(bp::args("frame","key"),
                                          "facilitate Hit extraction" ))
    .def("GetHits", 
          &pyHitSorting::I3MCHit_GetHits_pyList,
         "Extract hits from map at this key")
    .def("MapFromHits", 
         &pyHitSorting::I3MCHitSeriesMapFromHits_pyList,
         bp::arg("hits"), 
         "Convert Hits extracted from this HitFacility back to a regular I3Map")
    .def("GetHitObject", 
         &pyHitSorting::I3MCHit_HitObjectFromHitFacility, 
         (bp::arg("hit"),
          "Retrieve this hit back to a HitObject"));
  
  bp::class_<I3MCPulseSeriesMap_HitFacility>("I3MCPulseSeriesMap_HitFacility", bp::init<I3FramePtr, const std::string &>((bp::arg("frame"), bp::arg("key")),"facilitate Hit extraction" ))
    .def("GetHits", 
         &pyHitSorting::I3MCPulse_GetHits_pyList,
        "Extract hits from map at this key")
    .def("MapFromHits", 
         &pyHitSorting::I3MCPulseSeriesMapFromHits_pyList,
         bp::arg("hits"), 
         "Convert Hits extracted from this HitFacility back to a regular I3Map")
    .def("GetHitObject", 
         &pyHitSorting::I3MCPulse_HitObjectFromHitFacility, 
         bp::arg("hit"),
         "Retrieve this hit back to a HitObject");    
    

  bp::class_<I3RecoPulseSeriesMap_HitFacility>("I3RecoPulseSeriesMap_HitFacility", bp::init<I3FramePtr, const std::string &>((bp::arg("frame"), bp::arg("key")),"facilitate Hit extraction" ))
    .def("GetHits", 
         &pyHitSorting::I3RecoPulse_GetHits_pyList,
         bp::arg("key"), 
        "Extract hits from map/mask at this key")
    .def("MapFromHits", 
         &pyHitSorting::I3RecoPulseSeriesMapFromHits_pyList,
         bp::arg("hits"), 
         "Convert Hits extracted from this HitFacility back to a regular I3Map")
    .def("MaskFromHits", 
         &pyHitSorting::I3RecoPulseSeriesMapMaskFromHits_pyList,
         bp::arg("hits"), 
         "Convert Hits extracted from this HitFacility back to a I3Mask")
    .def("GetHitObject", 
         &pyHitSorting::I3RecoPulse_HitObjectFromHitFacility, 
         bp::arg("hit"),
         "Retrieve this hit back to a HitObject");
}
