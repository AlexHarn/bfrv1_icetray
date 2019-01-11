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
 * The central algorithm to split I3RecoPulseSeriesMaps by arguments of clustering
 */

#ifndef HIVESPLITTER_H
#define HIVESPLITTER_H

#include <cassert>
#include <limits>
#include <list>
#include <map>
#include <sstream>

#include "icetray/I3Units.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include <boost/make_shared.hpp>

#define MAX_RINGS 2 //DANGER HARDCODING

#include "HiveSplitter/OMKeyHash.h"
#include "HiveSplitter/HitSorting.h"


/// A set of parameters that steer HiveSplitter
struct HiveSplitter_ParameterSet{
  //parameters
  unsigned int multiplicity_;
  double timeWindow_;
  double timeConeMinus_;
  double timeConePlus_;
  bool domSpacingsOpt_;
  std::vector<double> SingleDenseRingLimits_, DoubleDenseRingLimits_, TrippleDenseRingLimits_;
  uint modeOpt_;
  //constructor
  HiveSplitter_ParameterSet();
};

/// A container to deliver subevents
struct SubEventStartStop{
  I3RecoPulseSeriesMap subevent_;
  double start_time_;
  double stop_time_;

  SubEventStartStop(const I3RecoPulseSeriesMap &subevent, const double start_time, const double stop_time):
    subevent_(subevent),
    start_time_(start_time),
    stop_time_(stop_time) {};
};

///The main splitter module
class HiveSplitter {
private: // --- MACHINERY PARTS ---
	//======================================
	// Data Structures and Helper Functions
	//======================================
	/// Topology helper function
	enum DOMTopo {IceTop=0, IceCube=1, DeepCore=2, Pingu=4};
	///An object which keeps track of a group of hits which are (mostly) causally connected to each other, 
	///and the number of distinct DOMs on which those hits occurred
	struct CausalCluster{
		///The ordered queue of hits within this cluster which are still within the time window of the current time
		std::list<HitSorting::Hit> hits;
		///Keeps track of the number of hits on each of the doms present in this cluster, keys are dom indices
		std::map<unsigned int, unsigned int> doms;
		///The hits which have formed a group surpassing the multiplicity and are now outside the time window
		HitSorting::TimeOrderedHitSet complete;
		///The (DOM)index of the most recent hit contributing to meeting the multiplicity condition
		unsigned int endDom;
		///The (Pulse)index of the most recent hit contributing to meeting the multiplicity condition
		unsigned int endPulse;
		///Whether the multiplicity condition is met
		bool multiplicityMet;
		///constructor
		CausalCluster();
		///Add a new hit to the cluster
		///\param h The hit to add
		///\param multiplicity The threshold for a group of hits within the time window to form a subevent
		void insert(HitSorting::Hit h, unsigned int multiplicity);
		///Finds the time of the earliest hit in this cluster
		///\return The earliest hit time or infinity if the cluster is empty
		double earliestTime() const;
		///Move this cluster forward in time to t, dropping hits which are no longer within the time window
		///\param time The current time to which the cluster should be moved
		///\param splitter_inst The owning splitter
		bool advanceTime(const double time, HiveSplitter& splitter_inst);
		///Take all hits in other's complete list and merge them into this cluster's complete list
		///\param c the cluster to be merged
		void takeComplete(const CausalCluster& c);
	};

	///Test whether c1 is a subset of c2
	static bool isSubset(const CausalCluster& c1, const CausalCluster& c2);
	/** @brief Determine whether two hits are related
	 * Two hits are compared, where h1 should be the earlier hit, ortherwise they are switched (self-correcting).
	 * First is checked if h1 has h2 in the inclusion volume,
	 * Than the causal connection determined by the Norm is imposed
	 * @param h1 the earlier hit, to be compared
	 * @param h2 the later hit, to be compared
	 * @return true, if Hit1 is causally connected to Hit2;
	 * false, if not 
	 */
	bool causallyConnected(const HitSorting::Hit& h1, const HitSorting::Hit& h2);
	
	//==================
	// Member Variables
	//==================
	//static
	///look-up table for dom-distances: if value==NAN, these DOMS should not be reconnected
	double DistanceMap_[(MAX_STRINGS*MAX_OMS)][(MAX_STRINGS*MAX_OMS)];
	
	//during runtime
	///all in-progress causal clusters
	std::list<CausalCluster> clusters_;
	///temporary storage for causal clusters generated while adding a single hit
	std::list<CausalCluster> newClusters_;
	///all in-progress subevents
	std::list<HitSorting::TimeOrderedHitSet> partialSubEvents_;
	///helper to establish timeorder in the set of SubEvents
	struct subEventTimeOrder {
	  bool operator()(const HitSorting::TimeOrderedHitSet &lhs, const HitSorting::TimeOrderedHitSet &rhs) const {
	    if (lhs.begin()->time == rhs.begin()->time) //break the tie
	      return lhs.begin()->domIndex < rhs.begin()->domIndex;
	    return lhs.begin()->time < rhs.begin()->time;}
	};
	///container for Subevents
	typedef std::set<HitSorting::TimeOrderedHitSet, subEventTimeOrder> TimeOrderedSubEvents;
	///set of completed subevents which are in time-order (in every aspect)
	TimeOrderedSubEvents subEvents_;

protected:
  //========================
  // Configurable Parameters
  //========================

