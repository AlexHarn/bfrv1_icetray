/**
 * Copyright (C) 2004
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file I3TopRecoPlane.cxx
 * @version $Rev: 29961 $
 * @date $Date$
 * @author $Author: csong $
 */

#include "toprec/I3TopRecoPlane.h"
#include "toprec/TopRecoFunctions.h"
#include "toprec/TTopRecoShower.h"

// class header files
#include "icetray/I3TrayHeaders.h"

#include "dataclasses/physics/I3EventHeader.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/geometry/I3TankGeo.h"
#include "dataclasses/geometry/I3OMGeo.h"

#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3RecoPulse.h"

using namespace std;
using TopRecoFunctions::OutputEmptyParticle;

const bool I3TopRecoPlane::DEFAULT_VERBOSE = false;
const string I3TopRecoPlane::DEFAULT_DATA_READOUT_LABEL = "ITSimData";
const string I3TopRecoPlane::DEFAULT_ARRAYHITNAME = "TopHit";
const string I3TopRecoPlane::DEFAULT_SHOWERPLANENAME = "ShowerPlane";
const bool I3TopRecoPlane::DEFAULT_USE_HITS = false;
const int I3TopRecoPlane::DEFAULT_TRIGGER=3; // at least 3 stations

const string I3TopRecoPlane::EVENT_HEADER_TAG = "EventHeaderName";
const string I3TopRecoPlane::VERBOSE_TAG = "verbose";
const string I3TopRecoPlane::DATA_READOUT_TAG = "datareadout";
const string I3TopRecoPlane::SHOWERPLANENAME_TAG = "showerplane";
const string I3TopRecoPlane::ARRAYHITNAME_TAG = "ArrayHitName";
const string I3TopRecoPlane::USE_HITS_TAG = "usehits";
const string I3TopRecoPlane::TRIGGER_TAG = "trigger";

const string I3TopRecoPlane::EVENT_HEADER_DESCRIPTION = "Event header name";
const string I3TopRecoPlane::VERBOSE_DESCRIPTION = "Verbosity of planing";
const string I3TopRecoPlane::DATA_READOUT_DESCRIPTION 
= "Label under which the data is found in DataReadoutDict";
const string I3TopRecoPlane::SHOWERPLANENAME_DESCRIPTION
= "Label under which the resulting shower plane will be found";
const string I3TopRecoPlane::ARRAYHITNAME_DESCRIPTION = "name of the array hit in the map";
const string I3TopRecoPlane::USE_HITS_DESCRIPTION
= "Set to true, the module will use I3RecoHits instead of I3RecoPulses";
const string I3TopRecoPlane::TRIGGER_DESCRIPTION
= "Minimum number of stations you want to require";

const string I3TopRecoPlane::PHYSICS_STREAM_NAME = "Physics";
const string I3TopRecoPlane::GEOMETRY_STREAM_NAME = "Geometry";
const string I3TopRecoPlane::PHYSICS_STREAM_TITLE = "Physics stream";
const string I3TopRecoPlane::GEOMETRY_STREAM_TITLE = "Geometry stream";
const string I3TopRecoPlane::OUTBOX_NAME = "OutBox";
const string I3TopRecoPlane::INBOX_NAME = "InBox";

I3_MODULE (I3TopRecoPlane);

/********************************************************************/

I3TopRecoPlane::I3TopRecoPlane (const I3Context& ctx)
  : I3ConditionalModule(ctx) {

  // transfer the default
  fEventHeader = I3DefaultName<I3EventHeader>::value();
  fVerbose = DEFAULT_VERBOSE;
  fDataReadoutLabel = DEFAULT_DATA_READOUT_LABEL; // for the input
  fShowerPlaneName = DEFAULT_SHOWERPLANENAME; // for the result
  fTrigger = DEFAULT_TRIGGER;  

  AddParameter (EVENT_HEADER_TAG, EVENT_HEADER_DESCRIPTION, fEventHeader);
  AddParameter (VERBOSE_TAG, VERBOSE_DESCRIPTION, fVerbose);
  AddParameter (DATA_READOUT_TAG,
		DATA_READOUT_DESCRIPTION,
		fDataReadoutLabel);
  AddParameter (SHOWERPLANENAME_TAG, SHOWERPLANENAME_DESCRIPTION,
		fShowerPlaneName);
  AddParameter (TRIGGER_TAG, TRIGGER_DESCRIPTION, fTrigger);

  // add an out box to the module into which we dump the events
  AddOutBox (OUTBOX_NAME);

}

/********************************************************************/


I3TopRecoPlane::~I3TopRecoPlane() {
}

/********************************************************************/

