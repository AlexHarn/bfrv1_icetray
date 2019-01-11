/**
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3IPdfLikelihoodModule.cxx
 * @version $Revision: 1.5 $
 * @date $Date$
 * @author Simon Robbins
 *
 * @brief An example muon log-likelihood reconstruction using IPDF.
 *
 * The original I3 module structure was based on muon-llh-reco.
 */

#include <cassert>
#include <sstream>


#include "ipdf/I3/I3IPdfLikelihoodModule.h"

#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3DetectorResponse.h"
#include "ipdf/I3/I3HitOm.h"
#include "ipdf/I3/I3OmReceiver.h"

#include "icetray/I3Frame.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3AMANDAAnalogReadout.h"

I3_MODULE(I3IPdfLikelihoodModule);

I3IPdfLikelihoodModule::I3IPdfLikelihoodModule(const I3Context& ctx) : 
  I3Module(ctx),
  inputDataReadout_("CleanHits"),
  inputSeedName_("NotSet"),
  printConfig_(false),
  likelihood_()
{
  AddParameter("InputDataReadout",
	       "The data source",
	       inputDataReadout_);
  
  AddParameter("InputSeedName",
	       "Name of the seed track (e.g. \"LineFit\")",
	       inputSeedName_);
  
  AddParameter("PrintConfig",
	       "Print the configuration at the start of the run (default: no)",
	       printConfig_);
  
  AddOutBox("OutBox");
}

I3IPdfLikelihoodModule::~I3IPdfLikelihoodModule() {
}

void I3IPdfLikelihoodModule::Configure(){
  log_debug("Configuring the I3IPdfLikelihoodModule");

  GetParameter("InputDataReadout",inputDataReadout_);
  GetParameter("InputSeedName",inputSeedName_);
  GetParameter("PrintConfig",printConfig_);

  if(printConfig_) {
    std::ostringstream conf;
    //    conf<<context_;
    log_debug("Configured with the following configuration: %s",
	conf.str().c_str());
  }
}

void I3IPdfLikelihoodModule::Geometry(I3FramePtr frame){
  log_debug("Entering I3IPdfLikelihoodModule::Geometry()");

  try {
    const I3Geometry& i3geom = frame->Get<I3Geometry>();
    dgeom_ = boost::shared_ptr<IPDF::I3DetectorConfiguration>(
		new IPDF::I3DetectorConfiguration(i3geom)
	     );
  } catch(std::exception& e) {
    log_error("failed to get detector geometry. %s",e.what());
    throw;
  }

  PushFrame(frame);
}

void I3IPdfLikelihoodModule::Physics(I3FramePtr frame){
  log_debug("Entering I3IPdfLikelihoodModule::Physics()");

  I3ParticleConstPtr hypothesis = this->retrieveSeed(frame);

  if(!this->checkTrack(hypothesis)) {
    log_debug("Skipping an event because the input track was unusable");
    return;
  }

  I3ParticlePtr recoresult;

  // Try to find some kind of detector response:
  // (can do I3RecoHitSeries, I3RecoPulseSeries or I3AMANDAAnalogReadoutMap)
  I3RecoHitSeriesMapConstPtr recohits =
      frame->Get<I3RecoHitSeriesMapConstPtr>(inputDataReadout_);

  if(recohits) {
    recoresult = this->DoReco(recohits,hypothesis);
  } else {

    I3RecoPulseSeriesMapConstPtr recopuls =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(inputDataReadout_);
    recoresult = this->DoReco(recopuls,hypothesis);

    if(!recoresult) {
      I3AMANDAAnalogReadoutMapConstPtr amareads =
	  frame->Get<I3AMANDAAnalogReadoutMapConstPtr>(inputDataReadout_);
      recoresult = this->DoReco(amareads,hypothesis);

    } else {
      log_fatal("Failed to get any detector response named \"%s\".",
	  inputDataReadout_.c_str());
    }
  }

  frame->Put("ipdf",recoresult);
//  frame->Put("ipdfParams", bestParams);

  PushFrame(frame,"OutBox");
}

template<class Readout>
I3ParticlePtr I3IPdfLikelihoodModule::DoReco(const Readout& response,
					     const I3ParticleConstPtr& hypothesis){
  if(!response) {
    log_debug("info: failed to get detector response named \"%s\" (may not be serious).",
	inputDataReadout_.c_str());
    return I3ParticlePtr(); /* empty */
  }
  log_debug("found detector response \"%s\".",inputDataReadout_.c_str());

  const IPDF::I3DetectorResponse dresponse(response,*dgeom_);

  IPDF::InfiniteMuon result_track =
      this->maxLikelihood(dresponse,hypothesis);

  I3ParticlePtr recoresult = result_track.makeI3Particle();

  if(!minimizer_.converged()) {
    log_debug("minimizer not converged");
    recoresult->SetFitStatus(I3Particle::FailedToConverge);
  } else {
    recoresult->SetFitStatus(I3Particle::OK);
  }
  return recoresult;
}

IPDF::InfiniteMuon I3IPdfLikelihoodModule::maxLikelihood(
      const IPDF::I3DetectorResponse& dresponse,
      I3ParticleConstPtr i3track
    ) {

  IPDF::InfiniteMuon hypothesis( i3track );

  {
    std::ostringstream info;
    info << hypothesis;
    log_debug("Seed:   %s", info.str().c_str());
  }
  {
    std::ostringstream info;
    info << likelihood_.getLikelihood(dresponse, hypothesis);
    log_debug("Initial log likelihood: %s", info.str().c_str());
  }
  
  IPDF::InfiniteMuon result_track =
      minimizer_.minimize(likelihood_,dresponse,hypothesis);

  IPDF::minimization_result resulting_likelihood =
      -minimizer_.result();

  {
    std::ostringstream info;
    info << result_track;
    log_debug("Result: %s", info.str().c_str());
  }
  {
    std::ostringstream info;
    info << resulting_likelihood;
    log_debug("Maximized log likelihood: %s", info.str().c_str());
  }
  return result_track;
}

// Try to get the seed from the given reconstruction result name ("seedTrackName_")
// Otherwise, get the seed from the MC truth muon
I3ParticleConstPtr I3IPdfLikelihoodModule::retrieveSeed(const I3FramePtr& frame) const
{
  I3ParticleConstPtr the_track = frame->Get<I3ParticleConstPtr>(inputSeedName_);
  if(the_track) return the_track;

  I3Frame::const_iterator iter;
  // for all tracks in the multimap with the right name, get the first
  // one that's a muon.
  for(iter = frame->begin() ;
      iter != frame->end();
      iter++)
  {
    I3ParticleConstPtr track 
	= boost::dynamic_pointer_cast<I3ParticleConstPtr::element_type>(iter->second);
    if(track)
      if(track->IsTrack() &&
         ((track->GetType() == I3Particle::MuMinus) ||
	  (track->GetType() == I3Particle::MuPlus)))
      {
	the_track = track;
	break;
      }
  }
  if(!the_track)
  {
    log_fatal("FATAL: couldn't find a seed (tried \"%s\").",inputSeedName_.c_str());
  }
//  log_debug(the_track->ToString().c_str());

  return the_track;
}

bool I3IPdfLikelihoodModule::checkTrack(const I3ParticleConstPtr& track)
{
  // checking that all of these are finite numbers
  // NB the multiplication will result in a NAN or INF if any of the track's 
  // fields are NAN or INF, so it's a big 'or'.
  return finite(track->GetPos().GetX() * track->GetPos().GetY() * track->GetPos().GetZ() * track->GetDir().GetZenith() * track->GetDir().GetAzimuth());
}
