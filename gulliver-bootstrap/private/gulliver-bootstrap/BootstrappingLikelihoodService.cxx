#include <gulliver-bootstrap/BootstrappingLikelihoodService.h>
#include <icetray/I3SingleServiceFactory.h>

namespace {
	double dummyArray[1]={1.0};
	
	bool compPulseIndex(unsigned int x, const std::pair<unsigned int, OMKey>& p){ return(x<p.first); }
}

BootstrappingLikelihoodService::BootstrappingLikelihoodService(const I3Context& c):
I3ServiceBase(c),iterations(4),randomName("I3RandomService"),multinomialDist(dummyArray,dummyArray+1){
	AddParameter("Pulses","The pulses being used as the basis for the likelihood",pulsesName);
	AddParameter("Bootstrapping","The type of resampling to use",bootstrapType);
	AddParameter("Iterations","The number of times that each event should be resampled",iterations);
	AddParameter("RandomService","The random service to use",randomName);
	AddParameter("WrappedLikelihood","The actual likelihood service to use",likelihoodName);
}

void BootstrappingLikelihoodService::Configure(){
	GetParameter("Pulses",pulsesName);
	GetParameter("Bootstrapping",bootstrapType);
	GetParameter("Iterations",iterations);
	GetParameter("RandomService",randomName);
	randomService = context_.Get<boost::shared_ptr<I3RandomService> >(randomName);
	if(!randomService)
		log_fatal_stream("No random service available under the name " << randomName);
	GetParameter("WrappedLikelihood",likelihoodName);
	wrappedLikelihood = context_.Get<boost::shared_ptr<I3EventLogLikelihoodBase> >(likelihoodName);
	if(!wrappedLikelihood)
		log_fatal_stream("No likelihood service available under the name " << likelihoodName);
}

//this function is redundant; the same job could just be done in set event
void BootstrappingLikelihoodService::SetGeometry(const I3Geometry& geo){
	wrappedLikelihood->SetGeometry(geo);
}

void BootstrappingLikelihoodService::SetEvent(const I3Frame& f){
	log_debug("BootstrappingLikelihoodService::SetEvent");
	sandbox.reset(new I3Frame(f));
	resampleCache.clear();
	originalPulses = sandbox->Get<boost::shared_ptr<const I3Map<OMKey,std::vector<I3RecoPulse> > > >(pulsesName);
	if(!originalPulses)
		log_fatal_stream(GetName() << ": Unable to find pulse map \"" << pulsesName << "\" in event");
	switch(bootstrapType){
		case POISSON:
			qtot=0.0;
			for(I3Map<OMKey,std::vector<I3RecoPulse> >::const_iterator domIt=originalPulses->begin(), domEnd=originalPulses->end(); domIt!=domEnd; domIt++){
				for(std::vector<I3RecoPulse>::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++)
					qtot+=pulseIt->GetCharge();
			}
			break;
		case MULTINOMIAL:
			//make a useful mapping of indices to OMKeys to aid fast lookup of individual pulses
			omIndex.clear();
			unsigned int counter=0;
			std::vector<double> probabilities;
			for(I3Map<OMKey,std::vector<I3RecoPulse> >::const_iterator domIt=originalPulses->begin(), domEnd=originalPulses->end(); domIt!=domEnd; domIt++){
				counter+=domIt->second.size();
				omIndex.push_back(std::make_pair(counter,domIt->first));
				for(std::vector<I3RecoPulse>::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++)
					probabilities.push_back(pulseIt->GetCharge());
			}
			if(!probabilities.empty()){
				qtot=std::accumulate(probabilities.begin(),probabilities.end(),0.0);
				multinomialDist = discrete_distribution(probabilities.begin(),probabilities.end());
			}
			else{
				qtot=0;
				log_warn_stream(pulsesName << " contains no pulses");
			}
			break;
	}
}