void I3TopRecoPlane::Configure() {

  log_info ("Configuring the I3TopRecoPlane Module");
  GetParameter (EVENT_HEADER_TAG, fEventHeader);
  GetParameter (VERBOSE_TAG, fVerbose);
  GetParameter (DATA_READOUT_TAG, fDataReadoutLabel);
  GetParameter (SHOWERPLANENAME_TAG, fShowerPlaneName);
  GetParameter (TRIGGER_TAG, fTrigger); 
}

/********************************************************************/

void I3TopRecoPlane::Physics(I3FramePtr frame) {
  log_debug("entering Physics");
  

  //*****************************************************************
  //                    READING THE INPUT
  //*****************************************************************

  TTopRecoShower * inputShower = new TTopRecoShower();
  if(!inputShower->ReadData(frame, fDataReadoutLabel)){
    log_debug("input pulses missing.");
    OutputEmptyParticle(frame, fShowerPlaneName, I3Particle::GeneralFailure);
    OutputEmptyParams(frame, fShowerPlaneName);
    PushFrame (frame,"OutBox");
    delete inputShower;
    return;
  }

  if(inputShower->SortOutBadTimes()<fTrigger){
    log_debug("Too few stations to make a Plane Fit!");
    OutputEmptyParticle(frame, fShowerPlaneName, I3Particle::InsufficientHits);
    OutputEmptyParams(frame, fShowerPlaneName);
    PushFrame (frame,"OutBox");
    delete inputShower;
    return;
  }
  log_debug("%f percent of all pulses are good.", (double)inputShower->GetNPulsesUsedTime()/inputShower->GetNPulses());

  //  -----  EVALUATING THE SHOWER PLANE

  // the result that will be put to the frame
  I3ParticlePtr shower_plane (new I3Particle());
  I3TopRecoPlaneFitParamsPtr plane_params(new I3TopRecoPlaneFitParams());
  shower_plane->SetShape(I3Particle::TopShower);
  
  if(EvaluateDirection(inputShower, shower_plane, plane_params)){

    // make a correction for the z values (once is enough).
    inputShower->SetDirection(shower_plane->GetDir());
    inputShower->CorrectForHeight();
    
    // reevaluate the direction
    EvaluateDirection(inputShower, shower_plane, plane_params);
  }

  //  ----- OUTPUT SECTION
  
  // do the timing
  const I3EventHeader& event_header = frame->Get<I3EventHeader>(fEventHeader);
  static int last_day (event_header.GetStartTime ().GetModJulianDay ());
  static int last_sec (event_header.GetStartTime ().GetModJulianSec ());
  static double last_nsec (event_header.GetStartTime ().GetModJulianNanoSec ());
  log_debug("Last day: %d Last Sec: %d Last ns: %f", last_day,last_sec,last_nsec);
  plane_params->ET = (int64_t) event_header.GetStartTime ().GetModJulianDay () * 86400
      + (int64_t) event_header.GetStartTime ().GetModJulianSec ();
  plane_params->DeltaT = (event_header.GetStartTime ().GetModJulianDay () - last_day) * I3Units::day
      + (event_header.GetStartTime ().GetModJulianSec () - last_sec) * I3Units::second
      + (event_header.GetStartTime ().GetModJulianNanoSec () - last_nsec) * I3Units::nanosecond;
  log_debug ("Time difference to last event: %f", plane_params->DeltaT/I3Units::s);

  // remember the current time for the next event
  last_day = event_header.GetStartTime ().GetModJulianDay ();
  last_sec = event_header.GetStartTime ().GetModJulianSec ();
  last_nsec = event_header.GetStartTime ().GetModJulianNanoSec ();

  // store the result
  log_debug ("Dumping into %s in Dict", fShowerPlaneName.c_str ());
  if(plane_params->Chi2>0){ 
    frame->Put(fShowerPlaneName, shower_plane);
    frame->Put(fShowerPlaneName + "Params", plane_params);
  }
  else {
    OutputEmptyParticle(frame, fShowerPlaneName, I3Particle::FailedToConverge);
    OutputEmptyParams(frame, fShowerPlaneName);
  }  
  PushFrame (frame,"OutBox");
  delete inputShower;
  return;
}

/********************************************************************/

