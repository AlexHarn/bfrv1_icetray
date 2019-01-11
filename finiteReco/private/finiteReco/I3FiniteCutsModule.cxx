/**
 * @brief implementation of the I3FiniteCutsModule class
 *
 * @file I3FiniteCutsModule.cxx
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This module collects the required objects from the frame and passes them to I3FiniteCalc. The returned I3FiniteCuts are placed in the frame.
 */

#include "finiteReco/I3FiniteCutsModule.h"
#include "finiteReco/I3FiniteCalc.h"
#include "recclasses/I3FiniteCuts.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Units.h"
#include "icetray/OMKey.h"
#include "icetray/I3PhysicsTimer.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "phys-services/I3Calculator.h"
#include "gulliver/I3EventHypothesis.h"

I3_MODULE(I3FiniteCutsModule);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3FiniteCutsModule::I3FiniteCutsModule(const I3Context& ctx) : I3Module(ctx)
{
  AddOutBox("OutBox");

  fitName_ = "LineFit";
  AddParameter("Name", "Name of the I3Particle the cuts are calculated with", 
	       fitName_);

  inputRecoPulses_ = "";
  AddParameter("InputRecoPulses",
	       "RecoPulseSeriesMap to use for input",
	       inputRecoPulses_);
  cylinderRadius_ = 100*I3Units::m;
  AddParameter("CylinderRadius",
	       "Cylinder radius for the cut calculation",
	       cylinderRadius_);
}

/* ******************************************************************** */
/* Destructor                                                            */
/* ******************************************************************** */
I3FiniteCutsModule::~I3FiniteCutsModule(){
}

/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3FiniteCutsModule::Configure()
{
  GetParameter("Name",fitName_);
  GetParameter("InputRecoPulses",inputRecoPulses_);
  GetParameter("CylinderRadius",cylinderRadius_);
}

/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
void I3FiniteCutsModule::Physics(I3FramePtr frame){
  
  // Physics timer for getting the execution time of Physics() method of module
  I3PhysicsTimer timer(frame, GetName());

  // get Geometry from frame
  const I3Geometry geometry = frame->Get<I3Geometry>();
  
  // get RecoPulseSeries info from the frame
  I3RecoPulseSeriesMapConstPtr pulseMap = 
    frame->Get<I3RecoPulseSeriesMapConstPtr>(inputRecoPulses_);
  
  // get the reconstructed particle
  I3ParticleConstPtr particle = frame->Get<I3ParticleConstPtr>(fitName_);
  
  // return cut value to frame  
  char outName[200];
  sprintf(outName,"%s%s%d",fitName_.c_str(),"FiniteCuts",(int)(cylinderRadius_+0.5));
  if(pulseMap){
    I3FiniteCalcPtr calc(new I3FiniteCalc(geometry,*particle,pulseMap,cylinderRadius_));
    I3FiniteCutsPtr finiteCuts(new I3FiniteCuts(calc->GetCuts()));
    frame->Put(outName,finiteCuts);
  } else{
    log_warn("No pulses given!");
  }
  PushFrame(frame,"OutBox");
}
