/**
 * @file HitSorting.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * @version $Revision$
 * @date $Date$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * 
 * Provides functionality to extract and sort Hit-objects from various kinds of representations of hits in icetray
 */

#ifndef HITSORTING_H
#define HITSORTING_H

#include <numeric>

#include "OMKeyHash.h"

#include "dataclasses/physics/I3RecoPulse.h" //one of templates that are explicitly supported

#include "dataclasses/I3MapOMKeyMask.h" //one of bitmasks that is explicitly supported

///provides functionality to extract and sort Hit-objects
namespace HitSorting {
  /** @brief A compact description of a hit
  * This type needs to be as small as possible so that copies are cheap.
  */
  struct Hit{
    ///The index of the DOM on which this hit occurred, within the set of hit DOMs of the current event
    OMKeyHash::SimpleIndex domIndex; //indexing starts with 0
    ///The index of this hit on the DOM where it occurred, index 0 is first hit
    unsigned int pulseIndex; //indexing starts with 0
    ///The actual time of the hit
    double time;
    ///The charge of this hit
    double charge;

    /** Constructor
    * @param di The index of the DOM where the hit occurred
    * @param pi The index of the hit on the DOM
    * @param t The time of the hit
    * @param c the charge of the hit; optional
    */
    Hit(const unsigned int di, const unsigned int pi, const double t, const double c=1.);
    
    /** Constructor
    * @param omkey The OMkey of the DOM where the hit occurred
    * @param pi The index of the hit on the DOM
    * @param t The time of the hit
    * @param c the charge of the hit
    */
    Hit(const OMKey omkey, const unsigned int pi, const double t, const double c=1.);

    ///Determine whether hits are in time order
    struct timeOrdered{
      bool operator()(const Hit& h1, const Hit& h2) const;
    };
    ///Determine whether Hits are in an order more suitable for retrieval from an I3RecoPulseSeriesMap
    struct retrievalOrdered{
      bool operator()(const Hit& h1, const Hit& h2) const;
    };
  };
  
  ///Dump the object into a string-stream (this is bound to python as string-cast)
  std::ostream& operator<<(std::ostream& oss, const Hit& h);

  ///A set of hits ordered by their times
  typedef std::set<Hit,Hit::timeOrdered> TimeOrderedHitSet;
  ///A set of hits ordered for fast lookup in the original I3RecoPulseSeriesMap
  typedef std::set<Hit,Hit::retrievalOrdered> RetrievalOrderedHitSet;
  //some more definitions for frequent use, objects
  typedef std::vector<Hit> HitSeries;
  typedef std::list<Hit> HitList;
  typedef std::vector<HitSeries> HitSeriesSeries;
  
  ///need to implement a ordering principle
  bool RetrievalOrdered (const Hit& h1, const Hit& h2);
  
  /** @brief A function that does convert a TimeOrderedHitSet to a RetrievalOrderedHitSet
   * @param hitSet a time ordered hit set
   */
  inline RetrievalOrderedHitSet TimeOrderedToRetrievalOrdered(const TimeOrderedHitSet& hitSet) 
    {return RetrievalOrderedHitSet(hitSet.begin(), hitSet.end());};
  
  /** @brief A function that does convert a RetrievalOrderedHitSet to a TimeOrderedHitSet
   * @param hitSet a retrieval ordered hit set
   */
  inline TimeOrderedHitSet RetrievalOrderedToTimeOrdered(const RetrievalOrderedHitSet& hitSet)
    {return TimeOrderedHitSet (hitSet.begin(), hitSet.end());};

  /** @brief Comparison Operator for Hits
  * just compare the (unique) index in the time ordered Hit-series
  */
  bool operator==(const Hit& h1, const Hit& h2);
  /** @brief Anti-Comparison Operator for Hits
  * just compare the (unique) index in the time ordered Hit-series
  */
  bool operator!=(const Hit& h1, const Hit& h2);

