#ifndef MUONL3_IC86_SPLITTIMEWINDOWCALCULATOR_H_INCLUDED
#define MUONL3_IC86_SPLITTIMEWINDOWCALCULATOR_H_INCLUDED

#include <algorithm>
#include <boost/make_shared.hpp>
#include <icetray/I3PacketModule.h>
#include <icetray/I3Logging.h>
#include <icetray/I3Units.h>
#include <icetray/I3Bool.h>
#include <dataclasses/physics/I3EventHeader.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/physics/I3FilterResult.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <dataclasses/I3TimeWindow.h>

#include "level3-filter-muon/MuonL3_IC86_Utils.h"

class SplitTimeWindowCalculator : public I3PacketModule{
  public:
	SplitTimeWindowCalculator(const I3Context& context);
	void Configure();
	void FramePacket(std::vector<I3FramePtr>& packet);

  private:
	std::string subeventStreamName_;
	std::string afterpulseStreamName_;
	std::string basePulsesName_;
	std::string splitPulsesName_;
	std::string outputPulsesName_;
	std::string triggerWindowName_;
};

I3_MODULE(SplitTimeWindowCalculator);

#endif // MUONL3_IC86_SPLITTIMEWINDOWCALCULATOR_H_INCLUDED
