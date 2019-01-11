/**
 * @brief implementation of the I3LengthLLH class
 *
 * @file I3LengthLLH.cxx
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * A module to reconstruct the length of a given track. The direction and position of the track is kept fixed, thus this only a 1D optimization.
 * For this optimization, first a fixed start point is chosen and the stop point is varied along the track. For each variation the likelihood value is calculated. Afterwards, the newly found stop point is fixed and the start point is optimized in the same way.
 * Finally start and stop point are known, and thus the length.
 */

#include "finiteReco/I3GulliverFinitePhPnh.h"
#include "finiteReco/I3LengthLLH.h"
#include "icetray/I3Frame.h"
#include "icetray/I3PhysicsTimer.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Vector.h"
#include "gulliver/I3EventHypothesis.h"

I3_MODULE(I3LengthLLH);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3LengthLLH::I3LengthLLH(const I3Context& ctx) : I3Module(ctx)
{
  AddOutBox("OutBox");

  fitName_ = "LineFit";
  AddParameter("InputName", "Input event", 
               fitName_);
  serviceName_ = "";
  AddParameter("ServiceName", "Name of a likelihood service (should be sensitive to the length of the track)", 
               serviceName_);
  stepSize_= 1*I3Units::m;
  AddParameter("StepSize", "Size of length steps", 
               stepSize_);
  maxLength_= 2*I3Units::km;
  AddParameter("MaxLength", "Maximum tested length", 
               maxLength_);
}

/* ******************************************************************** */
/* Destructor                                                            */
/* ******************************************************************** */
I3LengthLLH::~I3LengthLLH(){
}

/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3LengthLLH::Configure(){ 
  GetParameter("InputName",fitName_); 
  GetParameter("ServiceName",serviceName_);
  GetParameter("StepSize",stepSize_);
  GetParameter("MaxLength",maxLength_);
  if(!context_.Has<I3EventLogLikelihoodBasePtr>(serviceName_)){
    log_fatal("The service is not in the frame");
  }
  finite_ = context_.Get<I3EventLogLikelihoodBasePtr>(serviceName_);  
  if(!finite_){
    log_fatal("The pointer was not found");
  }
}

/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
static I3Position GetNewPos(const I3ParticleConstPtr& partPtr,
                            const std::map<double,double>& llhMap, 
                            const int& sgn){
  double trueLength = llhMap.begin()->first;
  double maxLLH = llhMap.begin()->second;
  std::map<double,double>::const_iterator illhMap = llhMap.begin();
  for(;illhMap!=llhMap.end();illhMap++){
    if(illhMap->second > maxLLH){
      maxLLH = illhMap->second;
      trueLength = illhMap->first;
    }
  }
  illhMap--;
  if(trueLength == illhMap->first){ // check whether the last entry was the maximum
    trueLength /= 2;
    log_warn("No maximum found. Probably not stopping or starting. Length set to %e.",trueLength);
  }
  
  double new_x = partPtr->GetPos().GetX() + sgn* trueLength * partPtr->GetDir().GetX();
  double new_y = partPtr->GetPos().GetY() + sgn* trueLength * partPtr->GetDir().GetY();
  double new_z = partPtr->GetPos().GetZ() + sgn* trueLength * partPtr->GetDir().GetZ();
  I3Position pos(new_x,new_y,new_z);
  return pos;
}

static I3Position GetStartPos(const I3ParticleConstPtr& partPtr,
                              const std::map<double,double>& llhMap){
  return GetNewPos(partPtr,llhMap,-1);
}
static I3Position GetStopPos(const I3ParticleConstPtr& partPtr,
                             const std::map<double,double>& llhMap){
  return GetNewPos(partPtr,llhMap,1);
}

double I3LengthLLH::GetProbability(const I3ParticlePtr& track)const{
  I3EventHypothesis hypo;
  hypo.particle = track;
  log_debug("x Pos %e",(*hypo.particle).GetPos().GetX());  
  return finite_->GetLogLikelihood(hypo);
}

