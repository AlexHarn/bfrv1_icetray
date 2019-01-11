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
#include <icetray/I3Units.h>
#include <dataclasses/I3DOMFunctions.h>
#include <dataclasses/calibration/I3DOMCalibration.h>
#include <dataclasses/status/I3DOMStatus.h>
#include <dataclasses/physics/I3Waveform.h>
#include <cmath>
#include <string>


namespace tpx {

  double GetSPEPeakCharge(const I3DOMStatus *domStatus, const I3DOMCalibration *domCalib)
  {
    // Get the mean single PE charge (Actually SPEMean refers to the most probable and not the mean charge !!!)
    double spePeakCharge = SPEMean(*domStatus, *domCalib);
    
    if (! std::isfinite(spePeakCharge) || spePeakCharge <= 0.0) {
      log_error("Invalid value (%f) of SPEMean. Cannot compute photoelectron number.", spePeakCharge);
      spePeakCharge = NAN;
    }
    return spePeakCharge;
  }


  // copied from WaveCalibrator/I3WaveformSplitter
  int8_t GetChannel(const I3Waveform &wf)
  {
    int8_t channel = 0;
    const std::vector<I3Waveform::StatusCompound> &winfo =
      wf.GetWaveformInformation();
    std::vector<I3Waveform::StatusCompound>::const_iterator it = winfo.begin();
    if (it != winfo.end())
      channel = it->GetChannel();
    else
      return channel;
  
    it++;
    for ( ; it != winfo.end(); it++)
      if (channel != it->GetChannel())
	log_error("Mixed channels %u and %d found!", channel, it->GetChannel());
  
    return channel;	
  }

}
