/**
 * \file I3IceHive.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 *
 * The IceTray I3Module wrapper around the central algorithm HiveSplitter and TriggerSplitter,
 * which in turn use a API trough SubEventStartStop
 */

#ifndef ICEHIVEHELPERS_H
#define ICEHIVEHELPERS_H


#include "icetray/OMKey.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3TimeWindow.h"
#include "dataclasses/physics/I3TriggerHierarchy.h"
#include <boost/foreach.hpp>
#include <boost/dynamic_bitset.hpp>


namespace Limits {
  class LimitPair {
  public:
    double minus_;
    double plus_;
    
    LimitPair(const double m, const double p):
      minus_(m), plus_(p)
    {};
  };
  
  class RingLimits {
  public:
    std::vector<LimitPair> limitPairs_;
    
    RingLimits():
      limitPairs_()
    {};
    
    RingLimits(const std::vector<LimitPair> &l):
      limitPairs_(l)
    {};
    
    void AddLimitPair(const LimitPair &lp) {
      limitPairs_.push_back(lp);
    };
    
    int NRings() 
      {return limitPairs_.size()-1;};
       
    LimitPair GetLimitsOnRing(const int r) 
      {return limitPairs_.at(r);};
  };
}


//some helper functions for IceHive
namespace IceHiveHelpers{
  //NOTE THESE COULD IN FACT MOVE TO SOME UNIVERSAL UTILITY PROJECT
  /** @brief Give me all pulses from this ResponseMap which fall within (inclusive) this time-window 
   *  A simple selection function
   *  @param pulses from this pulses
   *  @param tw_start starting at this time
   *  @param tw_stop ending at this time
   *  @return the subset of pulses
   */
  template <class Response>
  I3Map<OMKey,std::vector<Response> > GetPulsesInTimeRange (const I3Map<OMKey,std::vector<Response> > &pulses,
                                                            const double tw_start,
                                                            const double tw_stop);

  /** @brief Give me all pulses from this ResponseMap, which fall within (inclusive) this time-window
   * A simple selection function
   *  @param pulses from this pulses
   *  @param tw within this time-window
   *  @return the subset of pulses
   */
  template <class Response>
  I3Map<OMKey,std::vector<Response> > GetPulsesInTimeRange (const I3Map<OMKey,std::vector<Response> > &pulses,
                                                            const I3TimeWindow &tw)
    {return GetPulsesInTimeRange(pulses, tw.GetStart(), tw.GetStop());};
  
  /** @brief Clip out the triggers with the given configIDs and within the specified TimeWindow
   * Take all triggers firing within the specified time-window (triggers can thereby extend beyond the specified time-window)
   * If no trigger-configIDs are specified take every trigger in the time-window regardlessly
   * Each TroughPut-trigger, not the global, will have the length of the original trigger (a neccessity to maintain object-structure)
   * @param trigHier find triggers in there
   * @param configIDs the configured Trigger-IDs of thoese we gonna look for
   * @param tw_start the start time from which clipping commenses
   * @param tw_stop the stop time until which clipping commenses
   * @return a clipped trigger Hierarchy
   */
  I3TriggerHierarchy ClipTriggerHierarchy(const I3TriggerHierarchy &trigHier,
                                          const double tw_start,
                                          const double tw_stop,
                                          const std::vector<int> &configIDs = std::vector<int>());

  /** @brief Clip out the triggers with the given configIDs and within the specified TimeWindow
   * Take all triggers firing within the specified time-window (triggers can thereby extend beyond the specified time-window)
   * If no trigger-configIDs are specified take every trigger in the time-window regardlessly
   * Each TroughPut-trigger, not the global, will have the length of the original trigger (a neccessity to maintain object-structure)
   * @param trigHier find triggers in there
   * @param configIDs the configured Trigger-IDs of thoese we gonna look for
   * @param twindow the time window around which we gonna look for triggers
   * @return a clipped trigger Hierarchy
   */
  I3TriggerHierarchy ClipTriggerHierarchy(const I3TriggerHierarchy &trigHier,
                                          const I3TimeWindow& twindow,
                                          const std::vector<int> &configIDs = std::vector<int>());

  // =========================
  // Sneaky, Sneaky Lookup tables
  // =========================
  
  /**
   * @brief a two-dimentional map holding entries of type 'base'
   * 'internal' representation is a linear array of some art,
   * which can be evaluated in any field and typecast to 'base'
   * @tparam base a basic, or complex datatype
   * @tparam internal a vectorized array, which supports operator[]
   */
  template <typename base, class internal>
  class IndexMatrix {
  protected: //properties
    ///the size of the indexable range
    unsigned int biSize_;
    ///thingy holding information
    internal internal_;
    