  ///for sorting Hits we're mostly interested in their time, and only use their indices as tie-breakers
  bool operator<(const Hit& h1, const Hit& h2);

  //=== Now some Helper functions ===
  
  /**Test whether any item in a sorted container is also found in another container
  * NOTE: requirements: iterator and operator< (order!) defined
  * @param lhs the one container
  * @param rhs the other container having the same sorting as lhs_iter
  * @return (true) if lhs and rhs intersect by at least one element
  */
  template <class ordered_set>
  bool SetsIntersect(const ordered_set &lhs, const ordered_set &rhs);
  
  /** @brief a helper function that compares two sets and evaluates the identity by evaluating the identity of every element
  * NOTE: requirements: iterator and operator< (order!) defined
  * @param lhs the one container
  * @param rhs the other container having the same sorting as lhs_iter
  * @return (true) if sets are identical in every element
  */
  template <class ordered_set>
  bool SetsIdentical(const ordered_set &lhs, const ordered_set &rhs);
  
  /** @brief Return the union of lhs and rhs
  * @param lhs the one container
  * @param rhs the other container having the same sorting as lhs_iter
  * @return the union of the sets
  */
  template <class ordered_set>
  ordered_set UniteSets(const ordered_set &lhs, const ordered_set &rhs);

  //NEED THIS ESPECIALLY TO CREATE MASKS
  /** @brief A function object which facilitates creating pulse masks for given subevents
  * makes you able to use I3RecoPulseSeriesMapMask(*frame, inputName_, SubEventPredicate(RetrievalOrderedHitSet));
  */
  template <class HitsContainer, class Response>
  struct MaskPredicate {
    ///The collection of hits making up the subevent
    const HitsContainer& hits;
    //NOTE Implementation is governed by function template "dataclasses/I3MapOMKeyMask":Constructor_with_predicate
    ///constructor
    MaskPredicate(const HitsContainer& hitSet);
    /**Determine whether a particular pulse (hit) is part of the subevent
    * @param k The OMKey on which the pulse occurred
    * @param pulseIdx The index of the pulse on this specific DOM (position in the vector)
    * @param p the RecoPulse object to check against
    * @return true if the requested pulse is found in the set of hits
    */
    bool operator()(const OMKey& k, const size_t pulseIdx, const Response& p);
  };

  //===========
  // Functions to buffer up Pulses from an RecoPulseSeriesMap into an ordered vector of Hits
  //===========

  ///Helper to steer function ExtractHits
  enum Extract_Mode {
    Extract_AllHits,
    Extract_FirstHitOnly,
    Extract_TotalChargeToFirstHit
  };

  /** @brief Function to buffer up Pulses from an RecoPulseSeriesMap into an ordered vector of compact Hits
   * @param pulses the Pulses to buffer up
   * @param mode mode how the extraction should commence
   * Extract_AllHits = All Hits are extracted, [DEFAULT]
   * Extract_FirstHitOnly = Only the first hit on each DOM is extracted,
   * Extract_TotalChargeToFirstHit = Only the first hit is extracted and the total charge in this DOM assigned to it.
   * @return the retrieval ordered HitSeries
   */
  template <class HitSet, class Response>
  HitSet
  ExtractHits (const I3Map<OMKey, std::vector<Response> > &pulses,
               const Extract_Mode mode=Extract_AllHits);
    
  /** @brief Convert a series of Hits back to an native ResponseSeriesMap 
   * @param hits the retrieval ordered Hits to revert
   * @param pulses the Pulses the Hits were created from;
   * @param useAllHits use all Hits on each original OM; useful if only first hits were extracted
   * @return the ResponseMap of reverted Hits
   */
  template <class Response, class HitsContainer>
  I3Map<OMKey, std::vector<Response> >
  RevertHits (const HitsContainer& hits, 
              const I3Map<OMKey, std::vector<Response> >& pulses, 
              const bool useAllHits=false);

