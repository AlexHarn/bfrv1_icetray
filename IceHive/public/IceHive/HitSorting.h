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
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/physics/I3MCHit.h"
#include "simclasses/I3MCPulse.h"

#include "dataclasses/I3MapOMKeyMask.h" //one of bitmasks that is explicitly supported

///provides functionality to extract and sort Hit-objects
namespace HitSorting {
  
  //============ CLASS AbsHit ===========
  
  /** @brief A abstract description of a hit
  * This type needs to be as small as possible so that copies are cheap.
  */
  class AbsHit{
  private:
    ///The index of the DOM on which this hit occurred, within the set of hit DOMs of the current event
    OMKeyHash::SimpleIndex domIndex; //TODO make const
    ///The actual time of the hit
    double time; //TODO make const
  protected:
    /** Constructor
    * @param domIndex The index of the DOM where the hit occurred
    * @param time The time of the hit
    */
    AbsHit(OMKeyHash::SimpleIndex domIndex, const double time);
    
    /** Constructor
    * @param omkey The OMkey of the DOM where the hit occurred
    * @param time The time of the hit
    */
    AbsHit(const OMKey omkey, const double time);
  public:
    ///simply return the DOMindex of this hit
    OMKeyHash::SimpleIndex GetDOMIndex() const;    
    ///simply return the OMKey of this hit
    OMKey GetOMKey() const;
    ///simply return the Time of this hit
    double GetTime() const;
    ///Comparison Operator for AbsHits; just compare the (unique) index in the time ordered AbsHit-series
    bool operator==(const AbsHit& rhs) const;
    ///for sorting AbsHits we're mostly interested in their time, and only use their indices as tie-breakers
    bool operator<(const AbsHit& rhs) const;
  };
  
  std::ostream& operator<< (std::ostream& oss, const HitSorting::AbsHit& h);
  
  //=========== Helpers to get arbitrary time info from objects =============
  
  ///Get a sensible time-information of an object: this might be exact time, start-time,
  /// or whatever makes sense to order objects in time
  /// @param r the respones object interesed in
  template <class Response>
  double 
  GetInferredTime(const Response &r);
  
  //some specializations for explicitly supported types
  template<> double GetInferredTime(const I3RecoPulse &r);
  template<> double GetInferredTime(const I3DOMLaunch &r);
  template<> double GetInferredTime(const I3MCHit &r);
  template<> double GetInferredTime(const I3MCPulse &r);
  
  ///establish time-ordering
  template <class Response>
  bool
  time_sort (const Response &i, const Response &j)
    { return (GetInferredTime(i)<GetInferredTime(j)); };
  
  //============ CLASS HitObject ===========
    
  //forward declare this class
  class Hit;
  
  /** @brief the representation of a Hit as a pair of a OMKet and a (detector-)response-object like a pulse/lunch/hit/whatever
   * this class is templated and needs exzplicitly the the specialization of the GetTime()-routine
   */
  template <class Response>
  class HitObject {
  private:
    /// holds the OMKey
    OMKey omkey_; //TODO make const
    /// holds the pulse/lunch/hit/whatever
    Response response_obj_; //TODO make const
  public:
    /// constructor
    /// @param omkey a OMKey this response was registered on
    /// @param response_obj the Response object
    HitObject(OMKey omkey,
              Response response_obj):
      omkey_(omkey),
      response_obj_(response_obj)
    {};
    /// get the characteristic time-stamp of this pulse/launch/hit/whatever;
    double GetTime() const
      {return GetInferredTime(response_obj_);};
    /// get the OMKey of this object
    inline OMKey GetOMKey() const
      {return omkey_;};
    /// return the ResponseObject
    inline Response GetResponseObj() const
      {return response_obj_;};
    ///for sorting AbsHits we're mostly interested in their time, and only use their indices as tie-breakers
    inline bool operator<(const HitObject& rhs) const
      {return GetTime()<rhs.GetTime();};
    ///Create a Hit from this object
    Hit CreateAssociatedHit() const;
  };
  
  //typedef it for fast access
  typedef HitSorting::HitObject<I3RecoPulse> I3RecoPulse_HitObject;
  typedef HitSorting::HitObject<I3DOMLaunch> I3DOMLaunch_HitObject;
  typedef HitSorting::HitObject<I3MCHit> I3MCHit_HitObject;
  typedef HitSorting::HitObject<I3MCPulse> I3MCPulse_HitObject;
  
  ///dump the HitObject
  template <class Response>
  std::ostream& operator<<(std::ostream& oss, const HitSorting::HitObject<Response>& h);
  
  
  //============ CLASS Hit ===========
  
