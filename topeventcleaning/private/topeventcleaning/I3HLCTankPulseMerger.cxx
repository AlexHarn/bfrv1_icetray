/*
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id
 *
 * @version $Revision$
 * @date 20 Jan 2014
 * @author Fabian Kislat <fabian.kislat@desy.de>
 * @author Javier Gonzalez <javierg@udel.edu>
 * $LastChangedBy$
 * $LastChangedDate$
 */

#include "utility.h"
#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Frame.h>
#include <icetray/I3Units.h>
#include <dataclasses/I3Vector.h>
#include <dataclasses/TankKey.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3RecoPulse.h>

#include <boost/foreach.hpp>
#include <cassert>
#include <string>
#include <set>


typedef std::map<OMKey, I3DOMCalibration> I3DOMCalibrationMap;
typedef std::map<OMKey, I3DOMStatus> I3DOMStatusMap;
typedef std::map<OMKey, I3VEMCalibration> I3VEMCalibrationMap;


namespace {

  // internal use only
  struct TankPulse {
    OMKey om;
    I3RecoPulse pulse;
  };

  typedef std::map<TankKey, std::vector<TankPulse> > TankPulseSeriesMap;

}


class I3HLCTankPulseMerger : public I3ConditionalModule {

  SET_LOGGER("I3HLCTankPulseMerger");

public:
  I3HLCTankPulseMerger(const I3Context &ctx)
    : I3ConditionalModule(ctx),
      stream_(I3Frame::DAQ),
      inputVEMPulsesName_("IceTopHLCVEMPulses"),
      outputTankPulsesName_("IceTopHLCTankPulses"),
      maxHGLGTimeDiff_(40.0*I3Units::ns),
      badDOMListName_("IceTopBadDOMs"),
      badTankListName_("IceTopBadTanks"),
      excludedTanksName_("ExcludedHLCTanks")
  {
    AddParameter("InputVEMPulses", "Input I3RecoPulseSeriesMap", inputVEMPulsesName_);
    AddParameter("OutputTankPulses", "Output I3RecoPulseSeriesMap", outputTankPulsesName_);
    AddParameter("MaxHGLGTimeDiff", "Maximum time difference between high-gain and low-gain pulses when merging", maxHGLGTimeDiff_);
    AddParameter("BadDomList", "List of bad DOMs (optional)", badDOMListName_);
    AddParameter("BadTankList", "List of bad tanks (only used when merging tank pulses) (optional)", badTankListName_);
    AddParameter("ExcludedTanks", "List of tanks found during pulse merging (optional)", excludedTanksName_);
    AddParameter("Stream", "Frame type where to do the merging. Default is DAQ.", stream_);
    AddOutBox("OutBox");
  }

  void Configure();

  void DetectorStatus(I3FramePtr frame);

  void DAQ(I3FramePtr frame)
  { Merge(frame); }
  void Physics(I3FramePtr frame)
  { Merge(frame); }

private:
  void Merge(I3FramePtr frame);

  I3Frame::Stream stream_;

  std::string inputVEMPulsesName_;
  std::string outputTankPulsesName_;
  double maxHGLGTimeDiff_;
  std::string badDOMListName_;
  std::string badTankListName_;
  std::string excludedTanksName_;

  I3VectorOMKeyConstPtr badDOMList_;
  I3VectorTankKeyConstPtr badTankList_;
  std::set<OMKey> badDOMs_;
  std::set<TankKey> badTanks_;

  void FindHGandLG(I3RecoPulseSeriesMap &pulses,
		   const I3RecoPulseSeriesMap::iterator &iter,
		   const I3DOMStatusMap &status,
		   I3RecoPulseSeriesMap::iterator &hg,
		   I3RecoPulseSeriesMap::iterator &lg);
};


I3_MODULE(I3HLCTankPulseMerger);


void I3HLCTankPulseMerger::Configure()
{
  GetParameter("Stream", stream_);

  GetParameter("InputVEMPulses", inputVEMPulsesName_);
  if (inputVEMPulsesName_.empty()) log_fatal("You have to specify a name for the input pulse series.");

  GetParameter("OutputTankPulses", outputTankPulsesName_);
  if (outputTankPulsesName_.empty()) log_fatal("You have to specify a name for the output pulse series.");

  GetParameter("MaxHGLGTimeDiff", maxHGLGTimeDiff_);
  GetParameter("BadDomList", badDOMListName_);
  GetParameter("BadTankList", badTankListName_);
  GetParameter("ExcludedTanks", excludedTanksName_);

  if (excludedTanksName_.empty()) {
    log_warn("Parameter ExcludedTanks not set. "
	     "Tanks excluded in this processing step will not be written to the frame!");
  }
}


