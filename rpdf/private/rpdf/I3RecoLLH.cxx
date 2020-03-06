/**
 * @brief Provides a Gulliver likelihood service for muon based reconstructions
 *
 * @copyright (C) 2018 The Icecube Collaboration
 *
 * @file I3RecoLLH.cxx
 * @author Kevin Meagher
 * @date January 2018
 *
 */

#include "gulliver/I3EventHypothesis.h"
#include "rpdf/I3RecoLLH.h"

I3RecoLLH::I3RecoLLH(const std::string &input_readout,
                      const std::string &likelihood,
                      const std::string &peprob,
                      const double jitter,
                      const double noise,
                      const rpdf::IceModel& ice):
  name_("I3RecoLLH"), inputReadout_(input_readout),
  likelihood_(likelihood),
  peprob_(peprob),jitter_(jitter),noise_(noise),ice_model_(ice)
{
  if (likelihood_=="SPE1st"){
    dom_likelihood_func_=rpdf::SPEfunc();
  }
  else if(likelihood_=="MPE"){
    dom_likelihood_func_=rpdf::MPEfunc();
  }
  else {
    log_fatal("I3RecoLLH service was configured with unimplemented DOM likelihood '%s'",likelihood_.c_str());
  }
  if ((peprob_=="GaussConvoluted" )
      || ( peprob_ == "GaussConvolutedWithNumericIntegral")
      || ( peprob_ == "GaussConvolutedFastApproximation")) {
    //these names use to mean different approximations but they were slower than
    //the current approximation, leaving them here for now for backwards
    //compatibility with I3GulliverIPDFPandelFactory
    pe_prob_ = std::make_shared<rpdf::FastConvolutedPandel>(jitter_,ice_model_);
  }
  else if (peprob_=="UnconvolutedPandel"){
    pe_prob_ = std::make_shared<rpdf::UnconvolutedPandel>(ice_model_);
  }
  else{
    log_fatal("I3RecoLLH service was configured with unimplemented Photoelectron Probability: '%s'",peprob_.c_str());
  }

}

I3RecoLLH::~I3RecoLLH()
{}

void I3RecoLLH::SetGeometry( const I3Geometry &geo )
{
  //save a pointer to the geometry so we can find the DOMs location later
  geoptr_ = boost::make_shared<const I3Geometry>(geo);
}

void I3RecoLLH::SetEvent( const I3Frame &f )
{
  //get the pulse map from the frame
  I3RecoPulseSeriesMapConstPtr pulse_map = f.Get< I3RecoPulseSeriesMapConstPtr >(inputReadout_);
  //calculate the hit cache from this pulse map
  SetPulseMap(*pulse_map);
}

void I3RecoLLH::SetPulseMap(const I3RecoPulseSeriesMap& pulse_map)
{
  //empty the old hit cache
  hit_cache_.clear();

  //loop over every DOM in the pulse map
  for (auto map_itr: pulse_map) {
    const OMKey& omkey = map_itr.first;
    const I3RecoPulseSeries& pulse_series=map_itr.second;

    //search the geometry to get the DOM's position
    const I3OMGeo& omgeo = geoptr_->omgeo.find(omkey)->second;

    //usually we don't get empty pulse series, but if we do, just skip it
    if (pulse_series.size()==0){continue;}

    //initialize the cache with zero charge and t=infinity
    I3HitCache hit_om = {0,
                         std::numeric_limits<double>::infinity(),
                         omgeo.position};

    //loop over the pulse series to sum the charge and find the earliest hit
    for (auto pulse_itr: pulse_series){
      // sum the charge from each pulse
      hit_om.total_npe += pulse_itr.GetCharge();

      //if this is the earliest hit save it
      if ( pulse_itr.GetTime() < hit_om.first_pulse_time ){
        hit_om.first_pulse_time = pulse_itr.GetTime();
      }
    }

    hit_cache_.push_back(hit_om);
  }
}

double I3RecoLLH::GetLogLikelihood( const I3EventHypothesis &eh )
{
  //extract the track hypothesis from Gulliver's data structure
  const I3Particle &track = *(eh.particle);

  //this stores the total likelihood for the event
  double event_loglikelihood=0.0;

  //loop over each DOM in the hit cache
  for (auto itr: hit_cache_) {
    //for each DOM calculate the likelihood
    const double Npe = itr.total_npe;
    const I3Position& om_pos = itr.pos;

    //get the geometrical parameters: tgeo and deff
    std::pair<double,double> geo_params = rpdf::muon_geometry(om_pos,track,ice_model_);
    //get the residual time from the time of the first pulse and the geometrical time
    const double t_res = itr.first_pulse_time - geo_params.first;
    //the effective distance is the second parameter
    const double deff = geo_params.second;
    //get the likelihood for the individual DOM
    const double dom_likelihood = dom_likelihood_func_(*pe_prob_,t_res,deff,Npe);

    //Add noise and take the log
    event_loglikelihood+=log(dom_likelihood+noise_);
  }

  return event_loglikelihood;
}

unsigned int I3RecoLLH::GetMultiplicity()
{
  return hit_cache_.size();
}

void I3RecoLLH::SetName(const std::string name)
{
  name_=name;
}

const std::string I3RecoLLH::GetName() const
{
  return name_;
}
