/**
 * Copyright (C) 2004
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file I3TopRecoCore.cxx
 * @version $Rev: 27576 $
 * @date $Date$
 * @author $Author: klepser $
 */

#include "toprec/I3TopRecoCore.h"
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

#include "dataclasses/physics/I3Particle.h"


using namespace std;
using TopRecoFunctions::OutputEmptyParticle;

const bool I3TopRecoCore::DEFAULT_VERBOSE = false;
const string I3TopRecoCore::DEFAULT_DATA_READOUT_LABEL = "ITSimData";
const string I3TopRecoCore::DEFAULT_SHOWERCORENAME = "ShowerCOG";
const bool I3TopRecoCore::DEFAULT_USE_HITS = false;
const double I3TopRecoCore::DEFAULT_WEIGHTING_POWER = 0.5;
const short int I3TopRecoCore::DEFAULT_NTANKS = -1;

const string I3TopRecoCore::VERBOSE_TAG = "verbose";
const string I3TopRecoCore::DATA_READOUT_TAG = "datareadout";
const string I3TopRecoCore::SHOWERCORENAME_TAG = "showercore";
const string I3TopRecoCore::USE_HITS_TAG = "usehits";
const string I3TopRecoCore::WEIGHTING_POWER_TAG = "weighting_power";
const string I3TopRecoCore::NTANKS_TAG = "ntanks";

const string I3TopRecoCore::VERBOSE_DESCRIPTION
= "Verbosity of core reconstruction";
const string I3TopRecoCore::DATA_READOUT_DESCRIPTION
= "Label under which the data is found in the frame";
const string I3TopRecoCore::SHOWERCORENAME_DESCRIPTION
= "Label under which the core is put to the frame";
const string I3TopRecoCore::USE_HITS_DESCRIPTION
= "Set to true, the module will use I3RecoHits instead of I3RecoPulses";
const string I3TopRecoCore::WEIGHTING_POWER_DESCRIPTION
= "power for the weighting - the weight will be PE^power";
const string I3TopRecoCore::NTANKS_DESCRIPTION
= "Number of highest charge tanks to use in COG calculation. Uses all tanks if set to a value <= 0 (default).";

const string I3TopRecoCore::PHYSICS_STREAM_NAME = "Physics";
const string I3TopRecoCore::GEOMETRY_STREAM_NAME = "Geometry";
const string I3TopRecoCore::PHYSICS_STREAM_TITLE = "Physics stream";
const string I3TopRecoCore::GEOMETRY_STREAM_TITLE = "Geometry stream";
const string I3TopRecoCore::OUTBOX_NAME = "OutBox";
const string I3TopRecoCore::INBOX_NAME = "InBox";

I3_MODULE (I3TopRecoCore);

/********************************************************************/

I3TopRecoCore::I3TopRecoCore (const I3Context& ctx)
  : I3ConditionalModule (ctx) {

  fVerbose = DEFAULT_VERBOSE;
  fDataReadoutLabel = DEFAULT_DATA_READOUT_LABEL;
  fShowerCoreName = DEFAULT_SHOWERCORENAME;
  fPower = DEFAULT_WEIGHTING_POWER;
  fNtanks = DEFAULT_NTANKS; 

  AddParameter (VERBOSE_TAG, VERBOSE_DESCRIPTION, fVerbose);
  AddParameter (DATA_READOUT_TAG,
		DATA_READOUT_DESCRIPTION,
		fDataReadoutLabel);
  AddParameter (SHOWERCORENAME_TAG,
		SHOWERCORENAME_DESCRIPTION,
		fShowerCoreName);
  AddParameter (WEIGHTING_POWER_TAG,
                WEIGHTING_POWER_DESCRIPTION,
                fPower);
  AddParameter (NTANKS_TAG, 
		NTANKS_DESCRIPTION,
		fNtanks); 

  // add an out box to the module into which we dump the events
  AddOutBox (OUTBOX_NAME);
}

/********************************************************************/


I3TopRecoCore::~I3TopRecoCore() {
}

/********************************************************************/

void I3TopRecoCore::Configure() {

  log_info ("Configuring the I3TopRecoCore Module");

  GetParameter (VERBOSE_TAG, fVerbose);
  GetParameter (DATA_READOUT_TAG, fDataReadoutLabel);
  GetParameter (SHOWERCORENAME_TAG, fShowerCoreName);
  GetParameter (WEIGHTING_POWER_TAG, fPower);
  GetParameter (NTANKS_TAG, fNtanks);

}

/********************************************************************/

void I3TopRecoCore::Physics (I3FramePtr frame) {
  log_debug("entering Physics");

  //*****************************************************************
  //                    READING THE INPUT
  //*****************************************************************

  TTopRecoShower * inputShower = new TTopRecoShower();
  if(!inputShower->ReadData(frame, fDataReadoutLabel)){
    log_debug("input pulses missing.");
    OutputEmptyParticle(frame, fShowerCoreName, I3Particle::GeneralFailure);
    PushFrame (frame,"OutBox");
    delete inputShower;
    return;
  }

  if(inputShower->SortOutBadCharges() == 0){
    log_debug("No pulses to calculate a COG from!");
    OutputEmptyParticle(frame, fShowerCoreName, I3Particle::InsufficientHits);
    PushFrame (frame,"OutBox");
    delete inputShower;
    return;
  } 

  //  -----  CALCULATING THE COGs
  showerPulse meanPulse = inputShower->GetMeans(fPower, fNtanks);

  //  -----  PUTTING THE RESULTS TO THE FRAME

  // the result that is put to the frame
  I3ParticlePtr shower_core (new I3Particle());
  I3Position core_pos(meanPulse.x, meanPulse.y, meanPulse.z);
  shower_core->SetShape(I3Particle::TopShower);

  shower_core->SetFitStatus(I3Particle::OK);
  shower_core->SetPos(core_pos);
  shower_core->SetTime(meanPulse.t);
  frame->Put(fShowerCoreName, shower_core);
  log_debug ("Placed core in shower_core at %f, %f into %s",
	     shower_core->GetPos ().GetX (),
	     shower_core->GetPos ().GetY (),
	     fShowerCoreName.c_str ());

  // sending out the event
  PushFrame (frame, "OutBox");
  delete inputShower;
  return;
}

/********************************************************************/


