/**
* \file HitSorting.cxx
*
* (c) 2012 the IceCube Collaboration
*
* $Id: HitSorting.h 99900 2013-02-26 10:10:43Z mzoll $
* \version $Revision: 99900 $
* \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
* \author Marcel Zoll <marcel.zoll@fysik.su.se>
*/

#include "HiveSplitter/HitSorting.h"

//==================namespace HitSorting===============
HitSorting::Hit::Hit(const unsigned int di, const unsigned int pi,const double t,const double c):
  domIndex(di),
  pulseIndex(pi),
  time(t),
  charge(c)
{};

HitSorting::Hit::Hit(const OMKey omkey, const unsigned int pi,const double t,const double c):
  domIndex(OMKeyHash::OMKey2SimpleIndex(omkey)),
  pulseIndex(pi),
  time(t),
  charge(c)
{};

bool HitSorting::Hit::timeOrdered::operator()(const Hit& h1, const Hit& h2) const{
  if (h1.time!=h2.time)
    return(h1.time<h2.time);
  if (h1.domIndex!=h2.domIndex)
    return(h1.domIndex<h2.domIndex);
  return(h1.pulseIndex<h2.pulseIndex);
};

bool HitSorting::Hit::retrievalOrdered::operator()(const Hit& h1, const Hit& h2) const{
  if (h1.domIndex!=h2.domIndex)
    return(h1.domIndex<h2.domIndex);
  return(h1.pulseIndex<h2.pulseIndex);
};

HitSorting::RetrievalOrderedHitSet HitSorting::TimeOrderedToRetrievalOrdered(const TimeOrderedHitSet& hitSet) {
  RetrievalOrderedHitSet s;
  for (TimeOrderedHitSet::const_iterator set_iter=hitSet.begin(); set_iter!=hitSet.end(); set_iter++) {
    s.insert(*set_iter);
  }
  return s;
};

HitSorting::TimeOrderedHitSet HitSorting::RetrievalOrderedToTimeOrdered(const RetrievalOrderedHitSet& hitSet) {
  TimeOrderedHitSet s;
  for (RetrievalOrderedHitSet::const_iterator set_iter=hitSet.begin(); set_iter!=hitSet.end(); set_iter++) {
    s.insert(*set_iter);
  }
  return s;
};

bool HitSorting::operator==(const Hit& h1, const Hit& h2)
  {return(h1.domIndex==h2.domIndex && h1.pulseIndex==h2.pulseIndex);};

bool HitSorting::operator!=(const Hit& h1, const Hit& h2)
  {return(h1.domIndex!=h2.domIndex || h1.pulseIndex!=h2.pulseIndex);};

bool HitSorting::operator<(const Hit& h1, const Hit& h2) {
  if (h1.time!=h2.time)
    return(h1.time<h2.time);
  if (h1.domIndex!=h2.domIndex)
    return(h1.domIndex<h2.domIndex);
  return(h1.pulseIndex<h2.pulseIndex);
};

HitSorting::SubEventPredicate::SubEventPredicate(const HitSorting::RetrievalOrderedHitSet& hitSet):
  hits(hitSet) {};

bool HitSorting::SubEventPredicate::operator()(const OMKey& omkey, const size_t pulseIdx, const I3RecoPulse&) {
  //if the hit is in the set, include it in the mask
  const uint simpleIndex = OMKeyHash::OMKey2SimpleIndex(omkey);
  //count does use the comparision operator(==), which is implemeted to only evaluate Hit.di and Hit.pi (dom-index and pulse-index)
  return(hits.count(Hit(simpleIndex,pulseIdx,0,0)));
};

HitSorting::RetrievalOrderedHitSet HitSorting::ExtractHits (const I3RecoPulseSeriesMap& pulses, const HitSorting::Extract_Mode mode) {
  HitSorting::RetrievalOrderedHitSet hitSet;
  for (I3RecoPulseSeriesMap::const_iterator domIt=pulses.begin(); domIt!=pulses.end(); domIt++) {

    const OMKey& omkey = domIt->first;
    const uint simpleIndex = OMKeyHash::OMKey2SimpleIndex(omkey);

    if (domIt->second.size()==0) {
      log_warn_stream("This RecoPulseSeries contains an empty PulseSeries at DOM "<< omkey << std::endl);
      continue;
    }

    switch (mode) {
      case HitSorting::Extract_FirstHitOnly: {
	const I3RecoPulse& first_pulse = domIt->second[0];
	hitSet.insert(Hit(simpleIndex,0,first_pulse.GetTime(), first_pulse.GetCharge()));
      }
      break;
      case HitSorting::Extract_AllHits: {
	for (uint pulseIndex=0; pulseIndex<domIt->second.size(); pulseIndex++) {
	  const I3RecoPulse& pulse = domIt->second[pulseIndex];
	  hitSet.insert(Hit(simpleIndex,pulseIndex,pulse.GetTime(), pulse.GetCharge()));
	}
      }
      break;
      case HitSorting::Extract_TotalChargeToFirstHit: {
	double summed_charge = 0.;
	for (uint pulseIndex=0; pulseIndex<domIt->second.size(); pulseIndex++) {
	  const I3RecoPulse& pulse = domIt->second[pulseIndex];
	  summed_charge+=pulse.GetCharge();
	}
	const I3RecoPulse& first_pulse = domIt->second[0];
	hitSet.insert(Hit(simpleIndex,0,first_pulse.GetTime(),summed_charge));
      }
      break;
      default: {
	log_fatal("Bad Extraction Mode");
      }
    }
  }
  return hitSet;
};

I3RecoPulseSeriesMap HitSorting::RevertHits (const HitSorting::RetrievalOrderedHitSet& hits, const I3RecoPulseSeriesMap& pulses, const bool useAllHits) {
  I3RecoPulseSeriesMap recoMap;

  if (!useAllHits) {
    for (std::set<Hit>::const_iterator hit_iter=hits.begin(); hit_iter!=hits.end(); hit_iter++) {
      const OMKey omkey = OMKeyHash::SimpleIndex2OMKey(hit_iter->domIndex);
      const uint& pulse_index = hit_iter->pulseIndex;
      std::vector<I3RecoPulse>::const_iterator pPtr = (pulses.find(omkey)->second.begin()+pulse_index);
      recoMap[omkey].push_back(*pPtr);
    }
  }
  else{
    for (std::set<Hit>::const_iterator hit_iter=hits.begin(); hit_iter!=hits.end(); hit_iter++) {
      const OMKey omkey = OMKeyHash::SimpleIndex2OMKey(hit_iter->domIndex);
      const std::vector<I3RecoPulse>& pulse_series = pulses.find(omkey)->second;
      recoMap[omkey]= pulse_series;
    }
  }
  return recoMap;
};

I3RecoPulseSeriesMap HitSorting::ConvertHits2RecoPulseSeriesMap (const HitSorting::RetrievalOrderedHitSet& hits) {
  I3RecoPulseSeriesMap recoMap;

  for (std::set<Hit>::const_iterator hit_iter=hits.begin(); hit_iter!=hits.end(); hit_iter++) {
    const OMKey omkey = OMKeyHash::SimpleIndex2OMKey(hit_iter->domIndex);
    I3RecoPulse reco_pulse;
    reco_pulse.SetTime(hit_iter->time);
    reco_pulse.SetCharge(hit_iter->charge);
    reco_pulse.SetWidth(0.);
    reco_pulse.SetFlags(0);
    recoMap[omkey].push_back(reco_pulse);
  }
  return recoMap;

};