bool I3TopRecoPlane::EvaluateDirection(TTopRecoShower * inputShower, I3ParticlePtr& shower_plane, I3TopRecoPlaneFitParamsPtr& plane_params){

  // check how many bad pulses we have
  int used_pulses=inputShower->GetNPulsesUsedTime();

  // If trigger is not fulfilled, end here.
  if (inputShower->GetNStationsUsedTime() < fTrigger){
    plane_params->Chi2 = NAN;
    plane_params->NPulses = used_pulses;
    shower_plane->SetDir (NAN,NAN);
    shower_plane->SetFitStatus(I3Particle::InsufficientHits);
    return false;
  }
  double sigma = 5.*I3Units::ns;
  double weight=1./sigma/sigma;
  
  // recentralise the values
  showerPulse meanPulse = inputShower->GetMeans();

  // calculate the sums necessary for direction calculation:
  double Sxx = 0.;
  double Sxy = 0.;
  double Syy = 0.;

  double Stx = 0.;
  double Sty = 0.;

  vector<showerPulse> showerVector = inputShower->GetPulses();
  for (vector<showerPulse>::iterator it = showerVector.begin(); it != showerVector.end(); ++it) {

    if(it->usepulsetime){
      Sxx += (it->x-meanPulse.x) * (it->x-meanPulse.x) * weight;
      Sxy += (it->x-meanPulse.x) * (it->y-meanPulse.y) * weight;
      Syy += (it->y-meanPulse.y) * (it->y-meanPulse.y) * weight;
    
      Stx += (it->t-meanPulse.t) * (it->x-meanPulse.x) * weight;
      Sty += (it->t-meanPulse.t) * (it->y-meanPulse.y) * weight;
    }

  }
  
  // done calculating the sums.


  // now do the inversion of the matrix:
  double nx=0., ny=0.;
  double phi=0., costh2=0.; //   cos^2 (th)

  double det = (Sxx * Syy - Sxy * Sxy);

  log_debug ("Determinant is %f", det);

  if (abs (det) < 1E-4) {
    nx = 0.;
    ny = 0.;
    phi = NAN;
    costh2 = NAN;
  } else {
    nx = I3Constants::c * (Stx * Syy - Sty * Sxy) / det;
    ny = I3Constants::c * (Sty * Sxx - Stx * Sxy) / det;
    costh2 = 1. - nx * nx - ny * ny;

    phi = atan2 (ny, nx);
    log_debug ("nx, ny, costh2, phi: %f, %f, %f, %f", nx, ny, costh2, phi);
  }
  
  if (costh2 >= 0.) {
    log_debug ("setting proper values");
    
    /*
    // The sqrt has two roots.  Choose the downgoing track:
    double zen = acos (sqrt (costh2));
    double azi = phi;
    // Bugfix 5/31/06: this version will translate correctly 
    // from spherical coords into sky coords
    shower_plane->SetDir(zen, azi);
    */
    
    // Bugfix of the above Bugfix: Zenith and Azimuth are different to theta and phi!
    // The sqrt has two roots.  Choose the downgoing track:
    double theta = acos(-sqrt(costh2));
    shower_plane->SetThetaPhi(theta, phi);
    inputShower->SetNXNY(nx, ny);
    inputShower->SetCore(meanPulse.x, meanPulse.y);
    inputShower->SetTime(meanPulse.t);
    inputShower->SetPlaneParams(meanPulse.t, meanPulse.x, meanPulse.y);

    
    shower_plane->SetFitStatus(I3Particle::OK);
  } else {
    log_debug ("setting NAN values");
    shower_plane->SetDir (NAN, NAN);
    shower_plane->SetFitStatus(I3Particle::FailedToConverge);
  }

  // the t_0 is supposed to be 0, so no calculation's needed

  double chi2 = 0.;
  if (costh2 >= 0.) {
    for (vector<showerPulse>::iterator it = showerVector.begin(); it != showerVector.end(); ++it) {
      if(it->usepulsetime){
        double d_i = inputShower->GetDistToPlaneIt(it);
        chi2 += weight * d_i * d_i; 
      }
    }
  } else
    chi2 = NAN;
  
  chi2/=used_pulses-3; // divide by the degrees of freedom
  
  plane_params->T0 = meanPulse.t;
  plane_params->X0 = meanPulse.x;
  plane_params->Y0 = meanPulse.y;

  plane_params->Chi2 = chi2;
  plane_params->NPulses = used_pulses;
  log_debug ("Assuming theta = %f, phi = %f, Chi2 = %f",
	     shower_plane->GetZenith (),
	     shower_plane->GetAzimuth (),
             chi2);
  if(costh2 >= 0.) return true;
  else return false;
}

/********************************************************************/

void I3TopRecoPlane::OutputEmptyParams(I3FramePtr frame, string outShowerName) {
  I3TopRecoPlaneFitParamsPtr plane_params(new I3TopRecoPlaneFitParams());
  frame->Put(fShowerPlaneName + "Params", plane_params);
}
