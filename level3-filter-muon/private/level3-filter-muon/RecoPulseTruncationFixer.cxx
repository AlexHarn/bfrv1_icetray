#include <level3-filter-muon/RecoPulseTruncationFixer.h>

RecoPulseTruncationFixer::RecoPulseTruncationFixer(const I3Context& context):
	I3ConditionalModule(context){
	AddParameter("InputPulses", "The name of the pulses which should be fixed", inputPulsesName_);
	AddParameter("OutputPulses", "The name of the pulses which will be written in the frame", outputPulsesName_);
	AddOutBox("OutBox");
}

void RecoPulseTruncationFixer::Configure(){
	GetParameter("InputPulses", inputPulsesName_);
	GetParameter("OutputPulses", outputPulsesName_);
	if(inputPulsesName_.empty()){
		log_fatal("Need to at least specify input pulses");
	}
	if(outputPulsesName_.empty()){
		outputPulsesName_ = inputPulsesName_;
	}
}

void RecoPulseTruncationFixer::Physics(const I3FramePtr frame){
	I3RecoPulseSeriesMapConstPtr inputPulses = frame->Get<I3RecoPulseSeriesMapMaskConstPtr>(inputPulsesName_)->Apply(*frame);
	if(!inputPulses)
		log_error("Input pulses not in frame!");
		return;
	I3RecoPulseSeriesMapPtr outputPulses = I3RecoPulseSeriesMapPtr(new I3RecoPulseSeriesMap(*inputPulses));

	for(I3RecoPulseSeriesMap::const_iterator domIt=inputPulses->begin(), domEnd=inputPulses->end(); domIt!=domEnd; domIt++){
		if(domIt->second.empty())
			continue;
		I3RecoPulseSeries::iterator outPulseIt=outputPulses->find(domIt->first)->second.begin();
		for(I3RecoPulseSeries::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++){
			/* Between IC86-2011 and IC86-2014 I3RecoPulse::time was saved as float. Since the introduction of FRT/SLOP this can cause problems for
			 * events that are close to the end of the event. Floating point precision is around 7digits and FRT/SLOP are ~ms long.
			 * This way the time is truncated and it can happen that the trailing edge (time+width) of a pulse comes after the leading
			 * edge of the following pulse. In order to fix this bad behaviour the width must be truncated in a similar way.
			 */
			double width = float(pulseIt->GetTime()+pulseIt->GetWidth())-nextafterf(float(pulseIt->GetTime()), std::numeric_limits<float>::max());
			outPulseIt->SetWidth(width);
			outPulseIt++;
		}
	}
	if(outputPulsesName_==inputPulsesName_){
		frame->Delete(outputPulsesName_);
	}
	frame->Put(outputPulsesName_, outputPulses);
	PushFrame(frame);
}