  /// PARAM: Required multiplicity of a subevent
  unsigned int multiplicity_;
  /// PARAM: Time span within which the multiplicity requirement must be met
  double timeWindow_;
  /// PARAM: Maximum negative deviation from speed of light travel time which will allow a pair of hits to be considered connected
  double timeConeMinus_;
  /// PARAM: Maximum positive deviation from speed of light travel time which will allow a pair of hits to be considered connected
  double timeConePlus_;

  /// PARAM: are RingLimits parsed as Meter [True] distances or DOM spacings [False]
  bool domSpacingsOpt_;
  /// parameter-format for UI parsing
  typedef std::vector<double> RingLimits;
  /// PARAM: parameters for UI parsing
  RingLimits SingleDenseRingLimits_, DoubleDenseRingLimits_, TrippleDenseRingLimits_;
  /// better accessable limit parameter format
  typedef std::vector<std::pair<double, double> > RingLimitPairs;
  /// better accessable limit format
  RingLimitPairs SingleDenseRingLimitPairs_, DoubleDenseRingLimitPairs_, TrippleDenseRingLimitPairs_;

  /// PARAM: The Norm to use in causallyConnect
  enum Mode {Lorentz=1, Euclidean=2, Static=3, Advanced=4};
  /// PARAM: The Norm that is to be configured
  uint modeOpt_;
  ////Option to artifically make DOMs above and below the Dustlayer eligible
  //connectTroughDustOpt_;

public:
  //================
  // Main Interface
  //================
  /// Constructor from a ParameterSet
  HiveSplitter(const HiveSplitter_ParameterSet& params = HiveSplitter_ParameterSet());

  /// (Re)configure the module
  void Configure(const HiveSplitter_ParameterSet& params);
  
  /// Probe the feasibility of parameter ranges and do setup work
  void CheckParams_n_Setup();

  /** @brief Build the DistanceMap from the parameters
    * keep things simple and call this function from Geometry-frame
    * @param geo pointer to the I3Geometry from Geometry frame
    */
  void BuildDistanceMap(I3GeometryConstPtr geo);
  
  /** @brief ACTION
  * Perform the Splitting:<BR>
  * Get the Pulse-Series from the Frame and read all Hits into a time ordered Hit-Series.<BR>
  * Than Call the Main-Alorithm and iterate over all thee Hits and try to form clusters of Hits and add Hits to them, discard them or form a new cluster.<BR>
  * When Hit-Series is exhausted and Clusters/Subevents have been found write them to the datastream (either as seperate frames or into the same frame).<BR>
  * Save the Splitcount also if that is wanted,<BR>
  * Push everyting back into te pipeline.<BR>
  * Clean-up the memory.<BR>
   * @param pulses the pulses to process on
   * @return a series of subEventStartStops
   */
  std::vector<SubEventStartStop> HiveSplitting (const I3RecoPulseSeriesMap &pulses);
  
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
    * | CausalCluster::advanceTime() |   | AddHitToCluster() |
    * +------------------------------+   +-------------------+
    *                |
    *                v
    *        +---------------+
    *        | AddSubEvent() |
    *        +---------------+
    *
    */

  ///The main driver for the entire algorithm:
  ///Adds a new hit to all clusters with which it is connected (including subsets of existing clusters).
  ///By 'advancing' the clusters this function also causes subevents to be built when possible.
  ///\param h the hit to add
  void AddHit(const HitSorting::Hit h);

  ///Attempt to add Hit h to existing cluster c, or to the subset of c with which it is connected by enough hits in c
  ///to meet the multiplicity condition.
  ///\param c the cluster to add to
  ///\param h the hit to add 
  ///\return true, if h was added to c, or to a new subset of c,
  ///	     false, if h was not placed in any cluster
  bool AddHitToCluster(CausalCluster& c, const HitSorting::Hit h);

  ///Inserts a cluster of hits into the set of subevents, after merging it with any existing subevents
  ///with which it shares at least one hit
  ///This functon also moves any subevents which can no longer grow into the finished subevent collection.
  ///\param newSet the set to add
  void AddSubEvent(HitSorting::TimeOrderedHitSet& newSet);

  ///Pushes all hits through the clusters and completes all subevents,
  ///on the assumption that no more future hits will be added.
  void FinalizeSubEvents();

  /** @brief Distance between these two DOMs read from Look-Up table
   * @param omkey_a index of the first DOM, corresponding to rows: matrix_x
   * @param omkey_b index of the second DOM, corresponding to columns: matrix_y
   * @return NAN, if there is not connection between these DOMs;
   * (positive double), the distance
   */
  inline double GetDistance (const OMKey& omkey_a, const OMKey& omkey_b)
    {return DistanceMap_[OMKeyHash::OMKey2SimpleIndex(omkey_a)][OMKeyHash::OMKey2SimpleIndex(omkey_b)];};

  /** @brief Distance between these two DOMs read from Look-Up table
   * @param simpleindex_a index of the first DOM, corresponding to rows: matrix_x
   * @param simpleindex_b index of the first DOM, corresponding to columns: matrix_y
   * @return NAN, if there is not connection between these DOMs
   * (positive double), the distance
   */
  inline double GetDistance (const OMKeyHash::SimpleIndex simpleindex_a, const OMKeyHash::SimpleIndex simpleindex_b)
    {return DistanceMap_[simpleindex_a][simpleindex_b];};
};

#endif