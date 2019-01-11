#include "level3-filter-muon/SplitTimeWindowCalculator.h"

SplitTimeWindowCalculator::SplitTimeWindowCalculator(const I3Context& context):
  I3PacketModule(context, I3Frame::DAQ){
	AddParameter("SubEventStream", "The subevent stream including all subevents for which to calculate timewindows", subeventStreamName_);
	AddParameter("AfterpulseEventStream", "The subevent stream identifying afterpulses", subeventStreamName_);
	AddParameter("BasePulses", "The name of the complete, unsplit pulses", basePulsesName_);
	AddParameter("SplitPulses", "The name of the pulses in the subevents produced by previous splitter", splitPulsesName_);
	AddParameter("OutputPulses", "The name of the pulses to write into the frame", outputPulsesName_);
	AddParameter("TriggerSplitterTimeWindows", "The name of the trigger-splitter timewindows", triggerWindowName_);
	AddOutBox("OutBox");
}

void SplitTimeWindowCalculator::Configure(){
	GetParameter("SubEventStream", subeventStreamName_);
	GetParameter("AfterpulseEventStream", afterpulseStreamName_);
	GetParameter("BasePulses", basePulsesName_);
	GetParameter("SplitPulses", splitPulsesName_);
	GetParameter("OutputPulses", outputPulsesName_);
	GetParameter("TriggerSplitterTimeWindows", triggerWindowName_);
	if(triggerWindowName_.empty())
		log_fatal("Need to set a name of the trigger-splitter timewindows that should be taken into account");
	if(afterpulseStreamName_.empty())
		log_fatal("Need to set a subevent stream which contains afterpulses");
	if(subeventStreamName_.empty())
		log_fatal("Need to set a subevent stream for which to calculate timewindows");
	if(splitPulsesName_.empty())
		log_fatal("Need to set a name for the split pulses");
	if(basePulsesName_.empty())
		log_fatal("Need to set a name for the base pulses");
	if(outputPulsesName_.empty())
		log_fatal("Need to set a name for the output pulses");
}