  /// A description of a Hit, which can be traced to a HitObject of arbitray type
  class Hit:public AbsHit {
    //friend HitObject so that if has access to the constructor
    template<class Response> friend class HitObject;
  private:
    ///tracer back to the HitObject that is represented by this simplified object
    const void* base_obj_ptr_;
  public:
    ///blank constructor [[DEPRICATED]];
    ///sometimes need a blank constructor for comparisions/replacements
    //DANGER this can be possibly dangerous if used without causion
    Hit();
    ///get the associated HitObject to this hit 
    //DANGER user be warned; cast to the right type! 
    template<class Response>
    const HitObject<Response>&
    GetAssociatedHitObject() const;
  
  private:
    ///constructor; hide it from the user, giving only access to it by friend-classes/functions
    /// @param base_obj the HitObject, this new Hit is constructed from and associated with
    template<class Response>
    Hit(const HitObject<Response> &base_obj):
      AbsHit(OMKeyHash::OMKey2SimpleIndex(base_obj.GetOMKey()), base_obj.GetTime()),
      base_obj_ptr_(&base_obj)
    {};
  };
  
  ///Dump the object into a string-stream (this is bound to python as string-cast)
  std::ostream& operator<<(std::ostream& oss, const Hit& h);  
  
  //some more definitions for frequent use, objects
  typedef std::set<Hit> HitSet;
  typedef std::vector<Hit> HitSeries;
  typedef std::list<Hit> HitList;
  typedef std::vector<HitSeries> HitSeriesSeries; //NOTE maybe that is too crazy
  typedef std::deque<Hit> HitDeque;
  
  ///need to implement a ordering principle
  bool RetrievalOrdered (const Hit& h1, const Hit& h2);

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

   /** @brief Function to buffer up Pulses from an RecoPulseSeriesMap into an ordered vector of compact Hits
  * @param pulses the Pulses to buffer up
  * @return the retrieval ordered HitSeries
  */
  template <class Response, class HitObjectContainer>
  HitObjectContainer
  OMKeyMap_To_HitObjects (const I3Map<OMKey, std::vector<Response> > &pulses);
    
  /** @brief Convert a series of HitObjects back to a native ResponseSeriesMap 
  * @param hits the HitObjects to revert
  * @return the ResponseMap of reverted Hits
  */
  template <class Response, class HitObjectContainer>
  I3Map<OMKey, std::vector<Response> >
  HitObjects_To_OMKeyMap (const HitObjectContainer &hits);
  
  
  //===================== CLASS OMKeyMap_HitFacility =============================
  
  /** A class which helps to extract hits from an ResponseMap Object in the frame;
   * which also holds all the functionality so you do not do anything stupid
   */
  template<class Response>
  class OMKeyMap_HitFacility {
    SET_LOGGER("OMKeyMap_HitFacility");
  protected:  
    typedef I3Map<OMKey, std::vector<Response> > ResponseSeriesMap;
  protected://properties/parameters
    ///PARAM: the frame this might have been extracted from
    const I3FramePtr frame_;
    ///PARAM: name of this map in the frame
    const std::string key_;
  protected://properties
    ///hold a pointer to the OMKeyMap the Hits have been extracted from
    boost::shared_ptr<const ResponseSeriesMap> map_;
    /// here are the extracted objects
    const std::deque<HitObject<Response> > hitObjects_;
    
  public://methods
    /** constructor
     * @param frame frame to extract a map from 
     * @param key key in the frame at which we going to find a ResponseMap/Mask
     */
    OMKeyMap_HitFacility(I3FramePtr frame, const std::string &key);
    
    /** Extract Hit-objects from the ResponseMap and put/insert them into the container 
     * @return the container filled with timeordered Hits
     */
    template <class HitContainer> HitContainer GetHits() const;
    
    /** Revert once extracted Hits back to a subMap of the original ResonseMap
     * @param hits a container with the hits to revert
     * @return the subResponseMap
     */
    template<class HitContainer>
    ResponseSeriesMap
    MapFromHits (const HitContainer &hits) const;
    
    /** Get the HitObject associated to this hit
     * @param h the hit
     * @return the associated ResponseObject
     */
    const HitObject<Response>& GetHitObject(const Hit &h) const;
    //NOTE FUTURE here could be a general implementation of "MaskFromHits()", however the OMKeyMapMasks are not templated
  };
  
  typedef OMKeyMap_HitFacility<I3DOMLaunch> I3DOMLaunchSeriesMap_HitFacility;
  typedef OMKeyMap_HitFacility<I3MCHit> I3MCHitSeriesMap_HitFacility;
  typedef OMKeyMap_HitFacility<I3MCPulse> I3MCPulseSeriesMap_HitFacility;
  
