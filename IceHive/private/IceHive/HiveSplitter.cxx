/**
 * \file HiveSplitter.cxx
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 */

#include <dataclasses/geometry/I3OMGeo.h>
#include <IceHive/HiveSplitter.h>

using namespace std;
using namespace OMKeyHash;
using namespace HitSorting;
using namespace Limits;

//=============== class HiveSplitter_ParameterSet =================
static const LimitPair ic[] = {LimitPair(-300., 300.), LimitPair(-272.7, 272.7), LimitPair(-165.8, 165.8)};
static const LimitPair dc[] = {LimitPair(-150., 150.), LimitPair(-131.5, 131.5), LimitPair(-40.8, 40.8)};
static const LimitPair  p[] = {LimitPair(-150., 150.), LimitPair(-144.1, 144.1), LimitPair(-124.7, 124.7), LimitPair(-82.8, 82.8)};
static const LimitPair v_ic[] = {LimitPair(-300., 300.), LimitPair(-272.7, 272.7), LimitPair(-165.8, 165.8)};
static const LimitPair v_dc[] = {LimitPair(-150., 150.), LimitPair(-131.5, 131.5), LimitPair(-40.8,  40.8)};
static const LimitPair  v_p[] = {LimitPair(-150., 150.), LimitPair(-144.1, 144.1), LimitPair(-124.7, 124.7), LimitPair(-82.8, 82.8)};
//static const RingPair v_ic[] = {LimitPair(100.,100.), LimitPair(100.,100.)};
//static const RingPair v_dc[] = {LimitPair(100.,100.), LimitPair(100.,100.), LimitPair(100.,100.)};
//static const RingPair v_p[]  = {LimitPair(100.,100.), LimitPair(100.,100.), LimitPair(100.,100.), LimitPair(100.,100.)};
HiveSplitter_ParameterSet::HiveSplitter_ParameterSet():
  multiplicity(4),
  timeWindow(2000.*I3Units::ns),
  timeStatic(200.*I3Units::ns),
  timeCVMinus(200.*I3Units::ns),
  timeCVPlus(200.*I3Units::ns),
  timeCNMinus(200.*I3Units::ns),
  timeCNPlus(200.*I3Units::ns),
  selfconnect(true),
  SingleDenseRingLimitPairs(std::vector<LimitPair>(ic, ic + sizeof(ic) / sizeof(ic[0]) )),
  DoubleDenseRingLimitPairs(std::vector<LimitPair>(dc, dc + sizeof(dc) / sizeof(dc[0]) )),
  TripleDenseRingLimitPairs(std::vector<LimitPair>(p, p + sizeof(p) / sizeof(p[0]) )),
  SingleDenseRingVicinityPairs(std::vector<LimitPair>(v_ic, v_ic + sizeof(v_ic) / sizeof(v_ic[0]) )),
  DoubleDenseRingVicinityPairs(std::vector<LimitPair>(v_dc, v_dc + sizeof(v_dc) / sizeof(v_dc[0]) )),
  TripleDenseRingVicinityPairs(std::vector<LimitPair>(v_p, v_p + sizeof(v_p) / sizeof(v_p[0]) ))
{};


//=============== namespace hivesplitter::details =================

hivesplitter::detail::CausalCluster::CausalCluster():
  endHit(Hit()),
  multiplicityMet(false)
{};

double hivesplitter::detail::CausalCluster::earliestTime() const{
  if (!complete.empty() && !hits.empty())
    return(std::min(complete.begin()->GetTime(),hits.begin()->GetTime()));
  if (!complete.empty())
    return(complete.begin()->GetTime());
  if (!hits.empty())
    return(hits.begin()->GetTime());
  //else
  return(std::numeric_limits<double>::infinity());
}

