/**
 * @brief implementation of the I3StartStopPoint class
 *
 * @file I3StartStopPoint.cxx
 * @version $Revision: 48479 $
 * @date $Date: 2008-08-22 11:18:02 +0200 (Fr, 22 Aug 2008) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This module estimates the start and stop point of a track. The algorithm is based on a projection from the DOMs to the given track along the Cherenkov cone.
 */

#include "finiteReco/I3StartStopPoint.h"
#include "finiteReco/I3FiniteCalc.h"
#include "recclasses/I3FiniteCuts.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Units.h"
#include "icetray/OMKey.h"
#include "icetray/I3PhysicsTimer.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "phys-services/I3Calculator.h"
#include "gulliver/I3EventHypothesis.h"

I3_MODULE(I3StartStopPoint);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3StartStopPoint::I3StartStopPoint(const I3Context& ctx) : I3ConditionalModule(ctx),
                                                           recoOKCounter_(0),
                                                           recoFailedCounter_(0),
                                                           inputRecoFailedCounter_(0)
{
  AddOutBox("OutBox");

  fitName_ = "LineFit";
  AddParameter("Name", "Name of the input I3Particle", 
               fitName_);

  inputRecoPulses_ = "";
  AddParameter("InputRecoPulses",
               "RecoPulseSeriesMap to use for input",
               inputRecoPulses_);
  cylinderRadius_ = 100*I3Units::m;
  AddParameter("CylinderRadius",
               "Cylinder radius for the calculation",
               cylinderRadius_);
  shape_ = I3Particle::StoppingTrack;
  AddParameter("ExpectedShape",
               "The expected track shape: StartingTrack, StoppingTrack or ContainedTrack",
               shapeInt_);
}

/* ******************************************************************** */
/* Destructor                                                            */
/* ******************************************************************** */
I3StartStopPoint::~I3StartStopPoint(){
}

/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3StartStopPoint::Configure()
{
  GetParameter("Name",fitName_);
  GetParameter("InputRecoPulses",inputRecoPulses_);
  GetParameter("CylinderRadius",cylinderRadius_);
  GetParameter("ExpectedShape",shapeInt_);
  if(shapeInt_==0){
    shape_ = I3Particle::Null;
    log_fatal("The particle shape specified in ExpectedShape is not supported.");
  } else if(shapeInt_==10){
    shape_ = I3Particle::Primary;
    log_fatal("The particle shape specified in ExpectedShape is not supported.");
  } else if(shapeInt_==20){
    shape_ = I3Particle::TopShower;
    log_fatal("The particle shape specified in ExpectedShape is not supported.");
  } else if(shapeInt_==30){
    shape_ = I3Particle::Cascade;
    log_fatal("The particle shape specified in ExpectedShape is not supported.");
  } else if(shapeInt_==40){
    shape_ = I3Particle::InfiniteTrack;
    log_fatal("The particle shape specified in ExpectedShape is not supported.");
  } else if(shapeInt_==50){
    shape_ = I3Particle::StartingTrack;
  } else if(shapeInt_==60){
    shape_ = I3Particle::StoppingTrack;
  } else if(shapeInt_==70){
    shape_ = I3Particle::ContainedTrack;
  } else log_error("Value for ExpectedShape is unknown (%d)",shapeInt_);
  log_debug("Set shape to %i",shape_);
}

/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */

void I3StartStopPoint::CalculateStartStopPoint(I3FramePtr frame,
                                               I3RecoPulseSeriesMapConstPtr pulsemap,
                                               const I3Particle& particle){
  // Get Geometry from frame.
  if(! frame->Has(I3DefaultName<I3Geometry>::value())){
    log_fatal("No geometry found!");
    return;
  }
  const I3Geometry& geometry = frame->Get<I3Geometry>();
  
  // get a copy of the particle to modify it
  I3ParticlePtr track(new I3Particle(particle));
  track->SetShape(shape_);
  
  if(!(particle.GetFitStatus()==I3Particle::OK)){
    log_info("Fit failed, nothing done!");
    I3FiniteCutsPtr finiteCuts(new I3FiniteCuts());
    
    frame->Put(fitName_+"_FiniteCuts",finiteCuts);
    frame->Put(fitName_+"_Finite", track);
    ++inputRecoFailedCounter_;
    return;
  }
  
  I3Particle  cylinder(particle);
  I3FiniteCalc finiteCalc(geometry,cylinder,pulsemap,cylinderRadius_);
  I3FiniteCutsPtr finiteCuts(new I3FiniteCuts(finiteCalc.GetCuts()));
  if(!finiteCuts) log_fatal("Pointer has no value. Wrong assignment"); // better assert?
  
  I3Position  pos = particle.GetPos();
  I3Direction dir = particle.GetDir();
  double time = particle.GetTime();
  
  bool timeIsNotNormal = false;
  if(!std::isnormal(particle.GetTime())){
    log_warn("Input time has no normal value");
    timeIsNotNormal = true;
  }
  
  if(shape_==I3Particle::StoppingTrack){
    I3Position endPos = finiteCalc.GetEventStop();
    double end = (endPos.GetX() - pos.GetX())/track->GetDir().GetX();
    time += end / I3Constants::c;
    
    track->SetTime(time);
    track->SetPos(endPos);
  }
  else if(shape_==I3Particle::StartingTrack || shape_==I3Particle::ContainedTrack){
    I3Position startPos = finiteCalc.GetEventStart();
    double start = (startPos.GetX() - pos.GetX())/track->GetDir().GetX();
    time += start / I3Constants::c;
    
    track->SetTime(time);
    track->SetPos(startPos);
    if (shape_==I3Particle::ContainedTrack) track->SetLength(finiteCuts->Length);
  }
  else{
    // should never happen...
    log_error("Please select a finite muon track type for this calculations.");
  }
  
  if(!std::isnormal(track->GetTime()) && !timeIsNotNormal){
    log_info("Time has no normal value -> Reco failed");
    ++recoFailedCounter_;
    track->SetFitStatus(I3Particle::InsufficientHits);
  }
  else{
    ++recoOKCounter_;
  }
  
  frame->Put(fitName_+"_FiniteCuts",finiteCuts);
  frame->Put(fitName_+"_Finite", track);
}


void I3StartStopPoint::Physics(I3FramePtr frame){
  
  // Physics timer for getting the execution time of Physics() method of module
  I3PhysicsTimer timer(frame, GetName());
  
  // get the reconstructed particle
  I3ParticleConstPtr particle = frame->Get<I3ParticleConstPtr>(fitName_);
  
  if(!particle){
    log_warn("Reconstruction not found, mispelled key name? Nothing done.");
    PushFrame(frame,"OutBox"); 
    return;
  }
  
  // get RecoPulseSeries info from the frame
  I3RecoPulseSeriesMapConstPtr pulsemap = 
    frame->Get<I3RecoPulseSeriesMapConstPtr>(inputRecoPulses_);
  
  if(pulsemap) CalculateStartStopPoint(frame,pulsemap,*particle);
  else log_warn("No pulses found, misspelled key name? Nothing done.");
  
  PushFrame(frame,"OutBox");
  return;
}


void I3StartStopPoint::Finish(){
  if(recoFailedCounter_ > 0){
    log_warn("%d recos failed due to failed input reconstructions, "
             "%d recos failed due to bad input reconstructions, "
             "%d recos succeeded.",
             inputRecoFailedCounter_, recoFailedCounter_, recoOKCounter_);
  }
}
