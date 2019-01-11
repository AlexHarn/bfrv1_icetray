/**
 * \file TriggerSplitter.cxx
 *
 * copyright (c) 2011
 * the IceCube Collaboration
 * $Id$
 *
 * @date $Date: 2013-10-22$
 * @author mzoll <marcel.zoll@fysik.su.se>
 *
 * A modular algorithm as an rewrite from project TriggerSplitter by Naoko
 */

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign.hpp>
#include <icetray/I3Units.h>
#include <dataclasses/I3Double.h>
#include <dataclasses/I3TimeWindow.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <dataclasses/TriggerKey.h>
#include <dataclasses/physics/I3Trigger.h>
#include <dataclasses/physics/I3TriggerHierarchy.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cfloat>

#include <boost/assign.hpp>

#include "IceHive/TriggerSplitter.h"
#include "IceHive/HitSorting.h"
#include "IceHive/IceHiveHelpers.h"


const double TriggerSplitter::default_offset = 200.*I3Units::ns; 
const double TriggerSplitter::default_clockcycle = 25.*I3Units::ns;


static const int trigIDs[] = {1006, 1007, 1011, 21001}; //that are SMT8, SMT3(DC), String and Volume Trigger
TriggerSplitter_ParameterSet::TriggerSplitter_ParameterSet():
  configIDs(trigIDs, trigIDs + sizeof(trigIDs)/ sizeof(trigIDs[0]) ),
  noSplitDt(10000./I3Units::ns),
  tWindowMinus(4000./I3Units::ns),
  tWindowPlus(6000./I3Units::ns)
{};

TriggerSplitter_ParameterSet::TriggerSplitter_ParameterSet(
  const std::vector<int> &configIDs,
  const double noSplitDt,
  const double tWindowMinus,
  const double tWindowPlus
):
  configIDs(configIDs),
  noSplitDt(noSplitDt),
  tWindowMinus(tWindowMinus),
  tWindowPlus(tWindowPlus)
{};

// ==================== CLASS TriggerSplitter ======================

TriggerSplitter::TriggerSplitter(const TriggerSplitter_ParameterSet& params):
  configIDs_(params.configIDs),
  noSplitDt_(params.noSplitDt),
  tWindowMinus_(params.tWindowMinus),
  tWindowPlus_(params.tWindowPlus) {
  CheckParams_n_Setup();
};


void TriggerSplitter::Configure(const TriggerSplitter_ParameterSet& params) {
  configIDs_ = params.configIDs;
  noSplitDt_ = params.noSplitDt;
  tWindowMinus_ =params.tWindowMinus;
  tWindowPlus_ = params.tWindowPlus;

  CheckParams_n_Setup();
};


void TriggerSplitter::CheckParams_n_Setup() {
  if (noSplitDt_ < 0.) {
    log_fatal("DANGEROUS!!!! NoSplitDT is set to %f, a negative value."
    "There will be DUPLICATE pulses in different P-frames!!!", noSplitDt_);
  }
}


void TriggerSplitter::DefaultOffsetMap() {
  log_warn("Using default values. %f ns is used to GUESS the launch time pulse time offset.", default_offset);
  offsets_.clear();
  max_offset_ = default_offset;
  min_offset_ = default_offset;
}


void TriggerSplitter::BuildOffsetMap(const I3Calibration& calibration,
                                     const I3DetectorStatus& status) {
  log_info("Building OffsetMap");
  offsets_.clear();
  min_offset_ = DBL_MAX; //NOTE whats that?
  max_offset_ = -DBL_MAX;
  BOOST_FOREACH(const I3DOMStatusMap::value_type &dpair, status.domStatus) {
    const OMKey& omkey = dpair.first;

    if (OMKeyHash::IsHashable(omkey)) {
      I3DOMCalibrationMap::const_iterator c_it = calibration.domCal.find(omkey);

      if (c_it != calibration.domCal.end()) {
        double offset = GetLaunchDT(c_it->second, dpair.second);
        const OMKeyHash::SimpleIndex omhash = OMKeyHash::OMKey2SimpleIndex(omkey);
        offsets_[omhash] = offset;

        if (std::isfinite(offset) && offset < min_offset_)
          min_offset_ = offset;

        if (std::isfinite(offset) && offset > max_offset_)
          max_offset_ = offset;
      }
    }
    else {
        log_trace_stream("Ignoring " << omkey << ", which is not part of IceCube or IceTop.");
    }
  }
  log_info("Built OffsetMap");
}