  //============ CLASS 3RecoPulseSeriesMap_HitFacility ===========
  
  /// specialize OMKeyMap_HitFacility to I3RecoPulse, and add the functionality to produce Masks
  class I3RecoPulseSeriesMap_HitFacility:public OMKeyMap_HitFacility<I3RecoPulse> {
    SET_LOGGER("I3RecoPulseSeriesMap_HitFacility");
  public://methods
    /** constructor
     * @param frame frame to extract a map from 
     * @param key key in the frame at which we going to find a ResponseMap/Mask
     */
    I3RecoPulseSeriesMap_HitFacility(I3FramePtr frame, const std::string &key);
    
    /** get the subset of hits which corresponds to the mask at this key
     * @param key a the key of a Map/mask that pulses should be extracted from;
     *  needs to inherit from map;
     *  blank parameter returns the Hits from the bae-object map_
     * @return the container filled with timeordered Hits
     */
    template <class HitContainer> HitContainer GetHits (const std::string &key="") const;
    
    /** Revert once extracted Hits back to a subMap and a final ResponseMask of the original ResonseMap
     * @param hits a container with the hits to revert
     * @return the subResponseMapMask
     */
    template<class Container>
    I3RecoPulseSeriesMapMask MaskFromHits (const Container &hits) const;
  };
}; //end namespace HitSorting







//==============================================================================
//========================== IMPLEMENTATIONS ===================================
//==============================================================================


//============== class HitObject ==============
template <class Response>
HitSorting::Hit 
HitSorting::HitObject<Response>::CreateAssociatedHit() const {
  return Hit(*this);
}

template <class Response>
std::ostream& HitSorting::operator<<(std::ostream& oss, const HitSorting::HitObject<Response>& h) {
  oss << "Hit(" <<
  " omkey : " << h.GetOMKey() <<
  ", response : " << h.GetResponseObj() << " )";
  return oss;
};

//============ Helpers on sets of Hits =====
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

//====================== EXTRA ===================

template <class Response, class HitObjectContainer>
HitObjectContainer
HitSorting::OMKeyMap_To_HitObjects (const I3Map<OMKey, std::vector<Response> > &pulses)
{
  HitObjectContainer hitObjs;

  for (typename I3Map<OMKey, std::vector<Response> >::const_iterator domIt=pulses.begin(); domIt!=pulses.end(); ++domIt) {
    const OMKey& omkey = domIt->first;
    for (typename std::vector<Response>::const_iterator pulseIt =domIt->second.begin() ; pulseIt!=domIt->second.end(); ++pulseIt) {
      hitObjs.push_back(HitObject<Response>(omkey, *pulseIt));
    }
  }
  
  return hitObjs;
};


template <class Response, class HitObjectContainer>
I3Map<OMKey, std::vector<Response> >
HitSorting::HitObjects_To_OMKeyMap (const HitObjectContainer &hitObjs)
{
  I3Map<OMKey, std::vector<Response> > responseMap;

  for (typename HitObjectContainer::const_iterator hit_iter=hitObjs.begin(); hit_iter!=hitObjs.end(); ++hit_iter) {
    responseMap[hit_iter->GetOMKey()].push_back(hit_iter->GetResponseObj());
  }
  
  //need to do this as ResponseSeries in Maps need to be time ordered
  for (typename I3Map<OMKey, std::vector<Response> >::iterator omkey_vec= responseMap.begin(); omkey_vec!=responseMap.end(); omkey_vec++)
    sort(omkey_vec->second.begin(), omkey_vec->second.end(), time_sort<Response> ); 
  
  return responseMap;
};


//================== CLASS Hit =============

template<class Response>
const HitSorting::HitObject<Response>&
HitSorting::Hit::GetAssociatedHitObject () const {
  return (const HitSorting::HitObject<Response>&)*(HitSorting::HitObject<Response>*)base_obj_ptr_;
};


//========== CLASS OMKeyMap_HitFacility =============

template <class Response>
HitSorting::OMKeyMap_HitFacility<Response>::OMKeyMap_HitFacility(I3FramePtr frame, const std::string &key):
  frame_(frame),
  key_(key),
  map_(frame_->Get<boost::shared_ptr<const ResponseSeriesMap> >(key_)),
  hitObjects_(OMKeyMap_To_HitObjects<Response, std::deque<HitObject<Response> > > (*map_))
{
  if (!map_)
    log_fatal("No suitable map found at key '%s'", key_.c_str());
  //hitObjects_ = OMKeyMap_To_HitObjects<Response, std::deque<HitObject<Response> > > (*map_);
};

