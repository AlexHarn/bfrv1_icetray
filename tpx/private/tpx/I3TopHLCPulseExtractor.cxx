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
#include <icetray/I3Frame.h>
#include <icetray/I3Logging.h>
#include <icetray/I3Units.h>
#include <icetray/OMKey.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/physics/I3Waveform.h>
#include <dataclasses/I3DOMFunctions.h>
#include <dataclasses/TankKey.h>

#include "tpx/I3TopPulseInfo.h"

#include <boost/foreach.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>


class I3TopHLCPulseExtractor : public I3ConditionalModule {
public:
  I3TopHLCPulseExtractor(const I3Context &ctx)
    : I3ConditionalModule(ctx)
  {
    inputWaveformsName_ = "IceTopCalibratedATWD";
    AddParameter("Waveforms", "Input calibrated HLC I3WaveformSeriesMap", inputWaveformsName_);
    pePulsesName_ = "IceTopHLCPEPulses";
    AddParameter("PEPulses", "Output pulse series calibrated in pe (optional)", pePulsesName_);
    vemPulsesName_ = "IceTopHLCVEMPulses";
    AddParameter("VEMPulses", "Output pulse series calibrated in vem (optional)", vemPulsesName_);
    pulseInfoName_ = "IceTopHLCPulseInfo";
    AddParameter("PulseInfo", "Output pulse info (amplitudes, rise time, ...) (optional)", pulseInfoName_);
    tmin_ = -200.*I3Units::ns;
    AddParameter("MinimumLeadingEdgeTime", "Earliest possible time of the leading edge, "
        "measured from the beginning of the ATWD waveform. If the leading-edge extrapolation "
        "returns a time earlier that this minimum (for example, when the algorithm extrapolates "
        "from the peak rather than the true leading edge because the ATWD turns on halfway "
        "into the pulse), the leading-edge time will be snapped to this value.", tmin_);
    badDOMListName_ = "IceTopBadDOMs";
    AddParameter("BadDomList", "List of bad DOMs", badDOMListName_);
    
    AddOutBox("OutBox");
  }

  void Configure();

  void DetectorStatus(I3FramePtr frame);
  
  void DAQ(I3FramePtr frame);

private:
  std::string inputWaveformsName_;
  std::string pePulsesName_;
  std::string vemPulsesName_;
  std::string pulseInfoName_;
  double tmin_;
  std::string badDOMListName_;
  I3VectorOMKeyConstPtr badDOMList_;

  void ExtractPEPulses(const I3WaveformSeriesMapConstPtr &waveforms,
		       const I3OMGeoMap &geometry,
		       const I3DOMCalibrationMap &calibration,
		       const I3DOMStatusMap &status,
		       I3RecoPulseSeriesMapPtr pulses,
		       I3TopPulseInfoSeriesMapPtr pulseInfos);
  I3RecoPulseSeriesMapPtr CalibrateVEMPulses(const I3RecoPulseSeriesMapConstPtr &pePulses,
					     const I3VEMCalibrationMap &calibration);

  void EvaluateHLCWaveform(const OMKey &omKey, const I3Waveform &wf, double feImpedance,
			   double speMean, I3RecoPulse &pulse,
			   I3TopPulseInfo &pulseInfo);
  
  SET_LOGGER("I3TopHLCPulseExtractor")
};


I3_MODULE(I3TopHLCPulseExtractor)


void I3TopHLCPulseExtractor::Configure()
{
  GetParameter("Waveforms", inputWaveformsName_);
  if (inputWaveformsName_.empty())
    log_fatal("You have to specify a name for the input waveforms");

  GetParameter("PEPulses", pePulsesName_);
  GetParameter("VEMPulses", vemPulsesName_);
  if (pePulsesName_.empty() && vemPulsesName_.empty())
    log_fatal("You need to set a name for PEPulses and/or VEMPulses, but cannot leave both empty.");

  GetParameter("PulseInfo", pulseInfoName_);
  GetParameter("MinimumLeadingEdgeTime", tmin_);
  if (!(tmin_ <= 0))
    log_fatal("MinimumLeadingEdgeTime should be a negative number (not %f)", tmin_);
  
  GetParameter("BadDomList", badDOMListName_);
}


