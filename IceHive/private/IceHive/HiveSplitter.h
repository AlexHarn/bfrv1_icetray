/**
 * \file HiveSplitter.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: HiveSplitter.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 *
 * The central algorithm to split ResponseSeriesMaps by arguments of clustering
 */

#ifndef HIVESPLITTER_H
#define HIVESPLITTER_H

#include <limits>
#include <list>
#include <map>

#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3Geometry.h"

#include "IceHive/OMKeyHash.h"
#include "IceHive/HitSorting.h"
#include "IceHive/Hive-lib.h"
#include "IceHive/IceHiveHelpers.h"


/// A set of parameters that steer HiveSplitter
struct HiveSplitter_ParameterSet{
  /// PARAM: Required multiplicity of connected DOMs with hits in the time-window to form a subevent
  unsigned int multiplicity;
  /// PARAM: Time span within which the multiplicity requirement must be met
  double timeWindow;
  /// PARAM: Time within DOMs in vicinity are still considered
  double timeStatic;
  /// PARAM: Maximum negative deviation from speed of light travel time which will allow a pair of hits to be considered connected
  double timeCVMinus;
  /// PARAM: Maximum positive deviation from speed of light travel time which will allow a pair of hits to be considered connected
  double timeCVPlus;
  /// PARAM: Maximum negative deviation from light propagation time which will allow a pair of hits to be considered connected
  double timeCNMinus;
  /// PARAM: Maximum positive deviation from light propagation time which will allow a pair of hits to be considered connected
  double timeCNPlus;
  /// PARAM: Allow for DOMs to self-connect
  bool selfconnect;
  /// PARAM: limits for the single, double and tripple dense light connection
  Limits::RingLimits SingleDenseRingLimitPairs, DoubleDenseRingLimitPairs, TripleDenseRingLimitPairs;
  /// PARAM: limits for the single, double and tripple dense vicinity connection
  Limits::RingLimits SingleDenseRingVicinityPairs, DoubleDenseRingVicinityPairs, TripleDenseRingVicinityPairs;
  
  ///constructor
  HiveSplitter_ParameterSet();
};

// --- MACHINERY PARTS ---
//======================================
// Data Structures and Helper Functions
//======================================
namespace hivesplitter { namespace detail {
  ///An object which keeps track of a group of hits which are (mostly) causally connected to each other,
  ///and the number of distinct DOMs on which those hits occurred
  struct CausalCluster{
  //properties
    ///The ordered queue of hits within this cluster which are still within the time window of the current time
    std::list<HitSorting::Hit> hits;
    ///Keeps track of the number of hits on each of the doms present in this cluster, keys are dom indices
    std::map<unsigned int, unsigned int> doms;
    ///The hits which have formed a group surpassing the multiplicity and are now outside the time window
    HitSorting::HitSet complete;
    ///The most recent hit contributing to meeting the multiplicity condition
    HitSorting::Hit endHit; //!!!!
    ///Whether the multiplicity condition is met
    bool multiplicityMet;
  //methods
    ///constructor
    CausalCluster();
    ///Add a new hit to the cluster
    ///\param h The hit to add
    ///\param multiplicity The threshold for a group of hits within the time window to form a subevent
    void insert(const HitSorting::Hit &h, const unsigned int multiplicity);
    ///Finds the time of the earliest hit in this cluster
    ///\return The earliest hit time or infinity if the cluster is empty
    double earliestTime() const;
    ///Take all hits in other's complete list and merge them into this cluster's complete list
    ///\param c the cluster to be merged
    void takeComplete(const CausalCluster& c);
    ///is this cluster still active; thus can there still be found connected hits?
    bool isActive();
  };

  typedef std::list<CausalCluster> CausalClusterList;
  
  /**Test whether the hits in sub are a subset of super
   * @param sub cluster with certain hits which might be a subset
   * @param super cluster with a series of hits which might be a superset
   * @return true, if sub is a subset of super
   */
  bool isSubset(const CausalCluster& sub, const CausalCluster& super);
  
  ///wrapper around HitSets to establish timeorder in the set of SubEvents
  struct subEventTimeOrder {
    /// implement the order principle
    bool operator()(const HitSorting::HitSet &lhs, const HitSorting::HitSet &rhs) const {
      return (*lhs.begin() < *rhs.begin());
    }
  };
  
  typedef std::set<HitSorting::HitSet, subEventTimeOrder> HitSetSequence;
}// namespace detail
  
}// namespace hivesplitter


///The main splitter module
class HiveSplitter {
  SET_LOGGER("HiveSplitter");
private: //properties
  //==================
  // Properties
  //==================
  //initialized at configuration
  
  /// Geometry and Topology of the Hive that is describing the detector
  HiveGeometry hivegeo_;
  
  ///look-up table for dom-distances: if value==NAN, these DOMS should not be reconnected
  IceHiveHelpers::SymmetricIndexMatrix_Double distanceMap_;
  ///look-up table for dom-vicinity: if value==false, these DOMS should not be reconnected
  IceHiveHelpers::SymmetricIndexMatrix_Bool vicinityMap_;

