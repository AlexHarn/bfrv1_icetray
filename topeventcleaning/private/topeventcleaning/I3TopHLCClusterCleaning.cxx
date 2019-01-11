/*
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de> Last changed by: $LastChangedBy$
 */

#include "utility.h"
#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Units.h>
#include <icetray/I3Context.h>
#include <icetray/I3Frame.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3Constants.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <dataclasses/I3Position.h>
#include <dataclasses/TankKey.h>
#include <dataclasses/I3Vector.h>
#include <phys-services/I3Splitter.h>

#include <boost/foreach.hpp>
#include <algorithm>
#include <cassert>
#include <set>
#include <string>
#include <sstream>


namespace {

  struct TankSignal {
    OMKey key;
    const I3RecoPulse *pulse;
    
    TankSignal(const OMKey &k, const I3RecoPulse &p)
      : key(k), pulse(&p) {
    }
  };

  typedef std::vector<TankSignal> TankSignalSeries;

  bool time_sort_tanks(const TankSignal &lhs,
			  const TankSignal &rhs)
  {
    return lhs.pulse->GetTime() < rhs.pulse->GetTime();
  }

  I3Position get_tank_position(const I3Geometry &geo, const OMKey &key)
  {
    // We assume that the geometry is sane...
    I3StationGeoMap::const_iterator station_geo = geo.stationgeo.find(key.GetString());
    BOOST_FOREACH(const I3TankGeo &tank_geo, station_geo->second) {
      if (std::find(tank_geo.omKeyList_.begin(), tank_geo.omKeyList_.end(), key) != tank_geo.omKeyList_.end())
        return tank_geo.position;
    }
    log_fatal_stream("No tank position found for DOM " << key);
  }

}


class I3TopHLCClusterCleaning : public I3ConditionalModule, private I3Splitter {

  SET_LOGGER("I3TopHLCClusterCleaning");

public:
  I3TopHLCClusterCleaning(const I3Context &ctx)
    : I3ConditionalModule(ctx), I3Splitter(configuration_)
  {
    inputPulsesName_ = "IceTopHLCTankPulses";
    AddParameter("InputPulses", "Input pulse series", inputPulsesName_);
    outputMaskName_ = "CleanedHLCTankPulses";
    AddParameter("OutputPulses", "Output pulse series mask", outputMaskName_);
    badTankListName_ = "IceTopBadTanks";
    AddParameter("BadTankList", "Name of the bad tank list", badTankListName_);
    excludedTanksName_ = "ClusterCleaningExcludedTanks";
    AddParameter("ExcludedTanks", "Name of the list of tanks excluded by this module", excludedTanksName_);
    interStationTimeTolerance_ = 200*I3Units::ns;
    AddParameter("InterStationTimeTolerance", "Time tolerance between stations before event is split", interStationTimeTolerance_);
    intraStationTimeTolerance_ = 200*I3Units::ns;
    AddParameter("IntraStationTimeTolerance", "Time tolerance within one station", intraStationTimeTolerance_);
    AddParameter("SubEventStreamName",
		 "The name of the SubEvent stream.",
		 configuration_.InstanceName());

    AddOutBox("OutBox");
  }

  void Configure();

  void DAQ(I3FramePtr frame);

private:
  std::string inputPulsesName_;
  std::string outputMaskName_;
  std::string badTankListName_;
  std::string excludedTanksName_;
  double interStationTimeTolerance_;
  double intraStationTimeTolerance_;
};


I3_MODULE(I3TopHLCClusterCleaning);


void I3TopHLCClusterCleaning::Configure()
{
  GetParameter("InputPulses", inputPulsesName_);
  if (inputPulsesName_.empty()) log_fatal("InputPulses must be set.");

  GetParameter("OutputPulses", outputMaskName_);
  if (outputMaskName_.empty()) log_fatal("OutputPulses must be set.");

  GetParameter("BadTankList", badTankListName_);
  if (badTankListName_.empty()) log_warn("No bad tank list given.");

  GetParameter("ExcludedTanks", excludedTanksName_);
  if (excludedTanksName_.empty())
    log_warn("Parameter ExcludedTanksName empty, will not write excluded tanks to frame.");

  GetParameter("InterStationTimeTolerance", interStationTimeTolerance_);
  GetParameter("IntraStationTimeTolerance", intraStationTimeTolerance_);
  GetParameter("SubEventStreamName", sub_event_stream_name_);
}


