/**
 * class: I3DipoleFit.C
 *
 * Version $Id$
 *
 * Date 07 Nov 2003
 *
 * (c) 2003 IceCube Collaboration
 */

#include "icetray/I3TrayHeaders.h"
#include "icetray/I3PhysicsTimer.h"
#include "dipolefit/I3DipoleFit.h"
#include "recclasses/I3DipoleFitParams.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/I3Position.h"
#include "icetray/OMKey.h"
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "I3DipoleFitExtras.h"
#include <iostream>

using namespace std;
using namespace dipolefit;

I3_MODULE(I3DipoleFit);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3DipoleFit::I3DipoleFit(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  fitName_ = "DipoleFit";
  AddParameter("Name", "module's name", fitName_);
  
  minHits_ = 5;
  AddParameter("MinHits", 
	       "Minimum number of hits: a dipole fit will not be tried if "
	       "there are fewer.",   
	       minHits_);

  // dipolestep_ = 0 means using step=N/2
  dipoleStep_ = 0;
  AddParameter("DipoleStep", 		      
	       "The dipole fit will (after sorting the hits to time)\n"
	       "determine the 'dipole' direction as:\n"
	       "sum(i) weight(i,i+N) * (pos(i+N) - pos(i) ) \n"
	       "divided by the sum of the weights.\n"
	       "With the DipoleStep option you set the value of N.", 
	       dipoleStep_);

  ampWeightPower_ = 0.0;
  AddParameter("AmpWeightPower", 
	       "Hits are weighted with the amplitude raised to this power.\n"
	       "Typically 0.0 (for all hits weight=1) or 1.0 "
	       "(weight=amplitude).",
	       ampWeightPower_);

  inputRecoPulses_ = "";
  AddParameter("InputRecoPulses",
	       "RecoPulses to use for input",
	       inputRecoPulses_);

  AddOutBox("OutBox");
}


/* ******************************************************************** */
/* Destructor                                                            */
/* ******************************************************************** */
I3DipoleFit::~I3DipoleFit(){
}


/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3DipoleFit::Configure()
{
  GetParameter("Name",fitName_);
  GetParameter("MinHits",minHits_);
  GetParameter("DipoleStep",dipoleStep_);
  GetParameter("AmpWeightPower", ampWeightPower_);
  if (minHits_ < 1) {
    log_debug ("Number Of Min hits < 1 (=%i). It was set to 2",minHits_);
    minHits_=2;
  }
  GetParameter("InputRecoPulses",inputRecoPulses_);

  log_debug ("Input: Parameter Name=%s",fitName_.c_str());
  log_debug ("Input: MinHits=%i",minHits_);
  log_debug ("Input: InputRecoPulses=%s",inputRecoPulses_.c_str());
  log_debug ("Input: AmpWeightPower=%f",ampWeightPower_);
  log_debug ("Input: Step size=%i",dipoleStep_);
}


