/**
 * \file Hive-lib.cxx
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: Hive-lib.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 */

#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>

#include <cassert>
#include <boost/assign/list_of.hpp>

#include <iostream>
#include <cstdio> //<stdio.h>
#include <fstream>
#include <boost/lexical_cast.hpp>

#include "HiveSplitter/Hive-lib.h"

namespace honey{
  //=== class DOMHoneyComb === IMPLEMENTATIONS ================
  
  uint DOMHoneyComb::GetCenter() const {
    if (honey_.size()>0 && honey_.begin()->size()==1)
      return *honey_.begin()->begin();
    return -1; // center not set  
  }
  
  void DOMHoneyComb::AddCenter(const uint center){
    if (honey_.size()!=0) {
      log_fatal("violation against the ordering of rings!");
    }
    honey_.push_back(boost::assign::list_of(center));
  }
  
  void DOMHoneyComb::SetCenter(const uint center){
    SetRing(0, boost::assign::list_of(center));
  }
  
  uint DOMHoneyComb::GetNRings() const {
    return honey_.size()-1;
  } 
  
  std::set<uint> DOMHoneyComb::GetRing(const uint ringnbr) const {
    std::vector<std::set<uint> >::const_iterator get_ring = honey_.begin()+ringnbr;
    if (get_ring==honey_.end())
      return std::set<uint>();
    else 
      return *get_ring;
  }
  
  void DOMHoneyComb::AddRing(const uint ringnbr, const std::set<uint> &ring) {
    if (honey_.size()!=ringnbr) {
      log_fatal("violation against the ordering of rings!");
    }
    honey_.push_back(ring);
  }
  
  void DOMHoneyComb::SetRing(const uint ringnbr, const std::set<uint> &ring) {
    if (honey_.size()<=ringnbr){
      log_fatal("violation against the ordering of rings! Use AddRing() methode to add new rings first");
    }
    honey_[ringnbr]=ring;
  }
  
  DOMHoneyComb::DOMHoneyComb() {};
  
  DOMHoneyComb::DOMHoneyComb(const uint center, const std::set<uint>& ring1){
    AddCenter(center);
    AddRing(1,ring1);
  }
  
  DOMHoneyComb::DOMHoneyComb(const uint center, const std::set<uint>& ring1, const std::set<uint>& ring2){
    AddCenter(center);
    AddRing(1,ring1);
    AddRing(2,ring2);
  }

  DOMHoneyComb::DOMHoneyComb(const uint center, std::vector< std::set <uint> >& rings) {
    AddCenter(center);
    for (uint i=1; i <=rings.size(); i++)
      AddRing(i,rings[i]);
  }

  void DOMHoneyComb::AddStringToRing(const uint center, const uint ringnbr, const uint string) {
    const uint current_rings = GetNRings();
    if (current_rings+1<ringnbr) {
      log_fatal("Violation against the ordering of rings; Use AddRing() methode to add new rings first");
    }
    else if (current_rings+1==ringnbr) {
      std::set<uint> newring;
      newring.insert(string);
      AddRing(ringnbr, newring); // register a new ring
    }
    else if (current_rings==ringnbr){
      honey_[ringnbr].insert(string);
    }
    else
      log_fatal("Something bad");
  }
  
  // ================= IMPLEMENTATIONS ============

  void MutualAddStringToRing(DOMHoneyCombRegister &combs, const uint center_A, const uint center_B, const uint ringnbr) {
    DOMHoneyCombRegister::iterator honey_A = combs.find(center_A);
    DOMHoneyCombRegister::iterator honey_B = combs.find(center_B);
    if (honey_A==combs.end() ||honey_B==combs.end() ){
      log_fatal_stream("strings not registered to the combs; Use AddCenter() methode to add new central strings first" << std::endl);
    }
    honey_A->second.AddStringToRing(center_A, ringnbr, center_B);
    honey_B->second.AddStringToRing(center_B, ringnbr, center_A);
  }
  
  bool IsRingX (const DOMHoneyCombRegister &combs, const uint center, const uint ringnbr, const uint string) {
    DOMHoneyCombRegister::const_iterator honey = combs.find(center);
    if (honey==combs.end()) {
      log_trace_stream("Center string '" <<  center << "' not registered in combs" << std::endl);
      return false;
    }
    const std::set<uint>& ring= (honey->second).GetRing(ringnbr);
    return ring.count(string);
  }
  