  /** @brief Convert a series of Hits back to an regular ResponseSeriesMap by recreating the Pulses;
   * NOTE this function does only convert the format, and does not augment information
   * @param hits the retrieval ordered Hits to revert
   * @return the retrieval ordered HitSeries
   */
  template <class Response>
  I3Map<OMKey, std::vector<Response> >
  ConvertHits_To_ResponseSeriesMap (const HitSorting::RetrievalOrderedHitSet& hits);


  //NOTE Well there is only one object that has Masks currently and that is I3RecoPulse
//   template <class Response> template <class container>
//   ResponseSeriesMapMask 
//   MaskFromHits (I3FramePtr frame, std::string key, const HitsContainer &hits);
  
  /** @note Specialization to I3RecoPulse
   * @param frame frame to extract pulses from
   * @param key at the frame where the original map is found
   * @param hits the hits to use for the mask
   */
  template <class HitsContainer>
  I3RecoPulseSeriesMapMask 
  I3RecoPulseSeriesMapMaskFromHits (const I3Frame& frame, const std::string& key, const HitsContainer &hits);
  
  //===================== CLASS HitSortingFacility =============================
  
  /** A class which helps to extract hits from an ResponseMap Object in the frame;
   * which also holds all the functionality so you do not do anything stupid
   */
  template <class Response>
  class HitSortingFacility {
    SET_LOGGER("HitSortingFacility");
    
  private: //definitions
    typedef std::vector<Response> ResponseSeries;
    typedef I3Map<OMKey,ResponseSeries> ResponseSeriesMap;
    typedef boost::shared_ptr<ResponseSeriesMap> ResponseSeriesMapPtr;
    typedef boost::shared_ptr<const ResponseSeriesMap> ResponseSeriesMapConstPtr;
  //FIXME until there is no general I3MapOMKeyMask, use hardcodings
  //   typedef I3MapOMKeyMask<Response> ResponseSeriesMapMask;
  //   typedef boost::shared_ptr<ResponseSeriesMapMask > ResponseSeriesMapMaskPtr;
  //   typedef boost::shared_ptr<const ResponseSeriesMapMask> ResponseSeriesMapMaskConstPtr;
    typedef I3RecoPulseSeriesMapMask ResponseSeriesMapMask;
    typedef I3RecoPulseSeriesMapMaskPtr ResponseSeriesMapMaskPtr;
    typedef I3RecoPulseSeriesMapMaskConstPtr ResponseSeriesMapMaskConstPtr;
    
  public: //to the people
    ///Helper to steer function ExtractHits
    enum Extract_Mode {
      Extract_AllHits,
      Extract_FirstHitOnly,
      Extract_TotalChargeToFirstHit
    };
  private://properties/parameters
    ///hold a pointer to the object that the Hits have been extracted from
    ResponseSeriesMapConstPtr map_;
    ///PARAM: the frame this might have been extracted from
    I3FramePtr frame_;
    ///PARAM: name of this map in the frame
    std::string key_;
    
  public://methods
    /** constructor
     * @param frame frame to extract a map from 
     * @param key key in the frame at which we going to find a ResponseMap/Mask
     */
    HitSortingFacility(I3FramePtr frame, const std::string &key):
      frame_(frame),
      key_(key),
      map_(frame_->Get<ResponseSeriesMapConstPtr>(key_))
    {
      if (!map_)
        log_fatal("No <'Response'SeriesMapConstPtr> found at key '%s'", key_);
    };
    
    /** Extract Hit-objects from the ResponseMap and put/insert them into the container 
     * @param mode the mode how to extract the hits
     * @return the container filled with Hits
     */
    template<class container> 
    container GetHits (HitSortingFacility::Extract_Mode mode=Extract_AllHits);
    
    /** Revert once extracted Hits back to a subMap of the original ResonseMap
     * @param hits a container with the hits to revert
     * @param useAllHits if (true) one occurring hit a a certain DOM selects its full PulseSeries
     * @return the subResponseMap
     */
    template<class container>
    ResponseSeriesMap MapFromHits (const container &hits,
                                   const bool useAllHits=false);
    