  //initialized during runtime
  ///all in-progress causal clusters
  std::list<hivesplitter::detail::CausalCluster> clusters_;
  ///temporary storage for causal clusters generated while adding a single hit
  std::list<hivesplitter::detail::CausalCluster> newClusters_;
  ///all in-progress subevents
  std::list<HitSorting::HitSet> partialSubEvents_;
  
  ///set of completed subevents which are in time-order (in every aspect)
  hivesplitter::detail::HitSetSequence subEvents_;

protected: //parameters
  //========================
  // Configurable Parameters
  //========================
  /// PARAM: A parameter-set to run on
  HiveSplitter_ParameterSet params_;
  /// even better accessable limit parameter format for a whole group of parameters
  typedef std::map<unsigned int, Limits::RingLimits> RingLimitPairsMap;
  /// The RingLimits for light and particle propagation
  RingLimitPairsMap RingLimitPairsMap_;
  /// The RingLimits for displacement
  RingLimitPairsMap RingVicinityPairsMap_;

public: //interface
  //================
  // Main Interface
  //================
  /// Constructor from a ParameterSet
  HiveSplitter(const HiveSplitter_ParameterSet& params = HiveSplitter_ParameterSet());

  /// (Re)configure the module
  void Configure(const HiveSplitter_ParameterSet& params);

  /// Probe the feasibility of parameter ranges and do setup work
  void CheckParams_n_Setup();

  /** @brief Build the DistanceMap and VicinityMap from the parameters
   * keep things simple and call this function from Geometry-frame
   * @param geo pointer to the I3Geometry from Geometry frame
   */
  void BuildLookUpTables(const I3Geometry &geo);

  /** @brief ACTION
   * Perform the Splitting:<BR>
   * Get the Pulse-Series from the Frame and read all Hits into a time ordered Hit-Series.<BR>
   * Than call the Main-Algorithm and iterate over all the hits, try to form clusters of Hits and add Hits to them, discard them or form a new cluster.<BR>
   * When Hit-Series is exhausted and Clusters/Subevents have been found write them to the data-stream (either as separate frames or into the same frame).<BR>
   * Save the SplitCount also if that is wanted,<BR>
   * Push everything back into te pipeline.<BR>
   * Clean-up the memory.<BR>
   * @param hits the hits to process
   * @return a series of hits, which are the subevents (timeorder in sequence and in hit-order)
   */
  HitSorting::HitSeriesSeries Split (const HitSorting::HitSeries &hits);

private: // --- THE REAL MACHINERY ---
  /* Structural Overview:
    *
    *                          +----------+
    *                          | AddHit() |
    *                          +----------+
    *                               |
    *                +--------------+--------------+
    *                |                             |
    *                v                             v
    * +------------------------------+   +-------------------+
    * |    AdvanceClusterinTime()    |   | AddHitToCluster() |
    * +------------------------------+   +-------------------+
    *                |
    *                v
    *        +---------------+
    *        | AddSubEvent() |
    *        +---------------+
    *
    */

  /** @brief Determine whether two hits are related
    * Two hits are compared, where h1 should be the earlier hit, ortherwise they are switched (self-correcting).
    * First is checked if h1 has h2 in the inclusion volume,
    * Than the causal connection determined by the Norm is imposed
    * @param h1 the earlier hit, to be compared
    * @param h2 the later hit, to be compared
    * @return true, if Hit1 is causally connected to Hit2;
    * false, if not
    */
  bool CausallyConnected(const HitSorting::Hit& h1, const HitSorting::Hit& h2);
  
  /**The main driver for the entire algorithm:
   * Adds a new hit to all clusters with which it is connected (including subsets of existing clusters).
   * By 'advancing' the clusters this function also causes subevents to be built when possible.
   * @param h the hit to add
   */
  void AddHit(const HitSorting::Hit &h);
  
  /**Move this cluster forward in time to t, dropping hits which are no longer within the time window,
   * the request to merge clusters is accounted for
   * @param cluster the cluster that is worked on
   * @param time The current time to which the cluster should be moved
   */
  void AdvanceClusterInTime(hivesplitter::detail::CausalCluster& cluster, const double time);

  /** Attempt to add Hit h to existing cluster c, or to the subset of c with which it is connected by enough hits in c
   * to meet the multiplicity condition.
   * @param c the cluster to add to
   * @param h the hit to add
   * @param multiplicity this multiplicity of connected DOMs minus 1 in the subgroup must be met!
   * @return true, if h was added to c, or to a new subset of c;
   *	       false, if h was not placed in any cluster
   */
  bool AddHitToCluster(hivesplitter::detail::CausalCluster& c,
                       const HitSorting::Hit& h,
                       const unsigned int multiplicity);

  /** Inserts a cluster of hits into the set of subevents, after merging it with any existing subevents
   * with which it shares at least one hit
   * This function also moves any subevents which can no longer grow into the finished subevent collection.
   * \param newSet the set to add
   */
  void AddSubEvent(HitSorting::HitSet& newSet);

  /** Pushes all hits through the clusters and completes all subevents,
   * on the assumption that no more future hits will be added.
   */
  void FinalizeSubEvents();
};

#endif
