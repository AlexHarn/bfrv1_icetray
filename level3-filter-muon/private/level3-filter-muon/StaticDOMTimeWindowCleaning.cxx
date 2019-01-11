#include <dataclasses/I3MapOMKeyMask.h>
#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Units.h>

bool isHLC(const I3RecoPulse& p){
	return (p.GetFlags() & I3RecoPulse::LC);
}

class StaticDOMTimeWindowCleaning : public I3ConditionalModule{
  public:
	StaticDOMTimeWindowCleaning(const I3Context& context):
	  I3ConditionalModule(context),
	  inputPulseName("SRTHVPulses"),
	  outputPulseName("TWSRTHVPulses"),
	  maxTimeDifference(3e3*I3Units::ns){
		AddParameter("InputPulses", "The name of the pulses which will be cleaned", inputPulseName);
		AddParameter("OutputPulses", "The name of the pulses to produce", outputPulseName);
		AddParameter("MaximumTimeDifference", "All pulses on a DOM that are later than the time of the first HLC plus MaximumTimeDifference are cleaned out", maxTimeDifference);
		AddOutBox("OutBox");
	}

	virtual void Configure(){
		GetParameter("InputPulses", inputPulseName);
		GetParameter("OutputPulses", outputPulseName);
		GetParameter("MaximumTimeDifference", maxTimeDifference);
		if(maxTimeDifference<0)
			log_fatal("The time difference must not be negative!");
	}

	virtual void Physics(boost::shared_ptr<I3Frame> frame){
		if(!frame->Has(inputPulseName)) {
			log_warn("Missing input pulses: %s ...skipping frame", inputPulseName.c_str());
			PushFrame(frame);
		}
		boost::shared_ptr<const I3RecoPulseSeriesMap> inputPulses=frame->Get<boost::shared_ptr<const I3RecoPulseSeriesMapMask> >(inputPulseName)->Apply(*frame);
		boost::shared_ptr<I3RecoPulseSeriesMapMask> outputPulses(new I3RecoPulseSeriesMapMask(*frame, inputPulseName));
		for(I3RecoPulseSeriesMap::const_iterator domIt=inputPulses->begin(), domEnd=inputPulses->end(); domIt!=domEnd; domIt++){
			if(domIt->second.empty())
				continue;
			double firstPulseTime=domIt->second.begin()->GetTime();
			for(I3RecoPulseSeries::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++){
				if(isHLC(*pulseIt)){
					firstPulseTime=pulseIt->GetTime();
					break;
				}
			}
			for(I3RecoPulseSeries::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++){
				if(pulseIt->GetTime()>(firstPulseTime+maxTimeDifference))
					outputPulses->Set(domIt->first, *pulseIt, false);
			}
		}
		frame->Put(outputPulseName, outputPulses);
		PushFrame(frame);
	}

  private:
	std::string inputPulseName;
	std::string outputPulseName;
	double maxTimeDifference;
};

I3_MODULE(StaticDOMTimeWindowCleaning);
