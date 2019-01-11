// 
// MillipedeFisherMatrixCalculator: calculate and store the
// Fisher information matrix associated with a given energy
// loss pattern.
// 
// copyright  (C) 2012
// Jakob van Santen <vansanten@wisc.edu>
// The Icecube Collaboration: http://www.icecube.wisc.edu
// 
// $Id$
// 
// @version $Revision$
// @date $LastChangedDate$
// @author Jakob van Santen <vansanten@wisc.edu> Last changed by: $LastChangedBy$
// 

#include <millipede/Millipede.h>
#include <boost/make_shared.hpp>

class MillipedeFisherMatrixCalculator : public I3MillipedeConditionalModule {
public:
	MillipedeFisherMatrixCalculator(const I3Context &ctx)
	    : I3MillipedeConditionalModule(ctx)
	{
		AddParameter("Seed", "Name of seed fit in frame", "MillipededEdx");
		AddParameter("Output", "Name of output I3Matrix in frame",
		    "MillipededEdxFisherMatrix");
	
		AddOutBox("OutBox");
	}
	
	void Configure()
	{
		I3MillipedeConditionalModule::Configure();

		GetParameter("Seed", seed_name_);
		GetParameter("Output", output_name_);
	}
	
	void Physics(I3FramePtr frame)
	{
		if (!frame->Has(pulses_name_) || !frame->Has(seed_name_)) {
			PushFrame(frame);
			return;
		}
		
		typedef boost::shared_ptr<const I3Vector<I3Particle> >
		    I3VectorI3ParticleConstPtr;
		I3VectorI3ParticleConstPtr sources;
		if (!(sources =
		    frame->Get<I3VectorI3ParticleConstPtr>(seed_name_))) {
			const I3Particle &seed =
			    frame->Get<I3Particle>(seed_name_);
			if (seed.GetFitStatus() != I3Particle::OK) {
				PushFrame(frame);
				return;
			}
			sources =
			    boost::make_shared<I3Vector<I3Particle> >(1, seed);
		}
		
		DatamapFromFrame(*frame);
		cholmod_sparse *response_matrix = GetResponseMatrix(*sources);
		if (response_matrix == NULL)
			log_fatal("Null basis matrix");
		
		I3MatrixPtr fisher = Millipede::FisherMatrix(domCache_,
		    *sources, response_matrix, &c);
		
		frame->Put(output_name_, fisher);
		PushFrame(frame);
	}

private:
	std::string seed_name_, output_name_;
	SET_LOGGER("MillipedeFisherMatrixCalculator");

};

I3_MODULE(MillipedeFisherMatrixCalculator);
