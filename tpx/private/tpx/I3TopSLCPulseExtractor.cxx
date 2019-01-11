/*
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Timo Karg <karg@physik.uni-wuppertal.de> Last changed by: $LastChangedBy$
 */

#include "utility.h"

#include <algorithm>
#include <string>
#include <boost/foreach.hpp>

#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/calibration/I3DOMCalibration.h>
#include <dataclasses/calibration/I3VEMCalibration.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/physics/I3Waveform.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/status/I3DOMStatus.h>
#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Context.h>
#include <icetray/I3Frame.h>
#include <icetray/I3Logging.h>
#include <icetray/OMKey.h>


class I3TopSLCPulseExtractor : public I3ConditionalModule {
public:
  I3TopSLCPulseExtractor(const I3Context& ctx);
  
  virtual void Configure();
  virtual void DetectorStatus(I3FramePtr frame);
  virtual void DAQ(I3FramePtr frame);
  
protected:
  I3RecoPulseSeriesPtr PECalibration(const OMKey &omKey, const I3WaveformSeries &waveforms, const I3DOMStatus *domStatus, const I3DOMCalibration *domCalib);
  I3RecoPulseSeriesPtr VEMCalibration(const OMKey &omKey, const I3RecoPulseSeries &pulses, const I3VEMCalibration *vemCalib);
  
private:
  std::string inputWaveformsName_;
  std::string pePulsesName_;
  std::string vemPulsesName_;
  std::string badDOMListName_;
  I3VectorOMKeyConstPtr badDOMList_;
  
  SET_LOGGER("I3TopSLCPulseExtractor")
};


I3_MODULE(I3TopSLCPulseExtractor)


I3TopSLCPulseExtractor::I3TopSLCPulseExtractor(const I3Context& ctx)
  : I3ConditionalModule(ctx),
    inputWaveformsName_("IceTopCalibratedSLC"),
    pePulsesName_("IceTopSLCPEPulses"),
    vemPulsesName_("IceTopSLCVEMPulses"),
    badDOMListName_("IceTopBadDOMs") {
  AddParameter("Waveforms", "Input calibrated SLC I3WaveformSeriesMap", inputWaveformsName_);
  AddParameter("PEPulses", "Output pulse series calibrated in pe (optional)", pePulsesName_);
  AddParameter("VEMPulses", "Output pulse series calibrated in vem (optional)", vemPulsesName_);
  AddParameter("BadDomList", "List of bad DOMs", badDOMListName_);
  AddOutBox("OutBox");
}


void
I3TopSLCPulseExtractor::Configure() {
  GetParameter("Waveforms", inputWaveformsName_);
  if (inputWaveformsName_.empty())
    log_fatal("You have to specify a name for the input waveforms");
  
  GetParameter("PEPulses", pePulsesName_);
  GetParameter("VEMPulses", vemPulsesName_);
  if (pePulsesName_.empty() && vemPulsesName_.empty())
    log_fatal("You need to set a name for PEPulses and/or VEMPulses, but cannot leave both empty.");
  GetParameter("BadDomList", badDOMListName_);
}


void
I3TopSLCPulseExtractor::DetectorStatus(I3FramePtr frame) {
  badDOMList_ = frame->Get<I3VectorOMKeyConstPtr>(badDOMListName_);
  if (!badDOMList_)
    log_error_stream("Bad DOM List \"" << badDOMListName_ << "\" does not exist!");
  PushFrame(frame);
}