  inline bool IsRing1(DOMHoneyCombRegister &combs, const uint center, const uint string) {
    return IsRingX(combs, center, 1, string);
  }  
  
  inline bool IsRing2(DOMHoneyCombRegister &combs, const uint center, const uint string) {
    return IsRingX(combs, center, 2, string);
  }

  int WhichRing(const DOMHoneyCombRegister &combs, const uint center, const uint string, const uint search_depth) {
    DOMHoneyCombRegister::const_iterator honey = combs.find(center);
    if (honey==combs.end()) {
      log_info_stream("Could not locate center string '"<< center << "' in combs" << std::endl);
      return -1;
    }
    for (uint honey_iter =0; honey_iter<= (honey->second).GetNRings() && honey_iter<=search_depth; honey_iter++) {
      if (IsRingX(combs, center, honey_iter, string))
	return honey_iter;
    }
    log_trace_stream("Could not locate string rings around '"<< center << "' in combs" << std::endl);
    return -1;
  }


//TODO DISABLED FOR NOW; need to change interface for building functions
//   DOMHoneyCombRegister BuildCompleteTrippleDensePinguHoneyCombRegister () {
//     log_info("Building Complete TrippleDenseHoneyCombRegister with 8 Rings");
//     DOMHoneyCombRegister NewRegister;
//     
//     DOMHoneyCombRegister Single = BuildSingleDenseHive().combs_;
//     DOMHoneyCombRegister Double = BuildDoubleDenseHive().combs_;
//     DOMHoneyCombRegister Tripple = BuildTrippleDenseHive().combs_;
//     
//     NewRegister = Abstract(Single);
//     NewRegister = UniteHive(NewRegister, Double);
//     NewRegister = Abstract(NewRegister);
//     NewRegister = UniteHive(NewRegister, Tripple);
//     
//     return NewRegister;
//   }
  
  DOMHoneyCombRegister Abstract(const DOMHoneyCombRegister combs) {
    log_info("Abstracting HoneyCombRegister");
    DOMHoneyCombRegister NewRegister;
    
    for (DOMHoneyCombRegister::const_iterator CombPtr =combs.begin(); CombPtr!=combs.end(); ++CombPtr) {
      const uint center_string = CombPtr->first;
      //log_info("abstracting string "<<center_string <<", "<< std::flush;
      NewRegister[center_string].AddCenter(center_string);
      for (uint ring_index =1; ring_index<=CombPtr->second.GetNRings(); ring_index++) {
	NewRegister[center_string].AddRing(2*ring_index-1, std::set<uint>());
	NewRegister[center_string].AddRing(2*ring_index, CombPtr->second.GetRing(ring_index));
      }
    }
    log_debug("HoneyCombRegister Abstracted");
    return NewRegister;
  };
  
  int ExpandToNextRing(DOMHoneyCombRegister &combs) {
    const uint extremist_ring = combs.begin()->second.GetNRings();
    if (extremist_ring<1) {
      log_error("At least 1 Ring must be registered; cannot expand!");
      return 0;
    }
    
    for (std::map<uint, DOMHoneyComb>::iterator combs_iter=combs.begin(); combs_iter!=combs.end(); combs_iter++) {
      std::set<uint> outmost_strings = combs_iter->second.GetRing(extremist_ring);
      log_trace_stream("expanding string " << combs_iter->first << std::endl);
      std::set<uint> next_surrounders;
      for (std::set<uint>::iterator outer_stringPtr=outmost_strings.begin(); outer_stringPtr!=outmost_strings.end(); outer_stringPtr++) {
	DOMHoneyCombRegister::const_iterator outer_center = combs.find(*outer_stringPtr);
	
	if (outer_center==combs.end()){
	  log_fatal_stream("There are non mutual registered strings "<<combs_iter->first<<" and "<<*outer_stringPtr<<" ; cannot expand!" << std::endl);
	}
	  
	std::set<uint> local_surrounders = outer_center->second.GetRing(1);
	next_surrounders.insert(local_surrounders.begin(), local_surrounders.end());
      }
      //erase from the set the extremist_ring and (extremist_ring-1)
      std::set<uint> almost_outmost_strings = combs_iter->second.GetRing(extremist_ring-1);
      for (std::set<uint>::iterator outer_stringPtr=outmost_strings.begin(); outer_stringPtr!=outmost_strings.end(); outer_stringPtr++)
        next_surrounders.erase(*outer_stringPtr);
      for (std::set<uint>::iterator almost_outer_stringPtr=almost_outmost_strings.begin(); almost_outer_stringPtr!=almost_outmost_strings.end(); almost_outer_stringPtr++)
        next_surrounders.erase(*almost_outer_stringPtr);
      combs_iter->second.AddRing(extremist_ring+1, next_surrounders);
    }
    return extremist_ring+1;
  }
  
