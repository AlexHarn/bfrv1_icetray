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

#include <HiveSplitter/HiveSplitter.h>
#include <HiveSplitter/Hive-lib.h>

#include <math.h>

#include "boost/foreach.hpp"

using namespace OMKeyHash;
using namespace HitSorting;

HiveSplitter_ParameterSet::HiveSplitter_ParameterSet():
  multiplicity_(4),
  timeWindow_(2000.*I3Units::ns),
  timeConeMinus_(1000.*I3Units::ns),
  timeConePlus_(1000.*I3Units::ns),
  domSpacingsOpt_(false),
  modeOpt_(1)
{
  double ic[] = {300.,300., 272.7,272.7, 165.8,165.8};
  SingleDenseRingLimits_.assign(ic, ic+6);
  double dc[] = {150.,150., 131.5,131.5, 40.8,40.8};
  DoubleDenseRingLimits_.assign(dc, dc+6);
  double pingu[] = {150.,150., 144.1,144.1, 124.7,124.7, 82.8,82.8};
  TrippleDenseRingLimits_.assign(pingu, pingu+8);
}

//===============class HiveSplitter=================================
HiveSplitter::CausalCluster::CausalCluster():
  endDom(std::numeric_limits<unsigned int>::max()),
  multiplicityMet(false)
  {}

void HiveSplitter::CausalCluster::insert(HitSorting::Hit h, unsigned int multiplicity) {
  doms[h.domIndex]++;
  hits.push_back(h);
  //if the total number of DOMs meets the multiplicity threshold, make note,
  //and also record that this is the last known hit within the cluster contributing
  if (doms.size()>=multiplicity) {
    endDom=h.domIndex;
    endPulse=h.pulseIndex;
    multiplicityMet=true;
  }
}

double HiveSplitter::CausalCluster::earliestTime() const{
  if (!complete.empty() && !hits.empty())
    return(std::min(complete.begin()->time,hits.begin()->time));
  if (!complete.empty())
    return(complete.begin()->time);
  if (!hits.empty())
    return(hits.begin()->time);
  return(std::numeric_limits<double>::infinity());
}

bool HiveSplitter::CausalCluster::advanceTime(const double time, HiveSplitter& splitter_inst) {
  while (!hits.empty()) {
    Hit h=*hits.begin();
    if (time > h.time +splitter_inst.timeWindow_) {
      //decrement the number of hits on the DOM where h occurred
      if ((--doms[h.domIndex])<=0)
        doms.erase(doms.find(h.domIndex));
      //if the mutiplicity threshold was met include h in the finished cluster
      if (multiplicityMet) {
        //insert the hit, and hint that it should go at the end of the complete set
        complete.insert((complete.empty()?complete.begin():--complete.end()),h);
        //if h was the last in the multiplicity window,
        //shove the finished cluster back to the trigger for merging
        if (h.domIndex==endDom && h.pulseIndex==endPulse) {
          splitter_inst.AddSubEvent(complete);
          multiplicityMet=false;
          //no need to bother resetting endDom and endPulse;
          //they can't give a false positive because we'll never see h again
        }
      }
      hits.pop_front();
    }
    else
      break;
  }
  return(hits.empty());
}

void HiveSplitter::CausalCluster::takeComplete (const CausalCluster& c){
  complete.insert(c.complete.begin(), c.complete.end());
}

