/**
 * \file Hive-lib.cxx
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 */

#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>

#include <boost/assign/list_of.hpp>

#include <iostream>
#include <cstdio>
#include <fstream>
#include <boost/lexical_cast.hpp>

#include "IceHive/Hive-lib.h"

namespace honey{
  //=== class DOMHoneyComb === IMPLEMENTATIONS ================

  StringNbr DOMHoneyComb::GetCenter() const {
    if (honey_.size()>0 && honey_[0].size()==1)
      return *honey_[0].begin();
    return -1; // center not set
  }

  void DOMHoneyComb::AddCenter(const StringNbr center){
    if (honey_.size()!=0) {
      log_fatal("violation against the ordering of rings!");
    }
    honey_.push_back(boost::assign::list_of(center));
  }

  void DOMHoneyComb::SetCenter(const StringNbr center){
    SetRing(0, boost::assign::list_of(center));
  }

  Ring DOMHoneyComb::GetRing(const unsigned int ringnbr) const {
    if (honey_.size() <= ringnbr)
      return std::set<uint>();
    else
      return honey_[ringnbr];
  }

  void DOMHoneyComb::AddRing(const unsigned int ringnbr, const Ring &ring) {
    if (honey_.size()!=ringnbr) {
      log_fatal("violation against the ordering of rings!");
    }
    honey_.push_back(ring);
  }

  void DOMHoneyComb::SetRing(const unsigned int ringnbr, const Ring &ring) {
    if (honey_.size()<=ringnbr){
      log_fatal("violation against the ordering of rings! Use AddRing() method to add new rings first");
    }
    honey_[ringnbr]=ring;
  }

  DOMHoneyComb::DOMHoneyComb() {};

  DOMHoneyComb::DOMHoneyComb(const StringNbr center, const Ring& ring1){
    AddCenter(center);
    AddRing(1,ring1);
  }

  DOMHoneyComb::DOMHoneyComb(const StringNbr center, const Ring& ring1, const Ring& ring2){
    AddCenter(center);
    AddRing(1,ring1);
    AddRing(2,ring2);
  }

  DOMHoneyComb::DOMHoneyComb(const StringNbr center, std::vector<Ring>& rings) { //TODO
    AddCenter(center);
    for (unsigned int i=1; i <=rings.size(); i++)
      AddRing(i,rings[i]);
  }