void hivesplitter::detail::CausalCluster::insert(const HitSorting::Hit &h,
                                                 const unsigned int multiplicity) {
  ++doms[h.GetDOMIndex()];
  hits.push_back(h);
  //if the total number of DOMs meets the multiplicity threshold, make note,
  //and also record that this is the last known hit within the cluster contributing
  if (doms.size()>=multiplicity) {
    log_trace_stream("Just met multiplicity"<<multiplicity<<" within a cluster");
    endHit=h;
    multiplicityMet=true;
  }
}

void hivesplitter::detail::CausalCluster::takeComplete (const CausalCluster& c){
  complete.insert(c.complete.begin(), c.complete.end());
}

inline bool hivesplitter::detail::CausalCluster::isActive() {
  return !hits.empty();
}

bool hivesplitter::detail::isSubset(const CausalCluster& c1,
                                    const CausalCluster& c2) {
  if (c2.hits.size()<c1.hits.size())
    return(false);
  //use the fact that strict timeorder is enforced in the .hits
  std::list<Hit>::const_iterator it1=c1.hits.begin(), end1=c1.hits.end(), it2=c2.hits.begin(), end2=c2.hits.end();
  for (; it1!=end1 && it2!=end2; ++it1, ++it2) {
    //if the two current items don't match scan though the (potential) superset looking for a match
    while (it2!=end2 && *it2<*it1)
      ++it2;
    //three possible cases arise:
    //if the items are now equal, c1 still appears to be a subset
    //if the item in c2 is greater than the one in c1, or we've gone off of the end of c2
    // there is no match for this item, so c1 is not a subset
    if (it2==end2 || *it1<*it2)
      return(false);
  }
  //if all of the items matched until we ran off of the end of c2,
  //but there are still items left in c1, c1 is not a subset
  return(!(it1!=end1 && it2==end2));
}



//============================== IMPLEMENTATIONS =============================

#include <algorithm>
#include <math.h>
#include <boost/foreach.hpp>

//===============class HiveSplitter=================================


HiveSplitter::HiveSplitter(const HiveSplitter_ParameterSet& params):
  distanceMap_(OMKeyHash::MAX_SIMPLEINDEX+1), //+1 because index range is [0, MSI]
  vicinityMap_(OMKeyHash::MAX_SIMPLEINDEX+1),
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


void HiveSplitter::Configure(const HiveSplitter_ParameterSet& params) {
  params_ = params;
  CheckParams_n_Setup();
};


void HiveSplitter::CheckParams_n_Setup() {
  log_debug("Entering CheckParams_n_Setup()");
  
  if (params_.SingleDenseRingLimitPairs.NRings() > (int)(hivegeo_.hives_[0].max_rings_)
    || params_.DoubleDenseRingLimitPairs.NRings() > (int)(hivegeo_.hives_[1].max_rings_)
    || params_.TripleDenseRingLimitPairs.NRings() > (int)(hivegeo_.hives_[2].max_rings_)
    || params_.SingleDenseRingVicinityPairs.NRings() > (int)(hivegeo_.hives_[0].max_rings_)
    || params_.DoubleDenseRingVicinityPairs.NRings() > (int)(hivegeo_.hives_[1].max_rings_)
    || params_.TripleDenseRingVicinityPairs.NRings() > (int)(hivegeo_.hives_[2].max_rings_))
  {
    log_fatal("Sorry, currently only 2 rings are allowed; if you want more, implement more in Hive-lib");
  }
  
  //purge maps
  RingLimitPairsMap_.clear();
  RingVicinityPairsMap_.clear();

   RingLimitPairsMap_[0]=params_.SingleDenseRingLimitPairs;
  RingLimitPairsMap_[1]=params_.DoubleDenseRingLimitPairs;
  RingLimitPairsMap_[2]=params_.TripleDenseRingLimitPairs;
  RingVicinityPairsMap_[0] = params_.SingleDenseRingVicinityPairs;
  RingVicinityPairsMap_[1] = params_.DoubleDenseRingVicinityPairs;
  RingVicinityPairsMap_[2] = params_.TripleDenseRingVicinityPairs;

  if (params_.multiplicity<=0)
    log_fatal("Multiplicity should be greater than zero");
  if (params_.timeWindow<=0.0)
    log_fatal("TimeWindow should be greater than zero");
  if (params_.timeStatic<=0.0)
    log_fatal("TimeStatic should be greater than zero");
  if (params_.timeCVPlus<=0.0)
    log_fatal("TimeCVPlus should be greater than zero");
  if (params_.timeCVMinus<=0.0)
    log_fatal("TimeCVMinus should be greater than zero");
  if (params_.timeCNPlus<=0.0)
    log_fatal("TimeCNPlus should be greater than zero");
  if (params_.timeCNMinus<=0.0)
    log_fatal("TimeCNMinus should be greater than zero");

  log_info("This is HiveSplitter!");
  log_debug("Leaving Init()");
}

