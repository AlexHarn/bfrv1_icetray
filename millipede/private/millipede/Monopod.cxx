/*
 * copyright  (C) 2011
 * Jakob van Santen <vansanten@wisc.edu>
 * The Icecube Collaboration: http://www.icecube.wisc.edu
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Jakob van Santen <vansanten@wisc.edu> Last changed by: $LastChangedBy$
 */

#include <millipede/Millipede.h>
#include <phys-services/geo-selector/I3GeoTrimmers.h>

#include <boost/python.hpp>
#include <boost/make_shared.hpp>

class Monopod : public I3MillipedeConditionalModule {
public:
	Monopod(const I3Context&);
	void Physics(I3FramePtr);
	void Configure();
	
private:
	SET_LOGGER("Monopod");

	std::string seed_name_, solution_name_;
};

Monopod::Monopod(const I3Context &ctx) : I3MillipedeConditionalModule(ctx)
{
	AddParameter("Seed", "Name of seed fit in frame", "");
	AddParameter("Output", "Name of output particle in frame",
	    "Monopod");
	
	AddOutBox("OutBox");
}

void
Monopod::Configure()
{
	I3MillipedeConditionalModule::Configure();

	GetParameter("Seed", seed_name_);
	GetParameter("Output", solution_name_);
}

void
Monopod::Physics(I3FramePtr frame)
{
	cholmod_sparse *response_matrix;

	if (!frame->Has(pulses_name_) || !frame->Has(seed_name_)) {
		PushFrame(frame);
		return;
	}

	const I3Particle &seed = frame->Get<I3Particle>(seed_name_);
	
	if (seed.GetFitStatus() != I3Particle::OK) {
		PushFrame(frame);
		return;
	}
	
	std::vector<I3Particle> sources;
	sources.push_back(seed);
	
	DatamapFromFrame(*frame);
	response_matrix = GetResponseMatrix(sources);
	if (response_matrix == NULL)
		log_fatal("Null basis matrix");
	SolveEnergyLosses(sources, response_matrix);

	MillipedeFitParamsPtr params(new MillipedeFitParams);
	Millipede::FitStatistics(domCache_, sources, 0, response_matrix,
	    &*params, &c);

	cholmod_l_free_sparse(&response_matrix, &c);

	I3ParticlePtr source = boost::make_shared<I3Particle>(sources[0]);

	frame->Put(solution_name_, source);
	frame->Put(solution_name_ + "FitParams", params);
	PushFrame(frame);
}

I3_MODULE(Monopod);
