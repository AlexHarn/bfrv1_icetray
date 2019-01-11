/*
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Patrick Hallen <patrick.hallen@rwth-aachen.de> Last changed by: $LastChangedBy$
 */

#include <millipede/Millipede.h>

#include <phys-services/I3Calculator.h>
#include <lilliput/parametrization/I3SimpleParametrization.h>
#include <gulliver/I3ParametrizationBase.h>
#include <gulliver/I3EventHypothesis.h>
#include <icetray/I3ServiceFactory.h>
#include <icetray/I3SingleServiceFactory.h>

#include <string>

static void TauMillipedeHypothesis(I3ParticleConstPtr tau,
    std::vector<I3Particle> &hypothesis);

class TauMillipede : public I3MillipedeConditionalModule
{
	public:
		TauMillipede(const I3Context &);
		void Configure();
		void Hypothesis(I3FramePtr frame,
		    std::vector<I3Particle> &hypothesis);
		void Physics(I3FramePtr frame);
	private:
		SET_LOGGER("TauMillipede");

		std::string tau_;
		std::string output_name;
};

class TauMillipedeParametrization : public I3SimpleParametrization {
	public:
		/// constructor for use with icetray
		TauMillipedeParametrization(const I3Context& context);

		void UpdatePhysicsVariables();
		void ApplyChainRule();

	protected:
		SET_LOGGER("TauMillipedeParametrization");
};

typedef
    I3SingleServiceFactory<TauMillipedeParametrization, I3ParametrizationBase>
    TauMillipedeParametrizationFactory;
I3_SERVICE_FACTORY(TauMillipedeParametrizationFactory);
I3_MODULE(TauMillipede);

TauMillipede::TauMillipede(const I3Context &context) :
    I3MillipedeConditionalModule(context)
{
	AddOutBox("OutBox");

	AddParameter("Tau", "Tau track for which the energy deposition of the CC "
			"tau neutrino interaction will be set to the vertex and the tau "
			"decay to the end of the track.", "");
	AddParameter("Output", "Name of output", "MillipedeTau");
}

void
TauMillipede::Configure()
{
	I3MillipedeConditionalModule::Configure();

	GetParameter("Tau", tau_);
	GetParameter("Output", output_name);
}

void
TauMillipede::Physics(I3FramePtr frame)
{
	if (!frame->Has(pulses_name_) || !frame->Has(tau_)) {
		PushFrame(frame);
		log_debug("No pulses or seed found");
		return;
	}
	I3ParticleConstPtr tau = frame->Get<I3ParticleConstPtr>(tau_);
	if (tau->GetFitStatus() != I3Particle::OK) {
		log_info("Tau hypothesis has fit status %s", tau->GetFitStatusString().c_str());
		PushFrame(frame);
		return;
	}

	boost::shared_ptr<I3Vector<I3Particle> > sources(new
	    I3Vector<I3Particle>);
	TauMillipedeHypothesis(frame->Get<I3ParticleConstPtr>(tau_), *sources);
	if (sources->size() == 0) {
		PushFrame(frame);
		log_debug("Could not create tau hypothesis from seed");
		return;
	}

	cholmod_sparse *response_matrix;
	DatamapFromFrame(*frame);
	response_matrix = GetResponseMatrix(*sources);
	if (response_matrix == NULL)
		log_fatal("Null basis matrix");
	SolveEnergyLosses(*sources, response_matrix);

	MillipedeFitParamsPtr params(new MillipedeFitParams);
	Millipede::FitStatistics(domCache_, *sources, 0, response_matrix,
	    &*params, &c);
	cholmod_l_free_sparse(&response_matrix, &c);

	frame->Put(output_name, sources);
	frame->Put(output_name + "FitParams", params);

	PushFrame(frame);
}

TauMillipedeParametrization::TauMillipedeParametrization(const I3Context &context)
			: I3SimpleParametrization(context)
{
}