void I3HLCTankPulseMerger::DetectorStatus(I3FramePtr frame)
{
  if (!badDOMListName_.empty()) {
    I3VectorOMKeyConstPtr badDOMList =
      frame->Get<I3VectorOMKeyConstPtr>(badDOMListName_);
    if (badDOMList) {
      badDOMs_.clear();
      badDOMs_.insert(badDOMList->begin(), badDOMList->end());
    } else {
      log_warn("Bad DOM list '%s' not in DetectorStatus frame. Not updating.",
	       badDOMListName_.c_str());
    }
  }

  if (!badTankListName_.empty()) {
    I3VectorTankKeyConstPtr badTankList =
      frame->Get<I3VectorTankKeyConstPtr>(badTankListName_);
    if (badTankList) {
      badTanks_.clear();
      badTanks_.insert(badTankList->begin(), badTankList->end());
    } else {
      log_warn("Bad tank list '%s' not in DetectorStatus frame. Not updating.",
	       badTankListName_.c_str());
    }
  }

  PushFrame(frame);
}


void I3HLCTankPulseMerger::Merge(I3FramePtr frame)
{
  if (this->stream_ != frame->GetStop()) {
    PushFrame(frame);
    return;
  }

  typedef I3RecoPulseSeriesMap::iterator RPSMIter;

  static I3RecoPulseSeries empty_reco_pulse_series;   // just a dummy used below


  I3RecoPulseSeriesMapConstPtr inputVEMPulses =
    frame->Get<I3RecoPulseSeriesMapConstPtr>(inputVEMPulsesName_);

  if (!inputVEMPulses) {
    log_debug("Input pulses '%s' not in frame. Skipping.", inputVEMPulsesName_.c_str());
    PushFrame(frame);
    return;
  }

  // TODO: figure out a way to do this without copying
  I3RecoPulseSeriesMap pulses(*inputVEMPulses);

  const I3Calibration &calibration = frame->Get<I3Calibration>();
  const I3DetectorStatus &status = frame->Get<I3DetectorStatus>();

  TankPulseSeriesMap tankPulses;
  std::set<TankKey> badTanks(badTanks_);
  for (RPSMIter iter = pulses.begin(); iter != pulses.end(); ++iter) {
    const OMKey &omKey = iter->first;

    log_debug_stream("DOM " << omKey);

    TankKey tankKey(omKey);
 
    if (badTanks.find(tankKey) != badTanks.end()) continue;  // already marked bad
    
    if (tankPulses.find(tankKey) != tankPulses.end()) continue;  // done with that tank
    
    RPSMIter hg = pulses.end(), lg = pulses.end();
    FindHGandLG(pulses, iter, status.domStatus, hg, lg);

    if (hg == pulses.end() && lg == pulses.end()) {
      // there is already an error message in the FindHGandLG function if this happens
      log_debug("I will skip this tank because I3TopHLCPulseExtractor::FindHGandLG failed.");
      continue;
    }

    if (((lg != pulses.end()) && (hg == pulses.end())) || ((lg->second.size() > 0) && (hg->second.size() == 0))) {
      log_debug_stream("LG DOM " << lg->first << " has no HG partner. Excluding the whole tank!");
      badTanks.insert(tankKey);
      continue;
    }

    // at this point there has to be a HG pulse
    // this could have been checked more directly earlier on, but was done differently
    // for more useful debug messages
    assert(hg != pulses.end());
    OMKey hgOM = hg->first;

#ifndef NDEBUG
    if (lg != pulses.end())
      log_debug_stream("    HG: " << hgOM << ", LG: " << lg->first);
    else
      log_debug_stream("    HG: " << hgOM << ", no LG");
#endif

    I3RecoPulseSeries &hg_rps = hg->second;
    I3RecoPulseSeries &lg_rps = lg != pulses.end() ? lg->second : empty_reco_pulse_series;

    const I3VEMCalibration *vemCalib = topeventcleaning::get_dom_info(hgOM, calibration.vemCal);
    if (!vemCalib)
      log_fatal_stream("No VEM calibration for module " << hgOM);
    double pe_per_vem = vemCalib->pePerVEM/vemCalib->corrFactor;
    double hglg_crossover = vemCalib->hglgCrossOver/pe_per_vem;

    // Do it like topeventbuilder: use fallback cross-over value if negative or zero value in DB
    if (hglg_crossover <= 0) hglg_crossover = 3000./pe_per_vem; 
    
    bool tank_is_bad = false;
    
    // this may be a little inefficient if pulses are sorted in time
    BOOST_FOREACH(const I3RecoPulse &hg_pulse, hg_rps) {
      if (!std::isfinite(hg_pulse.GetTime())) {
        log_debug_stream("Invalid time in HG DOM " << hgOM << ": " << hg_pulse.GetTime() << ". Marking tank as bad.");
        tank_is_bad = true;
        break;
      }
      
      // Check for HG pulses with charge exactly 0 and width either 0 or 1.
      // Charge 0, width 0 pulses are produced for (drooped) waveforms, where the whole waveform is < 0 V
      // sDST will assign these pulses a width of 1
      if (hg_pulse.GetCharge() == 0.0 && (hg_pulse.GetWidth() == 0.0 || hg_pulse.GetWidth() == 1.0)) {
        log_debug_stream("Found pulse with charge " << hg_pulse.GetCharge() << " and width " << hg_pulse.GetWidth() << " in " << hgOM << ". Marking tank as bad.");
        tank_is_bad = true;
        break;
      }

      I3RecoPulseSeries::iterator lg_match = lg_rps.end();
      double min_deltaT = maxHGLGTimeDiff_;
      for (I3RecoPulseSeries::iterator lg_iter = lg_rps.begin(); lg_iter != lg_rps.end(); ++lg_iter) {
        double deltaT = std::fabs(hg_pulse.GetTime() - lg_iter->GetTime());
        if (deltaT < min_deltaT) {
          min_deltaT = deltaT;
          lg_match = lg_iter;
        }
      }

      TankPulse tankSignal = { hgOM, hg_pulse };
      if (!std::isfinite(hg_pulse.GetCharge()) || (hg_pulse.GetCharge() > hglg_crossover)) {
        if (lg_match != lg_rps.end()) {
          tankSignal.om = lg->first;
          tankSignal.pulse.SetCharge(lg_match->GetCharge());
        } else {
          tankSignal.pulse.SetCharge(NAN);
        }
      }

      if (lg_match != lg_rps.end()) lg_rps.erase(lg_match);

      if (tankSignal.pulse.GetCharge() < 0) {
        // log_warn("Discarding pulse with negative charge in tank %s (DOM %s). Probably afterpulse?!",
        // tankKey.str().c_str(), tankSignal.om.str().c_str());
      } else {
        tankPulses[tankKey].push_back(tankSignal);
      }
    }

    if (lg_rps.size() > 0)
      log_trace_stream("WARN: There are " << lg_rps.size() << " LOW gain pulses without HIGH gain partners in Tank " << tankKey << ". Discarding all of them!");
    
    if (tank_is_bad) {
      badTanks.insert(tankKey);
      
      TankPulseSeriesMap::iterator i = tankPulses.find(tankKey);
      if (i != tankPulses.end())
        tankPulses.erase(i);
    }
  }

  I3RecoPulseSeriesMapPtr outputTankPulses(new I3RecoPulseSeriesMap);
  for (TankPulseSeriesMap::const_iterator iter = tankPulses.begin(); iter != tankPulses.end(); ++iter) {
    BOOST_FOREACH(const TankPulse &tp, iter->second) {
      (*outputTankPulses)[tp.om].push_back(tp.pulse);
    }
  }

  I3VectorTankKeyPtr excludedTanks(new I3VectorTankKey(badTanks.begin(), badTanks.end()));

  frame->Put(outputTankPulsesName_, outputTankPulses);
  if (!excludedTanksName_.empty()) frame->Put(excludedTanksName_, excludedTanks);

  PushFrame(frame);
}


