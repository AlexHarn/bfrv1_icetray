#ifndef BOOTSTRAPPINGLIKELIHOODSERVICE_H
#define BOOTSTRAPPINGLIKELIHOODSERVICE_H

#include <icetray/I3ServiceBase.h>
#include <gulliver/I3EventLogLikelihoodBase.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <phys-services/I3RandomService.h>
#include "discreteDistribution.h"

/**
 * \brief A likelihood service which supports bootstrapping.
 *
 * This likelihood service wraps another likelihood service which is used for 
 * all actual likelihood calculations. Its contribution is to create resampled
 * collections of pulses, which are then passed to the underlying likelihood. 
 * In order to do this it should be used together with a BootstrappingSeedService
 * which will cause the fitter to sequentially fit all of the resampled 
 * realizations.
 */
class BootstrappingLikelihoodService : public I3EventLogLikelihoodBase, public I3ServiceBase{
public:
	BootstrappingLikelihoodService(const I3Context& c);
	virtual void Configure();
	virtual void SetGeometry(const I3Geometry& geo);
	virtual void SetEvent(const I3Frame& f);
	virtual double GetLogLikelihood(const I3EventHypothesis& ehypo){
		return(wrappedLikelihood->GetLogLikelihood(ehypo));
	}
	virtual double GetLogLikelihoodWithGradient(const I3EventHypothesis& ehypo,I3EventHypothesis& gradient,double weight=1){
		return(wrappedLikelihood->GetLogLikelihoodWithGradient(ehypo,gradient,weight));
	}
	virtual bool HasGradient(){
		return(wrappedLikelihood->HasGradient());
	}
	virtual unsigned int GetMultiplicity(){
		return(wrappedLikelihood->GetMultiplicity());
	}
	virtual I3FrameObjectPtr GetDiagnostics(const I3EventHypothesis& fitresult){
		return(wrappedLikelihood->GetDiagnostics(fitresult));
	}
	virtual const std::string GetName() const{
		return(I3ServiceBase::GetName());
	}
	unsigned int GetNIterations() const{
		return(iterations);
	}
	boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > resampleEvent(unsigned int index);
	
	enum BootstrapOption{
		///Resample pulses by drawing a number of copies of each pulse according
		///to a poisson distribution with mean equal to that pulse's charge
		POISSON=0,
		///Resample pulses by drawing copies of each pulse proportionally to the
		///ratio of that pulse's charge to the total charge of the event, until
		///the total resampled charge is approximately the original total charge
		MULTINOMIAL=1
	};
private:
	///The name of the input pulses, which will be resampled
	std::string pulsesName;
	///The type of resampling to perform
	BootstrapOption bootstrapType;
	///The number of resamplings to perform
	unsigned int iterations;
	///The name of the random service to use
	std::string randomName;
	///The name of the wrapped likelihood service
	std::string likelihoodName;
	
	///The random service used for resampling
	boost::shared_ptr<I3RandomService> randomService;
	///The actual likelihood
	boost::shared_ptr<I3EventLogLikelihoodBase> wrappedLikelihood;
	///Dummy frame used to present the resampled pulses to the wrapped likelihood
	boost::shared_ptr<I3Frame> sandbox;
	///The current base set of pulses
	boost::shared_ptr<const I3Map<OMKey,std::vector<I3RecoPulse> > > originalPulses;
	///The collection of resampled pulse realizations.
	///Calculating these once and storing them makes the calculation robust against
	///a fitter which requests seeds out of order or multiple times.
	std::map<unsigned int, boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > > resampleCache;
	///Index structure for sampling pulses multinomially
	std::vector<std::pair<unsigned int,OMKey> > omIndex;
	///Probability distribution for multinomial pulse sampling
	discrete_distribution multinomialDist;
	///The total charge of the current event
	double qtot;
};

#endif