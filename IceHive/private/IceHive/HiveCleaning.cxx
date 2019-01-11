/**
 * \file HiveCleaning.cxx
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 */

#include "IceHive/HiveCleaning.h"

#include "icetray/I3Units.h"

using namespace std;
using namespace OMKeyHash;
using namespace HitSorting;
using namespace Limits;

static const LimitPair v_ic[] = {LimitPair(-70., 70.), LimitPair(-70., 70.)};
static const LimitPair v_dc[] = {LimitPair(-70., 70.), LimitPair(-70., 70.)};
static const LimitPair  v_p[] = {LimitPair(-70., 70.), LimitPair(-70., 70.)};
HiveCleaning_ParameterSet::HiveCleaning_ParameterSet():
  multiplicity(1),
  timeStaticMinus(200*I3Units::ns),
  timeStaticPlus(200*I3Units::ns),
  selfconnect(false),
  SingleDenseRingVicinityPairs(std::vector<LimitPair>(v_ic, v_ic + sizeof(v_ic)/sizeof(v_ic[0]) )),
  DoubleDenseRingVicinityPairs(std::vector<LimitPair>(v_dc, v_dc + sizeof(v_dc)/sizeof(v_dc[0]) )),
  TripleDenseRingVicinityPairs(std::vector<LimitPair>(v_p, v_p + sizeof(v_p)/sizeof(v_p[0]) ))
{}


//===================== IMPLEMENTATIONS ============================
#include <math.h>
#include <algorithm>
#include <boost/foreach.hpp>
#include "IceHive/Hive-lib.h"

//===============class HiveCleaning=================================

HiveCleaning::HiveCleaning(const HiveCleaning_ParameterSet& params):
  vicinityMap_(OMKeyHash::MAX_SIMPLEINDEX+1), // +1 because index[0] counts as 1
  params_(params)
{
  HiveGeometry hivegeo;
  hivegeo.hives_.push_back(honey::ReadHiveFromFile(boost::lexical_cast<std::string>(getenv("I3_BUILD")) +"/IceHive/resources/data/SingleDenseHive.dat"));
  hivegeo.hives_.push_back(honey::ReadHiveFromFile(boost::lexical_cast<std::string>(getenv("I3_BUILD")) +"/IceHive/resources/data/DoubleDenseHive.dat"));
  hivegeo.hives_.push_back(honey::ReadHiveFromFile(boost::lexical_cast<std::string>(getenv("I3_BUILD")) +"/IceHive/resources/data/TripleDenseHive.dat"));

  hivegeo.topo_ = builder::ReadTopologyFromFile(boost::lexical_cast<std::string>(getenv("I3_BUILD")) +"/IceHive/resources/data/IC86Topology.dat");
  hivegeo_ = hivegeo;
  
  CheckParams_n_Setup();
};


void HiveCleaning::Configure(const HiveCleaning_ParameterSet& params) {
  params_ = params;
  CheckParams_n_Setup();
};


void HiveCleaning::CheckParams_n_Setup() {
  if (params_.SingleDenseRingVicinityPairs.NRings() > (int)(hivegeo_.hives_[0].max_rings_)
    || params_.DoubleDenseRingVicinityPairs.NRings() > (int)(hivegeo_.hives_[1].max_rings_)
    || params_.TripleDenseRingVicinityPairs.NRings() > (int)(hivegeo_.hives_[2].max_rings_))
  {
    log_fatal("Sorry, currently only 2 Rings are allowed; if you want more, implement more in Hive-lib");
  }
  
  RingVicinityPairsMap_[0] = params_.SingleDenseRingVicinityPairs;
  RingVicinityPairsMap_[1] = params_.DoubleDenseRingVicinityPairs;
  RingVicinityPairsMap_[2] = params_.TripleDenseRingVicinityPairs;
  
  if (params_.multiplicity<0)
    log_fatal("Multiplicity has to be greater than zero");
  if (params_.multiplicity==0)
    log_warn("This mode of operation will select all pulses regardless: CHECK CORRECTNESS OF PARAMETER 'Multiplicity'!");
  if (params_.timeStaticMinus<=0.0)
    log_fatal("TimeStaticMinus should be greater than zero");
  if (params_.timeStaticPlus<=0.0)
    log_fatal("TimeStaticPlus should be greater than zero");

  log_info("This is HiveCleaning!");
  log_debug("Leaving Init()");
}