void
I3TopSLCPulseExtractor::DAQ(I3FramePtr frame) {
  I3WaveformSeriesMapConstPtr waveforms = frame->Get<I3WaveformSeriesMapConstPtr>(inputWaveformsName_);
  if (! waveforms) {
    log_debug("Input waveforms not found. Skipping this event.");
    PushFrame(frame);
    return;
  }
  
  /***************************************************************************
   *
   * PE calibration
   *
   ***************************************************************************/
  
  const I3Calibration& calibration = frame->Get<I3Calibration>();
  const I3DetectorStatus& status = frame->Get<I3DetectorStatus>();
  
  I3RecoPulseSeriesMapPtr pePulses(new I3RecoPulseSeriesMap());
  
  BOOST_FOREACH(I3WaveformSeriesMap::const_reference dom, *waveforms) {
    const OMKey& omKey = dom.first;
    const I3WaveformSeries& waveforms = dom.second;
    
    if (badDOMList_ && std::find(badDOMList_->begin(), badDOMList_->end(), omKey) != badDOMList_->end()) {
      log_debug_stream("Ignoring signals from " << omKey << " which is on the Bad DOM List.");
      continue;
    }
    
    const I3DOMStatus *domStatus = tpx::get_dom_info<I3DOMStatus>(omKey, status.domStatus);
    const I3DOMCalibration *domCalib = tpx::get_dom_info<I3DOMCalibration>(omKey, calibration.domCal);

    if (!domStatus || !domCalib) {
      log_error("Waveform in DOM %s, for which either the calibration or the DOM status is missing!",
		omKey.str().c_str());
      continue;
    }

    (*pePulses)[omKey] = *PECalibration(omKey, waveforms, domStatus, domCalib);
  }
  
  if (! pePulsesName_.empty()) frame->Put(pePulsesName_, pePulses);

  /***************************************************************************
   *
   * VEM calibration
   *
   ***************************************************************************/
  
  if (vemPulsesName_.empty()) {
    PushFrame(frame);
    return;
  }
  
  I3RecoPulseSeriesMapPtr vemPulses(new I3RecoPulseSeriesMap());
  
  BOOST_FOREACH(I3RecoPulseSeriesMap::const_reference dom, *pePulses) {
    const OMKey& omKey = dom.first;
    const I3RecoPulseSeries& pulses = dom.second;
    
    const I3VEMCalibration *vemCalib = tpx::get_dom_info<I3VEMCalibration>(omKey, calibration.vemCal);

    if (!vemCalib) {
      log_error("Pulse in DOM %s but no VEM calibration!", omKey.str().c_str());
      continue;
    }

    (*vemPulses)[omKey] = *VEMCalibration(omKey, pulses, vemCalib);
  }
  
  if (! vemPulsesName_.empty()) frame->Put(vemPulsesName_, vemPulses);
  
  PushFrame(frame);
}


I3RecoPulseSeriesPtr
I3TopSLCPulseExtractor::PECalibration(const OMKey &omKey, const I3WaveformSeries &waveforms, const I3DOMStatus *domStatus, const I3DOMCalibration *domCalib) {
  I3RecoPulseSeriesPtr res(new I3RecoPulseSeries());
  
  BOOST_FOREACH(const I3Waveform& waveform, waveforms) {
    if (!waveform.IsSLC()) {
      log_debug("Skipping SLC waveform.");
      continue;
    }
    
    // Check size
    if (waveform.GetWaveform().size() != 1) {
      log_error("Ill-formed waveform (%zu samples). Skipping.", waveform.GetWaveform().size());
      continue;
    }
    
    // Check for saturation IS THIS POSSIBLE FOR SLC??
    if ((waveform.GetStatus() & I3Waveform::SATURATED) != 0) {
      log_debug("Waveform is completely saturated. Skipping.");
      continue;
    }
    
    // Evaluate SLC waveform
    double amplitude = waveform.GetWaveform().at(0);
    
    // Calculate the total charge 
    double deltat      = waveform.GetBinWidth();
    double feImpedance = domCalib->GetFrontEndImpedance();
    if (! std::isfinite(feImpedance) || feImpedance <= 0.0)
      log_fatal_stream("Bad front-end impedance for " << omKey << ". This DOM should be on the Bad DOM List.");
    double charge      = amplitude * deltat / feImpedance;
    double charge_pe   = charge / tpx::GetSPEPeakCharge(domStatus, domCalib);
    
    I3RecoPulse pulse;
    
    pulse.SetTime(waveform.GetStartTime());
    pulse.SetWidth(deltat);
    pulse.SetCharge(charge_pe);
    pulse.SetFlags(I3RecoPulse::ATWD);
    res->push_back(pulse);
  }
  return res;
}


I3RecoPulseSeriesPtr
I3TopSLCPulseExtractor::VEMCalibration(const OMKey &omKey, const I3RecoPulseSeries &pulses, const I3VEMCalibration *vemCalib) {
  I3RecoPulseSeriesPtr res(new I3RecoPulseSeries(pulses));
  
  BOOST_FOREACH(I3RecoPulse& pulse, *res) {
    pulse.SetCharge(pulse.GetCharge() / (vemCalib->pePerVEM / vemCalib->corrFactor));
  }
  return res;
}