    /** Revert once extracted Hits back to a subMap and a final ResponseMask of the original ResonseMap
     * @param hits a container with the hits to revert
     * @return the subResponseMapMask
     */
    template<class container>
    ResponseSeriesMapMask MaskFromHits (const container &hits);
  };
}; //end namespace HitSorting


//==============================================================================
//========================== IMPLEMENTATIONS ===================================
//==============================================================================

template <class ordered_set>
bool 
HitSorting::SetsIntersect(const ordered_set &lhs,
                          const ordered_set &rhs)
{
  typename ordered_set::const_iterator first1 = lhs.begin();
  typename ordered_set::const_iterator last1 = lhs.end();
  typename ordered_set::const_iterator first2 = rhs.begin();
  typename ordered_set::const_iterator last2 = rhs.end();

  while (first1!=last1 && first2!=last2) {
    if (*first1<*first2)
      ++first1;
    else if (*first2<*first1)
      ++first2;
    else
      return(true);
  }
  return(false);
};

template <class ordered_set>
bool 
HitSorting::SetsIdentical(const ordered_set &lhs,
                          const ordered_set &rhs)
{
  if (lhs.size() != rhs.size())
    return false;
  
  return (std::equal(lhs.begin(), lhs.end(), rhs.begin()) );
};

template <class ordered_set>
ordered_set 
HitSorting::UniteSets(const ordered_set &lhs,
                      const ordered_set &rhs)
{
  ordered_set unite;

  for (typename ordered_set::const_iterator lhs_iter = lhs.begin(); lhs_iter!=lhs.end(); ++lhs_iter){
    unite.insert(*lhs_iter);
  }
  for (typename ordered_set::const_iterator rhs_iter = rhs.begin(); rhs_iter!=rhs.end(); ++rhs_iter){
    unite.insert(*rhs_iter);
  }
  return unite;
};

template <class HitsContainer, class Response>
HitSorting::MaskPredicate<HitsContainer, Response>::MaskPredicate (const HitsContainer& hitSet):
  hits(hitSet) {};

template <class HitsContainer, class Response>
bool HitSorting::MaskPredicate<HitsContainer, Response>::operator() (const OMKey& omkey, const size_t pulseIdx, const Response& p) {
  //if the hit is in the set, include it in the mask
  const uint simpleIndex = OMKeyHash::OMKey2SimpleIndex(omkey);
  //count does use the comparison operator(==), which is implemented to only evaluate the hits DOM-index and pulse-index
  return(hits.count(Hit(simpleIndex,pulseIdx,p->GetTime(),p->GetCharge())));
};


template <class HitsContainer, class Response>
HitsContainer
HitSorting::ExtractHits (const I3Map<OMKey, std::vector<Response> > &pulses,
                         const HitSorting::Extract_Mode mode)
{
  log_debug("Entering ExtractHits()");
  HitsContainer hits;

  for (typename I3Map<OMKey, std::vector<Response> >::const_iterator domIt=pulses.begin(); domIt!=pulses.end(); ++domIt) {

    const OMKey& omkey = domIt->first;
    const unsigned int simpleIndex = OMKeyHash::OMKey2SimpleIndex(omkey);
    log_trace_stream("Working "<<omkey<<" with size "<<domIt->second.size()<<std::endl);

    if (domIt->second.size()==0) {
      log_warn_stream("This RecoPulseSeries contains an empty PulseSeries at DOM "<< omkey << std::endl);
      continue;
    }

    switch (mode) {
      case HitSorting::Extract_FirstHitOnly: {
        const Response& first_pulse = domIt->second[0];
        hits.insert(hits.end(), Hit(simpleIndex,0,first_pulse.GetTime(), first_pulse.GetCharge()));
      }
      break;
      case HitSorting::Extract_AllHits: {
        for (unsigned int pulseIndex=0; pulseIndex<domIt->second.size(); ++pulseIndex) {
          const Response& pulse = domIt->second[pulseIndex];
          hits.insert(hits.end(), Hit(simpleIndex,pulseIndex,pulse.GetTime(), pulse.GetCharge()));
        }
      }
      break;
      case HitSorting::Extract_TotalChargeToFirstHit: {
        double summed_charge = 0.;
        for (unsigned int pulseIndex=0; pulseIndex<domIt->second.size(); ++pulseIndex) {
          const Response& pulse = domIt->second[pulseIndex];
          summed_charge+=pulse.GetCharge();
        }
        const Response& first_pulse = domIt->second[0];
        hits.insert(hits.end(), Hit(simpleIndex,0,first_pulse.GetTime(),summed_charge));
      }
      break;
      default: {
        log_fatal("Bad Extraction Mode");
      }
    }
  }
  log_debug("Leaving ExtractHits()");
  return hits;
};