HitSorting::HitSeries HiveCleaning::Clean (const HitSorting::HitSeries& hits) {
  log_debug("Entering Cleaning()");
  using namespace HitSorting;

  HitSorting::HitSeries outhits;

  if (hits.size()==0) {
    log_warn("The series of hits is empty; Will do nothing");
    return outhits;
  }

  log_debug("Starting Cleaning routine");
  for (HitSorting::HitSeries::const_iterator hit_iter = hits.begin(); hit_iter !=hits.end(); ++hit_iter) { //for all hits
    log_trace_stream(" Probing next hit: " << *hit_iter);
    int connected_neighbors=0;
    HitSorting::HitSeries::const_reverse_iterator past_riter(hit_iter);
    while (past_riter != hits.rend() //
      && (hit_iter->GetTime() - past_riter->GetTime())<=params_.timeStaticMinus)
    { // iterate over all past hits within the time limitation
      if (vicinityMap_.Get(hit_iter->GetDOMIndex(), past_riter->GetDOMIndex())) { //find connected neighbours
        log_trace_stream("found a past hit to link to : " << *past_riter);
        ++connected_neighbors;
      }
      ++past_riter; //try the next possible neighbour
    }
    
    HitSorting::HitSeries::const_iterator future_iter(hit_iter);
    while (future_iter!=hits.end()
      && (future_iter->GetTime() - hit_iter->GetTime())<=params_.timeStaticPlus)
    {
      if (vicinityMap_.Get(future_iter->GetDOMIndex(), hit_iter->GetDOMIndex())) {
        log_trace_stream("found a future hit to link to : " << *future_iter);
        ++connected_neighbors;
      }
      ++future_iter; //try the next possible neighbour
    }
    
    if (connected_neighbors>=params_.multiplicity) {
      log_trace("found enough connected neighbors");
      outhits.push_back(*hit_iter); //and keep the hit
    }  
  }
  log_debug("Finished Cleaning routine");

  if (outhits.size()==0) {
    log_info("The output series of hits is empty: Everything was cleaned away; if this should happen too often check your settings!");
    return HitSorting::HitSeries();
  }

  sort(outhits.begin(), outhits.end());

  log_debug("Leaving Cleaning()");
  return outhits;
};