  int ExpandRings(DOMHoneyCombRegister &combs, const uint scale_factor) {
    if (combs.begin()==combs.end()) {
      log_fatal("Hive is not filled with any rings; cannot expand!");
    }
    
    const uint extremist_ring = combs.begin()->second.GetNRings();
    if (extremist_ring<scale_factor) {
      log_error("At least as many rings as the scale_factor must be registered; cannot expand");
      return 0;
    }
    
    for (std::map<uint, DOMHoneyComb>::iterator combs_iter=combs.begin(); combs_iter!=combs.end(); combs_iter++) {
      std::set<uint> outmost_strings = combs_iter->second.GetRing(extremist_ring);
      //log_info("expanding string " << combs_iter->first << " with a scaling of "<< scale_factor << std::endl;
      
      //construct n new rings in these sets
      std::vector<std::set<uint> > next_surrounders;
      for (uint scale =0; scale <=scale_factor; scale++) {
	next_surrounders.push_back(std::set<uint>());
      }
      //loop over the strings on the outmost ring
      for (std::set<uint>::iterator outer_stringPtr=outmost_strings.begin(); outer_stringPtr!=outmost_strings.end(); outer_stringPtr++) {
	//log_info(*outer_stringPtr << std::endl;
	DOMHoneyCombRegister::const_iterator outer_center = combs.find(*outer_stringPtr);
	
	if (outer_center==combs.end()) {
	  log_fatal_stream("There are non mutual registered strings "<<combs_iter->first<<" and "<<*outer_stringPtr<<" ; cannot expand;" << std::endl);
	}
	//add the nth ring from the outer strings the the next_surrounders n-th to construct rings
	for (uint scale =1; scale <=scale_factor; scale++) {
	  //std::cout<< "Adding "<< scale<<". ring on string "<<*outer_stringPtr<<" to next_surrounders["<<scale-1<<"]"<< std::endl; 
	  std::set<uint> local_surrounders = outer_center->second.GetRing(scale);
	  next_surrounders[scale-1].insert(local_surrounders.begin(), local_surrounders.end());
	}
      }
      
      //erase the inner rings from the new constructed rings
      for (uint scale =1; scale <=scale_factor; scale++) {
	
	for (uint curr =0; curr <=scale; curr++) {
	  //std::cout<< "Removing "<< (extremist_ring-curr)<<". ring on string "<<combs_iter->first<<" from next_surrounders["<<scale-1<<"]"<< std::endl;
	  std::set<uint> almost_outmost_strings = combs_iter->second.GetRing(extremist_ring - curr);
	  for (std::set<uint>::iterator almost_outer_stringPtr=almost_outmost_strings.begin(); almost_outer_stringPtr!=almost_outmost_strings.end(); almost_outer_stringPtr++)
	    next_surrounders[scale-1].erase(*almost_outer_stringPtr);
	}
      }
      
      // in the new constructed rings, remove cascade the next inner ring from the now to add finished constructed one
      //std::cout<< "Registering next_surrounders["<<0<<"] as "<<(extremist_ring+1)<<". ring on string "<<combs_iter->first<< std::endl;
      combs_iter->second.AddRing(extremist_ring+1, next_surrounders[0]);
      
      for  (uint scale =2; scale <=scale_factor; scale++) {
	//std::cout<< "Removing "<< (extremist_ring+scale-1)<<". ring on string "<<combs_iter->first<<" from next_surrounders["<<scale-1<<"]"<< std::endl;
	std::set<uint> almost_outmost_strings = combs_iter->second.GetRing(extremist_ring + scale-1);
	for (std::set<uint>::iterator almost_outer_stringPtr=almost_outmost_strings.begin(); almost_outer_stringPtr!=almost_outmost_strings.end(); almost_outer_stringPtr++)
	  next_surrounders[scale-1].erase(*almost_outer_stringPtr);
	
	//std::cout<< "Registering next_surrounders["<<scale-1<<"] as "<<(extremist_ring+scale)<<". ring on string "<<combs_iter->first<< std::endl;
	combs_iter->second.AddRing(extremist_ring+scale, next_surrounders[scale-1]);
      }
    }
    return extremist_ring+scale_factor;
  }
  