HitSorting::HitSeriesSeries HiveSplitter::Split (const HitSorting::HitSeries &inhits) {
  log_debug("Entering Split()");
  HitSorting::HitSeriesSeries outhits_series;

  //feed the hits into the clustering machinery
  for (HitSorting::HitSeries::const_iterator hit=inhits.begin(); hit!=inhits.end(); ++hit){
    AddHit(*hit);
  }
  FinalizeSubEvents();

  //move the outputs subEvents to the outbox
  for (hivesplitter::detail::HitSetSequence::const_iterator subEvent_iter=subEvents_.begin(); subEvent_iter!=subEvents_.end(); ++subEvent_iter){
    HitSorting::HitSeries outhits(subEvent_iter->begin(), subEvent_iter->end());
    outhits_series.push_back(outhits);
  }
  subEvents_.clear();

  log_debug("Leaving Split()");  
  return outhits_series;
};

void HiveSplitter::AdvanceClusterInTime(hivesplitter::detail::CausalCluster& cluster,
                                                  const double time) {
  log_debug("Entering AdvanceClusterInTime");
  while (!cluster.hits.empty()) {
    const HitSorting::Hit &h=*cluster.hits.begin();
    if (time > h.GetTime()+params_.timeWindow) {
      //decrement the number of hits on the DOM where h occurred
      if ((--cluster.doms[h.GetDOMIndex()])<=0)
        cluster.doms.erase(h.GetDOMIndex());
      //if the mutiplicity threshold was met include h in the finished cluster
      if (cluster.multiplicityMet) {
        //insert the hit, and hint that it should go at the end of the complete set
        if (cluster.complete.empty())
          cluster.complete.insert(h);
        else
          cluster.complete.insert(--cluster.complete.end(),h);
        //if h was the last in the multiplicity window,
        //shove the finished cluster back to the trigger for merging
        if (h==cluster.endHit) {
          AddSubEvent(cluster.complete);
          cluster.multiplicityMet=false;
          //no need to bother resetting endHit;
          //there can't be a false positive because we'll never see h again
        }
      }
      cluster.hits.pop_front();
    }
    else
      break;
  }
}