    /// hidden constructor
    IndexMatrix (const unsigned int biSize, const unsigned int mapSize) :
      biSize_(biSize),
      internal_(mapSize) {};
  private:
    //forward declaration
    virtual unsigned int BiIndex_To_UniIndex(const unsigned int indexA, const unsigned int indexB) const = 0;

  public:
    /** @brief get the value for field
     * @param indexA this one
     * @param indexB and this one
     */
    inline base Get (const unsigned int indexA, const unsigned int indexB) const
      {return internal_[BiIndex_To_UniIndex(indexA, indexB)];};
    /** @brief set the value for field
     * @param indexA this one
     * @param indexB and this one
     * @param value to this value
     */
    inline void Set (const unsigned int indexA, const unsigned int indexB, const base value)
      {internal_[BiIndex_To_UniIndex(indexA, indexB)]=value;};
  };
  
  ///dynamic symmetric BiIndexed map; input is anything [0..x][0..x]; all fields are filled
  ///NOTE beware, there is no explicit check if you leave the indexable range, that is your responsibile
  template <typename base, class internal>
  class AsymmetricIndexMatrix : public IndexMatrix<base, internal> {
  private: // hidden methods
    ///convert from Bi-indexed representation to a linear index
    inline unsigned int BiIndex_To_UniIndex(const unsigned int indexA, const unsigned int indexB) const
      {return (indexA*IndexMatrix<base, internal>::biSize_)+indexB;};
  public:
    /**allocate enough memory
     * @param biSize that is the range of the biIndex
     */
    AsymmetricIndexMatrix<base, internal>(const int biSize) :
      IndexMatrix<base, internal>(biSize, biSize*biSize) {};
  };

  ///dynamic symmetric BiIndexed -map; input is anything [0..x][0..x]; only upper half and diagonal is filled
  ///NOTE beware, there is no explicit check if you leave the indexable range, that is your responsibility
  template <typename base, class internal> class SymmetricIndexMatrix : public IndexMatrix<base, internal> {
  private: // hidden methods
    ///convert from Bi-indexed representation to a linear index
    inline unsigned int BiIndex_To_UniIndex(const unsigned int indexA, const unsigned int indexB) const {
      if (indexB > indexA)
        return BiIndex_To_UniIndex(indexB, indexA);
      return (indexA*indexA+indexA)/2+indexB;
    };
  public:
    /** constructor
     * @param biSize that is the range of the biIndex
     */
    SymmetricIndexMatrix<base, internal> (const unsigned int biSize) :
      IndexMatrix<base, internal>(biSize, (biSize*biSize+biSize)/2) {};
  };

  //specialized classes for Bool and Double; watch out for filling the correct default values on init
  
  /// specialized class for Double
  class AsymmetricIndexMatrix_Double : public AsymmetricIndexMatrix<double, std::vector<double> > {
  public:
    //need constructor that initializes the fields to dafaultvalue : NAN
    AsymmetricIndexMatrix_Double (const unsigned int biSize) :
      AsymmetricIndexMatrix<double, std::vector<double> > (biSize) 
    {
      BOOST_FOREACH(double& field, internal_) {
        field = NAN;
      }
    };
  };
  
  /// specialized class for Double
  class SymmetricIndexMatrix_Double : public SymmetricIndexMatrix<double, std::vector<double> > {
  public:
    //need constructor that initializes the fields to dafaultvalue : NAN
    SymmetricIndexMatrix_Double (const int biSize) :
      SymmetricIndexMatrix<double, std::vector<double> > (biSize) 
    {
      BOOST_FOREACH(double& field, internal_) {
        field = NAN;
      }
    };
  };
  
  typedef AsymmetricIndexMatrix<bool, boost::dynamic_bitset<> > AsymmetricIndexMatrix_Bool;
  typedef SymmetricIndexMatrix<bool, boost::dynamic_bitset<> > SymmetricIndexMatrix_Bool;
}; //end namespace IceHiveHelpers

//============================= IMPLEMENTATIONS ==================

//================ ICEHIVE_HELPERS ======================
template <class Response>
I3Map<OMKey, std::vector<Response> >
IceHiveHelpers::GetPulsesInTimeRange (const I3Map<OMKey, std::vector<Response> > &pulses,
                                      const double tw_start,
                                      const double tw_stop) {
  typedef std::vector<Response> ResponseSeries;
  typedef I3Map<OMKey,ResponseSeries> ResponseSeriesMap;
  ResponseSeriesMap pulses_inRange;
  BOOST_FOREACH(const typename ResponseSeriesMap::value_type &omkey_pulseseries, pulses) {
    const OMKey& omkey = omkey_pulseseries.first;
    BOOST_FOREACH(const Response& pulse, omkey_pulseseries.second) {
      if (tw_start <= pulse.GetTime() && pulse.GetTime() <= tw_stop) {
        pulses_inRange[omkey].push_back(pulse);
      }
    }
  }
  return pulses_inRange;
};

#endif // ICEHIVEHELPERS_H