template <class Response, class HitsContainer>
I3Map<OMKey, std::vector<Response> >
HitSorting::RevertHits (const HitsContainer& hits, 
                        const I3Map<OMKey, std::vector<Response> >& pulses, 
                        const bool useAllHits)
{
  I3Map<OMKey, std::vector<Response> > recoMap;

  for (typename HitsContainer::const_iterator hit_iter=hits.begin(); hit_iter!=hits.end(); ++hit_iter) {
    const OMKey omkey = OMKeyHash::SimpleIndex2OMKey(hit_iter->domIndex);
    typename I3Map<OMKey, std::vector<Response> >::const_iterator opPtr= pulses.find(omkey);
    //if (opPtr==pulses.end()) //NOTE possible safeguard
      //log_fatal("this OMKey does not exist on the PulseMap; are hits really extracted from this ResonseMap?");
    if (useAllHits) {
      recoMap[omkey]= pulses.find(omkey)->second;
    } else {        
      const unsigned int& pulse_index = hit_iter->pulseIndex;
      //if (opPtr->second.size()<pulse_index+1) //NOTE possible safeguard
        //log_fatal("requested hit can not be found in the pulse-series ");
      //NOTE possible safeguard: to be sure here could be another check if the Response signature in time/charge matches with the Hit
      recoMap[omkey].push_back(opPtr->second.at(pulse_index));
    }
  }
  return recoMap;
};


template <class Response>
I3Map<OMKey, std::vector<Response> >
HitSorting::ConvertHits_To_ResponseSeriesMap (const HitSorting::RetrievalOrderedHitSet& hits)
{
  I3Map<OMKey, std::vector<Response> > recoMap;

  for (std::set<Hit>::const_iterator hit_iter=hits.begin(); hit_iter!=hits.end(); ++hit_iter) {
    const OMKey omkey = OMKeyHash::SimpleIndex2OMKey(hit_iter->domIndex);
    Response reco_pulse;
    reco_pulse.SetTime(hit_iter->time);
    reco_pulse.SetCharge(hit_iter->charge);
    reco_pulse.SetWidth(0.);
    reco_pulse.SetFlags(0);
    recoMap[omkey].push_back(reco_pulse);
  }
  return recoMap;
};


template <class HitsContainer>
I3RecoPulseSeriesMapMask 
HitSorting::I3RecoPulseSeriesMapMaskFromHits (const I3Frame& frame, const std::string &key, const HitsContainer &hits)
{
  //DANGER there should be a check if keys and pulses found in the hits also exist in the map
  // but we won't do it, because the user should not spawn anything on the external handles
  
  I3RecoPulseSeriesMapMask output(frame, key);
  output.SetNone();
  BOOST_FOREACH(const HitSorting::Hit &hit, hits) {
    output.Set(OMKeyHash::SimpleIndex2OMKey(hit.domIndex), hit.pulseIndex, true);
  }
  return output;
};