bool HiveSplitter::CausallyConnected(const HitSorting::Hit& h1,
                                     const HitSorting::Hit& h2) {
  log_debug("Evaluating causallyConnected()");
  if (h1.GetTime() > h2.GetTime()) {
    return CausallyConnected(h2, h1); //recursive call to enforce timeorder at this point
  }
  
  const double dist = distanceMap_.Get(h1.GetDOMIndex(), h2.GetDOMIndex());
  const bool in_distance = !std::isnan(dist);
  const bool in_vicinity = vicinityMap_.Get(h1.GetDOMIndex(), h2.GetDOMIndex());
  
  if (! (in_vicinity || in_distance) ) {
    log_debug_stream("Hits "<<h1<<" & "<<h2<<" are: "<<std::endl
      << (!in_distance ? "NOT" :"") << " in distance"<<std::endl
      << (!in_vicinity ? "NOT" :"") << " in vicinity");
    log_debug_stream("Hits are NOT CONNECTED");
   return false;
  }
  const double dt=std::abs(h1.GetTime() - h2.GetTime()); 
  const bool time_causal = (dt<= params_.timeStatic);
  const double time_residual_cv = (dt - dist/I3Constants::c);
  const bool particle_causal = (- params_.timeCVMinus<=time_residual_cv && time_residual_cv<= params_.timeCVPlus);
  const double time_residual_cn = (dt - dist/I3Constants::c*I3Constants::n_ice_group);
  const bool photon_causal = (- params_.timeCNMinus<=time_residual_cn && time_residual_cn<= params_.timeCNPlus);

  const bool eval = ((in_vicinity && time_causal) || (in_distance && (particle_causal || photon_causal)));
 
  log_debug_stream("Hits "<<h1<<" & "<<h2<<" are: "<<std::endl
    << (!in_distance ? "NOT" :"") << " in distance"<<std::endl
    << (!in_vicinity ? "NOT" :"") << " in vicinity"<<std::endl
    << (!time_causal ? "NOT" :"") << " time causal ("<<dt<<")"<<std::endl
    << (!photon_causal ? "NOT" :"") << " photon causal ("<<time_residual_cn<<")"<<std::endl
    << (!particle_causal ? "NOT" :"") << " particle_causal ("<<time_residual_cv<<")");
  log_debug_stream("Hits are "<<(!eval ? "NOT " :"")<<"CONNECTED");
  return (eval);
}


void HiveSplitter::AddHit (const HitSorting::Hit& h) {
  log_debug("Entering AddHit()");
  newClusters_.clear();
  bool addedToCluster=false; //keep track of whether h has been added to any cluster
  
  hivesplitter::detail::CausalClusterList::iterator cluster=clusters_.begin();
  while (cluster != clusters_.end()) {
    //each cluster is advanced in time:
    //removing all too old/expired hits, which cannot make any connections any more;
    //concluded clusters, which do not have any connecting hits left, become 'Inactive' and are put to the garbage
    //if the cluster is still active, try to add the Hit to the cluster
    AdvanceClusterInTime(*cluster, h.GetTime());
    if (cluster->isActive()) {
      addedToCluster |= AddHitToCluster(*cluster, h, params_.multiplicity);
      ++cluster;
    }
    else
      cluster = clusters_.erase(cluster);
  }

  //Move all newly generated clusters into the main cluster list,
  //eliminating clusters which are subsets of other clusters
  for (hivesplitter::detail::CausalClusterList::iterator newCluster=newClusters_.begin(), nend=newClusters_.end(); newCluster!=nend; newCluster++) {
    bool add=true;

    cluster=clusters_.begin();
    while (cluster != clusters_.end()) {
      //check whether the new cluster is a subset of the old cluster
      //if the old cluster does not contain h, it cannot be a superset of the new cluster which does,
      //and if the old cluster contains h, it will be the last hit in that cluster
      if (cluster->hits.back()==h){
        if (isSubset(*newCluster,*cluster)) {
          add=false;
          break;
        }
        ++cluster;
      }
      //otherwise, the new cluster may still be a superset of the old cluster
      else if (isSubset(*cluster,*newCluster)) {
        //if replacing, make sure not to lose any hits already shifted to the old cluster's 'complete' list
        newCluster->takeComplete(*cluster);
        cluster = clusters_.erase(cluster);
      }
      else
        ++cluster;
    }
    if (add)
      clusters_.push_back(*newCluster);
  }
  newClusters_.clear();

  //if h was not added to any cluster, put it in a cluster by itself
  if (!addedToCluster) {
    clusters_.push_back(hivesplitter::detail::CausalCluster());
    clusters_.back().insert(h, params_.multiplicity);
  }
  log_debug("Leaving AddHit()");
}


