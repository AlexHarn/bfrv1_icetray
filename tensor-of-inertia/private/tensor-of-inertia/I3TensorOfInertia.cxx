/**
 * copyright  (C) 2004
 * the icecube collaboration
 * $Id$
 *
 * @file I3TensorOfInertia.cxx
 * @version $Revision: 1.6 $
 * @date $Date$
 * @author grullon
 */

/*
 * The main module for the tensor of inertia reconstruction.  The main useful parameter created 
 * is the ratio of the minimum eigenvalue of the Inertia Tensor to the other eigenvalues. 
 */

#include <boost/tuple/tuple.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/foreach.hpp>

#include "tensor-of-inertia/I3TensorOfInertia.h"
#include "tensor-of-inertia/I3TensorOfInertiaCalculator.hpp"
#include "recclasses/I3TensorOfInertiaFitParams.h"
#include "icetray/I3TrayHeaders.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Position.h"
#include "icetray/OMKey.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "phys-services/I3Cuts.h"

using namespace std;
using boost::filter_iterator;
using boost::make_filter_iterator;

I3_MODULE(I3TensorOfInertia);


I3TensorOfInertia::I3TensorOfInertia(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  AddOutBox("OutBox");

  moduleName_ = "ti";
  inputReadout_ = "RecoPulses";
  minHits_ = 3;
  ampWeight_=1.0;
  
  AddParameter("Name","Name given to the fit the module adds to the event",
	       moduleName_);

  inputSelection_.clear();

  AddParameter("InputSelection","OMResponse selector to use for input",
	       inputSelection_);

   AddParameter("InputReadout",
	       "Data Readout to use for input",
	       inputReadout_);

  // minimum hits needed for the TOI algorithm
   AddParameter("MinHits",
	       "Minimum number of hits needed for reconstruction",
	       minHits_);

  // the following parameter is needed for the Inertia Tensor Algorithm,
  // a default value is assigned to 1
  AddParameter("AmplitudeWeight",
               "Weight applied to virtual masses in the reconstruction", 
               ampWeight_);

  // Amplitude option: treatment of small pulses
  // a default value = 1 for IceCube and TWR-Amanda; 
  // value = 0 for pre-TWR Amanda (used before) 
  AddParameter("AmplitudeOption","DEPRECATED : Treatment of small pulses in amplitude calculation",
              1);
 
}


void I3TensorOfInertia::Configure()
{
  GetParameter("Name",moduleName_);
  GetParameter("InputSelection",inputSelection_);
  GetParameter("InputReadout",inputReadout_);
  GetParameter("AmplitudeWeight",ampWeight_);
  GetParameter("MinHits",minHits_);
  int ampopt(1);
  GetParameter("AmplitudeOption",ampopt);
  if(ampopt != 1){
    log_error("This option is deprecated.");
  }
}

