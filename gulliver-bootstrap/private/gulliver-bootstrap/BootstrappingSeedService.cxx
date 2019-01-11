#include <icetray/I3ServiceBase.h>
#include <icetray/I3SingleServiceFactory.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <gulliver/I3SeedServiceBase.h>
#include <boost/make_shared.hpp>
#include <gulliver-bootstrap/BootstrappingLikelihoodService.h>

/**
 * \brief A seed service which supports bootstrapping.
 *
 * This seed service can either wrap another existing seed service, or can 
 * generate trivial 'LineFit' (least-squares) seeds based on the resampled 
 * pulses. If another seed service is wrapped, the total number of seeds this 
 * service will provide is the product of the number of seeds provided by the
 * wrapped service with the number of resamplings to be performed. 
 */
class BootStrappingSeedService : public I3ServiceBase, public I3SeedServiceBase{
public:
	BootStrappingSeedService(const I3Context& c):
	I3ServiceBase(c)
	{
		AddParameter("WrappedSeed","The actual seed service to use",seedName);
		AddParameter("BootstrappingLikelihood","The likelihood wrapper to use",likelihoodName);
	}
	
	void Configure(){
		GetParameter("WrappedSeed",seedName);
		GetParameter("BootstrappingLikelihood",likelihoodName);
		if(!seedName.empty()){
			wrappedSeed = context_.Get<boost::shared_ptr<I3SeedServiceBase> >(seedName);
			if(!wrappedSeed)
				log_fatal_stream("No seed service available under the name " << seedName);
		}
		likelihood = boost::dynamic_pointer_cast<BootstrappingLikelihoodService>(context_.Get<boost::shared_ptr<I3EventLogLikelihoodBase> >(likelihoodName));
		if(!likelihood)
			log_fatal_stream("No bootstrapping likelihood service available under the name " << likelihoodName);
	}
	
	virtual unsigned int SetEvent(const I3Frame &f){
		if(wrappedSeed)
			nSeeds = wrappedSeed->SetEvent(f);
		else
			nSeeds = 1;
		likelihood->SetEvent(f);
		geometry=f.Get<boost::shared_ptr<const I3Geometry> >();
		if(!geometry)
			log_fatal("Frame does not contain geometry data");
		return(nSeeds*likelihood->GetNIterations());
	}
	
	virtual I3EventHypothesis GetSeed(unsigned int iseed) const{
		unsigned int nResamplings = likelihood->GetNIterations();
		if(iseed>=(nResamplings*nSeeds))
			log_fatal_stream("Request for out of range seed (" << iseed << ") with " << nSeeds << " actual seeds and " << nResamplings << " resamplings");
		if(wrappedSeed){
			likelihood->resampleEvent(iseed%nResamplings);
			return(wrappedSeed->GetSeed(iseed/nResamplings));
		}
		else{
			boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > resampledPulses = likelihood->resampleEvent(iseed%nResamplings);
			return(reLineFit(resampledPulses));
		}
	}
	
	virtual I3EventHypothesis GetDummy() const{
		if(wrappedSeed)
			return(wrappedSeed->GetDummy());
		I3EventHypothesis result;
		result.particle->SetShape(I3Particle::InfiniteTrack);
		result.particle->SetFitStatus(I3Particle::OK);
		result.particle->SetPos(0,0,0);
		result.particle->SetTime(0);
		result.particle->SetDir(0,0,1);
		return(result);
	}
	
	virtual I3EventHypothesis GetCopy(const I3EventHypothesis& eh) const{
		if(wrappedSeed)
			return(wrappedSeed->GetCopy(eh));
		//TODO: This isn't general. Maybe that's a problem in some case?
		I3EventHypothesis result;
		if(eh.particle)
			result.particle = boost::make_shared<I3Particle>(*eh.particle);
		return(result);
	}
	
	virtual void Tweak(I3EventHypothesis& eh) const{
		if(wrappedSeed)
			wrappedSeed->Tweak(eh);
	}
	
	virtual void FillInTheBlanks(I3EventHypothesis& eh) const{
		if(wrappedSeed)
			wrappedSeed->FillInTheBlanks(eh);
	}
	
	I3EventHypothesis reLineFit(boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > pulses) const{
		double qtot=0.0, tSum=0.0, ttSum=0.0;
		I3Position posSum(0,0,0), postSum(0,0,0);
		for(I3Map<OMKey,std::vector<I3RecoPulse> >::const_iterator domIt=pulses->begin(), domEnd=pulses->end(); domIt!=domEnd; domIt++){
			std::map<OMKey,I3OMGeo>::const_iterator geomIt=geometry->omgeo.find(domIt->first);
			if(geomIt==geometry->omgeo.end())
				log_fatal_stream("Found a pulse on DOM " << domIt->first << " which does not appear in the geometry");
			const I3Position& ompos = geomIt->second.position;
			for(std::vector<I3RecoPulse>::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end(); pulseIt!=pulseEnd; pulseIt++){
				double q=pulseIt->GetCharge();
				double t=pulseIt->GetTime();
				qtot+=q;
				posSum+=q*ompos;
				tSum+=q*t;
				postSum+=q*t*ompos;
				ttSum+=q*t*t;
			}
		}
		posSum/=qtot;
		tSum/=qtot;
		postSum/=qtot;
		ttSum/=qtot;
		
		if((ttSum - tSum*tSum)==0) //might need to do something better, but at least this way the user is alerted
			log_error_stream("Singular problem; no valid linefit solution");
		
		I3EventHypothesis result;
		result.particle->SetShape(I3Particle::InfiniteTrack);
		result.particle->SetFitStatus(I3Particle::OK);
		result.particle->SetPos(posSum);
		result.particle->SetTime(tSum);
		I3Position velocity=(postSum-tSum*posSum)/(ttSum - tSum*tSum);
		result.particle->SetDir(I3Direction(velocity));
		//cursory testing seemed to suggest that leaving the speed unset gives slightly better results
		//result.particle->SetSpeed(velocity.Magnitude());
		
		return(result);
	}
	
private:
	///The name of the wrapped seed service, or empty if no base seed should be used
	std::string seedName;
	///The name of the corresponding BootstrappingLikelihoodService
	std::string likelihoodName;
	
	///The wrapped seed service, if any
	boost::shared_ptr<I3SeedServiceBase> wrappedSeed;
	///The associated likelihood
	boost::shared_ptr<BootstrappingLikelihoodService> likelihood;
	///The number of actual seeds, no counting resamplings
	unsigned int nSeeds;
	///The current geometry
	boost::shared_ptr<const I3Geometry> geometry;
};

typedef I3SingleServiceFactory<BootStrappingSeedService,I3SeedServiceBase> BootStrappingSeedServiceFactory;
I3_SERVICE_FACTORY(BootStrappingSeedServiceFactory);
