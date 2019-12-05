/**
 * @brief implementation of the I3StartStopLProb class
 *
 * @file I3StartStopLProb.cxx
 * @version $Revision: 55694 $
 * @date $Date: 2009-06-08 19:39:53 +0200 (Mon, 08 Jun 2009) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This module returns the probability for a missed muon before the reconstructed start point and after the stop point. This can be compared to the probabilities for no muon in these regions causing no hit (i.e. a starting and/or stopping track). The ratio is a measure whether the track is infinite or not.
 */

#include "finiteReco/I3GulliverFinitePhPnh.h"
#include "finiteReco/I3StartStopLProb.h"
#include "recclasses/I3StartStopParams.h"
#include "icetray/I3Units.h"
#include "icetray/I3Frame.h"
#include "icetray/I3PhysicsTimer.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/physics/I3Particle.h"
#include "gulliver/I3EventHypothesis.h"

I3_MODULE(I3StartStopLProb);

/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3StartStopLProb::I3StartStopLProb(const I3Context& ctx) : I3ConditionalModule(ctx)
{ 
  AddOutBox("OutBox");

  fitName_ = "LineFit";
  AddParameter("Name", "Name of the fit used by the module", 
               fitName_);
  serviceName_ = "";
  AddParameter("ServiceName", "Name of a likelihood service (should be sensitive to the length of the track)",
               serviceName_);
}
/* ******************************************************************** */
/* Destructor                                                           */
/* ******************************************************************** */
I3StartStopLProb::~I3StartStopLProb(){
}

/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3StartStopLProb::Configure()
{
  GetParameter("Name",fitName_);
  GetParameter("ServiceName",serviceName_);

  if(!context_.Has<I3EventLogLikelihoodBasePtr>(serviceName_)){
    log_fatal("The service (%s) is not in the frame",serviceName_.c_str());
  }
  finite_ = context_.Get<I3EventLogLikelihoodBasePtr>(serviceName_);
  if(!finite_){
    log_fatal("The pointer to %s was not found",serviceName_.c_str());
  }
}

/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
double I3StartStopLProb::GetProbability(const I3ParticlePtr& track) const{
  I3EventHypothesis hypo;
  hypo.particle = track;
  return finite_->GetLogLikelihood(hypo);
}

void I3StartStopLProb::Physics(I3FramePtr frame){
  std::string instanceName = GetName();
  // Physics timer for getting the execution time of Physics() method of module
  I3PhysicsTimer timer(frame, instanceName);
  
  // output frame object
  I3StartStopParamsPtr SSPPtr(new I3StartStopParams());
  
  I3ParticleConstPtr track = frame->Get<I3ParticleConstPtr>(fitName_);
  if(!track){
    log_warn("Reconstruction not found, mispelled key name? Nothing done.");
    PushFrame(frame,"OutBox");
    return;
  }
  else if(track->GetFitStatus()!=I3Particle::OK){
    log_info("Fit failed, nothing done!");
    frame->Put(instanceName+"_StartStopParams",SSPPtr);
    PushFrame(frame,"OutBox");
    return;
  }
  I3ParticlePtr partPtr(new I3Particle(*track));
  
  finite_->SetEvent(*frame);
  
  double logStartProb = 0.;
  double logStopProb = 0.;
  double logInfProb = 0.;
  if(partPtr->GetShape()==I3Particle::InfiniteTrack){
    log_warn("This module is for starting/stopping or contained tracks!");
    logInfProb = GetProbability(partPtr);
    logStartProb = NAN;
    logStopProb = NAN;
  }
  else if(partPtr->GetShape()==I3Particle::ContainedTrack){
    double length = partPtr->GetLength();
    partPtr->SetLength(-1);
    partPtr->SetShape(I3Particle::StartingTrack);
    logStartProb = GetProbability(partPtr);
    I3Position stopPoint(partPtr->GetPos().GetX() + length * partPtr->GetDir().GetX(),
                         partPtr->GetPos().GetY() + length * partPtr->GetDir().GetY(),
                         partPtr->GetPos().GetZ() + length * partPtr->GetDir().GetZ());
    partPtr->SetPos(stopPoint);
    partPtr->SetShape(I3Particle::StoppingTrack);
    logStopProb = GetProbability(partPtr);
    partPtr->SetShape(I3Particle::InfiniteTrack);
    logInfProb = GetProbability(partPtr);
  }
  else if (partPtr->GetShape()==I3Particle::StartingTrack){
    partPtr->SetLength(-1);
    logStartProb = GetProbability(partPtr);
    partPtr->SetShape(I3Particle::InfiniteTrack);
    logInfProb = GetProbability(partPtr);
    logStopProb = NAN;
  }
  else if(partPtr->GetShape()==I3Particle::StoppingTrack){
    partPtr->SetLength(-1);
    logStopProb = GetProbability(partPtr);
    partPtr->SetShape(I3Particle::InfiniteTrack);
    logInfProb = GetProbability(partPtr);
    logStartProb = NAN;
  }
  else
    log_warn("This module is only for track-like particles, this particle "
             "has shape \"%d\".", partPtr->GetShape());
  
  SSPPtr->LLHStartingTrack = logStartProb;
  SSPPtr->LLHStoppingTrack = logStopProb;
  SSPPtr->LLHInfTrack      = logInfProb;
  frame->Put(instanceName+"_StartStopParams",SSPPtr);
  
  PushFrame(frame,"OutBox");
}