/**
 * estimate the delay between launches (trigger) times and pulse times
 * This is an un-doing of what is done in I3WaveCalibrator::GetStartTime() to shift the times
 * In no way is this a general physical estimate of the various delays, and therefore,
 * this calculates a wavecalibrator setting dependent parameter.
 * fadc delay is included to account for the longest DT situation of having a pulse
 * start in the first FADC bin
 */
double TriggerSplitter::GetLaunchDT(const I3DOMCalibration &calib,
                                    const I3DOMStatus &status) const {
  const LinearFit &fit = calib.GetTransitTime();
  const double pmtHV = status.pmtHV/I3Units::V;
  const double transit_time = fit.slope/sqrt(pmtHV) + fit.intercept;
  return transit_time - calib.GetFADCDeltaT(); //fadc dt larger than atwd
};


HitSorting::HitSeriesSeries TriggerSplitter::Split (const I3TriggerHierarchy& trigHier,
                                                               const HitSorting::HitSeries& hits) {
  log_trace("Entering TriggerSplitter::TriggerSplitting()");

  std::vector<Trigs> SplitTrigs;
  Trigs trigs;
  for(I3TriggerHierarchy::iterator trigIter = trigHier.begin() ; trigIter != trigHier.end(); ++trigIter) {
    if(trigIter->GetTriggerKey().CheckConfigID()) {
      log_debug("Got trigger %i", trigIter->GetTriggerKey().GetConfigID());

      if( find(configIDs_.begin(), configIDs_.end(), trigIter->GetTriggerKey().GetConfigID()) != configIDs_.end() ) {
        trigs.time = trigIter->GetTriggerTime();
        trigs.length = trigIter->GetTriggerLength();
        SplitTrigs.push_back(trigs);
      }
    } else {
      log_debug("Got trigger %s NA", TriggerKey::GetTypeString(trigIter->GetTriggerKey().GetType()));
    }
  }

  // figure out chopping blocks
  log_debug("Got %i triggers designated as splitters", (int)SplitTrigs.size());
  if (SplitTrigs.size() == 0){ //nothing to do
    return HitSorting::HitSeriesSeries();
  }

  std::sort(SplitTrigs.begin(), SplitTrigs.end());

  std::vector<SubTrigger> subtriggers;
  double start = SplitTrigs[0].time-tWindowMinus_;
  double end = 0.0;
  for (unsigned int i=0; i<SplitTrigs.size(); ++i){
    const double endTimeNow = SplitTrigs[i].time + SplitTrigs[i].length;
    if (endTimeNow > end)
      end = endTimeNow; //current chopping block's trigger end
    if (i == SplitTrigs.size()-1){
      subtriggers.push_back(SubTrigger(start, end+tWindowPlus_, &offsets_, min_offset_, max_offset_));
    } else {
      const double startTimeNext = SplitTrigs[i+1].time;
      if (startTimeNext-end > noSplitDt_) {
        subtriggers.push_back(SubTrigger(start, end+tWindowPlus_, &offsets_, min_offset_, max_offset_));
        start = startTimeNext-tWindowMinus_;
      }
    }
  }

  HitSorting::HitSeriesSeries outhits_series;
  BOOST_FOREACH(SubTrigger &subtrigger, subtriggers) {
    outhits_series.push_back(subtrigger.GetPulses(hits));
  }

  return outhits_series;
}


HitSorting::HitSeries TriggerSplitter::SubTrigger::GetPulses(const HitSorting::HitSeries& hits) const {
  using namespace HitSorting;
  HitSorting::HitSeries outhits;
  
  for(HitSorting::HitSeries::const_iterator hits_iter= hits.begin(); hits_iter!= hits.end(); ++hits_iter) {
    double current_offset = default_offset;

    offset_map_t::const_iterator it = offsets_->find(hits_iter->GetDOMIndex());
    if (it != offsets_->end())
      current_offset = it->second;

    if ((hits_iter->GetTime() + current_offset + default_clockcycle >= start_time_) && (hits_iter->GetTime() + current_offset < end_time_)) {
      outhits.push_back(*hits_iter);
    }
  }

  return outhits;
};