/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
void I3DipoleFit::Physics(I3FramePtr frame)
{
  log_debug("Entering DipoleFit Physics()... ");
  
  // Physics timer for getting the execution time of Physics() method of module
  I3PhysicsTimer timer(frame, GetName());

  // Get geometry info from the frame.
  const I3Geometry& geometry = frame->Get<I3Geometry>();
  log_debug("Obtained total number of OMs: %i", 
	    (int)geometry.omgeo.size());

  // Store RecoPulse info locally into a vector of hits.
  vector<Hit> hits;
  I3RecoPulseSeriesMapConstPtr pulsemap = 
    frame->Get<I3RecoPulseSeriesMapConstPtr>(inputRecoPulses_);

  //The Fill() function is defined in private/I3DipoleFitExtras.h
  if (pulsemap) {
    log_debug("Using RecoPulses '%s'",inputRecoPulses_.c_str());
    Fill(pulsemap, geometry, hits);
  }
  else {
    log_error("RecoPulses '%s' is "
	      "present in the frame.", 
	      inputRecoPulses_.c_str());

    I3ParticlePtr track(new I3Particle());
    I3DipoleFitParamsPtr params(new I3DipoleFitParams());
    frame->Put(fitName_, track);
    frame->Put(fitName_+"Params", params);
    PushFrame(frame,"OutBox");
    return;
  }

  // Prepare result container objects.
  I3ParticlePtr track(new I3Particle());
  I3DipoleFitParamsPtr params(new I3DipoleFitParams());

  log_debug ("Read %zd hits",hits.size());
  params->nHits = hits.size();


  // Sort hits according to time.
  sort(hits.begin(), hits.end());
 
  double maxAmp=0;
  double vx=0., vy=0., vz=0.;
  double rx=0., ry=0., rz=0.;
  double t_mean=0;
  //double maxweight=0;
  int nPairs=0;
  double ampsum=0;
  int StepSize= dipoleStep_==0 ? int(hits.size()/2) : dipoleStep_;

  // Check minimum number of hits.
  if (int(hits.size())<minHits_) {
    log_info("Not enough hits were found (found %zd, min set to %i). Exiting.",
	      hits.size(),minHits_);
    track->SetFitStatus(I3Particle::InsufficientHits);
    frame->Put(fitName_, track);
    PushFrame(frame,"OutBox");
    return;
  }

  // Check step size.
  double ratio = 1.*StepSize/hits.size();
  log_debug("StepSize is %d/%zd=%f from number of hits",
	    StepSize, hits.size(), ratio);

  if (int(hits.size())<StepSize){
    log_info ("StepSize bigger than number of hits. Exiting.");
    track->SetFitStatus(I3Particle::InsufficientHits);
    frame->Put(fitName_, track);
    PushFrame(frame,"OutBox");
    return;
  }


  // Main calculation loop.
  for (int i1=0; i1 < int(hits.size())-StepSize; i1++){
    int i2=i1+StepSize;
    double ampValue=1.;
    if (ampWeightPower_ > 0.0) { 
      ampValue = 0.5*(hits[i2].amp + hits[i1].amp);
      ampValue = pow(ampValue,ampWeightPower_);
    }
    if ( ampValue > maxAmp ) maxAmp=ampValue;
      
    double xdiff = (hits[i2].x - hits[i1].x);
    double ydiff = (hits[i2].y - hits[i1].y);
    double zdiff = (hits[i2].z - hits[i1].z);   
    t_mean += ampValue*0.5 * (hits[i2].t + hits[i1].t);
    double distance = sqrt(xdiff*xdiff + ydiff*ydiff + zdiff*zdiff);
    if (distance == 0.) {
      log_debug ("Dipole Fit not converging. Distance=0 ");
      continue;
    }
    
    vx += ampValue*xdiff / distance;
    vy += ampValue*ydiff / distance;
    vz += ampValue*zdiff / distance;
    log_debug("vx=%f, vy=%f, vz=%f , xdiff=%f, ydiff=%f, zdiff=%f, distance=%f"
	      ,vx,vy,vz,xdiff,ydiff,zdiff,distance);
    
    rx += 0.5 * (hits[i2].x + hits[i1].x)*ampValue; 
    ry += 0.5 * (hits[i2].y + hits[i1].y)*ampValue; 
    rz += 0.5 * (hits[i2].z + hits[i1].z)*ampValue; 
    nPairs ++;
    ampsum=ampsum+ampValue;
  }
  
  params->nPairs = nPairs;


  if (ampsum <= 0.) {
    log_info("Reconstruction not converged, ampsum<=0. Exiting");
    track->SetFitStatus(I3Particle::FailedToConverge);
    frame->Put(fitName_, track);
    PushFrame(frame,"OutBox");
    return;
  }
  
  double magnet = sqrt(vx*vx + vy*vy + vz*vz);
  if (magnet <= 0.) {
    log_info("Reconstruction not converged, magnet<=0. vx=%f, vy=%f, vz=%f."
	      "Exiting.",vx,vy,vz);
    track->SetFitStatus(I3Particle::FailedToConverge);
    frame->Put(fitName_, track);
    PushFrame(frame,"OutBox");
    return;
  }

  track->SetShape(I3Particle::InfiniteTrack);
  track->SetFitStatus(I3Particle::OK);
  track->SetTime(t_mean/ampsum);
  track->SetPos(rx/ampsum, ry/ampsum, rz/ampsum );
  double zenith = I3Constants::pi - acos(vz/magnet);
  double azimuth = -atan2(vy,-vx);
  if (azimuth<0.0) azimuth=2.*I3Constants::pi+azimuth;
  track->SetDir(zenith,azimuth);

  params->magnet  = magnet/ampsum;
  params->magnetX = vx/ampsum;
  params->magnetY = vy/ampsum;
  params->magnetZ = vz/ampsum;
  params->ampSum  = ampsum;
  params->maxAmp  = maxAmp;

  // Store result in event.
  frame->Put(fitName_, track);
  frame->Put(fitName_+"Params", params);
  PushFrame(frame,"OutBox");

  log_debug("Exiting DipoleFit Physics.");
}