  DOMHoneyCombRegister UniteHive (const DOMHoneyCombRegister& combs1, const DOMHoneyCombRegister& combs2) {
    log_info("========= UNITING HIVES =========="); //TODO check scale factors
    if (combs1.size()<combs2.size()) //for efficiency:
      return UniteHive(combs2, combs1); //recursive call
    
    if (combs1.size()<=0 || combs2.size()<=0) {
      log_fatal("There have to be strings registered in the combs!");
    }
    
    const uint n_rings = combs1.begin()->second.GetNRings();
    if (combs1.begin()->second.GetNRings() != combs1.begin()->second.GetNRings()) {
      //DANGER actually all entries have to be checked to have equal size
      log_fatal("Cannot unite combs with different numbers of rings registered!");
    }
    
    DOMHoneyCombRegister NewRegister = combs1; //make a copy of combs1
    
    for (DOMHoneyCombRegister::const_iterator combs2_iter = combs2.begin(); combs2_iter!=combs2.end(); combs2_iter++) {
      //log_trace("UNITE string"<< combs2_iter->first<<", " << std::flush;
      //try brute force insert first
      std::pair<DOMHoneyCombRegister::iterator,bool> string_added = NewRegister.insert(*combs2_iter);

      if (!string_added.second) { //if that was not successful, need to iterate
	//log_trace("iterate " << std::flush;
	
	//loop over all rings :unite the strings on that ring and substitute the union
	for (uint ring_count =0; ring_count<=n_rings; ring_count++) {
	  // log_trace("ring" << ring_count<< ","<< std::flush;
	  std::set<uint> united = string_added.first->second.GetRing(ring_count);
	  std::set<uint> strings_to_add = combs2_iter->second.GetRing(ring_count);
	  
	  united.insert(strings_to_add.begin(), strings_to_add.end());
	  
	  NewRegister[combs2_iter->first].SetRing(ring_count, united);
	}
      }
    }
	
    return NewRegister;
  }    

  //===========
  std::string DumpCenter (const DOMHoneyCombRegister &combs, const uint center) {
   std::ostringstream dump;
   dump << "========= DUMPING STRING "<<center<<"==========" << std::endl;
    DOMHoneyCombRegister::const_iterator combs_iter = combs.find(center);
    if (combs_iter==combs.end()) {
      dump << "string "<<center<<" not registered in combs!" << std::endl;
      return dump.str();
    }
    
    const uint nRings=combs_iter->second.GetNRings();
    
    for (uint ring = 0; ring<=nRings; ring++){
      dump << std::endl << " Ring "<<ring<<" : ";
      std::set<uint> strings = combs_iter->second.GetRing(ring);
      for (std::set<uint>::iterator strings_iter=strings.begin(); strings_iter!=strings.end(); strings_iter++){
        dump << *strings_iter << ", ";
      }
    }
    dump << std::endl;;
    return dump.str();
  }

  std::string DumpHive (const Hive &hive) {
    std::ostringstream dump;
    const DOMHoneyCombRegister &combs = hive.combs_;

    dump << "# Density "<< hive.scaleFactor_ <<" Rings "<< hive.max_rings_<< std::endl; // line: # Density 1 Rings 2
    
    for (DOMHoneyCombRegister::const_iterator combs_iter=combs.begin(); combs_iter!=combs.end(); combs_iter++) {

      const uint nRings=combs_iter->second.GetNRings();

      dump << "S "<< combs_iter->first << std::endl;
      for (uint ring = 1; ring<=nRings; ring++){
	dump << "R"<<ring<<" ";
	std::set<uint> strings = combs_iter->second.GetRing(ring);
	for (std::set<uint>::iterator strings_iter=strings.begin(); strings_iter!=strings.end(); strings_iter++){
	  dump << *strings_iter << " ";
	}
	dump << std::endl;
      }
    }
    dump << std::endl;
    
    return dump.str();
  }

