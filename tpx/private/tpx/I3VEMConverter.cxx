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
#include <icetray/OMKey.h>
#include <icetray/I3Units.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/physics/I3RecoPulse.h>

#include <boost/foreach.hpp>
#include <string>


class I3VEMConverter : public I3ConditionalModule {
public:
  I3VEMConverter(const I3Context &ctx)
    : I3ConditionalModule(ctx)
  {
    pePulsesName_ = "";
    AddParameter("PEPulses", "Input pulses with charge in PE units", pePulsesName_);
    vemPulsesName_ = "";
    AddParameter("VEMPulses", "Output pulses, will be calibrated in VEM", vemPulsesName_);
  }

  void Configure();

  void DAQ(I3FramePtr frame);

private:
  std::string pePulsesName_;
  std::string vemPulsesName_;

  SET_LOGGER("I3VEMConverter");
};

I3_MODULE(I3VEMConverter);


void I3VEMConverter::Configure()
{
  GetParameter("PEPulses", pePulsesName_);
  if (pePulsesName_.empty()) {
    log_fatal("No input pulses specified.");
  }

  GetParameter("VEMPulses", vemPulsesName_);
  if (vemPulsesName_.empty()) {
    log_fatal("No name for the output pulses specified.");
  }

  if (vemPulsesName_ == pePulsesName_) {
    log_fatal("Output pulses have the same name as input pulses, "
	      "but must be different.");
  }
}


void I3VEMConverter::DAQ(I3FramePtr frame)
{
  I3RecoPulseSeriesMapConstPtr pePulses =
    frame->Get<I3RecoPulseSeriesMapConstPtr>(pePulsesName_);
  if (!pePulses) {
    log_info("Input pulses ('%s') not in frame. Skipping.",
	     pePulsesName_.c_str());
    PushFrame(frame);
    return;
  }

  const I3Calibration &calibration = frame->Get<I3Calibration>();

  // copy pulses
  I3RecoPulseSeriesMapPtr vemPulses(new I3RecoPulseSeriesMap(*pePulses));

  typedef std::pair<const OMKey, std::vector<I3RecoPulse> > PairOMKeyRecoPulseSeries;
  BOOST_FOREACH(PairOMKeyRecoPulseSeries &item, *vemPulses) {
    const OMKey &omKey = item.first;
    const I3VEMCalibration *vemcal = tpx::get_dom_info(omKey, calibration.vemCal);
    if (!vemcal) {
      log_error("Pulse in DOM %s, but no VEM calibration! Charge will be NAN.",
		omKey.str().c_str());
    }
    double pe_per_vem = vemcal ? vemcal->pePerVEM/vemcal->corrFactor : NAN;

    BOOST_FOREACH(I3RecoPulse &pulse, item.second) {
      pulse.SetCharge(pulse.GetCharge()/pe_per_vem);
    }
  }

  frame->Put(vemPulsesName_, vemPulses);
  PushFrame(frame);
}