std::map<double,double> I3LengthLLH::DoLengthIter(const I3ParticlePtr& partPtr)const { 
  std::map<double,double> llhMap;
  double ilength = 1*I3Units::m;
  I3ParticlePtr track( new I3Particle ( * partPtr ) );
  track->SetShape(I3Particle::ContainedTrack);
  for(;ilength < maxLength_;ilength+=stepSize_){
    track->SetLength(ilength);
    if(partPtr->GetShape() == I3Particle::StoppingTrack){
      I3Position stopPos = partPtr->GetPos();
      double new_x = stopPos.GetX() - ilength * partPtr->GetDir().GetX();
      double new_y = stopPos.GetY() - ilength * partPtr->GetDir().GetY();
      double new_z = stopPos.GetZ() - ilength * partPtr->GetDir().GetZ();
      track->SetPos(new_x,new_y,new_z);
    }
    llhMap.insert(std::make_pair(ilength,GetProbability(track)));
  }
  return llhMap;
}


void I3LengthLLH::Physics(I3FramePtr frame){
  
  // Physics timer for getting the execution time of Physics() method of module
  I3PhysicsTimer timer(frame, GetName());

  // Get the reconstructed particle
  I3ParticleConstPtr particle = frame->Get<I3ParticleConstPtr>(fitName_);
  I3ParticlePtr partPtr(new I3Particle(*particle));

  finite_->SetEvent(*frame);
  double length = 0;
  
  std::map<double,double> LLHLengthStart;
  std::map<double,double> LLHLengthStop;
  if(partPtr->GetShape()==I3Particle::InfiniteTrack){
    log_fatal("Not possible to handle infinite tracks, yet. Use I3StartStopPoint from finiteReco first!");
  }
  else if (partPtr->GetShape()==I3Particle::StoppingTrack){
    LLHLengthStart = DoLengthIter(partPtr);
    I3Position startPos = GetStartPos(partPtr,LLHLengthStart);
    partPtr->SetPos(startPos);
    partPtr->SetShape(I3Particle::StartingTrack);

    LLHLengthStop = DoLengthIter(partPtr);
    I3Position stopPos = GetStopPos(partPtr,LLHLengthStop);
    length = (startPos-stopPos).Magnitude();
    partPtr->SetPos(startPos);
  }
  else if (partPtr->GetShape()==I3Particle::StartingTrack ||
           partPtr->GetShape()==I3Particle::ContainedTrack){
    LLHLengthStop = DoLengthIter(partPtr);
    I3Position stopPos = GetStopPos(partPtr,LLHLengthStop);
    partPtr->SetPos(stopPos);
    partPtr->SetShape(I3Particle::StoppingTrack);
    
    LLHLengthStart = DoLengthIter(partPtr);
    I3Position startPos = GetStartPos(partPtr,LLHLengthStart);
    length = (startPos-stopPos).Magnitude();
    partPtr->SetPos(startPos);
  }
  else log_fatal("Track shape '%s' is not supported!", partPtr->GetShapeString().c_str());
  
  partPtr->SetLength(length);
  partPtr->SetShape(I3Particle::ContainedTrack);
  
  //Writing stuff to the frame (unfortunately we can't write the maps)
  I3VectorDoublePtr startLLH(new I3VectorDouble());
  I3VectorDoublePtr lengthStart(new I3VectorDouble());
  I3VectorDoublePtr stoppLLH(new I3VectorDouble());
  I3VectorDoublePtr lengthStopp(new I3VectorDouble());
  
  std::map<double,double>::const_iterator imap=LLHLengthStart.begin();
  for(;imap!=LLHLengthStart.end();imap++){
    lengthStart->push_back(imap->first);
    startLLH->push_back(imap->second);
  }
  imap=LLHLengthStop.begin();
  for(;imap!=LLHLengthStop.end();imap++){
    lengthStopp->push_back(imap->first);
    stoppLLH->push_back(imap->second);
  }
  
  frame->Put(fitName_+"_FitLengthTrack",partPtr);

  frame->Put(fitName_+"_startLLH",startLLH);
  frame->Put(fitName_+"_lengthStart",lengthStart);
  frame->Put(fitName_+"_stoppLLH",stoppLLH);
  frame->Put(fitName_+"_lengthStopp",lengthStopp);
  
  PushFrame(frame,"OutBox"); 
}
