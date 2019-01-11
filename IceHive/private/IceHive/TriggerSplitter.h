/**
 * \file TriggerSplitter.h
 *
 * copyright (c) 2011
 * the IceCube Collaboration
 * $Id: TriggerSplitter.cxx 94325 2012-10-20 15:13:28Z mzoll $
 *
 * @date $Date: 2013-10-22$
 * @author mzoll <marcel.zoll@fysik.su.se>
 *
 * A modular algorithm as an rewrite from project TriggerSplitter by naoko
 */

#ifndef TRIGGERSPLITTER_H
#define TRIGGERSPLITTER_H

#include "dataclasses/TriggerKey.h"
#include "dataclasses/physics/I3Trigger.h"
#include "dataclasses/physics/I3TriggerHierarchy.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "icetray/I3Units.h"

#include "IceHive/OMKeyHash.h"
#include "IceHive/HitSorting.h"

/// A collection of parameters for the TriggerSplitter
struct TriggerSplitter_ParameterSet {
  //properties
  /// the configuration-IDs of the triggers which should be considered
  std::vector<int> configIDs;
  /// A padding: when two (same) triggers are close by this value in time, they won't be split
  double noSplitDt;
  /// A padding in front which will be added to the trigger time
  double tWindowMinus;
  /// A padding in the back which will be added to the trigger time
  double tWindowPlus;
  //methods
  /// Constructor : parameterless
  TriggerSplitter_ParameterSet();
  /// Constructor : explicit
  TriggerSplitter_ParameterSet(
    const std::vector<int> &configIDs,
    const double noSplitDt,
    const double tWindowMinus,
    const double tWindowPlus
  );
};


/// The TriggerSplitter Module
/// with the functionality tp split up RecoPulses by trigger information
class TriggerSplitter {
  SET_LOGGER("TriggerSplitter");

public: //some static global variables (former magic numbers)
  ///Generic offset time between DOMLaunch time and pulse
  static const double default_offset; 
  ///25 ns refers to FPGA clock cycle
  static const double default_clockcycle;
  
protected://parameters
  //========================
  // Configurable Parameters
  //========================
  /// PARAM: list of config ids of all the triggers to split on
  std::vector<int> configIDs_;
  /// PARAM: a lower bound on the separation of each trigger
  double noSplitDt_;
  /// PARAM: the negative padding of triggers (TriggerWindow)
  double tWindowMinus_;
  /// PARAM: the positive padding of triggers (TriggerWindow)
  double tWindowPlus_;

public://methods
  /// Constructor from a ParameterSet
  TriggerSplitter(const TriggerSplitter_ParameterSet& params = TriggerSplitter_ParameterSet());
  /// (Re)configure the module
  void Configure(const TriggerSplitter_ParameterSet& params);
  /// Probe the feasibility of parameter ranges and do setup work
  void CheckParams_n_Setup();
  ///Builds blank offset nmap with default-values for all further operation
  void DefaultOffsetMap();
  /// Builds centrally needed objects of the module at configuration time
  void BuildOffsetMap(const I3Calibration& calibration,
                      const I3DetectorStatus& status);
  /** @brief Perform the Splitting
   * @param trigHier the TriggerHierarchy, which is used as the template fo splitting the pulses
   * @param hits the hits which are about to be split
   * @return a series of subevents which got split; not neccessarily containing all the pulses from the original series
   */
  HitSorting::HitSeriesSeries Split(const I3TriggerHierarchy &trigHier,
                                    const HitSorting::HitSeries& hits);
private: //methods
  ///subroutine for CalcLaunchedDTs
  double GetLaunchDT(const I3DOMCalibration &calib,
                     const I3DOMStatus &status) const;
private: //internals
  //convenience for internal use
  typedef std::map<OMKeyHash::SimpleIndex, double> offset_map_t;
  ///a map to hold all offsets per DOM
  offset_map_t offsets_;
  /// holds the global maximum and mimimum offsets
  double min_offset_, max_offset_;

  /// a simplifed Trigger representation
  struct Trigs {
    /// start time of the trigger firing
    double time;
    /// length of the trigger
    double length;
    /// comparision operator for time-ordering
    bool inline operator<(const Trigs &other) const
      {return time < other.time;};
  };

  /// A reduced readout window, with functions to trim
  class SubTrigger {
  private:
    double start_time_;
    double end_time_;
    offset_map_t *offsets_;
    double min_offset_, max_offset_;
  public:
    ///constructor : explicit
    SubTrigger(const double start,
               const double end,
               offset_map_t *offsets,
               const double min_offset,
               const double max_offset) :
      start_time_(start),
      end_time_(end),
      offsets_(offsets),
      min_offset_(min_offset),
      max_offset_(max_offset)
    {};
    /** @brief Select pulses that fall within this subtrigger
     * @param hits the hits which contain this subwindow
     * @return the subevent wich is enveloped by this trigger
     */
    HitSorting::HitSeries GetPulses(const HitSorting::HitSeries& hits) const;
  };
};


#endif