namespace{
	bool earlierTime(const I3RecoPulse& p1, const I3RecoPulse& p2){
		return(p1.GetTime()<p2.GetTime());
	}
	bool samePulse(const I3RecoPulse& p1, const I3RecoPulse& p2){
		//we actually want exact floating-point comparison here
		return(p1.GetTime()==p2.GetTime() && p1.GetWidth()==p2.GetWidth() && p1.GetFlags()==p2.GetFlags());
	}
}

boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > BootstrappingLikelihoodService::resampleEvent(unsigned int index){
	log_debug_stream("BootstrappingLikelihoodService::resampleEvent " << index);
	if(resampleCache.count(index)){
		log_debug(" returning cached resampling");
		return(resampleCache[index]);
	}
	log_debug(" resampling pulses");
	boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > resampledPulses(new I3Map<OMKey,std::vector<I3RecoPulse> >());
	switch(bootstrapType){
		case POISSON:
			//for each existing pulse, draw the number of duplicates to include from a poisson distribution
			for(I3Map<OMKey,std::vector<I3RecoPulse> >::const_iterator domIt=originalPulses->begin(), domEnd=originalPulses->end(); domIt!=domEnd; domIt++){
				for(std::vector<I3RecoPulse>::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++){
					for(int count=randomService->Poisson(pulseIt->GetCharge()); count>0; count--){
						I3RecoPulse pulse=*pulseIt;
						pulse.SetCharge(1.);
						(*resampledPulses)[domIt->first].push_back(pulse);
					}
				}
			}
			break;
		case MULTINOMIAL:
			//draw from a multinomial distribution until there is (roughly) the same total charge
			random_adapter gen(randomService.get());
			double resampledQ=0.0;
			while(resampledQ<qtot){
				unsigned int idx=multinomialDist(gen);
				std::vector<std::pair<unsigned int,OMKey> >::const_iterator keyData = std::upper_bound(omIndex.begin(),omIndex.end(),idx,compPulseIndex);
				I3RecoPulse pulse = (*originalPulses).find(keyData->second)->second[(keyData!=omIndex.begin()?idx-(keyData-1)->first:idx)];
				pulse.SetCharge(1.0);
				(*resampledPulses)[keyData->second].push_back(pulse);
				resampledQ+=1.0;
			}
			//sort pulses by time again, and coalesce duplicates
			for(I3Map<OMKey,std::vector<I3RecoPulse> >::iterator domIt=resampledPulses->begin(), domEnd=resampledPulses->end(); domIt!=domEnd; domIt++){
				if(domIt->second.empty())
					continue;
				std::sort(domIt->second.begin(),domIt->second.end(),&earlierTime);
				std::vector<I3RecoPulse> coalesced;
				coalesced.push_back(domIt->second.front()); //first pulse can't be a duplicate
				for(std::vector<I3RecoPulse>::iterator pulseIt=domIt->second.begin()+1, pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++){
					if(!samePulse(coalesced.back(),*pulseIt)) //if unique, add to final list
						coalesced.push_back(*pulseIt);
					else //if not unique, just increase the weight of the copy already in the list
						coalesced.back().SetCharge(coalesced.back().GetCharge()+pulseIt->GetCharge());
				}
				if(coalesced.size()!=domIt->second.size()) //if any changes were made, swap lists
					domIt->second.swap(coalesced);
			}
			log_trace_stream("  Original Qtot: " << qtot << ", Resampled Qtot: " << resampledQ);
			break;
	}
	//replace the pulses in the sandbox frame and tell the real likelihood to redo any set-up
	sandbox->Delete(pulsesName);
	sandbox->Put(pulsesName,resampledPulses);
	wrappedLikelihood->SetEvent(*sandbox);
	resampleCache[index]=resampledPulses;
	return(resampledPulses);
}

typedef I3SingleServiceFactory<BootstrappingLikelihoodService,I3EventLogLikelihoodBase> BootstrappingLikelihoodServiceFactory;
I3_SERVICE_FACTORY(BootstrappingLikelihoodServiceFactory);