void I3TopHLCClusterCleaning::DAQ(I3FramePtr daq)
{
  const I3Geometry &geometry = daq->Get<I3Geometry>();

  I3RecoPulseSeriesMapConstPtr inputPulses =
    daq->Get<I3RecoPulseSeriesMapConstPtr>(inputPulsesName_);
  if (!inputPulses) {
    log_info("Input pulses %s not in frame. Skipping event.",
	     inputPulsesName_.c_str());
    PushFrame(daq);
    return;
  }


#ifndef NDEBUG
  {
    size_t npulses = 0;
    BOOST_FOREACH(I3RecoPulseSeriesMap::const_reference pulses, *inputPulses) {
      npulses += pulses.second.size();
    }
    log_debug("Event with a total of %zu pulses", npulses);
  }
#endif


  I3VectorTankKeyPtr excludedTanks;
  if (!badTankListName_.empty()) {
    I3VectorTankKeyConstPtr badTankList =
      daq->Get<I3VectorTankKeyConstPtr>(badTankListName_);
      if (badTankList != 0) {
        excludedTanks = I3VectorTankKeyPtr(new I3Vector<TankKey>(badTankList->begin(), badTankList->end()));
      } else {
        log_info("Bad tank list '%s' not in frame, using an empty list.",
          badTankListName_.c_str());
        excludedTanks = I3VectorTankKeyPtr(new I3Vector<TankKey>);
      }
  }

  TankSignalSeries tankSignals;
  BOOST_FOREACH(I3RecoPulseSeriesMap::const_reference pulses, *inputPulses) {
    TankKey tankKey(pulses.first);
    
    if (std::find(excludedTanks->begin(), excludedTanks->end(), tankKey) != excludedTanks->end())
      continue; // Tank already marked bad
    
    BOOST_FOREACH(const I3RecoPulse &pulse, pulses.second) {
      TankSignal signal(pulses.first, pulse);
      tankSignals.push_back(signal);
    }
  }

  log_debug("This event has %zu tank signals", tankSignals.size());
  log_debug("Excluded tanks: %zu", excludedTanks->size());

  // put excluded stations into DAQ frame since they will be excluded for all sub-events
  if (!excludedTanksName_.empty()) {
    daq->Put(excludedTanksName_, excludedTanks);
  }

  PushFrame(daq);

  if (tankSignals.empty()) return;

  std::sort(tankSignals.begin(), tankSignals.end(), &time_sort_tanks);

  I3FramePtr frame = GetNextSubEvent(daq);
  I3RecoPulseSeriesMapMaskPtr mask(new I3RecoPulseSeriesMapMask(*frame, inputPulsesName_));
  mask->SetNone();
  const TankSignal *last_tank = &tankSignals[0];
  I3Position last_position = get_tank_position(geometry, last_tank->key);
  log_trace_stream("Adding pulse in DOM " << last_tank->key << " to event");
  mask->Set(last_tank->key, *(last_tank->pulse), true);

  // split event
  for (size_t s = 1; s < tankSignals.size(); ++s) {
    const TankSignal &tankSignal = tankSignals[s];
    I3Position tank_position = get_tank_position(geometry, tankSignal.key);
    double maxDeltaT =
      (tank_position-last_position).Magnitude() / I3Constants::c
        + ((last_tank->key.GetString() == tankSignal.key.GetString()) ? intraStationTimeTolerance_ : interStationTimeTolerance_);
    if (std::fabs(tankSignal.pulse->GetTime() - last_tank->pulse->GetTime()) > maxDeltaT) {
      assert(mask->GetAnySet());   // at this point, this really has to be true
      log_debug("Split off event with %d pulses", mask->GetSum());
      frame->Put(outputMaskName_, mask);
      PushFrame(frame);
      frame = GetNextSubEvent(daq);
      mask = I3RecoPulseSeriesMapMaskPtr(new I3RecoPulseSeriesMapMask(*frame, inputPulsesName_));
      mask->SetNone();
    }
    log_trace_stream("Adding pulse in DOM " << tankSignal.key << " to event");
    mask->Set(tankSignal.key, *(tankSignal.pulse), true);
    last_tank = &tankSignal;
    last_position = tank_position;
  }

  // push remaining sub-event (if any)
  if (mask->GetAnySet()) {
    log_debug("Split off event with %d pulses", mask->GetSum());
    frame->Put(outputMaskName_, mask);
    PushFrame(frame);
  }
}