bool HiveSplitter::AddHitToCluster (hivesplitter::detail::CausalCluster& c,
                                              const HitSorting::Hit& h,
                                              const unsigned int multiplicity) {
  log_debug("Entering AddHitToCluster()");
  //First, find out how many of the DOMs with hits in this cluster are causally connected to h
  //if there are more than multiplicity, h will be added to this cluster
  
  log_debug_stream("Adding hit"<<h<<" to cluster of size : "<<c.hits.size());
  
  std::set<unsigned int> connectedDOMs;
  bool allConnected=true;
  for (std::list<HitSorting::Hit>::reverse_iterator it=c.hits.rbegin(), end=c.hits.rend(); it!=end; ++it) {
    if (CausallyConnected(*it,h))
      connectedDOMs.insert(it->GetDOMIndex());
    else
      allConnected=false;
    
    if (connectedDOMs.size()+1>=multiplicity) //NOTE plus one for h itself!
      break;
  }
  
  
  if (connectedDOMs.size()+1 >=multiplicity || allConnected) {
    log_debug("Adding Hit to custer");
    c.insert(h, multiplicity); //h is connected to enough hits to belong in this cluster
  }
  else if (connectedDOMs.size()>0) {
    //if h is connected to some of the hits in the cluster, but not enough to be a member,
    //we need to build a new cluster containing only those hits with which it is connected to
    hivesplitter::detail::CausalCluster newSubset;
    for (std::list<HitSorting::Hit>::iterator it=c.hits.begin(), end=c.hits.end(); it!=end; ++it) {
      if (CausallyConnected(*it, h))
        newSubset.insert(*it, params_.multiplicity);
    }
    newSubset.insert(h, params_.multiplicity);
    //we add the new cluster to our list of newly created clusters,
    //but if it is a subset of a cluster we already have we discard it as redundant,
    //and likewise if any of the clusters we have is a subset we discard that one
    bool keep=true;

    hivesplitter::detail::CausalClusterList::iterator it=newClusters_.begin();
    while (it != newClusters_.end()) {
      if (isSubset(*it,newSubset))
        it = newClusters_.erase(it); //remove a redundant, existing cluster
      else if (isSubset(newSubset,*it)) {
        keep=false; //this cluster is redundant, so abort adding it
        break;
      }
      else
        ++it;
    }
    if (keep) //finally, actually add the new cluster, as long as it isn't redundant
      newClusters_.push_back(newSubset);
  }
  log_debug("Leaving AddHitToCluster()");
  return(connectedDOMs.size()>0);
};


void HiveSplitter::AddSubEvent(HitSorting::HitSet& newSet) {
  log_debug("Entering AddSubEvent()");
  //find any existing subevents which overlap the new one, and merge them into it
  std::list<HitSorting::HitSet>::iterator set=partialSubEvents_.begin();
  while (set != partialSubEvents_.end()) {
    if (SetsIntersect(*set, newSet)) {
      newSet.insert(set->begin(),set->end());
      set = partialSubEvents_.erase(set);
    }
    else
      ++set;
  }
  
  partialSubEvents_.push_back(newSet);
  newSet.clear();

  //find the earliest time of all hits currently percolating through the clusters
  double earliestUpcomingTime = std::numeric_limits<double>::max();
  BOOST_FOREACH(const hivesplitter::detail::CausalCluster &cluster, clusters_)
    earliestUpcomingTime=std::min(earliestUpcomingTime,cluster.earliestTime());
  //any subevent whose last hit time is before the earliest time found above
  //cannot be merged again, and so is complete
  if (earliestUpcomingTime!=std::numeric_limits<double>::max()) {
    set=partialSubEvents_.begin();
    while (set != partialSubEvents_.end()) {
      if (set->rbegin()->GetTime() < earliestUpcomingTime) {
        //copy the contents of this subevent to a new subevent with ordering suitable
        //for retrieval of the actual hits and file it under the time of its first hit
        subEvents_.insert(*set);
        set = partialSubEvents_.erase(set);
      }
      else
        ++set;
    }
  }
}


void HiveSplitter::FinalizeSubEvents() {
  log_debug("Entering FinalizeSubEvents()");
  //dump all hits out of the clusters in progress
  BOOST_FOREACH(hivesplitter::detail::CausalCluster &cluster, clusters_)
    AdvanceClusterInTime(cluster, std::numeric_limits<double>::infinity());
  clusters_.clear(); //should already be empty
  //collect all leftover subevents
  BOOST_FOREACH(HitSorting::HitSet &set, partialSubEvents_)
    subEvents_.insert(set);
  partialSubEvents_.clear();
};