bool HiveSplitter::isSubset(const CausalCluster& c1, const CausalCluster& c2) {
  if (c2.hits.size()<c1.hits.size())
    return(false);
  std::list<Hit>::const_iterator it1=c1.hits.begin(), end1=c1.hits.end(),
    it2=c2.hits.begin(), end2=c2.hits.end();
  for (; it1!=end1 && it2!=end2; it1++, it2++) {
    //if the two current items don't match scan though the (potential) superset looking for a match
    while (it2!=end2 && *it2<*it1)
      it2++;
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

bool HiveSplitter::causallyConnected(const Hit& h1, const Hit& h2) {
  log_debug("Evaluating causallyConnected()");
  if (h1.time > h2.time) {
    return causallyConnected(h2, h1); //recursive call to enforce timeorder at this point
  }
  const double dist = GetDistance(h1.domIndex, h2.domIndex);

  log_debug_stream("Evaluating OMKey_A " << SimpleIndex2OMKey(h1.domIndex) <<
  " against OMKey_B " << SimpleIndex2OMKey(h2.domIndex) <<
  " with distance " << GetDistance(SimpleIndex2OMKey(h1.domIndex), SimpleIndex2OMKey(h2.domIndex)));
  if (std::isnan(dist)) {
    log_debug("DOMs not connected to each other because of distance");
    return(false);
  }
  //Finally, two hits are connected if the difference in their times is within the [timeConePlus_,timeConeMinus_] value of
  //the time it would take light to propagate from one hit's location to the other hit's location.
  const double dt=std::abs(h1.time - h2.time);
  
  switch (modeOpt_) { //the here the causal Argument is applied. Read it up in the documentation
    case Lorentz: {
      const double time_residual = (dt - dist/I3Constants::c);
      const double decision = (-timeConeMinus_<=time_residual && time_residual<=timeConePlus_);
      log_debug_stream("Lorentz Norm: "<<(decision ? "NOT" : "")<< " causally Connected");
      return decision;
    }
    case Euclidean: {
      const bool decision = (dist/I3Constants::c + dt <= timeConePlus_);
      log_debug_stream("Euclidean Norm: "<<(decision ? "NOT" : "")<< " causally Connected");
      return decision;
    }
    case Static: {
      const bool decision = (dt <= timeConePlus_);
      log_debug_stream("Static Norm: "<<(decision ? "NOT" : "")<< " causally Connected");
      return decision;
    }
    case Advanced: {
      const double timeCVMinus_ = 200*I3Units::ns;
      const double timeCVPlus_ = 200*I3Units::ns;
      const double timeCNMinus_ = 300*I3Units::ns;
      const double timeCNPlus_ = 0*I3Units::ns;

      const double timeStaticPlus_ = 100*I3Units::ns;

      const double dist_max_=200*I3Units::m;

      const double time_residual_cv = (dt - dist/I3Constants::c);
      const bool particle_causal = (-timeCVMinus_<=time_residual_cv && time_residual_cv<=timeCVPlus_);
      const double time_residual_cn = (dt - dist/I3Constants::c*I3Constants::n_ice_group);
      const bool photon_causal = (-timeCNMinus_<=time_residual_cn && time_residual_cn<=timeCNPlus_);

      const bool displacement_causal = (dist<=dist_max_ && dt<timeStaticPlus_);

      const bool decision = (particle_causal || photon_causal || displacement_causal );
      log_debug_stream("Advanced Norm: "<<(decision ? "NOT" : "")<< " causally Connected");
      return decision;
    }
    default: {
      return false;
    }
  }
}
//==================

HiveSplitter::HiveSplitter(const HiveSplitter_ParameterSet& params):
  multiplicity_(params.multiplicity_),
  timeWindow_(params.timeWindow_),
  timeConeMinus_(params.timeConePlus_),
  timeConePlus_(params.timeConeMinus_),
  domSpacingsOpt_(params.domSpacingsOpt_),
  SingleDenseRingLimits_(params.SingleDenseRingLimits_),
  DoubleDenseRingLimits_(params.DoubleDenseRingLimits_),
  TrippleDenseRingLimits_(params.TrippleDenseRingLimits_),
  modeOpt_(params.modeOpt_)
{
  CheckParams_n_Setup();
};

void HiveSplitter::Configure(const HiveSplitter_ParameterSet& params) {
  multiplicity_ = params.multiplicity_;
  timeWindow_ = params.timeWindow_;
  timeConePlus_ = params.timeConePlus_;
  timeConeMinus_ = params.timeConeMinus_;
  domSpacingsOpt_ = params.domSpacingsOpt_;
  SingleDenseRingLimits_ = params.SingleDenseRingLimits_;
  DoubleDenseRingLimits_ = params.DoubleDenseRingLimits_;
  TrippleDenseRingLimits_ = params.TrippleDenseRingLimits_;
  modeOpt_ = params.modeOpt_;

  CheckParams_n_Setup();
};

void HiveSplitter::CheckParams_n_Setup() {
  if (domSpacingsOpt_)
    log_info("You choose to specify RingLimits by DOM-spacings");

  if (domSpacingsOpt_) {
    for (uint i=0; i<SingleDenseRingLimits_.size();i++) {
      if (SingleDenseRingLimits_[i]-(int)SingleDenseRingLimits_[i]>0.) {
        log_fatal("You specified DOMspacings as a decimal number, while in fact only natural numbers are comprehensive");
      }
    }
  }

  if (SingleDenseRingLimits_.size()%2 != 0 || DoubleDenseRingLimits_.size()%2  != 0 || TrippleDenseRingLimits_.size()%2  != 0)
    log_fatal("All RingLimits must be configured as a List of pairs of positive numbers");

  if (SingleDenseRingLimits_.size() > (3*MAX_RINGS+1) || DoubleDenseRingLimits_.size() > 2*(3*MAX_RINGS+1) || TrippleDenseRingLimits_.size() > 4*(3*MAX_RINGS+1))
    log_fatal("Sorry, currently only 2 Rings are allowed; if you want more, implement more in Hive-lib");

  for (uint i=0; i+1<SingleDenseRingLimits_.size();i+=2) {
    if (SingleDenseRingLimits_[i] <0. || SingleDenseRingLimits_[i+1]<0.)
      log_fatal("All RingLimits must be configured as a List of pairs of positive numbers");
    SingleDenseRingLimitPairs_.push_back(std::make_pair(SingleDenseRingLimits_[i],SingleDenseRingLimits_[i+1]));
    //log_debug_stream("SingleDensePair:"<<SingleDenseRingLimits_[i]<<","<<SingleDenseRingLimits_[i+1]);
  }
  for (uint i=0; i+1<DoubleDenseRingLimits_.size();i+=2) {
    if (DoubleDenseRingLimits_[i] <0. || DoubleDenseRingLimits_[i+1]<0.)
      log_fatal("All RingLimits must be configured as a List of pairs of positive numbers");
    DoubleDenseRingLimitPairs_.push_back(std::make_pair(DoubleDenseRingLimits_[i],DoubleDenseRingLimits_[i+1]));
    //log_debug_stream("DoubleDensePair:"<<DoubleDenseRingLimits_[i]<<","<<DoubleDenseRingLimits_[i+1]);
  }
  for (uint i=0; i+1<TrippleDenseRingLimits_.size();i+=2) {
    if (TrippleDenseRingLimits_[i] <0. || TrippleDenseRingLimits_[i+1]<0.)
      log_fatal("All RingLimits must be configured as a List of pairs of positive numbers");
    TrippleDenseRingLimitPairs_.push_back(std::make_pair(TrippleDenseRingLimits_[i],TrippleDenseRingLimits_[i+1]));
    //log_debug_stream("TrippleDensePair:"<<TrippleDenseRingLimits_[i]<<","<<TrippleDenseRingLimits_[i+1]);
  }
  
  log_debug_stream("SingleDenseRingLimitPairs_:"<<SingleDenseRingLimitPairs_.size());
  log_debug_stream("DoubleDenseRingLimitPairs_:"<<DoubleDenseRingLimitPairs_.size());
  log_debug_stream("TrippleDenseRingLimitPairs_:"<<TrippleDenseRingLimitPairs_.size());

  if (multiplicity_<=0)
    log_fatal("Multiplicity should be greater than zero");
  if (timeWindow_<=0.0)
    log_fatal("TimeWindow should be greater than zero");
  if (timeConePlus_<=0.0)
    log_fatal("TimeConePlus should be greater than zero");
  if (timeConeMinus_<=0.0)
    log_fatal("TimeConeMinus should be greater than zero");

  if (modeOpt_<1 || 3<modeOpt_)
    log_fatal("Unrecognized Mode to be used");

  if (modeOpt_==2 || modeOpt_==3)
    log_info("Using Mode %d :Your setting of the parameter TimeConeMinus will be ignored", modeOpt_);

  log_debug("Leaving Init()");
}

std::vector<SubEventStartStop> HiveSplitter::HiveSplitting (const I3RecoPulseSeriesMap& pulses) {
  log_debug("Entering HiveSplitting()");
  std::vector<SubEventStartStop> subpulses;

  //do setup work: put all hits in time order and make the fast geometry lookup table
  HitSorting::RetrievalOrderedHitSet hits_retrieval = ExtractHits(pulses);
  HitSorting::TimeOrderedHitSet hits_time = RetrievalOrderedToTimeOrdered(hits_retrieval);

  //feed the hits into the clustering machinery
  for (HitSorting::TimeOrderedHitSet::const_iterator hit=hits_time.begin(); hit!=hits_time.end(); hit++){
    AddHit(*hit);
  }
  FinalizeSubEvents();

  for (TimeOrderedSubEvents::const_iterator subEvent_iter=subEvents_.begin(); subEvent_iter!=subEvents_.end(); ++subEvent_iter){
    SubEventStartStop sESS(HitSorting::RevertHits (HitSorting::TimeOrderedToRetrievalOrdered(*subEvent_iter), pulses),
      subEvent_iter->begin()->time,
      subEvent_iter->rbegin()->time
    );
    subpulses.push_back(sESS);
  }
  //clean up
  subEvents_.clear();

  log_debug("Leaving HiveSplitting()");
  return subpulses;
};

void HiveSplitter::AddHit(const Hit h) {
        newClusters_.clear();
        bool addedToCluster=false; //keep track of whether h has been added to any cluster
        for (std::list<CausalCluster>::iterator cluster=clusters_.begin(), next=cluster, end=clusters_.end();
          cluster!=end; cluster=next) {
                next++;
                if (cluster->advanceTime(h.time,*this))
                        clusters_.erase(cluster);
                else
                        addedToCluster |= AddHitToCluster(*cluster,h);
        }

        //Move all newly generated clusters into the main cluster list,
        //eliminating clusters which are subsets of other clusters
        for (std::list<CausalCluster>::iterator newCluster=newClusters_.begin(), nend=newClusters_.end();
    newCluster!=nend; newCluster++) {
    bool add=true;
    for (std::list<CausalCluster>::iterator cluster=clusters_.begin(), next=cluster, end=clusters_.end();
      cluster!=end; cluster=next) {
      next++;
      //check whether the new cluster is a subset of the old cluster
      //if the old cluster does not contain h, it cannot be a superset of the new cluster which does,
      //and if the old cluster contains h, it will be the last hit in that cluster
      if (cluster->hits.back()==h){
        if (isSubset(*newCluster,*cluster)) {
          add=false;
          break;
        }
      }
      //otherwise, the new cluster may still be a superset of the old cluster
      else if (isSubset(*cluster,*newCluster)) {
        //if replacing, make sure not to lose any hits already shifted to the old cluster's 'complete' list
        newCluster->takeComplete(*cluster);
        clusters_.erase(cluster);
      }
    }
    if (add)
      clusters_.push_back(*newCluster);
  }
  newClusters_.clear();

  //if h was not added to any cluster, put it in a cluster by itself
  if (!addedToCluster) {
    clusters_.push_back(CausalCluster());
    clusters_.back().insert(h,multiplicity_);
  }
}

bool HiveSplitter::AddHitToCluster(CausalCluster& c, const Hit h) {
  //First, find out how many of the doms with hits in this cluster are causally connected to h
  //if there are more than multiplicity_, h will be added to this cluster
  std::set<unsigned int> connectedDOMs;
  bool allConnected=true;
  for (std::list<Hit>::reverse_iterator it=c.hits.rbegin(), end=c.hits.rend(); it!=end; it++) {
    if (causallyConnected(*it,h))
      connectedDOMs.insert(it->domIndex);
    else
      allConnected=false;
    if (connectedDOMs.size()+1>=multiplicity_) //NOTE plus one for h itself!
      break;
  }
  if (connectedDOMs.size()+1 >=multiplicity_ || allConnected)
    c.insert(h, multiplicity_); //h is conected to enough hits to belong in this cluster
  else if (connectedDOMs.size()>0) {
    //if h is connected to some of the hits in the cluster, but not enough to be a member,
    //we need to build a new cluster containing only those hits with which it is connected
    CausalCluster newSubset;
    for (std::list<Hit>::iterator it=c.hits.begin(), end=c.hits.end(); it!=end; it++) {
      if (causallyConnected(*it, h))
        newSubset.insert(*it, multiplicity_);
    }
    newSubset.insert(h, multiplicity_);
    //we add the new cluster to our list of newly created clusters,
    //but if it is a subset of a cluster we already have we discard it as redundant,
    //and likewise if any of the clusters we have is a subset we discard that one
    bool keep=true;
    for (std::list<CausalCluster>::iterator it=newClusters_.begin(), next=it, end=newClusters_.end();
      it!=end; it=next) {
      next++; //if we end up erasing it it will be invalidated, so pre-emptively find the next item now
      if (isSubset(*it,newSubset))
        newClusters_.erase(it); //remove a redundant, existing cluster
      else if (isSubset(newSubset,*it)) {
        keep=false; //this cluster is redundant, so abort adding it
        break;
      }
    }
    if (keep) //finally, actually add the new cluster, as long as it isn't redundant
      newClusters_.push_back(newSubset);
  }
  return(connectedDOMs.size()>0);
};

void HiveSplitter::AddSubEvent(TimeOrderedHitSet& newSet) {
  //find any existing subevents which overlap the new one, and merge them into it
  for (std::list<TimeOrderedHitSet>::iterator set=partialSubEvents_.begin(), next=set, end=partialSubEvents_.end();
    set!=end; set=next) {
    next++;
    if (setsIntersect(set->begin(),set->end(),newSet.begin(),newSet.end())) {
      newSet.insert(set->begin(),set->end());
      partialSubEvents_.erase(set);
    }
  }
  partialSubEvents_.push_back(newSet);
  newSet.clear();

  //find the earliest time of all hits currently percolating through the clusters
  double earliestUpcomingTime = std::numeric_limits<double>::max();
  for (std::list<CausalCluster>::iterator cluster=clusters_.begin(), end=clusters_.end(); cluster!=end; cluster++)
    earliestUpcomingTime=std::min(earliestUpcomingTime,cluster->earliestTime());
  //any subevent whose last hit time is before the earliest time found above
  //cannot be merged again, and so is complete
  if (earliestUpcomingTime!=std::numeric_limits<double>::max()) {
    for (std::list<TimeOrderedHitSet>::iterator set=partialSubEvents_.begin(), next=set, end=partialSubEvents_.end();
      set!=end; set=next) {
      next++;
      if (set->rbegin()->time < earliestUpcomingTime) {
        //copy the contents of this subevent to a new subevent with ordering suitable
        //for retrieval of the actual hits and file it under the time of its first hit
        subEvents_.insert(*set);
        partialSubEvents_.erase(set);
      }
    }
  }
}

void HiveSplitter::FinalizeSubEvents() {
  //dump all hits out of the clusters
  for (std::list<CausalCluster>::iterator cluster=clusters_.begin(), end=clusters_.end();
    cluster!=end; cluster++)
    cluster->advanceTime(std::numeric_limits<double>::infinity(),*this);
  clusters_.clear();
  //collect all leftover subevents
  for (std::list<TimeOrderedHitSet>::const_iterator set=partialSubEvents_.begin(), end=partialSubEvents_.end();
    set!=end; set++) {
    subEvents_.insert(*set);
}
  partialSubEvents_.clear();
};

void HiveSplitter::BuildDistanceMap(I3GeometryConstPtr geo) {
  log_debug("Entering BuildDistanceMap()");
  using namespace Topology;
  using namespace honey;

  log_info("Building DOM-Hive Look-Up tables");

  Hive SingleDenseHive = ReadHiveFromFile(boost::lexical_cast<std::string>(getenv("I3_SRC")) +"/HiveSplitter/resources/SingleDenseHive.dat");
  Hive DoubleDenseHive = ReadHiveFromFile(boost::lexical_cast<std::string>(getenv("I3_SRC")) +"/HiveSplitter/resources/DoubleDenseHive.dat");
  Hive TrippleDenseHive = ReadHiveFromFile(boost::lexical_cast<std::string>(getenv("I3_SRC")) +"/HiveSplitter/resources/TrippleDenseHive.dat");

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
  // This matrices has a clear symmetry and blockwise structure, which makes it possible to fast fill certain regions.
  // This does also mean that some places will be filled twice, which is but faster than many conditional-statements
  // - Fill diagonals explicitly
  // - Loop only lower half and switch indexes to fill upper half
  // - do not double set entries, if they are set by another routine
  // - if strings are not indexed in HiveLib for any given central string, all its entries will be NAN on this string
  // - for the WhichRing() from the hives use some semi-hardcodings and differentiations into subhives for fastest access and iteration depth


  //precash the I3Position for each DOM from OMGeo and its topology; do this once
  std::vector<I3Position> domPos_cache(MAX_STRINGS*MAX_OMS, I3Position(NAN,NAN,NAN));
  std::vector<uint> domTopo_cache(MAX_STRINGS*MAX_OMS, 0);
  BOOST_FOREACH(const I3OMGeoMap::value_type &omkey_pos, geo->omgeo) {
    //for (uint matrice_x=0;matrice_x<(MAX_STRINGS*MAX_OMS);matrice_x++) {
    const OMKey& omkey = omkey_pos.first;
    if((omkey.GetString()<1) || (omkey.GetString()>86) || (omkey.GetOM()<1) || (omkey.GetOM()>64)) {
        log_trace_stream("Ignoring " << omkey << "which is not part of InIce or IceTop");
        continue;
    } 
    const I3Position& pos = omkey_pos.second.position;
    const uint matrice_x = OMKey2SimpleIndex(omkey);

    const uint domTopo = (1 + IsDC_VetoCapFidDOM(omkey) + 2*IsPinguString(omkey))*(!IsIceTopDOM(omkey));
    domPos_cache[matrice_x]=pos;
    domTopo_cache[matrice_x]=domTopo;
  }

  //pave the way, set everything to NAN
  for (uint matrice_x=0;matrice_x<(MAX_STRINGS*MAX_OMS);matrice_x++) {
    for (uint matrice_y=0;matrice_y<(MAX_STRINGS*MAX_OMS);matrice_y++) {
      DistanceMap_[matrice_x][matrice_y]=NAN;
    }
  }

  // fill diagonals for all doms:
  // fill them with distance 0. first and than correct the icetop omkeys later
  for (uint matrice_x=0;matrice_x<(MAX_STRINGS*MAX_OMS);matrice_x++) {
    log_trace_stream("Looking for diagonal " << SimpleIndex2OMKey(matrice_x) << std::endl);
    if (IsIceTopDOM(SimpleIndex2OMKey(matrice_x))) {
      log_trace("IceTop: never connect");
      continue;
    }
    log_trace("Diagonal element: distance == 0.");
    DistanceMap_[matrice_x][matrice_x]=0.;
  }
  log_debug_stream("(diag)"<<std::flush);


  //loop over all remaining indices and thereby fill everything
  //===== LOOP A =====
  for (uint matrice_x=0;matrice_x<(MAX_STRINGS*MAX_OMS);matrice_x++) {
    log_trace_stream("====Fill next row === : " <<  matrice_x << ":"  << SimpleIndex2OMKey(matrice_x) << std::endl);
    if (matrice_x%MAX_OMS==0) //indicate the progress
      log_debug_stream("S"<<(int)(matrice_x/MAX_OMS)+1<<" "<<std::flush);

    const uint& domTopo_A = domTopo_cache[matrice_x];

    //if matrice_x is icetop, set all entries to NAN immediately
    if (domTopo_A==0) {
      log_trace("IceTop or not included: never connect");
      continue;
    }

    const OMKey omkey_A(SimpleIndex2OMKey(matrice_x));
    const I3Position& pos_A = domPos_cache[matrice_x];
    const double z_A(pos_A.GetZ());

    //===== LOOP B =====
    for (uint matrice_y=matrice_x+1;matrice_y<(MAX_STRINGS*MAX_OMS);matrice_y++) {
      log_trace_stream("Looking for " << SimpleIndex2OMKey(matrice_x) << " and " << SimpleIndex2OMKey(matrice_y) << std::endl);

      const uint& domTopo_B = domTopo_cache[matrice_y];

      if (domTopo_B==0) {// any is IceTop: never connect
        log_trace("IceTop: never connect");
        //NOTE (speed tweak) do nothing here, because all entries will be set by the IceTop treatment of matrix_x;
        //DistanceMap_[matrice_x][matrice_y] = NAN;
        continue;
      }

      const OMKey omkey_B(SimpleIndex2OMKey(matrice_y));
      
      //Figure out which ring-limits to use for the comparision of these two DOMs
      // remember that we have to take into acoount that we have to conect in an d out of denser zones:
      // for this use the ring-limits of the not so dense one and take the ring which is the smallest:
      // so if a a Pingu-DOM sits on ring 1.75 relative to a IceCube-DOM, treat it as if it sits on ring 1
      
      //this is a smart but ugly way to check combinations of 2 values, which can have 3 states
      log_debug_stream("domTopo_A " << domTopo_A <<" domTopo_B " << domTopo_B << std::endl;);
      //choose the right values when comparing different topologies
      Hive* hivePtr = 0; //the hive to perform th lookup on
      uint rel_scale_factor =1; //relative scale factor in compared DOMs; that many rings outward on the smaller scaled hive, is the next ring on the bigger scaled hive
      RingLimitPairs* limitPairsPtr_ = 0;
      switch (domTopo_A+domTopo_B) {
        case 2: //(IceCube+IceCube):
          hivePtr = &SingleDenseHive;
          rel_scale_factor=1;
          limitPairsPtr_ = &SingleDenseRingLimitPairs_;
          break;
        case 3: //(IceCube+DeepCore):
          hivePtr = &DoubleDenseHive;
          rel_scale_factor=2;
          limitPairsPtr_ = &SingleDenseRingLimitPairs_;
          break;
        case 5: //(IceCube+Pingu):
          hivePtr = &TrippleDenseHive;
          rel_scale_factor=3;
          limitPairsPtr_ = &SingleDenseRingLimitPairs_;
          break;
        case 4: //(DeepCore+DeepCore):
          hivePtr = &DoubleDenseHive;
          rel_scale_factor=1;
          limitPairsPtr_ = &DoubleDenseRingLimitPairs_;
          break;
        case 6: //(DeepCore+Pingu):
          hivePtr = &TrippleDenseHive;
          rel_scale_factor=2;
          limitPairsPtr_ = &DoubleDenseRingLimitPairs_;
          break;
        case 8: //(Pingu+Pingu):
          hivePtr = &TrippleDenseHive;
          rel_scale_factor=1;
          limitPairsPtr_ = &TrippleDenseRingLimitPairs_;
          break;
        default:
          log_fatal("non registered topology prescription");
      }

      //use that string as center, of which the hive is actually build around
      const uint center_string = ((domTopo_A >= domTopo_B) ? omkey_A.GetString() : omkey_B.GetString() );
      const uint lookup_string = ((domTopo_A >= domTopo_B) ? omkey_B.GetString() : omkey_A.GetString() );

      const int ring = WhichRing(hivePtr->combs_, center_string, lookup_string, MAX_RINGS*hivePtr->scaleFactor_); //NOTE DANGER SpeedTweak: with a semihardcoded parameter

      if (ring == -1) {//not in the ring indexing range
        log_trace("Not included in ring index range");
        //NOTE (speed tweak) make it snappy and set all entries on that string in matrice_y to NAN, because we will never connect
        //DANGER this is only possible, if the indexed ring range (in m) of HiveLib is equal for all Hive configurations;
        // DeepCore-Hive must index twice as many rings as IceCube-Hive, Pingu-Hive in turn must index twice as many rings as DeepCore-Hive
        // in any other case only this exact entry(x,y) can be set NAN
        const uint string_end = String_OM_Nbr2SimpleIndex(SimpleIndex2StringNbr(matrice_y),MAX_OMS);
//        for (; matrice_y<=string_end-4; matrice_y++){ //there are 4 icetopDOMs remember
//          DistanceMap_[matrice_x][matrice_y]=NAN;
//          DistanceMap_[matrice_y][matrice_x]=NAN;
//        }
        matrice_y=string_end; //account for the overstepping of the for loop
        continue;
      }

      //IN RING SYSTEM evaluation
      const int rescaled_ring = int((ring+rel_scale_factor-1)/rel_scale_factor);// 0,1,2,3,4... -> 0,1,1,1,2,2,2... for rel_scale_factor=3
      if (size_t(ring/rel_scale_factor) > (limitPairsPtr_->size()-1) ) {
        log_trace_stream("Ring "<<ring<<", rescaled "<<rescaled_ring<<" too far away; max Ringlimits "<<(limitPairsPtr_->size()-1));
        continue;
      }
      
      const double minusLimit = (*limitPairsPtr_)[rescaled_ring].first;
      const double plusLimit = (*limitPairsPtr_)[rescaled_ring].second;

      const I3Position& pos_B = domPos_cache[matrice_y];
      const double z_B(pos_B.GetZ());

      log_trace_stream("ring "<< ring << " z_A "<< z_A << " z_B " << z_B << " ring- " <<minusLimit<< " ring+ " <<plusLimit<< std::endl);
      const int dom_spacing_AB = omkey_B.GetOM() - omkey_A.GetOM();
      const double zdist_AB = z_B-z_A;
      const double xdist_AB = pos_B.GetX() - pos_A.GetX();
      const double ydist_AB = pos_B.GetY() - pos_A.GetY();
      const double dist = sqrt(xdist_AB*xdist_AB + ydist_AB*ydist_AB + zdist_AB*zdist_AB);

      if ((domSpacingsOpt_ && (-minusLimit<= dom_spacing_AB && dom_spacing_AB <= plusLimit))
        || (!domSpacingsOpt_ && (-minusLimit<= zdist_AB && zdist_AB <= plusLimit)))
      {
        DistanceMap_[matrice_x][matrice_y] = dist;
        log_trace("Distance %f", dist);
      }
      else {
//        DistanceMap_[matrice_x][matrice_y] = NAN;
        log_trace("Not on ring spacers");
      }

      const double zdist_BA = -zdist_AB; //notice: switched indexes
      const int dom_spacing_BA = -dom_spacing_AB;
      if ((domSpacingsOpt_ && (-minusLimit<= dom_spacing_BA && dom_spacing_BA <= plusLimit))
        || (!domSpacingsOpt_ && (-minusLimit<= zdist_BA && zdist_BA <= plusLimit)))
      {
        DistanceMap_[matrice_y][matrice_x] = dist;
        log_trace("Distance %f", dist);
      }
      else {
//        DistanceMap_[matrice_y][matrice_x] = NAN;
        log_trace("Not on ring spacers");
      }
    }
  }
  log_debug_stream("DONE"<<std::endl);
  log_debug("Leaving BuildDistanceMap()");
};