template <class Response> template <class HitContainer>
HitContainer
HitSorting::OMKeyMap_HitFacility<Response>::GetHits() const {
  //make the according hits and link them
  HitContainer hits;
  BOOST_FOREACH(const HitObject<Response> &ho, hitObjects_) {
    hits.push_back(ho.CreateAssociatedHit());
  }
  //enforce the time ordering
  sort(hits.begin(), hits.end());

  return hits;
}

template <class Response> template<class HitContainer>
typename HitSorting::OMKeyMap_HitFacility<Response>::ResponseSeriesMap
HitSorting::OMKeyMap_HitFacility<Response>::MapFromHits (const HitContainer &hits) const {
  
  I3Map<OMKey, std::vector<Response> > responseMap;

  for (typename HitContainer::const_iterator hit_iter=hits.begin(); hit_iter!=hits.end(); ++hit_iter) {
    const OMKey omkey = hit_iter->GetOMKey();
    responseMap[omkey].push_back(hit_iter->template GetAssociatedHitObject<Response>().GetResponseObj());
  }
  
  //need to do this as ResponseSeries in Maps need to be time ordered
  for (typename I3Map<OMKey, std::vector<Response> >::iterator omkey_vec= responseMap.begin(); omkey_vec!=responseMap.end(); omkey_vec++)
    sort(omkey_vec->second.begin(), omkey_vec->second.end(), time_sort<Response> );
  
  return responseMap;
};

template <class Response>
const HitSorting::HitObject<Response>&
HitSorting::OMKeyMap_HitFacility<Response>::GetHitObject(const HitSorting::Hit &h) const {
  return (h.GetAssociatedHitObject<Response>());
}


//========== CLASS I3RecoPulseSeriesMapHitFacility =============
template<class Container>
I3RecoPulseSeriesMapMask
HitSorting::I3RecoPulseSeriesMap_HitFacility::MaskFromHits (const Container &hits) const {
  
  I3RecoPulseSeriesMap responseMap;

  BOOST_FOREACH(const Hit &h, hits) {
    const OMKey omkey = h.GetOMKey();
    responseMap[omkey].push_back(GetHitObject(h).GetResponseObj());
  }
 
  //need to do this as ResponseSeries in Maps need to be time ordered
  for (I3RecoPulseSeriesMap::iterator omkey_vec= responseMap.begin(); omkey_vec!=responseMap.end(); omkey_vec++)
    sort(omkey_vec->second.begin(), omkey_vec->second.end(), time_sort<I3RecoPulse> ); 
  
  return I3RecoPulseSeriesMapMask(*frame_, key_, responseMap);      
};

template <class HitContainer>
HitContainer
HitSorting::I3RecoPulseSeriesMap_HitFacility::GetHits (const std::string &key) const {
  
  //first check if the mask is really derived object from 'key_'
  if (key.empty() || key==key_)
    return OMKeyMap_HitFacility<I3RecoPulse>::GetHits<HitContainer>(); //go though the base-class method
  
  I3RecoPulseSeriesMapMaskConstPtr derived_mask = frame_->Get<I3RecoPulseSeriesMapMaskConstPtr>(key);
  if (!derived_mask) {
    log_fatal("Key is no Mask!");
  }
  while (derived_mask && (derived_mask->GetSource() != key_)) {
    std::string ancestor_name = derived_mask->GetSource();
    derived_mask = frame_->Get<I3RecoPulseSeriesMapMaskConstPtr>(ancestor_name);
  }
  if (!derived_mask)
    log_fatal("the mask is not derived from key_");
  
  //just reextract the hitobjects
  std::deque<HitObject<I3RecoPulse> > hitObjects_tmp = OMKeyMap_To_HitObjects<I3RecoPulse, std::deque<HitObject<I3RecoPulse> > >(*derived_mask->Apply(*frame_));//as an alternative choose a priporityque

  //make the hits by finding the correct hitobject
  HitContainer hits;
  
  std::deque<HitObject<I3RecoPulse> >::const_iterator iter=hitObjects_.begin();
  BOOST_FOREACH(const HitObject<I3RecoPulse> &ho, hitObjects_tmp) {
    while (iter != hitObjects_.end()) {
      if (iter->GetOMKey() == ho.GetOMKey() && iter->GetResponseObj() == ho.GetResponseObj()) {
        hits.push_back(iter->CreateAssociatedHit());
        break;
      }
      iter++;
    }
  }

  //enforce the time ordering
  sort(hits.begin(), hits.end());
  return hits;
};

#endif