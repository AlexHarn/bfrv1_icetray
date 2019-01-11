/**
 * \file HiveCleaning.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: HiveCleaning.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 *
 * The central algorithm to split I3RecoPulseSeriesMaps by arguments of clustering
 */

#ifndef HIVESPLITTER_H
#define HIVESPLITTER_H

#include <limits>
#include <list>
#include <map>
#include <sstream>

#include "dataclasses/I3Map.h"
#include "dataclasses/geometry/I3Geometry.h"

#include "IceHive/OMKeyHash.h"
#include "IceHive/HitSorting.h"
#include "IceHive/Hive-lib.h"
#include "IceHive/IceHiveHelpers.h"


/// A set of parameters that steer HiveCleaning
struct HiveCleaning_ParameterSet{
  //parameters
  ///PARAM: A required Multiplicity to the number of surrounding hit DOMs
  int multiplicity;
  /// PARAM: Maximum negative time within DOMs in vicinity are still considered
  double timeStaticMinus;
  /// PARAM: Maximum positive time within DOMs in vicinity are still considered
  double timeStaticPlus;
  /// PARAM: Allow for DOMs to self-connect
  bool selfconnect;
  /// PARAM: limits for the single, double and tripple dense vicinity connection
  Limits::RingLimits SingleDenseRingVicinityPairs, DoubleDenseRingVicinityPairs, TripleDenseRingVicinityPairs;

  ///constructor
  HiveCleaning_ParameterSet();
};

///The main cleaning class
class HiveCleaning {
  SET_LOGGER("HiveCleaning");
private: //properties
  //==================
  // Member Variables
  //==================
  // created at configuration time
  /// Geometry and Topology of the Hive that is describing the detector
  HiveGeometry hivegeo_;
  ///look-up table for dom-distances: if value==NAN, these DOMS should not be reconnected
  IceHiveHelpers::SymmetricIndexMatrix_Bool vicinityMap_;

protected://parameters
  //========================
  // Configurable Parameters
  //========================
  /// A parameter-set to run on
  HiveCleaning_ParameterSet params_;
  /// even better accessable limit parameter format for a whole group of parameters
  typedef std::map<unsigned int, Limits::RingLimits> RingLimitPairsMap;
  /// The RingLimits for displacement
  RingLimitPairsMap RingVicinityPairsMap_;

public://methods
  //================
  // Main Interface
  //================
  /// Constructor from a ParameterSet
  HiveCleaning(const HiveCleaning_ParameterSet& params = HiveCleaning_ParameterSet());
  
  /// (Re)configure the module
  void Configure(const HiveCleaning_ParameterSet& params);

  /// Probe the feasibility of parameter ranges and do setup work
  void CheckParams_n_Setup();

  /** @brief Build the VicinityMap from the parameters
    * keep things simple and call this function from Geometry-frame
    * @param geo pointer to the I3Geometry from Geometry frame
    */
  void BuildLookUpTables(const I3Geometry& geo);

  /** @brief ACTION
  * Perform the Splitting:<BR>
  * Get the Pulse-Series from the Frame and read all Hits into a time ordered Hit-Series.<BR>
  * Than Call the Main-Alorithm and iterate over all thee Hits and try to form clusters of Hits and add Hits to them, discard them or form a new cluster.<BR>
  * When Hit-Series is exhausted and Clusters/Subevents have been found write them to the datastream (either as seperate frames or into the same frame).<BR>
  * Save the Splitcount also if that is wanted,<BR>
  * Push everyting back into the pipeline.<BR>
  * Clean-up the memory.<BR>
   * @param hits the hits to process on
   * @return a EventStartStops
   */
  HitSorting::HitSeries Clean (const HitSorting::HitSeries &hits);
};

#endif