/**
 * copyright  (C) 2004
 * the icecube collaboration
 * $Id$
 *
 * @file I3LineFit.cxx
 * @version $Revision: 1.7 $
 * @date $Date$
 * @author deyoung
 * cc
 *
 * Changed to comply with dcv2 by bchristy.
 */

#include "linefit/I3LineFit.h"
#include "recclasses/I3LineFitParams.h"
#include "icetray/I3TrayHeaders.h"
#include "icetray/I3PhysicsTimer.h"
#include "dataclasses/I3Position.h"
#include "icetray/OMKey.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/Utility.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "I3LineFitExtras.h"

#include <iostream>
#include <boost/iterator/filter_iterator.hpp>

using namespace std;
using boost::filter_iterator;
using boost::make_filter_iterator;
using namespace LineFitExtras;

I3_MODULE(I3LineFit);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3LineFit::I3LineFit(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  AddOutBox("OutBox");

  fitName_ = "LineFit";
  AddParameter("Name", "Name to give the fit the module adds to the event", 
	       fitName_);

  minHits_ = 2;
  AddParameter("MinHits", 
	       "Minimum number of hits: events with fewer hits will not be "
	       "reconstructed.", 
	       minHits_);

  ampWeightPower_ = 0.0;
  AddParameter("AmpWeightPower", 
	       "Hits are weighted with the amplitude raised to this power.\n"
	       "Typically 0. (for all hits weight=1) or 1. (weight=amplitude)."
	       , ampWeightPower_);

  leadingEdge_ = "ALL";
  AddParameter("LeadingEdge", 
	       "What hits to use. ALL=all leading edges, FLE=just first LE", 
	       leadingEdge_);

  inputRecoPulses_ = "";
  AddParameter("InputRecoPulses",
	       "RecoPulseSeriesMap to use for input",
	       inputRecoPulses_);

}




/* ******************************************************************** */
/* Destructor                                                            */
/* ******************************************************************** */
I3LineFit::~I3LineFit(){
}



/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3LineFit::Configure()
{
  GetParameter("Name",fitName_);
  GetParameter("MinHits",minHits_);
  if (minHits_ < 1) {
    log_error ("Number Of Min hits < 1 (=%i). It was set to 2",minHits_);
    minHits_=2;
  }
  GetParameter("AmpWeightPower",ampWeightPower_);
  GetParameter("LeadingEdge",leadingEdge_);
  GetParameter("InputRecoPulses",inputRecoPulses_);

  log_info ("Input: Name=%s",fitName_.c_str());
  log_info ("Input: MinHits=%i",minHits_);
  log_info ("Input: AmpWeightPower=%f",ampWeightPower_);
  log_info ("Input: LeadingEdge=%s",leadingEdge_.c_str());
  log_info ("Input: InputRecoPulses=%s",inputRecoPulses_.c_str());

}


/** 
 * Execute the line fit reconstruction on the event in the provided frame.  The
 * reconstruction has an analytic solution consisting of a vertex and a
 * velocity.  The velocity is given by 
 * \f[\vec{v} = \frac{<\vec{r_i} * t_i> - <\vec{r_i}> <t_i>}{<t_i^2> - <t_i>^2}\f]
 * and the vertex is given by
 * \f$ \vec{r_0} = < \vec{r_i} > - \vec{v} * <t_i> \f$,
 * where <> indicates an average over hits i.         
 * 
 */ 