void I3TopHLCPulseExtractor::DetectorStatus(I3FramePtr frame)
{
  badDOMList_ = frame->Get<I3VectorOMKeyConstPtr>(badDOMListName_);
  if (!badDOMList_)
    log_error_stream("Bad DOM List \"" << badDOMListName_ << "\" does not exist!");
  PushFrame(frame);
}


void I3TopHLCPulseExtractor::DAQ(I3FramePtr frame)
{
  
  I3WaveformSeriesMapConstPtr waveforms =
    frame->Get<I3WaveformSeriesMapConstPtr>(inputWaveformsName_);
  if (!waveforms) {
    log_debug("Input waveforms not found. Skipping this event.");
    PushFrame(frame);
    return;
  }

  const I3Geometry &geometry = frame->Get<I3Geometry>();
  const I3Calibration &calibration = frame->Get<I3Calibration>();
  const I3DetectorStatus &status = frame->Get<I3DetectorStatus>();

  I3RecoPulseSeriesMapPtr pePulses(new I3RecoPulseSeriesMap());
  I3TopPulseInfoSeriesMapPtr pulseInfo(new I3TopPulseInfoSeriesMap());
  ExtractPEPulses(waveforms, geometry.omgeo, calibration.domCal, status.domStatus,
		  pePulses, pulseInfo);

  if (!pePulsesName_.empty()) frame->Put(pePulsesName_, pePulses);
  if (!pulseInfoName_.empty()) frame->Put(pulseInfoName_, pulseInfo);

  if (vemPulsesName_.empty()) {
    PushFrame(frame);
    return;
  }

  I3RecoPulseSeriesMapPtr vemPulses = CalibrateVEMPulses(pePulses, calibration.vemCal);

  frame->Put(vemPulsesName_, vemPulses);

  PushFrame(frame);
}


void I3TopHLCPulseExtractor::ExtractPEPulses(const I3WaveformSeriesMapConstPtr &waveforms,
					     const I3OMGeoMap &geometry,
					     const I3DOMCalibrationMap &calibration,
					     const I3DOMStatusMap &status,
					     I3RecoPulseSeriesMapPtr pulses,
					     I3TopPulseInfoSeriesMapPtr pulseInfos)
{
  static bool alreadyWarnedAboutInIce = false;

  for (I3WaveformSeriesMap::const_iterator wf_iter = waveforms->begin();
       wf_iter != waveforms->end(); ++wf_iter)
    {
      const OMKey &omKey = wf_iter->first;
      const I3WaveformSeries &wf_series = wf_iter->second;
      
      if (geometry.find(omKey)->second.omtype!=I3OMGeo::IceTop ) {
        if (!alreadyWarnedAboutInIce) {
          log_warn("Found an InIce DOM. Are you sure this is IceTop data?");
          log_warn("I will warn about this only once!");
          alreadyWarnedAboutInIce = true;
        } else {
          log_debug("Found an InIce DOM. Are you sure this is IceTop data?");
        }
        continue;
      }
      
      if (badDOMList_ && std::find(badDOMList_->begin(), badDOMList_->end(), omKey) != badDOMList_->end()) {
        log_debug_stream("Ignoring signals from " << omKey << " which is on the Bad DOM List.");
        continue;
      }
      
      const I3DOMStatus *domStatus = tpx::get_dom_info(omKey, status);
      if (!domStatus) continue;

      const I3DOMCalibration *domCalibration = tpx::get_dom_info(omKey, calibration);
      if (!domStatus) continue;

      double feImpedance = domCalibration->GetFrontEndImpedance();
      if (! std::isfinite(feImpedance) || feImpedance <= 0.0)
        log_fatal_stream("Bad front-end impedance for " << omKey << ". This DOM should be on the Bad DOM List.");
      
      double speMean = tpx::GetSPEPeakCharge(domStatus, domCalibration);

      for (size_t i = 0; i < wf_series.size(); ++i) {
	const I3Waveform &wf = wf_series[i];

	if (wf.IsSLC()) {
	  log_debug("Skipping SLC waveform");
	  continue;
	}

	if (wf.GetDigitizer() != I3Waveform::ATWD) {
	  log_debug("Not an ATWD waveform. Don't know what to do with this. Skipping.");
	  continue;
	}

	if (wf.GetWaveform().size() == 0) {
	  log_debug("Found a zero-length waveform. Skipping it.");
	  continue;
	}

	I3RecoPulse pulse;
	I3TopPulseInfo pulseInfo;
	EvaluateHLCWaveform(omKey,wf, feImpedance, speMean, pulse, pulseInfo);
	(*pulses)[omKey].push_back(pulse);
	(*pulseInfos)[omKey].push_back(pulseInfo);
      }
    }
}