//======================== CLASS HitSortingFacility ===================
template <class Response> template <class HitContainer>
HitContainer
HitSorting::HitSortingFacility<Response>::GetHits (const typename HitSorting::HitSortingFacility<Response>::Extract_Mode mode)
{
  log_debug("Entering GetHits()");
  HitContainer hits;

  for (typename ResponseSeriesMap::const_iterator domIt=map_->begin(); domIt!=map_->end(); ++domIt) {

    const OMKey& omkey = domIt->first;
    const unsigned int simpleIndex = OMKeyHash::OMKey2SimpleIndex(omkey);
    log_trace_stream("Working "<<omkey<<" with size "<<domIt->second.size()<<std::endl);

    if (domIt->second.size()==0) {
      log_warn_stream("This RecoPulseSeries contains an empty PulseSeries at DOM "<< omkey << std::endl);
      continue;
    }

    switch (mode) {
      case HitSorting::Extract_FirstHitOnly: {
        const Response& first_pulse = domIt->second[0];
        hits.insert(hits.end(), Hit(simpleIndex,0,first_pulse.GetTime(), first_pulse.GetCharge()));
      }
      break;
      case HitSorting::Extract_AllHits: {
        for (unsigned int pulseIndex=0; pulseIndex<domIt->second.size(); ++pulseIndex) {
          const Response& pulse = domIt->second[pulseIndex];
          hits.insert(hits.end(), Hit(simpleIndex,pulseIndex,pulse.GetTime(), pulse.GetCharge()));
        }
      }
      break;
      case HitSorting::Extract_TotalChargeToFirstHit: {
        double summed_charge = 0.;
        for (unsigned int pulseIndex=0; pulseIndex<domIt->second.size(); ++pulseIndex) {
          const Response& pulse = domIt->second[pulseIndex];
          summed_charge+=pulse.GetCharge();
        }
        const Response& first_pulse = domIt->second[0];
        hits.insert(hits.end(), Hit(simpleIndex,0,first_pulse.GetTime(),summed_charge));
      }
      break;
      default: {
        log_fatal("Bad Extraction Mode");
      }
    }
  }
  log_debug("Leaving GetHits");
  return hits;
};


template <class Response> template <class HitContainer>
typename HitSorting::HitSortingFacility<Response>::ResponseSeriesMap
HitSorting::HitSortingFacility<Response>::MapFromHits(const HitContainer &hits,
                                                      const bool useAllHits)
{
  //DANGER there should be a check if keys and pulses found in the hits also exist in the map
  // but we won't do it, because the usewr should not spawn anything on the external handles
  
  ResponseSeriesMap recoMap;

  if (!useAllHits) {
    for (HitSeries::const_iterator hit_iter=hits.begin(); hit_iter!=hits.end(); ++hit_iter) {
      const OMKey omkey = OMKeyHash::SimpleIndex2OMKey(hit_iter->domIndex);
      const unsigned int& pulse_index = hit_iter->pulseIndex;
      typename ResponseSeries::const_iterator pPtr = (map_->find(omkey)->second.begin()+pulse_index);
      recoMap[omkey].push_back(*pPtr);
    }
  }
  else{
    for (HitSeries::const_iterator hit_iter=hits.begin(); hit_iter!=hits.end(); ++hit_iter) {
      const OMKey omkey = OMKeyHash::SimpleIndex2OMKey(hit_iter->domIndex);
      const ResponseSeries& pulse_series = map_->find(omkey)->second;
      recoMap[omkey]= pulse_series;
    }
  }
  return recoMap;
};

template <class Response> template <class HitContainer>
typename HitSorting::HitSortingFacility<Response>::ResponseSeriesMapMask 
HitSorting::HitSortingFacility<Response>::MaskFromHits (const HitContainer &hits)
{
  //DANGER there should be a check if keys and pulses found in the hits also exist in the map
  // but we won't do it, because the user should not spawn anything on the external handles
  
  ResponseSeriesMapMask output(*frame_, key_);
  output.SetNone();
  BOOST_FOREACH(const Hit &hit, hits) {
    output.Set(OMKeyHash::SimpleIndex2OMKey(hit.domIndex), hit.pulseIndex, true);
  }
  return output;
};

#endif