void HiveCleaning::BuildLookUpTables (const I3Geometry& geo) {
  log_debug("Entering BuildLookUpTables()");
  using namespace honey;
  using namespace builder;
  using namespace OMKeyHash;
  
  log_info_stream("Building DistanceMap"<< std::flush);
  /* MATRICE LOOKS LIKE THIS: horizontal index_x, vertical index_y;
   * index row annotates IceTop-strings (T) (=OM 61,62,63,64 per string),
   * 0 denotes double(0), d denotes a real double (=distance), N denotes NAN (="Not A Neighbor")

      | x               ... T T T T                 ... T T T T ..... T T T T (too distant ring)  T T T T ..
    --------------------------------------------------------------------------------------------------------
    y | 0 d d d N N N N ... N N N N d d d d d N N N ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | d 0 d d d N N N ... N N N N N d d d d d N N ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | d d 0 d d d N N ... N N N N N N d d d d d N ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | d d d 0 d d d N ... N N N N N N N d d d d d ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | N d d d 0 d d d ... N N N N N N N N d d d d ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | N N d d d 0 d d ... N N N N N N N N N d d d ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | N N N d d d 0 d ... N N N N N N N N N N d d ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | N N N N d d d 0 ... N N N N N N N N N N d d ... N N N N ..... N N N N N N N N N N N N ... N N N N
      |     ...             N N N N     ...             N N N N  ...  N N N N     ...             N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
      | d N N N N N N N ... N N N N 0 d d d N N N N ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | d d N N N N N N ... N N N N d 0 d d d N N N ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | d d d N N N N N ... N N N N d d 0 d d d N N ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | d d d d N N N N ... N N N N d d d 0 d d d N ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | N d d d d N N N ... N N N N N d d d 0 d d d ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | N N d d d d N N ... N N N N N N d d d 0 d d ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | N N N d d d d N ... N N N N N N N d d d 0 d ... N N N N ..... N N N N N N N N N N N N ... N N N N
      | N N N N d d d d ... N N N N N N N N d d d 0 ... N N N N ..... N N N N N N N N N N N N ... N N N N
      |       ...           N N N N     ...             N N N N  ...  N N N N     ...             N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
      |      .....          N N N N    .....            N N N N ..... N N N N     .....           N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
    T | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
      | N N N N N N N N ... N N N N N N N N N N N N ... N N N N ..... N N N N 0 d d d N N N N ... N N N N
      | N N N N N N N N ... N N N N N N N N N N N N ... N N N N ..... N N N N d 0 d d d N N N ... N N N N
      | N N N N N N N N ... N N N N N N N N N N N N ... N N N N ..... N N N N d d 0 d d d N N ... N N N N
      | N N N N N N N N ... N N N N N N N N N N N N ... N N N N ..... N N N N d d d 0 d d d N ... N N N N
      | N N N N N N N N ... N N N N N N N N N N N N ... N N N N ..... N N N N N d d d 0 d d d ... N N N N
      | N N N N N N N N ... N N N N N N N N N N N N ... N N N N ..... N N N N N N d d d 0 d d ... N N N N
      | N N N N N N N N ... N N N N N N N N N N N N ... N N N N ..... N N N N N N N d d d 0 d ... N N N N
      | N N N N N N N N ... N N N N N N N N N N N N ... N N N N ..... N N N N N N N N d d d 0 ... N N N N
      |     ...             N N N N     ...             N N N N  ...  N N N N     ...             N N N N
      | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
      | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
      | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
      | N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N
      | ...                 N N N N ...                 N N N N ..... N N N N     .....           N N N N
      */

  //NOTE SPEED TWEAKS for filling matrices with symmetrical indexes:
  // This matrix has a clear symmetry and block structure, which makes it possible to fast fill certain regions.
  // Some places will be filled twice, which is faster than many conditional-statements:
  // - Fill diagonals explicitly
  // - Loop only lower half and switch indexes to fill upper half
  // - do not double set entries, if they are set by another routine
  // - if strings are not indexed in HiveLib for any given central string, all its entries will be NAN on this string
  // - for the WhichRing() from the hives use some semi-hardcodings and differentiations into subhives for fastest access and iteration depth


  //precache the I3Position for each DOM from OMGeo and its topology; do this once
  std::vector<I3Position> domPos_cache(MAX_SIMPLEINDEX+1, I3Position(NAN,NAN,NAN));
  std::vector<int> domCom_cache(MAX_SIMPLEINDEX+1, -1);
  BOOST_FOREACH(const I3OMGeoMap::value_type &omkey_pos, geo.omgeo) {
    const OMKey& omkey = omkey_pos.first;

    if (IsHashable(omkey)) {
      const I3Position& pos = omkey_pos.second.position;
      const unsigned int index_A = OMKey2SimpleIndex(omkey);
    
      const int domTopo = hivegeo_.topo_.topoReg_[omkey.GetString()].GetOmTopo(omkey.GetOM());
      const int domCom = (domTopo<=0)? domTopo: (1 << (domTopo-1)); 
    
      domPos_cache[index_A]=pos;
      domCom_cache[index_A]=domCom;
      }
    else {
      log_trace_stream("Ignoring " << omkey << ", which is not part of IceCube or IceTop.");
    }
  }

  //pave the way, set everything to false
//   for (unsigned int index_A=0;index_A<=MAX_SIMPLEINDEX;++index_A) {
//     for (unsigned int index_B=0;index_B<=MAX_SIMPLEINDEX;++index_B) {
//       vicinityMap_.Set(index_A, index_B, false); //NOTE done by constructor
//     }
//   }

  if (params_.selfconnect) {
    // fill diagonal for all DOMs:
    // fill them with 'true' first and than correct the icetop omkeys later
    for (unsigned int index_A=0;index_A<=MAX_SIMPLEINDEX;++index_A) {
      log_trace_stream("Looking for diagonal " << SimpleIndex2OMKey(index_A) << std::endl);
      if (domCom_cache[index_A]<=0) { 
        log_trace("IceTop or untreated DOM: never connect");
        //vicinityMap_.Set(index_A, index_A, false);
        continue;
      }
      else { //any other dom
        log_trace("Diagonal element: true");
        vicinityMap_.Set(index_A, index_A, true);
      }
    }
  }
  log_debug_stream("(diag)"<<std::flush);


  //loop over all remaining indices and thereby fill everything
  //===== LOOP A =====
  for (unsigned int index_A=0;index_A<=MAX_SIMPLEINDEX;++index_A) {
    log_trace_stream("====Fill next row === : " <<  index_A << ":"  << SimpleIndex2OMKey(index_A) << std::endl);
    if (index_A%MAX_OMS==0) //indicate the progress
      log_debug_stream("S"<<(int)(index_A/MAX_OMS)+1<<" "<<std::flush);

    const int& domTopo_A = domCom_cache[index_A];

    //if index_A is icetop, set all entries to NAN immediately
    if (domTopo_A<=0) {
      log_trace("IceTop or not included: never connect");
      continue;
    }

    const OMKey omkey_A(SimpleIndex2OMKey(index_A));
    const I3Position& pos_A = domPos_cache[index_A];
    const double z_A(pos_A.GetZ());

    //===== LOOP B =====
    for (unsigned int index_B=index_A+1;index_B<=MAX_SIMPLEINDEX;++index_B) {
      log_trace_stream("Looking for " << SimpleIndex2OMKey(index_A) << " and " << SimpleIndex2OMKey(index_B) << std::endl);

      const int& domTopo_B = domCom_cache[index_B];

      if (domTopo_B<=0) {// any is IceTop: never connect
        log_trace("IceTop or not included: never connect");
        //NOTE (speed tweak) do nothing here, because all entries will be set by the IceTop treatment of matrix_x;
        continue;
      }

      const OMKey omkey_B(SimpleIndex2OMKey(index_B));

      const I3Position& pos_B = domPos_cache[index_B];

      //this is a smart but ugly way to check combinations of 2 values, which can have 3 states
      log_trace_stream("domTopo_A " << domTopo_A <<" domTopo_B " << domTopo_B << std::endl;);
      //choose the right values when comparing different topologies
      Hive* hivePtr = 0; //the hive to perform th lookup on
      unsigned int rel_scale_factor =1; //relative scale factor in compared DOMs
      RingLimits* vicinityPairsPtr_ = 0;
      switch (domTopo_A+domTopo_B) {
        case 2: //(IceCube+IceCube):
          hivePtr = &(hivegeo_.hives_[0]);
          rel_scale_factor=1;
          vicinityPairsPtr_ = &RingVicinityPairsMap_[0];
          break;
        case 3: //(IceCube+DeepCore):
          hivePtr = &(hivegeo_.hives_[1]);
          rel_scale_factor=2;
          vicinityPairsPtr_ = &RingVicinityPairsMap_[0];
          break;
        case 5: //(IceCube+Pingu):
          hivePtr = &(hivegeo_.hives_[2]);
          rel_scale_factor=3;
          vicinityPairsPtr_ = &RingVicinityPairsMap_[0];
          break;
        case 4: //(DeepCore+DeepCore):
          hivePtr = &(hivegeo_.hives_[1]);
          rel_scale_factor=1;
          vicinityPairsPtr_ = &RingVicinityPairsMap_[1];
          break;
        case 6: //(DeepCore+Pingu):
          hivePtr = &(hivegeo_.hives_[2]);
          rel_scale_factor=2;
           vicinityPairsPtr_ = &RingVicinityPairsMap_[1];
          break;
        case 8: //(Pingu+Pingu):
          hivePtr = &(hivegeo_.hives_[2]);
          rel_scale_factor=1;
          vicinityPairsPtr_ = &RingVicinityPairsMap_[2];
          break;
        default:
          log_fatal_stream("non registered topology prescription "<< domTopo_A+domTopo_B <<": "
            <<"domTopoA("<<omkey_A<<"): " << domTopo_A <<" domTopo_B("<<omkey_B<<"): "<< domTopo_B);
      }

      //use that string as center, of which the hive is actually build around
      const unsigned int center_string = ((domTopo_A >= domTopo_B) ? omkey_A.GetString() : omkey_B.GetString());
      const unsigned int lookup_string = ((domTopo_A >= domTopo_B) ? omkey_B.GetString() : omkey_A.GetString());

      const int ring = WhichRing(hivePtr->combs_, center_string, lookup_string);

      if (ring == -1) {//not in the ring indexing range
        log_trace("Not included in ring index range");
        //NOTE (speed tweak) make it snappy and set all entries on that string in index_B to NAN, because we will never connect
        //DANGER this is only possible, if the indexed ring range (in m) of HiveLib is equal for all Hive configurations;
        // DeepCore-Hive must index twice as many rings as IceCube-Hive, Pingu-Hive in turn must index twice as many rings as DeepCore-Hive
        // in any other case only this exact entry(x,y) can be set NAN
        const unsigned int string_end = StringOmNbr2SimpleIndex(SimpleIndex2StringNbr(index_B),MAX_OMS);
        //for (; index_B<=string_end-4; ++index_B){ //there are 4 icetopDOMs remember //NOTE already set
        //  DistanceMap_[index_A, index_B]=NAN;
        //  DistanceMap_[index_B, index_A]=NAN;
        //}
        index_B=string_end; //account for the overstepping of the for loop
        continue;
      }

      
      //IN RING SYSTEM evaluation
      const int rescaled_ring = int((ring+rel_scale_factor-1)/rel_scale_factor);
      
      double minusVicinity;
      double plusVicinity;
      if (rescaled_ring > int(vicinityPairsPtr_->NRings()) ) {
        log_trace_stream("Ring "<<ring<<", rescaled "<<rescaled_ring<<" too far away; max Ringlimits "<<(vicinityPairsPtr_->NRings()));
        minusVicinity = NAN;
        plusVicinity = NAN;
      }
      else{
        minusVicinity = vicinityPairsPtr_->GetLimitsOnRing(rescaled_ring).minus_;
        plusVicinity = vicinityPairsPtr_->GetLimitsOnRing(rescaled_ring).plus_;
      }
      
      const double z_B(pos_B.GetZ());

      const double zdist_AB = z_B-z_A;

      //compare A to B
      if (std::isnan(minusVicinity) && std::isnan(plusVicinity)) {
        log_trace("Not configured vicinity");
      } else if (minusVicinity<= zdist_AB && zdist_AB <= plusVicinity) {
        vicinityMap_.Set(index_A, index_B, true);
        log_trace("DOMs are connected; Vicinity True");
      } else {
        //DistanceMap_[index_A, index_B] = NAN; //NOTE already set
        log_trace("Not on ring vicinty");
      }

      //compare B to A
      const double zdist_BA = -zdist_AB; //notice: switched indices
      if (std::isnan(minusVicinity) && std::isnan(plusVicinity)) {
        log_trace("Not configured vicinity");
      } else if (minusVicinity<= zdist_BA && zdist_BA <= plusVicinity) {
        vicinityMap_.Set(index_B, index_A, true);
        log_trace("DOMs are connected; Vicinity True");
      } else {
        //DistanceMap_[index_A, index_B] = NAN; //NOTE already set
        log_trace("Not on ring vicinty");
      }
    }
  }
  log_info("DistanceMap built");
  log_debug("Leaving BuildLookUpTables()");
};