  Hive ReadHiveFromFile (const std::string& fileName) {
    //Prepwork
    log_info_stream("Reading HoneyCombRegister from file" << fileName << std::endl);
    uint density = 0;
    uint nRings = 0;
    DOMHoneyCombRegister NewRegister;

    // READ
    std::ifstream file ( fileName.c_str());
    assert(file.is_open());
    //containers
    std::string line, word;
    uint centerNbr, strNbr, ringNbr;

    getline(file, line);
    if (line[0]=='#') {
      std::istringstream buffer(line);
      buffer>>word; buffer>>word;
      buffer>>density;
      buffer>>word;
      buffer>>nRings;
      getline(file, line);
    }
    while(!file.eof()){
      std::istringstream buffer(line);
      buffer>>word;
      if (word=="S") {
	//new string
	buffer>>centerNbr;
	NewRegister[centerNbr].AddCenter(centerNbr);
	while(true){
	  getline(file, line);
	  if (line[0]=='R') {
	    std::istringstream ring_buffer(line);
	    ring_buffer>>word;
	    word.erase(word.begin());
	    std::stringstream(word)>>ringNbr;
	    std::set<uint> str_in_ring;
	    while(ring_buffer>>strNbr){
	      str_in_ring.insert(strNbr);
	    }
	    NewRegister[centerNbr].AddRing(ringNbr, str_in_ring);
	  }
	  else{
	    break;
	  }
	}
      }
    }
    uint scaleFactor;
    switch (density) {
      case 1: {scaleFactor=1;} break;
      case 2: {scaleFactor=2;} break;
      case 3: {scaleFactor=3;} break;
      default: {scaleFactor=0;} break;
    }
    return Hive(scaleFactor, NewRegister, nRings);
  };
}

//==========================================

namespace Topology{  
  bool IsDCDOM(const OMKey& omkey) {
    const uint str = omkey.GetString();
    const uint om = omkey.GetOM();
    if (79<=str && str<=86) { // which is a DC string
      if (11<=om && om<=60) {
	return(true);
      }
    }
    return(false);
  }
  
  bool IsVetoCapDOM(const OMKey& omkey) {
    const uint str = omkey.GetString();
    const uint om = omkey.GetOM();
    if (79<=str && str<=86) { // which is a DC string
      if (1<=om && om<=10) {
	return(true);
      }
    }
    return(false);
  }
  
  bool IsDCFidDOM(const OMKey& omkey) {
    const uint str = omkey.GetString();
    const uint om = omkey.GetOM();
    if (79<=str && str<=86) {
      if (11<=om && om<=60) {
	return true;
      }
    }
    if (str==36 || str==45 || str==46 || str==35 || str==37 || str==26 || str==27) {
      if (40<=om && om<=60) { //TODO DANGER find right values
	return true;
      }
    }
    return(false);
  }
  
  bool IsVetoCapFidDOM(const OMKey& omkey) {
    const uint str = omkey.GetString();
    const uint om = omkey.GetOM();
    if (79<=str && str<=86){
      if (1<=om && om<=10){
	return(true);
      }
    }
    if (str==36 || str==45 || str==46 || str==35 || str==37 || str==26 || str==27) {
      if (20<=om && om<=25) { //TODO DANGER find right values
	return(true);
      }
    }
    return(false);
  }
  
  bool IsDC_VetoCapFidDOM(const OMKey& omkey) {
    return (IsVetoCapFidDOM(omkey) || IsDCFidDOM(omkey));
  }
  
  bool IsIceTopDOM(const OMKey& omkey) {
    const uint om = omkey.GetOM();
    return (61 <= om && om <= 64);
  }
  
  // =========just strings =====================
  bool IsDCString(const OMKey& omkey) {
    const uint str = omkey.GetString();
    if (81<=str && str<=86) {
      return true;
    }
    return(false);
  }
  
  bool IsPinguString(const OMKey& omkey) {
    const uint str = omkey.GetString();
    if (str==79 || str==80) { // which is a DC string
      return(true);
    }
    return(false);
  }

  bool IsICString(const OMKey& omkey) {
    return !(IsPinguString(omkey) || IsDCString(omkey));
  }
}