I3RecoPulseSeriesMapPtr I3TopHLCPulseExtractor::CalibrateVEMPulses(const I3RecoPulseSeriesMapConstPtr &pePulses,
								   const I3VEMCalibrationMap &calibration)
{
  I3RecoPulseSeriesMapPtr vemPulses(new I3RecoPulseSeriesMap);

  for (I3RecoPulseSeriesMap::const_iterator iter = pePulses->begin();
       iter != pePulses->end(); ++iter)
    {
      const OMKey &omKey = iter->first;
      const I3VEMCalibration *vemcal = tpx::get_dom_info(omKey, calibration);
      if (!vemcal)
	log_error("Pulse in DOM %s, but no VEM calibration!",
		  omKey.str().c_str());
      double pe_per_vem = vemcal ? vemcal->pePerVEM/vemcal->corrFactor : NAN;
      BOOST_FOREACH(const I3RecoPulse &pePulse, iter->second) {
	I3RecoPulse vemPulse(pePulse);
	vemPulse.SetCharge(vemPulse.GetCharge()/pe_per_vem);
	(*vemPulses)[omKey].push_back(vemPulse);
      }
    }

  return vemPulses;
}


void I3TopHLCPulseExtractor::EvaluateHLCWaveform(const OMKey &omKey,const I3Waveform &wf,
						 double feImpedance, double speMean,
						 I3RecoPulse &pulse,
						 I3TopPulseInfo &pulseInfo)
{
  const std::vector<double> &trace = wf.GetWaveform();
  int numBins   = static_cast<int>(trace.size());
  double tstart = wf.GetStartTime();
  double deltat = wf.GetBinWidth();
  
  assert(numBins > 0);  // this was checked in the calling function

  int maxBin = 0;
  double integral = 0.0;
  double maxValue = 0.0;
  double initialValue = trace[0];

  // Integrate over all bins to get the total charge and find the maximum amplitude of the waveform    
  for (int i = 0; i < numBins; i++) {
    double value = trace[i];
    // Don't subtract charges if there under the baseline, only add all bins above the baseline !
    if (value < 0.)
      log_trace("Waveform bin %i is under the baseline and has value %lf mV, won't be subtracted.",
		i, value/I3Units::mV);
    else
      integral += value;
	
    if (value-initialValue > maxValue) {
      maxValue = value-initialValue;
      maxBin   = i;
    }
  }
  log_debug("deltat=%.2f ns, integral=%f mV, maximum=%f mV",
	    deltat/I3Units::ns, integral/I3Units::mV, maxValue/I3Units::mV);
    
  // Calculate the total charge of the waveform
  double charge    = integral*deltat/feImpedance;
  double charge_pe = charge/speMean;
	    
  //Calculate the Leading Edge of the FIRST PULSE: 
  //First, find the first pulse, then calculate 90% and 10% values and bins of the first pulse.
  //LE definition: time bin which 90% to 10% slope intersects with the baseline. 
  //This is the standard EE definition of LE, also the official LE definition for IceCube waveforms.
  
  //FK: First Peak must be higher than value in first bin + 5% of the highest peak
  //The value in the first bin is also subtracted from the 90% and 10% values
  //This fixes problems with droopy waveforms where the baseline is far below the
  //value of the waveform in the first bin

  for (int i = 1; i < maxBin-1; i++) {
    assert(i+1 < numBins);  // should be given by construction
    if ( (trace[i]-initialValue > maxValue*0.05) && (trace[i+1] < trace[i]) ) {
      maxValue = trace[i]-initialValue;
      maxBin = i;
      break;
    }
  }

  double Amplitude = maxValue+initialValue;

  int ninetyPerCentBin = 0;
  int tenPerCentBin = 0;

  for (int i=1; i <= maxBin; i++) {
    if ( trace[i]-initialValue > 0.9*maxValue ) {
      ninetyPerCentBin = i;
      break;
    }
  }

  for (int i=0; i<maxBin; i++) {
    assert(i+1 < numBins);  // by construction
    if ( trace[i+1]-initialValue > 0.1*maxValue ) {
      tenPerCentBin = i;
      break;
    }
  }

  assert(ninetyPerCentBin < numBins);
  assert(tenPerCentBin < numBins);

  double ninetyToTenSlope = (trace[ninetyPerCentBin] - trace[tenPerCentBin])/(ninetyPerCentBin - tenPerCentBin);

  double le = tstart + (double(tenPerCentBin) - trace[tenPerCentBin]/ninetyToTenSlope)*deltat;
  if ((ninetyToTenSlope <= 0.) || !std::isfinite(ninetyToTenSlope) || !(le >= tstart + tmin_)) {
    log_info("Flat waveform or maximum in first bin. Setting leading edge to tstart + %f.", tmin_);
    le = tstart + tmin_;
    pulseInfo.status = I3TopPulseInfo::BadTime;
  }
  log_debug("Leading edge = %.2f ns", le/I3Units::ns);

  double riseTime = (ninetyPerCentBin - tenPerCentBin)*deltat;
	
  // Calculate the Trailing edge: last bin with >=2% of waveform maximum
  double te = le;
  for (int i = numBins-1; i >= 0; --i) {
    if (trace[i] >= 0.02*(maxValue+initialValue)) {
      if(i == numBins-1) {
	te = tstart + numBins*deltat;
      } else {
	assert(i+1 < numBins);  // by construction
	te = tstart + (i + (0.02*(maxValue+initialValue) - trace[i])/(trace[i+1] - trace[i]))*deltat;
	log_debug("TE: max=%g y1=%g y2=%g dt=%g itwp_tstart=%g TE=%g",
		  maxValue+initialValue, trace[i], trace[i+1], deltat, tstart, te);
      }
      break;
    }
  }
  log_debug("Trailing edge = %.2f ns", te/I3Units::ns);
        
  double pulseWidth = te - le;
  if (pulseWidth <= 0) {
    if (pulseInfo.status == I3TopPulseInfo::BadTime) {
      log_debug("Negative pulse width: %f  -  Leading edge: %f  -  Trailing edge: %f", pulseWidth, le, te);
    } else {
      log_warn("Negative pulse width: %f", pulseWidth);
      log_warn("Leading edge: %f", le);
      log_warn("Trailing edge: %f", te);
    }
  }

  // Fill I3RecoPulse values
  pulse.SetTime(le);
  pulse.SetWidth(pulseWidth);
  pulse.SetCharge(charge_pe);
  pulse.SetFlags(I3RecoPulse::LC | I3RecoPulse::ATWD);

  // IceTop specific extra waveform information
  pulseInfo.amplitude = Amplitude;
  pulseInfo.risetime = riseTime;
  pulseInfo.trailingEdge = te;
  pulseInfo.channel = tpx::GetChannel(wf);
  pulseInfo.sourceID = wf.GetSourceIndex();
}