  void DOMHoneyComb::AddStringToRing(const StringNbr center, const unsigned int ringnbr, const StringNbr string) {
    const unsigned int current_rings = GetNRings();
    if (current_rings+1<ringnbr) {
      log_fatal("Violation against the ordering of rings; Use AddRing() method to add new rings first");
    }
    else if (current_rings+1==ringnbr) {
      Ring newring;
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
  ///constructor
  Hive::Hive(const unsigned int scaleFactor,
             const DOMHoneyCombRegister& combs,
             unsigned int max_rings):
    scaleFactor_(scaleFactor),
    max_rings_(max_rings),
    combs_(combs)
  {};

  

  void MutualAddStringToRing(DOMHoneyCombRegister &combs, const StringNbr center_A, const StringNbr center_B, const unsigned int ringnbr) {
    DOMHoneyCombRegister::iterator honey_A = combs.find(center_A);
    DOMHoneyCombRegister::iterator honey_B = combs.find(center_B);
    if (honey_A==combs.end() ||honey_B==combs.end() ){
      log_fatal_stream("strings not registered to the combs; Use AddCenter() method to add new central strings first" << std::endl);
    }
    honey_A->second.AddStringToRing(center_A, ringnbr, center_B);
    honey_B->second.AddStringToRing(center_B, ringnbr, center_A);
  }

  bool IsRingX (const DOMHoneyCombRegister &combs, const StringNbr center, const unsigned int ringnbr, const StringNbr string) {
    DOMHoneyCombRegister::const_iterator honey = combs.find(center);
    if (honey==combs.end()) {
      log_trace_stream("Center string '" <<  center << "' not registered in combs" << std::endl);
      return false;
    }
    const Ring& ring= (honey->second).GetRing(ringnbr);
    return ring.count(string);
  }

  inline bool IsRing1(DOMHoneyCombRegister &combs, const StringNbr center, const StringNbr string) {
    return IsRingX(combs, center, 1, string);
  }

  inline bool IsRing2(DOMHoneyCombRegister &combs, const StringNbr center, const StringNbr string) {
    return IsRingX(combs, center, 2, string);
  }

  int WhichRing(const DOMHoneyCombRegister &combs, const StringNbr center, const StringNbr string, const unsigned int search_depth) {
    DOMHoneyCombRegister::const_iterator honey = combs.find(center);
    if (honey==combs.end()) {
      log_info_stream("Could not locate center string '"<< center << "' in combs" << std::endl);
      return -1;
    }
    for (unsigned int honey_iter =0; honey_iter<= (honey->second).GetNRings() && honey_iter<=search_depth; honey_iter++) {
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
      const StringNbr center_string = CombPtr->first;
      //log_info("abstracting string "<<center_string <<", "<< std::flush;
      NewRegister[center_string].AddCenter(center_string);
      for (unsigned int ring_index =1; ring_index<=CombPtr->second.GetNRings(); ring_index++) {
        NewRegister[center_string].AddRing(2*ring_index-1, Ring());
        NewRegister[center_string].AddRing(2*ring_index, CombPtr->second.GetRing(ring_index));
      }
    }
    log_debug("HoneyCombRegister Abstracted");
    return NewRegister;
  };

  int ExpandRings(DOMHoneyCombRegister &combs, const unsigned int scale_factor) {
    if (combs.begin()==combs.end()) {
      log_fatal("Hive is not filled with any rings; cannot expand!");
    }

    const unsigned int extremist_ring = combs.begin()->second.GetNRings();
    if (extremist_ring<scale_factor) {
      log_error("At least as many rings as the scale-factor must be registered; cannot expand!");
      return -1;
    }

    for (DOMHoneyCombRegister::iterator combs_iter=combs.begin(); combs_iter!=combs.end(); combs_iter++) {
      Ring outmost_strings = combs_iter->second.GetRing(extremist_ring);
      //log_trace("expanding string " << combs_iter->first << " with a scaling of "<< scale_factor << std::endl;

      //construct n new rings in these sets
      std::vector<Ring> next_surrounders;
      for (unsigned int scale =0; scale <=scale_factor; scale++) {
        next_surrounders.push_back(Ring());
      }
      //loop over the strings on the outmost ring
      for (Ring::iterator outer_stringPtr=outmost_strings.begin(); outer_stringPtr!=outmost_strings.end(); outer_stringPtr++) {
        //log_trace(*outer_stringPtr << std::endl;
        DOMHoneyCombRegister::const_iterator outer_center = combs.find(*outer_stringPtr);

        if (outer_center==combs.end()) {
          log_fatal_stream("There are non mutual registered strings "<<combs_iter->first<<" and "<<*outer_stringPtr<<" ; cannot expand;" << std::endl);
        }
        //add the nth ring from the outer strings to the next_surrounders n-th to construct rings
        for (unsigned int scale =1; scale <=scale_factor; scale++) {
          //log_trace_stream( "Adding "<< scale<<". ring on string "<<*outer_stringPtr<<" to next_surrounders["<<scale-1<<"]"<< std::endl;
          Ring local_surrounders = outer_center->second.GetRing(scale);
          next_surrounders[scale-1].insert(local_surrounders.begin(), local_surrounders.end());
        }
      }

      //erase the inner rings from the new constructed rings
      for (unsigned int scale =1; scale <=scale_factor; scale++) {

        for (unsigned int curr =0; curr <=scale; curr++) {
          //log_trace_stream( "Removing "<< (extremist_ring-curr)<<". ring on string "<<combs_iter->first<<" from next_surrounders["<<scale-1<<"]"<< std::endl;
          Ring almost_outmost_strings = combs_iter->second.GetRing(extremist_ring - curr);
          for (Ring::iterator almost_outer_stringPtr=almost_outmost_strings.begin(); almost_outer_stringPtr!=almost_outmost_strings.end(); almost_outer_stringPtr++)
            next_surrounders[scale-1].erase(*almost_outer_stringPtr);
        }
      }

      // in the new constructed rings, remove cascade the next inner ring from the now to add finished constructed one
      //log_trace_stream( "Registering next_surrounders["<<0<<"] as "<<(extremist_ring+1)<<". ring on string "<<combs_iter->first<< std::endl;
      combs_iter->second.AddRing(extremist_ring+1, next_surrounders[0]);

      for  (unsigned int scale =2; scale <=scale_factor; scale++) {
        //log_trace_stream( "Removing "<< (extremist_ring+scale-1)<<". ring on string "<<combs_iter->first<<" from next_surrounders["<<scale-1<<"]"<< std::endl;
        Ring almost_outmost_strings = combs_iter->second.GetRing(extremist_ring + scale-1);
        for (Ring::iterator almost_outer_stringPtr=almost_outmost_strings.begin(); almost_outer_stringPtr!=almost_outmost_strings.end(); almost_outer_stringPtr++)
          next_surrounders[scale-1].erase(*almost_outer_stringPtr);

        //log_trace_stream( "Registering next_surrounders["<<scale-1<<"] as "<<(extremist_ring+scale)<<". ring on string "<<combs_iter->first<< std::endl;
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

    const unsigned int n_rings = combs1.begin()->second.GetNRings();
    if (combs1.begin()->second.GetNRings() != combs2.begin()->second.GetNRings()) {
      //DANGER actually all entries have to be checked to have equal size; 
      // for now just check the first element as an representative
      log_fatal("Cannot unite combs with different numbers of rings registered!");
    }

    DOMHoneyCombRegister NewRegister(combs1); //make a copy of combs1

    for (DOMHoneyCombRegister::const_iterator combs2_iter = combs2.begin(); combs2_iter!=combs2.end(); combs2_iter++) {
      //log_trace("UNITE string"<< combs2_iter->first<<", " << std::flush;
      //try brute force insert first
      std::pair<DOMHoneyCombRegister::iterator,bool> string_added = NewRegister.insert(*combs2_iter);

      if (!string_added.second) { //if that was not successful, need to iterate
        //log_trace("iterate " << std::flush;

        //loop over all rings :unite the strings on that ring and substitute the union
        for (unsigned int ring_count =0; ring_count<=n_rings; ring_count++) {
          // log_trace("ring" << ring_count<< ","<< std::flush;
          Ring united = string_added.first->second.GetRing(ring_count);
          Ring strings_to_add = combs2_iter->second.GetRing(ring_count);

          united.insert(strings_to_add.begin(), strings_to_add.end());

          NewRegister[combs2_iter->first].SetRing(ring_count, united);
        }
      }
    }

    return NewRegister;
  }

  std::string DumpCenter (const DOMHoneyCombRegister &combs, const StringNbr center) {
   std::ostringstream dump;
   dump << "========= DUMPING STRING "<<center<<"==========" << std::endl;
    DOMHoneyCombRegister::const_iterator combs_iter = combs.find(center);
    if (combs_iter==combs.end()) {
      dump << "string "<<center<<" not registered in combs!" << std::endl;
      return dump.str();
    }

    const unsigned int nRings=combs_iter->second.GetNRings();

    for (unsigned int ring = 0; ring<=nRings; ring++){
      dump << std::endl << " Ring "<<ring<<" : ";
      Ring strings = combs_iter->second.GetRing(ring);
      for (Ring::iterator strings_iter=strings.begin(); strings_iter!=strings.end(); strings_iter++){
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

      const unsigned int nRings=combs_iter->second.GetNRings();

      dump << "S "<< combs_iter->first << std::endl;
      for (unsigned int ring = 1; ring<=nRings; ring++){
        dump << "R"<<ring<<" ";
        Ring strings = combs_iter->second.GetRing(ring);
        for (Ring::iterator strings_iter=strings.begin(); strings_iter!=strings.end(); strings_iter++){
          dump << *strings_iter << " ";
        }
        dump << std::endl;
      }
    }
    dump << std::endl;

    return dump.str();
  }

  Hive ReadHiveFromFile (const std::string& fileName) {
    // PREPWORK
    log_info_stream("Reading HoneyCombRegister from file " << fileName);
    unsigned int density = 0;
    unsigned int nRings = 0;
    DOMHoneyCombRegister NewRegister;

    // READ
    std::ifstream file(fileName.c_str());
    if(!file){
      log_fatal_stream("Unable to read HoneyCombRegister from " << fileName);
    }
    //containers
    std::string line, word;
    unsigned int centerNbr, strNbr, ringNbr;

    getline(file, line);
    if (line[0]=='#') {
      std::istringstream buffer(line);
      buffer>>word; buffer>>word;
      buffer>>density;
      buffer>>word;
      buffer>>nRings;
      getline(file, line);
    }
    while(file){
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
            Ring str_in_ring;
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
    file.close();

    unsigned int scaleFactor;
    switch (density) {
      case 1: {scaleFactor=1;} break;
      case 2: {scaleFactor=2;} break;
      case 3: {scaleFactor=3;} break;
      default: {scaleFactor=0;} break;
    }
    return Hive(scaleFactor, NewRegister, nRings);
  };
}

//============ NAMESPACE builder ==============================

std::string builder::DumpTopology (const builder::Topology &topo) {
  std::ostringstream dump;
  
  dump << "# Strings "<< topo.max_strings_<< std::endl; // line: # Strings 0
  
  const TopologyRegister &reg = topo.topoReg_;
  for (TopologyRegister::const_iterator reg_iter=reg.begin(); reg_iter!=reg.end(); reg_iter++) {//all strings in the reg

    dump << "S "<< reg_iter->first << "O" << reg_iter->second.GetMaxOms() << std::endl;
    for (unsigned int om=1; om<=reg_iter->second.GetMaxOms(); ++om) {
      dump << reg_iter->second.GetOmTopo(om);
    }
    dump << std::endl;
  }
  dump << std::endl;

  return dump.str();
};


builder::Topology builder::ReadTopologyFromFile (const std::string& fileName) {
  log_info_stream("Reading Topology from file " << fileName);
  Topology NewTopo;

  // READ
  std::fstream file ( fileName.c_str());
  assert(file.is_open());
  //containers
  std::string line, word;
  unsigned int strings, oms;
  unsigned int strNbr, omTopo;

  getline(file, line);
  if (line[0]=='#') {
    std::istringstream buffer(line);
    buffer>>word; buffer>>word; //word is 'Strings'
    buffer>>strings;
    getline(file, line);
  }
  else
    log_fatal_stream("File '"<<fileName<<"' seems not to have the correct format; please review the IceHive documentation");
  
  while(!file.eof()){
    std::istringstream buffer(line);
    buffer>>word;
    if (word=="S") {//new string
      buffer>>strNbr;
      buffer>>word; //if (word=="O")
      buffer>>oms;
      getline(file, line);
      std::istringstream omTopo_buffer(line);
      for (unsigned int om=1; om<=oms; ++om) {
        omTopo_buffer>>omTopo;
        NewTopo.topoReg_[strNbr].AddOmTopo(omTopo);
      }
    }
    else if (word=="#END")
      break;
    getline(file, line);
  }
  file.close();

  NewTopo.max_strings_= strings;
  return NewTopo;
};


