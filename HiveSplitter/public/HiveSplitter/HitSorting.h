/**
 * @file HitSorting.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: HitSorting.h 99900 2013-02-26 10:10:43Z mzoll $
 * @version $Revision: 99900 $
 * @date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * @author Marcel Zoll <marcel.zoll@fysik.su.se>
 */

#ifndef HITSORTING_H
#define HITSORTING_H

#include "HiveSplitter/OMKeyHash.h"

#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

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
    * @param c the charge of the hit
    */
    Hit(const unsigned int di, const unsigned int pi, const double t, const double c=NAN);
    /** Constructor
    * @param omkey The omkey of the DOM where the hit occurred
    * @param pi The index of the hit on the DOM
    * @param t The time of the hit
    * @param c the charge of the hit
    */
    Hit(const OMKey omkey, const unsigned int pi, const double t, const double c=0.);
    
    ///Determine whether hits are in time order
    struct timeOrdered{
      bool operator()(const Hit& h1, const Hit& h2) const;
    };
    ///Determine whether Hits are in an order more suitable for retrieval from an I3RecoPulseSeriesMap
    struct retrievalOrdered{
      bool operator()(const Hit& h1, const Hit& h2) const;
    };
  };

  ///A set of hits ordered by their times
  typedef std::set<Hit,Hit::timeOrdered> TimeOrderedHitSet;
  ///A set of hits ordered for fast lookup in the original I3RecoPulseSeriesMap
  typedef std::set<Hit,Hit::retrievalOrdered> RetrievalOrderedHitSet;

  /** @brief A function that does convert a TimeOrderedHitSet to a RetrievalOrderedHitSet
   * @param hitSet a time ordered hit set
   */
  RetrievalOrderedHitSet TimeOrderedToRetrievalOrdered(const TimeOrderedHitSet& hitSet);
  
  /** @brief A function that does convert a RetrievalOrderedHitSet to a TimeOrderedHitSet
   * @param hitSet aretrieval ordered hit set
   */
  TimeOrderedHitSet RetrievalOrderedToTimeOrdered(const RetrievalOrderedHitSet& hitSet);
  
  /** @brief Comparision Operator for Hits
  * just compare the (unique) index in the time ordered Hit-series
  */
  bool operator==(const Hit& h1, const Hit& h2);
  /** @brief Anti-Comparision Operator for Hits
  * just compare the (unique) index in the time ordered Hit-series
  */
  bool operator!=(const Hit& h1, const Hit& h2);

  ///for sorting Hits we're mostly interested in their time, and only use their indices as tie-breakers
  bool operator<(const Hit& h1, const Hit& h2);

  ///Test whether any item in the sorted range [first1,last1) is also in the sorted range [first2,last2)
  template <class InputIterator1, class InputIterator2>
  bool setsIntersect(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2) {
    while (first1!=last1 && first2!=last2) {
      if (*first1<*first2)
	first1++;
      else if (*first2<*first1)
	first2++;
      else
	return(true);
    }
    return(false);
  };

  /** @brief a helper class that can compares to sets and evaluates the identity by evaluating the identity of every element
   * Use this operator if both sets are extracted from the same pulsemap
  * @return true, if sets are identical in every element; false if not
  */
  template <class ordered_set>
  bool SetsIdentical(const ordered_set &lhs, const ordered_set &rhs) {
    if (lhs.size() != rhs.size())
      return false;

    typename ordered_set::const_iterator lhs_iter = lhs.begin();
    typename ordered_set::const_iterator rhs_iter = rhs.begin();
    while (lhs_iter!=lhs.end()) {
      if (!(*lhs_iter==*rhs_iter)) //compare the unique index
	return false;
      lhs_iter++;
      rhs_iter++;
    }
    return true;
  };
  
  /** @brief Return the union of lhs and rhs
  * @return the union of the sets
  */
  template <class ordered_set>
  inline ordered_set UniteSets(const ordered_set &lhs, const ordered_set &rhs) {
    ordered_set unite;

    for (typename ordered_set::const_iterator lhs_iter = lhs.begin(); lhs_iter!=lhs.end(); ++lhs_iter){
      unite.insert(*lhs_iter);
    }
    for (typename ordered_set::const_iterator rhs_iter = rhs.begin(); rhs_iter!=rhs.end(); ++rhs_iter){
      unite.insert(*rhs_iter);
    }
    return unite;
  };
  
  //NEED THIS ESPECIALLY TO CREATE MASKS
  /** @brief A function object which facilitates creating pulse masks for given subevents
  * makes you able to use I3RecoPulseSeriesMapMask(*frame, inputName_, SubEventPredicate(RetrievalOrderedHitSet));
  */
  struct SubEventPredicate{
    ///The collection of hits making up the subevent
    const HitSorting::RetrievalOrderedHitSet& hits;
    //NOTE Implementation is governed by function template "dataclasses/I3MapOMKeyMask":Constructor_with_predicate
    ///constructor
    SubEventPredicate(const HitSorting::RetrievalOrderedHitSet& hitSet);
    ///Determine whether a particular pulse (hit) is part of the subevent
    ///\param k The DOM on which the pulse occurred
    ///\param pulseIdx The index of the pulse on the DOM
    bool operator()(const OMKey& k, const size_t pulseIdx, const I3RecoPulse&);
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
   * @param mode extraction_mode:
   * Extract_AllHits = All Hits are extracted,
   * Extract_FirstHitOnly = Only the first hit on each DOM is extracted,
   * Extract_TotalChargeToFirstHit = Only the first hit is extracted and the total charge in this DOM assigned to it.
   * @return the retrievel ordered HitSeries
   */
   HitSorting::RetrievalOrderedHitSet ExtractHits (const I3RecoPulseSeriesMap& pulses, const Extract_Mode mode=Extract_AllHits);
  
  /** @brief Convert a series of Hits back to an native I3RecoPulseSeriesMap
   * @param hits the retrievel ordered Hits to revert
   * @param pulses the Pulses the Hits were created from; DANGER if not so, this will fail with a segfault
   * @param useAllHits use all Hits on each original OM; usefull if only first hits were extracted DANGER currently disabled
   * @return the retrievel ordered HitSeries
   */
  I3RecoPulseSeriesMap RevertHits (const HitSorting::RetrievalOrderedHitSet& hits, const I3RecoPulseSeriesMap& pulses, const bool useAllHits=false);

  /** @brief Convert a series of Hits back to an regular I3RecoPulseSeriesMap by recreating the Pulses; DANGER this function does only convert the format not information
   * @param hits the retrievel ordered Hits to revert
   * @return the retrievel ordered HitSeries
   */
  I3RecoPulseSeriesMap ConvertHits2RecoPulseSeriesMap (const HitSorting::RetrievalOrderedHitSet& hits);
};

#endif