void
TauMillipedeParametrization::UpdatePhysicsVariables()
{
	// Do the hard work, i.e. write the time, vertex, length and direction values
	// to hypothesis_->particle
	I3SimpleParametrization::UpdatePhysicsVariables();

	// Set energy to the total energy of the two sources
	boost::shared_ptr<I3Vector<I3Particle> > sources =
	  boost::dynamic_pointer_cast<I3Vector<I3Particle> >
	  (hypothesis_->nonstd);
	if (sources && sources->size() == 2) {
		hypothesis_->particle->SetEnergy((*sources)[0].GetEnergy()
									   + (*sources)[1].GetEnergy());
	}
	// Create updated tau hypothesis
	sources.reset(new I3Vector<I3Particle>);
	TauMillipedeHypothesis(hypothesis_->particle, *sources);
	hypothesis_->nonstd = sources;

	if (gradient_) {
		boost::shared_ptr<I3Vector<I3Particle> > gradsources(new
		    I3Vector<I3Particle>);
		gradsources->resize(sources->size());
		for (unsigned i = 0; i < gradsources->size(); ++i) {
			I3Particle &gradpart = (*gradsources)[i];
			gradpart.SetPos(0., 0., 0.);
			gradpart.SetDir(0., 0.);
			gradpart.SetTime(0.);
			gradpart.SetEnergy(0.);
			gradpart.SetLength(0.);
			gradpart.SetSpeed(0.);
		}
		gradient_->nonstd = gradsources;
	}
}

void
TauMillipedeParametrization::ApplyChainRule()
{
	I3Particle& gradient = *(gradient_->particle);
	boost::shared_ptr<I3Vector<I3Particle> > gradsources =
	  boost::dynamic_pointer_cast<I3Vector<I3Particle> >(gradient_->nonstd);
	boost::shared_ptr<I3Vector<I3Particle> > sources =
	  boost::dynamic_pointer_cast<I3Vector<I3Particle> >
	  (hypothesis_->nonstd);

	I3Particle &grad_cc = (*gradsources)[0];
	I3Particle &grad_decay = (*gradsources)[1];
	I3Particle &cc = (*sources)[0];
	I3Particle &decay = (*sources)[1];

	double grad_x	= grad_cc.GetX()		+ grad_decay.GetX();
	double grad_y	= grad_cc.GetY()		+ grad_decay.GetY();
	double grad_z	= grad_cc.GetZ()		+ grad_decay.GetZ();
	double grad_t	= grad_cc.GetTime()		+ grad_decay.GetTime();

	double length	= cc.GetLength();
	double zenith	= cc.GetZenith();
	double azimuth	= cc.GetAzimuth();
	double grad_zen	= grad_cc.GetZenith()	+ grad_decay.GetZenith()
		- length * (  grad_decay.GetX() * cos(zenith) * cos(azimuth)
					+ grad_decay.GetY() * cos(zenith) * sin(azimuth)
					- grad_decay.GetZ() * sin(zenith));
	double grad_azi	= grad_cc.GetAzimuth()	+ grad_decay.GetAzimuth()
		- length * (- grad_decay.GetX() * sin(zenith) * sin(azimuth)
					+ grad_decay.GetY() * sin(zenith) * cos(azimuth));
	double grad_l	= grad_decay.GetTime() / decay.GetSpeed()
		-          (  grad_decay.GetX() * sin(zenith) * cos(azimuth)
					+ grad_decay.GetY() * sin(zenith) * sin(azimuth)
					+ grad_decay.GetZ() * cos(zenith));

	gradient.SetPos(grad_x, grad_y, grad_z);
	gradient.SetTime(grad_t);
	gradient.SetDir(grad_zen, grad_azi);
	gradient.SetLength(grad_l);

	I3SimpleParametrization::ApplyChainRule();
}

static void
TauMillipedeHypothesis(I3ParticleConstPtr tau,
    std::vector<I3Particle> &hypothesis)
{
	double xspeed, yspeed, zspeed;

	// Projections of particle speed on each axis
	xspeed = tau->GetSpeed()*sin(tau->GetZenith() / I3Units::radian)*
	    cos(tau->GetAzimuth() / I3Units::radian);
	yspeed = tau->GetSpeed()*sin(tau->GetZenith() / I3Units::radian)*
	    sin(tau->GetAzimuth() / I3Units::radian);
	zspeed = tau->GetSpeed()*cos(tau->GetZenith() / I3Units::radian);

	I3Particle cc(*tau);
	// change tau shape to cascade, because we want to reconstruct the nu_tau
	// cc interaction and not the tau track
	cc.SetShape(I3Particle::Cascade);
	hypothesis.push_back(cc);

	// add tau decay cascade
	double delta_t = tau->GetLength() / tau->GetSpeed();
	I3Particle decay;
	decay.SetDir(tau->GetDir());
	decay.SetLength(0);
	decay.SetSpeed(tau->GetSpeed());
	decay.SetShape(I3Particle::Cascade);
	decay.SetTime(tau->GetTime() + delta_t);
	decay.SetPos(tau->GetX() - xspeed*delta_t,
			  tau->GetY() - yspeed*delta_t,
			  tau->GetZ() - zspeed*delta_t);
	hypothesis.push_back(decay);
}