void HiveSplitter::BuildLookUpTables (const I3Geometry& geo) {
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

  //NOTE SPEED TWEAKS for filling martix with symmetrical indexes:
  // This martix has a clear symmetry and block-structure, which makes it possible to fast fill certain regions.
  // This does also mean that some places will be filled twice, which is still faster than many conditional-statements
  // - Fill main diagonal explicitly
  // - Loop only lower half and switch indexes to fill upper half
  // - do not double set entries, if they are set by another routine
  // - if strings are not indexed in HiveLib for any given central string, all its entries will be NAN on this string
  // - for the WhichRing() from the hives use some semi-hardcodings and differentiations into subhives for fastest access and iteration depth


  //precash the I3Position for each DOM from OMGeo and its topology; do this once
  std::vector<I3Position> domPos_cache(MAX_SIMPLEINDEX+1, I3Position(NAN,NAN,NAN));
  std::vector<int> domCom_cache(MAX_SIMPLEINDEX+1, -1); // a cache for comparing DOM topologies (use a bitwise comparison)
  
  BOOST_FOREACH(const I3OMGeoMap::value_type &omkey_pos, geo.omgeo) {
    //for (unsigned int index_A=0;index_A<(MAX_STRINGS*MAX_OMS);++index_A) {
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
  
  log_debug("Init everything NAN/false");
  //pave the way, set everything to NAN //NOTE this is paranoid and might be already done by the blank constructor
//   for (unsigned int index_A=0;index_A<=MAX_SIMPLEINDEX;++index_A) {
//     for (unsigned int index_B=0;index_B<=MAX_SIMPLEINDEX;++index_B) {
//       //distanceMap_.Set(index_A, index_B, NAN); //NOTE done by the constructor
//       //vicinityMap_.Set(index_A, index_B, false); //NOTE done by the constructor
//     }
//   }

  log_debug("Fill Diagonal");
  if (params_.selfconnect) {
    // fill diagonal for all DOMs:
    // fill them with distance 0. first and than correct the icetop omkeys later
    for (unsigned int index_A=0;index_A<=MAX_SIMPLEINDEX;++index_A) {
      log_trace_stream("Looking for diagonal " << SimpleIndex2OMKey(index_A) << std::endl);
      if (domCom_cache[index_A]<=0) { 
        log_trace("IceTop or untreated DOM: never connect");
        //distanceMap_.Set(index_A, index_A, NAN);
        //vicinityMap_.Set(index_A, index_A, false);
        continue;
      }
      else {
        log_trace("Diagonal element: distance == 0.");
        distanceMap_.Set(index_A, index_A, 0.);
        vicinityMap_.Set(index_A, index_A, true);
      }
    }
  }

  log_debug("Fill remaining elements");
  //loop over all remaining indices and thereby fill everything
  //===== LOOP A =====
  for (unsigned int index_A=0;index_A<=MAX_SIMPLEINDEX;++index_A) {
    log_trace_stream("====Fill next row === : " <<  index_A << ":"  << SimpleIndex2OMKey(index_A) << std::endl);
    if (index_A%MAX_OMS==0) //indicate the progress
      log_debug_stream("String"<<(int)(index_A/MAX_OMS)+1<<" "<<std::flush);

    const int& domTopo_A = domCom_cache[index_A];

    //if index_A is icetop, set all entries to NAN immediately 
    if (domTopo_A<=0) {
      log_trace("IceTop or not included: never connect");
      //NOTE jump over the setting process for speed
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
        //distanceMap_.Set(index_A, index_B, NAN);
        //vicinityMap_(index_A, index_B, NAN);
        continue;
      }

      const OMKey omkey_B(SimpleIndex2OMKey(index_B));

      const I3Position& pos_B = domPos_cache[index_B];
      
      //this is a smart way to check combinations of 2 values, which can have 3 states
      log_trace_stream("domTopo_A " << domTopo_A <<" domTopo_B " << domTopo_B);
      //choose the right values when comparing different topologies
      Hive* hivePtr = 0; //the hive to perform the lookup on
      unsigned int rel_scale_factor =1; //relative scale factor in compared DOMs
      RingLimits* limitPairsPtr_ = 0;
      RingLimits* vicinityPairsPtr_ = 0;
      switch (domTopo_A+domTopo_B) {
        case 2: //(IceCube+IceCube):
          hivePtr = &(hivegeo_.hives_[0]);
          rel_scale_factor=1;
          limitPairsPtr_ = &RingLimitPairsMap_[0];
          vicinityPairsPtr_ = &RingVicinityPairsMap_[0];
          break;
        case 3: //(IceCube+DeepCore):
          hivePtr = &(hivegeo_.hives_[1]);
          rel_scale_factor=2;
          limitPairsPtr_ = &RingLimitPairsMap_[0];
          vicinityPairsPtr_ = &RingVicinityPairsMap_[0];
          break;
        case 5: //(IceCube+Pingu):
          hivePtr = &(hivegeo_.hives_[2]);
          rel_scale_factor=3;
          limitPairsPtr_ = &RingLimitPairsMap_[0];
          vicinityPairsPtr_ = &RingVicinityPairsMap_[0];
          break;
        case 4: //(DeepCore+DeepCore):
          hivePtr = &(hivegeo_.hives_[1]);
          rel_scale_factor=1;
          limitPairsPtr_ = &RingLimitPairsMap_[1];
          vicinityPairsPtr_ = &RingVicinityPairsMap_[1];
          break;
        case 6: //(DeepCore+Pingu):
          hivePtr = &(hivegeo_.hives_[2]);
          rel_scale_factor=2;
          limitPairsPtr_ = &RingLimitPairsMap_[1];
          vicinityPairsPtr_ = &RingVicinityPairsMap_[1];
          break;
        case 8: //(Pingu+Pingu):
          hivePtr = &(hivegeo_.hives_[2]);
          rel_scale_factor=1;
          limitPairsPtr_ = &RingLimitPairsMap_[2];
          vicinityPairsPtr_ = &RingVicinityPairsMap_[2];
          break;
        default:
          log_fatal_stream("non registered topology prescription "<< domTopo_A+domTopo_B <<": "
            <<"domTopoA("<<omkey_A<<"): " << domTopo_A <<" domTopo_B("<<omkey_B<<"): "<< domTopo_B);
      }

      //use that string as center, of which the hive is actually build around
      const unsigned int center_string = domTopo_A >= domTopo_B ? omkey_A.GetString() : omkey_B.GetString();
      const unsigned int lookup_string = domTopo_A >= domTopo_B ? omkey_B.GetString() : omkey_A.GetString();

      const int ring = WhichRing(hivePtr->combs_, center_string, lookup_string);
      //log_error_stream(DumpCenter(hivePtr->combs_, center_string));
      //log_fatal_stream(DumpHive(*hivePtr));
      if (ring == -1) {//not in the ring indexing range
        log_trace("Not included in ring index range");
        //NOTE (speed tweak) make it snappy and set all entries on that string in index_B to NAN, because we will never connect
        const unsigned int string_end = StringOmNbr2SimpleIndex(SimpleIndex2StringNbr(index_B), MAX_OMS);
//        for (; index_B<=string_end-4; ++index_B){ //there are 4 icetopDOMs remember
//          distanceMap_.Set(index_A, index_B, NAN);
//          distanceMap_.Set(index_B, index_A, NAN);
//          vincintyMap_.Set(index_A, index_B, NAN);
//          vicinityMap_.Set(index_B, index_A, NAN);
//        }
        index_B=string_end; //account for the overstepping of the for-loop
        continue;
      }

      //IN RING SYSTEM evaluation
      const int rescaled_ring = int((ring+rel_scale_factor-1)/rel_scale_factor);
      
      double minusLimit;
      double plusLimit;
      if (rescaled_ring > int(limitPairsPtr_->NRings()) ) {
        log_trace_stream("Ring "<<ring<<", rescaled "<<rescaled_ring<<" too far away; max Ringlimits "<<(limitPairsPtr_->NRings()));
        minusLimit = NAN;
        plusLimit = NAN;
      }
      else{
        minusLimit = limitPairsPtr_->GetLimitsOnRing(rescaled_ring).minus_;
        plusLimit = limitPairsPtr_->GetLimitsOnRing(rescaled_ring).plus_;
      }
        
      double minusVicinity;
      double plusVicinity;
      if (rescaled_ring > int(vicinityPairsPtr_->NRings()) ) {
        log_trace_stream("Ring "<<ring<<", rescaled "<<rescaled_ring<<" too far away; max Vicinitylimits "<<(limitPairsPtr_->NRings()));
        minusVicinity = NAN;
        plusVicinity = NAN;
      }
      else{
        minusVicinity = vicinityPairsPtr_->GetLimitsOnRing(rescaled_ring).minus_;
        plusVicinity = vicinityPairsPtr_->GetLimitsOnRing(rescaled_ring).plus_;
      }
      
      const double z_B(pos_B.GetZ());

      log_trace_stream("ring "<< ring << " z_A "<< z_A << " z_B " << z_B << " ring- " <<minusLimit<< " ring+ " <<plusLimit);
      const double zdist_AB = z_B-z_A;
      const double xdist_AB = pos_B.GetX() - pos_A.GetX();
      const double ydist_AB = pos_B.GetY() - pos_A.GetY();
      const double dist = sqrt(xdist_AB*xdist_AB + ydist_AB*ydist_AB + zdist_AB*zdist_AB);

      //compare A to B
      if (std::isnan(minusLimit) && std::isnan(plusLimit)) {
        log_trace("Not configured rings");
      }
      else if (minusLimit<= zdist_AB && zdist_AB <= plusLimit) {
        distanceMap_.Set(index_A, index_B, dist);
        log_trace("DOMs are connected; Distance %f", dist);
      }
      else {
        //distanceMap_.Set(index_A, index_B, NAN); //NOTE already NAN by construction
        log_trace("Not on ring distance");
      }

      //compare for vincintity also
      if (std::isnan(minusVicinity) && std::isnan(plusVicinity)) {
        log_trace("Not configured vicinty");
      }
      else if (minusVicinity<= zdist_AB && zdist_AB <= plusVicinity) {
        vicinityMap_.Set(index_A, index_B, true);
        log_trace("DOMs are connected; Vicintiy True");
      }
      else {
        //vincintityMap_(index_A, index_B, false); //NOTE already NAN by construction
        log_trace("Not on ring vicinty");
      }

      //compare B to A
      const double zdist_BA = -zdist_AB; //notice: switched indices

      if (std::isnan(minusLimit) && std::isnan(plusLimit)) {
        log_trace("Not configured rings");
      }
      else if (minusLimit<= zdist_BA && zdist_BA <= plusLimit) {
        distanceMap_.Set(index_B, index_A, dist);
        log_trace("DOMs are connected; Distance %f", dist);
      }
      else {
        //distanceMap_.Set(index_B, index_A, NAN); //NOTE already NAN by construction
        log_trace("Not on ring distance");
      }

      //compare for vincintity also
      if (std::isnan(minusVicinity) && std::isnan(plusVicinity)) {
        log_trace("Not configured vicinty");
      }
      else if (minusVicinity<= zdist_BA && zdist_BA <= plusVicinity) {
        vicinityMap_.Set(index_B, index_A, true);
        log_trace("DOMs are connected; Vicintiy True");
      }
      else {
        //vicinityMap_.Set(index_B, index_A, false); //NOTE already NAN by construction
        log_trace("Not on ring vicinty");
      }
    }
  }
  log_info("DistanceMap built");
  log_debug("Leaving BuildDistanceMap()");
};