void I3HLCTankPulseMerger::FindHGandLG(I3RecoPulseSeriesMap &pulses,
				       const I3RecoPulseSeriesMap::iterator &iter,
				       const I3DOMStatusMap &status, 
				       I3RecoPulseSeriesMap::iterator &hg,
				       I3RecoPulseSeriesMap::iterator &lg)
{
  OMKey key1 = iter->first;
  OMKey key2 = key1;
  if (key1.GetOM() % 2 == 1) // 61 or 63
    key2.SetOM(key2.GetOM()+1);
  else                            // 62 or 64
    key2.SetOM(key2.GetOM()-1);

  const I3DOMStatus *status1 = 0;
  if (badDOMs_.find(key1) == badDOMs_.end()) // key1 is not on the BadDOMList
    status1 = topeventcleaning::get_dom_info(key1, status);

  const I3DOMStatus *status2 = 0;
  if (badDOMs_.find(key2) == badDOMs_.end()) // key2 is not on the BadDOMList
    status2 = topeventcleaning::get_dom_info(key2, status);
  I3DOMStatus::DOMGain gain1 = status1 ? status1->domGainType : I3DOMStatus::UnknownGainType;
  I3DOMStatus::DOMGain gain2 = status2 ? status2->domGainType : I3DOMStatus::UnknownGainType;

  // Make sure that there is at least one HG DOM (with known gain type) in each tank
  if (gain1 != I3DOMStatus::High && gain2 != I3DOMStatus::High) {
    log_error_stream("No high-gain DOM in tank " << TankKey(key1) << "!");
    return;
  }

  if (gain1 == gain2) {
    log_error_stream("Both DOMs in tank " << TankKey(key1) << " have the same gain!");
    return;
  }

  hg = (gain1 == I3DOMStatus::High ? iter : pulses.find(key2));
  lg = (gain1 == I3DOMStatus::Low ? iter : pulses.find(key2));
}