void I3TensorOfInertia::Physics(I3FramePtr frame)
{
  
  log_debug("Entering TensorOfInertia Physics().");
  
  const I3Geometry &geometry=frame->Get<I3Geometry>();
  const I3OMGeoMap& om_geo=geometry.omgeo;
  
  // The quantities I need to calculate are the Center of Gravity 
  // of the Hits, the Inertia Tensor, and the Eigenvalues & eigenvectors of 
  // the Inertia Tensor.  For the matrix related manipulations, I will use
  // ROOT's Matrix classes.  
  // I have replaced the ROOT matrix classes with calls to gsl. -ck

  // Get the recopulse series from the frame
  I3RecoPulseSeriesMapConstPtr pulse_series=frame->Get<I3RecoPulseSeriesMapConstPtr>(inputReadout_);

  if(pulse_series ==0) {
    
    I3ParticlePtr particle(new I3Particle);
    particle->SetFitStatus(I3Particle::GeneralFailure);
    frame->Put(moduleName_,particle);
    PushFrame(frame,"OutBox");
    log_info("Could not find pulses in the frame.  Leaving TensorOfInertia Physics().");
    return;
  }
  else if((int) pulse_series->size() < minHits_)
    {
      I3ParticlePtr particle(new I3Particle);
      particle->SetFitStatus(I3Particle::InsufficientHits);
      frame->Put(moduleName_,particle);
      PushFrame(frame,"OutBox");
      log_debug("Less hits than minhits parameter.  Leaving TensorOfInertia Physics().");
      return;
    }

  I3TensorOfInertiaCalculator calc(ampWeight_);

  // first, the center of gravity  
  I3Position cogPosition=I3Cuts::COG(geometry, *pulse_series);

  // check for NaN, paranoias sake
  if (std::isnan(cogPosition.GetX()) > 0) {
    log_warn("Got a nan for COG x.  Setting it to 0 instead.  Something could be screwy with a DOM calibration!!");
    cogPosition.SetX(0);
  
  }
   if (std::isnan(cogPosition.GetY()) > 0) {
    log_warn("Got a nan for COG y.  Setting it to 0 instead.  Something could be screwy with a DOM calibration!!");
    cogPosition.SetY(0);
   }
 if (std::isnan(cogPosition.GetZ()) > 0) {
    log_warn("Got a nan for COG z.  Setting it to 0 instead.  Something could be screwy with a DOM calibration!!");
    cogPosition.SetZ(0);
 }
    //ow, we can get the tensor of inertia.  
  I3Matrix toi=calc.CalculateTOI(pulse_series, om_geo, cogPosition);
  
  double evalratio=0.0;
  double eval2=0.0;
  double eval3=0.0;
  double mineval=0.0;
 

  // Here, we pass by reference the eigenvalue variables, and we calculate 
  // the eigenvalues of the tensor of inertia and the eigenvector.  
  eval_tuple_t reval_tuple = calc.DiagonalizeTOI(toi,mineval,eval2,eval3,evalratio);
  std::vector<double> minevect = boost::tuples::get<0>(reval_tuple);
  std::vector<double> eval2evect = boost::tuples::get<1>(reval_tuple);
  std::vector<double> eval3evect = boost::tuples::get<2>(reval_tuple);
  
  // now there is a corresponding ambiguity in the direction of the track given
  // by the eigenvector. This needs to be corrected 
  
  int corr = calc.CorrectDirection(pulse_series, om_geo, cogPosition, minevect); 
  
  // now with everything in place, time to use I3position, I3direction
  // and I3Particle and create a reconstructed single track.
  
  I3ParticlePtr particle(new I3Particle);
  
  if(!particle) {
    log_error("there is a problem with the pointer");
  }
    
  // Set the result time to the charge-weighted mean of the hit times.
  // Not the best choice, perhaps, but better than nothing
  double cog_time = 0, qtot = 0;
  for (I3RecoPulseSeriesMap::const_iterator i = pulse_series->begin();
   i != pulse_series->end(); i++) {
    BOOST_FOREACH(const I3RecoPulse &pulse, i->second) {
      cog_time += pulse.GetTime()*pulse.GetCharge();
      qtot += pulse.GetCharge();
    }
  }
  cog_time /= qtot;
  
  // the direction of the cascade is given by the eigenvector corresponding
  // to the smallest eigenvalue. The correction is taken into account  
  I3Direction dparticle;
  dparticle = I3Direction(corr*minevect[0],corr*minevect[1],corr*minevect[2]);
  particle->SetPos(cogPosition);
  particle->SetTime(cog_time);
  particle->SetDir(dparticle);
  particle->SetType(I3Particle::unknown);
  particle->SetShape(I3Particle::Cascade);
  particle->SetFitStatus(I3Particle::OK);

  I3ParticlePtr e2particle(new I3Particle);
  I3Direction e2dir;
  e2dir = I3Direction(eval2evect[0],eval2evect[1],eval2evect[2]);
  e2particle->SetPos(cogPosition);
  e2particle->SetTime(cog_time);
  e2particle->SetDir(e2dir);
  e2particle->SetType(I3Particle::unknown);
  e2particle->SetShape(I3Particle::Cascade);
  e2particle->SetFitStatus(I3Particle::OK);

  I3ParticlePtr e3particle(new I3Particle);
  I3Direction e3dir;
  e3dir = I3Direction(eval3evect[0],eval3evect[1],eval3evect[2]);
  e3particle->SetPos(cogPosition);
  e3particle->SetTime(cog_time);
  e3particle->SetDir(e3dir);
  e3particle->SetType(I3Particle::unknown);
  e3particle->SetShape(I3Particle::Cascade);
  e3particle->SetFitStatus(I3Particle::OK);


  I3TensorOfInertiaFitParamsPtr params(new I3TensorOfInertiaFitParams);
  // set all the TOI particle parameters
  params->mineval=mineval;
  params->evalratio=evalratio;
  params->eval2=eval2;
  params->eval3=eval3;

  double phi=particle->GetAzimuth();
  
  if (phi > 2.0*M_PI)
    {
      phi=phi-2.0*M_PI;
    }
  particle->SetDir(particle->GetZenith(),phi);
  
  log_debug("Fit Complete. We have a COG position of %f %f %f and a minimum eigenvalue of %f",
            cogPosition.GetX(), cogPosition.GetY(), cogPosition.GetZ(), mineval);

  frame->Put(moduleName_,particle);
  frame->Put(moduleName_+"Eval2",e2particle);
  frame->Put(moduleName_+"Eval3",e3particle);
  frame->Put(moduleName_+"Params",params);
    
  PushFrame(frame,"OutBox");
  log_debug("Leaving TensorOfInertia Physics().");

}