/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
void I3LineFit::Physics(I3FramePtr frame){
  log_debug("Entering LineFit Physics()... ");

  // Physics timer for getting the execution time of Physics() method of module
  I3PhysicsTimer timer(frame, GetName());

  // Declare the various parameters I need to calculate.
  double averageTime = 0;
  double averagePosition[3] = { 0, 0, 0};
  double averageTimeSquared = 0;
  double averageTP[3] = { 0, 0, 0};
  double ampsum=0;
  int nHits = 0;

  // Get Geometry from frame.
  const I3Geometry& geometry = frame->Get<I3Geometry>();
 
  // Get either RecoPulseSeries info from the frame.
  I3RecoPulseSeriesMapConstPtr pulsemap = 
    frame->Get<I3RecoPulseSeriesMapConstPtr>(inputRecoPulses_);

  //The Fill() function is defined in private/I3LineFitExtras.h
  if (pulsemap) {
    log_debug("Using RecoPulses '%s'",inputRecoPulses_.c_str());
    Fill(pulsemap, geometry, leadingEdge_, ampWeightPower_, 
	 averageTime, averageTimeSquared, averagePosition, 
	 averageTP, ampsum, nHits);
  }
  else {
    log_debug("RecoPulses '%s' is not present in the frame.", 
	      inputRecoPulses_.c_str());
    I3ParticlePtr track(new I3Particle);
    I3LineFitParamsPtr params(new I3LineFitParams);
    frame->Put(fitName_, track);
    frame->Put(fitName_+"Params", params);
    PushFrame(frame,"OutBox");
    return;
  }

  log_debug ("Read %d hits",nHits);


  // Create the result particle so we can fill it later and put it in frame.
  I3ParticlePtr track(new I3Particle());

  // Check minimum number of hits.
  if (nHits < minHits_) {
    log_info("Not Enough hits were found (read %i, Min set to %i.  Exiting.)",
 	      nHits,minHits_);
    track->SetFitStatus(I3Particle::InsufficientHits);
    frame->Put(fitName_, track);
    PushFrame(frame,"OutBox");
    return;
  }
 
  // Get the averages, and then calculate the velocity vector.
  averageTime /= ampsum;
  averageTimeSquared /= ampsum;
  for(int i = 0; i < 3; i++) {
    averagePosition[i] /= ampsum;
    averageTP[i] /= ampsum;
  }
  if (averageTimeSquared == averageTime*averageTime) {
    log_info("Reco will not converge");
    track->SetFitStatus(I3Particle::FailedToConverge);
    frame->Put(fitName_, track);
    PushFrame(frame,"OutBox");
    return;
  }

  log_debug("averageTime = %f", averageTime);
  log_debug("averageTimeSquared = %f", averageTimeSquared);
  log_debug("averagePosition = %f, %f, %f", 
	    averagePosition[0],averagePosition[1],averagePosition[2]);
  log_debug("averageTP = %f, %f, %f", averageTP[0],averageTP[1],averageTP[2]);

  // Calculate 'velocity' of track.
  double velocity[3];
  for(int i = 0; i < 3; i++) {
    velocity[i] = (averageTP[i] - averagePosition[i] * averageTime)
      / (averageTimeSquared - averageTime*averageTime);
  }
  log_debug("velocity = %f, %f, %f", velocity[0],velocity[1],velocity[2]);
  
  // Calculate 'speed' of track.
  double speed = 0;
  for(int i = 0; i < 3; i++) {
    speed += velocity[i] * velocity[i];
  }
  if (speed <= 0 ) { 
    log_info ("speed=%f is not positive. ",speed);
    track->SetFitStatus(I3Particle::GeneralFailure);
    frame->Put(fitName_, track);
    PushFrame(frame,"OutBox");
    return;
  }

  speed = sqrt(speed);
  log_debug("speed = %f", speed);

  // Calculate position of track.
  I3Position ptrack;
  ptrack.SetX(averagePosition[0]);
  ptrack.SetY(averagePosition[1]);
  ptrack.SetZ(averagePosition[2]);

  // Reco succeeded: Set track members.
  track->SetShape(I3Particle::InfiniteTrack);
  track->SetFitStatus(I3Particle::OK);
  track->SetPos(ptrack);
  track->SetDir(velocity[0],velocity[1],velocity[2]);
  track->SetTime(averageTime);
  track->SetSpeed(speed);

  I3LineFitParamsPtr params(new I3LineFitParams());
  params->LFVel = speed;
  params->LFVelX = velocity[0];
  params->LFVelY = velocity[1];
  params->LFVelZ = velocity[2];
  params->nHits = nHits;

  frame->Put(fitName_, track);
  frame->Put(fitName_+"Params", params);
  PushFrame(frame,"OutBox");

  log_debug("Exiting LineFit Physics.");
}