void SplitTimeWindowCalculator::FramePacket(std::vector<I3FramePtr>& packet){
	log_info("Received %d frames.", static_cast<unsigned int>(packet.size()));
	if(packet.empty())
		return;
	const I3FramePtr qFrame = packet.front();
	const I3RecoPulseSeriesMapConstPtr fullPulses = qFrame->Get<I3RecoPulseSeriesMapConstPtr>(basePulsesName_);
	if(!fullPulses){
		log_error("Packet has no original pulses, it will be dropped");
		return;
	}
	// I3TimeWindowSeries overloads vector::push_back to combine I3TimeWindows which is unwanted here
	boost::shared_ptr<std::map<OMKey,std::vector<I3TimeWindow> > > allTimeWindows(new std::map<OMKey,std::vector<I3TimeWindow> >);
	// make a time window around each pulse on each dom
	for(I3RecoPulseSeriesMap::const_iterator domIt=fullPulses->begin(), domEnd=fullPulses->end(); domIt!=domEnd; domIt++){
		if(domIt->second.empty())
			continue;
		double prevEnd = -std::numeric_limits<double>::infinity();
		const double safety = 1e-3;
		for(I3RecoPulseSeries::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++){
			double nextTime;
			if((pulseIt+1) == pulseEnd){
				nextTime = std::numeric_limits<double>::infinity();
			}
			else{
				nextTime = (pulseIt+1)->GetTime();
			}
			double time = pulseIt->GetTime();
			double endTime = (time+nextTime)/2;
			(*allTimeWindows)[domIt->first].push_back(I3TimeWindow(prevEnd-safety, endTime+safety));
			prevEnd = endTime;
		}
	}

	// we need a "I3TimeWindowSeriesMap" for each subevent except for afterpulse subevents
	std::vector<I3FramePtr> cutFrames;
	for(std::vector<I3FramePtr>::const_iterator firstIt=packet.begin(), firstEnd=packet.end(); firstIt!=firstEnd; ++firstIt){
		const I3FramePtr firstFrame = *firstIt;
		if(firstFrame->GetStop()!=I3Frame::Physics)
			continue;
		if(firstFrame->Get<I3EventHeader>().GetSubEventStream()!=subeventStreamName_)
			continue;
		if(!firstFrame->Has(splitPulsesName_)){
			log_error("Frame does not include split pulses!");
			continue;
		}
		log_debug_stream("Calculating timewindows for subevent stream " << firstFrame->Get<I3EventHeader>().GetSubEventStream());
		
		// we are only interested in events that are contained within a trigger window
		// find trigger window which includes split pulses
		const I3TimeWindowSeriesConstPtr triggerWindows = qFrame->Get<I3TimeWindowSeriesConstPtr>(triggerWindowName_);
		// find times of last and first pulse
		double firstPulseTime = std::numeric_limits<double>::infinity();
		double lastPulseTime = -std::numeric_limits<double>::infinity();
		const I3RecoPulseSeriesMapConstPtr splitPulses = firstFrame->Get<I3RecoPulseSeriesMapConstPtr>(splitPulsesName_);
		for(I3RecoPulseSeriesMap::const_iterator domIt=splitPulses->begin(), domEnd=splitPulses->end();
			domIt!=domEnd; ++domIt){
			if(domIt->second.empty())
				continue;
			firstPulseTime = std::min(firstPulseTime, domIt->second.begin()->GetTime());
			lastPulseTime = std::max(lastPulseTime, (domIt->second.end()-1)->GetTime());
		}
		I3TimeWindow pulsesWindow = I3TimeWindow(firstPulseTime, lastPulseTime);
		// check if all pulses are included in a trigger-splitter timewindow
		I3TimeWindowSeries::const_iterator trigswIt = triggerWindows->begin();
		for( ; trigswIt!=triggerWindows->end(); trigswIt++){
			if(MuonL3_IC86_Utils::GetOverlapType(pulsesWindow, *trigswIt) == MuonL3_IC86_Utils::WITHIN){
				break;
			}
		}
		// if we did not find a trigger window do not push frame
		if(trigswIt == triggerWindows->end()){
			cutFrames.push_back((*firstIt));
			continue;
		}

		// abuse the xor operator to create a mask in which the pulses from every other subevent are removed
		const I3RecoPulseSeriesMapMaskPtr relevantPulses(new I3RecoPulseSeriesMapMask(*qFrame, basePulsesName_));
		const I3RecoPulseSeriesMapMaskConstPtr fullPulsesMask(new I3RecoPulseSeriesMapMask(*qFrame, basePulsesName_));
		for(std::vector<I3FramePtr>::const_iterator secondIt=packet.begin(), secondEnd=packet.end(); secondIt!=secondEnd; ++secondIt){
			const I3Frame& secondFrame = **secondIt;
			if(secondFrame.GetStop()!=I3Frame::Physics)
				continue;
			if((secondFrame.Get<I3EventHeader>().GetSubEventStream()!=subeventStreamName_) && (secondFrame.Get<I3EventHeader>().GetSubEventStream()!=afterpulseStreamName_))
				continue;
			if(!secondFrame.Has(splitPulsesName_))
				continue;
			if(firstIt==secondIt)
				continue;
			log_debug_stream("Removing pulses from " << secondFrame.Get<I3EventHeader>().GetSubEventStream());
			
			// in used dataclasses version there was no possibility to repoint mask to different map
			I3RecoPulseSeriesMapConstPtr splitPulses = secondFrame.Get<I3RecoPulseSeriesMapConstPtr>(splitPulsesName_);
			const I3RecoPulseSeriesMapMaskConstPtr repointedPulses(new I3RecoPulseSeriesMapMask(secondFrame, relevantPulses->GetSource(), *splitPulses));
			*relevantPulses = (*fullPulsesMask ^ *repointedPulses) & *relevantPulses;
		}

		// remove all pulses from relevantPulses that are outside the trigger window
		const I3RecoPulseSeriesMapConstPtr actualRelevantPulses = relevantPulses->Apply(*firstFrame);
		for(I3RecoPulseSeriesMap::const_iterator actualRelevantPulseIt=actualRelevantPulses->begin(), actualRelevantPulseEnd=actualRelevantPulses->end();
			actualRelevantPulseIt!=actualRelevantPulseEnd; ++actualRelevantPulseIt){
			for(I3RecoPulseSeries::const_iterator pulseIt=actualRelevantPulseIt->second.begin(), pulseEnd=actualRelevantPulseIt->second.end();
				pulseIt!=pulseEnd; pulseIt++){
				if(!MuonL3_IC86_Utils::contains((*trigswIt), pulseIt->GetTime()))
					relevantPulses->Set(actualRelevantPulseIt->first, *pulseIt , false);
			}
		}

		// keep all timewindows that exclude pulses inside the readout window
		const I3TimeWindowSeriesMapPtr maskedTimes(new I3TimeWindowSeriesMap);
		for(I3RecoPulseSeriesMap::const_iterator domIt=fullPulses->begin(), domEnd=fullPulses->end(); domIt!=domEnd; ++domIt){
			I3RecoPulseSeriesMap::const_iterator relevantDomIt = actualRelevantPulses->find(domIt->first);
			//if the whole DOM is not relevant, put in an exclusion window covering all times within the trigger window
			if(relevantDomIt==actualRelevantPulses->end()){
				(*maskedTimes)[domIt->first].push_back(I3TimeWindow(trigswIt->GetStart(), trigswIt->GetStop()));
				continue;
			}
			I3RecoPulseSeries::const_iterator relevantPulseIt = relevantDomIt->second.begin();
			for(std::vector<I3TimeWindow>::const_iterator windowIt = allTimeWindows->find(domIt->first)->second.begin(),
				windowEnd=allTimeWindows->find(domIt->first)->second.end(); windowIt!=windowEnd; windowIt++){
				if((MuonL3_IC86_Utils::contains(*windowIt, relevantPulseIt->GetTime())) && !(relevantPulseIt==relevantDomIt->second.end())){
					relevantPulseIt++;
				}
				else{
					// exclusion windows for millipede must be within the readoutwindow
					if( MuonL3_IC86_Utils::GetOverlapType(*windowIt, *trigswIt) == MuonL3_IC86_Utils::NONE )
						continue;
					I3TimeWindow tw(std::max(windowIt->GetStart(), trigswIt->GetStart()), std::min(windowIt->GetStop(), trigswIt->GetStop()));
					(*maskedTimes)[domIt->first].push_back(tw);
				}
			}
		}

		// put relevant pulses and excluded time ranges in the frame
		firstFrame->Put(outputPulsesName_, relevantPulses);
		firstFrame->Put(outputPulsesName_+"ExcludedTimeRange", maskedTimes);
		firstFrame->Put(outputPulsesName_+"ReadoutWindow", boost::make_shared<I3TimeWindow>((*trigswIt)));
	}
	// now push all frames
	for(std::vector<I3FramePtr>::const_iterator it=packet.begin(), end=packet.end(); it!=end; ++it){
		bool doNotPush = false;
		for(std::vector<I3FramePtr>::const_iterator cutIt=cutFrames.begin(), cutEnd=cutFrames.end(); cutIt!=cutEnd; ++cutIt){
			if((*it)==(*cutIt)){
				doNotPush = true;
				break;
			}
		}
		if(!doNotPush)
			PushFrame(*it);
	}
}






