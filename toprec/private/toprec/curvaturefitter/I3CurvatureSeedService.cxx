/**
 *
 * @file I3CurvatureSeedService.cxx
 * @brief implementaration of the I3CurvatureSeedService class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id: I3CurvatureSeedService.cxx 152938 2017-01-24 17:24:07Z olivas $
 *
 * @version $Revision: 152938 $
 * @date $Date: 2017-01-24 11:24:07 -0600 (Tue, 24 Jan 2017) $
 * @author kath
 *
 */

#include <string>
#include <cassert>
#include <cmath>
#include "toprec/I3CurvatureSeedService.h"
#include "recclasses/I3TopRecoPlaneFitParams.h"
#include "toprec/TTopRecoShower.h"
#include "recclasses/I3TopLateralFitParams.h"
#include "icetray/I3SingleServiceFactory.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Constants.h"
#include "toprec/LateralFitFunctions.h"
/*
 * I3CurvatureSeedService
 * Seeding the (very simple) CurvatureFitter
 */

// Defaults. Tags. Descriptions.
//const double I3CurvatureSeedService::DEFAULT_SEED_A = 4.823e-4;  // from original top_curv_gausspar, moved to recclasses
//const double I3CurvatureSeedService::DEFAULT_SEED_D = 118.1;  
//const double I3CurvatureSeedService::DEFAULT_SEED_N = 19.41;  
const std::string I3CurvatureSeedService::SEED_A_TAG = "A";
const std::string I3CurvatureSeedService::SEED_D_TAG = "D";
const std::string I3CurvatureSeedService::SEED_N_TAG = "N";
const std::string I3CurvatureSeedService::SEED_T_TAG = "T";
const std::string I3CurvatureSeedService::SEED_A_DESC = "Parameter describing curvature parabola coefficient A";
const std::string I3CurvatureSeedService::SEED_D_DESC = "Parameter describing curvature gaussian parameter D";
const std::string I3CurvatureSeedService::SEED_N_DESC = "Parameter describing curvature gaussian normalization N";
const std::string I3CurvatureSeedService::SEED_T_DESC = "Track time T";

const std::string I3CurvatureSeedService::DEFAULT_SEEDTRACK = "Laputop";
const std::string I3CurvatureSeedService::SEEDTRACK_TAG = "SeedTrackName";
const std::string I3CurvatureSeedService::SEEDTRACK_DESC = "Name of the seed track";

/// default constructor for unit tests
I3CurvatureSeedService::I3CurvatureSeedService(std::string name,
					   double seedA
					   ) : 
  I3ServiceBase(name),
  fSeedA_(seedA) {

  log_debug("Hello, I am a nearly empty constructor for unit tests");
  fSeedA_ = -1;
  fSeedD_ = -1;
  fSeedN_ = -1;
  fSeedT_ = -1;
  fInTrackName_ = DEFAULT_SEEDTRACK;
}


/// constructor I3Tray
I3CurvatureSeedService::I3CurvatureSeedService(const I3Context &c):
  I3ServiceBase(c), I3SeedServiceBase(){

  log_debug("Entering the constructor");

  // Set the (only) free parameter:
  fSeedA_ = -1;  // These should never be zero.  So if they are, that means the user didn't set them.
  fSeedD_ = -1;
  fSeedN_ = -1;
  fSeedT_ = -1;
  fInTrackName_ = DEFAULT_SEEDTRACK;
  AddParameter (SEED_A_TAG, SEED_A_DESC, fSeedA_);
  AddParameter (SEED_D_TAG, SEED_D_DESC, fSeedD_);
  AddParameter (SEED_N_TAG, SEED_N_DESC, fSeedN_);
  AddParameter (SEED_T_TAG, SEED_T_DESC, fSeedT_);
  AddParameter (SEEDTRACK_TAG, SEEDTRACK_DESC, fInTrackName_);
}

// set parameters (in I3Tray)
void
I3CurvatureSeedService::Configure(){
  GetParameter (SEED_A_TAG, fSeedA_);
  GetParameter (SEED_D_TAG, fSeedD_);
  GetParameter (SEED_N_TAG, fSeedN_);
  GetParameter (SEED_T_TAG, fSeedT_);
  GetParameter (SEEDTRACK_TAG, fInTrackName_);

  // This line has been moved to SetEvent(), so that it can be filled
  // from a frame object (an existing Params) if the users calls for it,
  // instead of just being created empty.
  //ldfParams_ = I3TopLateralFitParamsPtr(new I3TopLateralFitParams());
}

/*
 * provide event data
 * - purge old seed
 * - get the track seed, add decay time
 * @param[in] f Frame with event data
 * @returns number of available seeds
 */
unsigned int
I3CurvatureSeedService::SetEvent( const I3Frame &f ){
  log_debug("(%s) Entering seed SetEvent", GetName().c_str());

  I3ParticleConstPtr laputopptr = f.Get<I3ParticleConstPtr>( fInTrackName_ );
  seedParticle_ = I3ParticlePtr(new I3Particle());

  if (!laputopptr) { 
    log_debug("Didn't find seed track %s... this'll be bad!",  
	     fInTrackName_.c_str());
    return 0;
  }
  /* Trying to use MCPrimary as a seed makes this fail... so comment it out.
  else if (laputopptr->GetFitStatus() != I3Particle::OK){ 
    log_debug("This core %s has a bad fitstatus, probably missing inputdata",
	      fInTrackName_.c_str());
    return 0;
    }*/

  seedParticle_->SetPos(laputopptr->GetX(), 
			laputopptr->GetY(), 
			laputopptr->GetZ());
  seedParticle_->SetDir(laputopptr->GetZenith(), 
			laputopptr->GetAzimuth());
  seedParticle_->SetTime(laputopptr->GetTime());

  seedParticle_->SetFitStatus(I3Particle::OK);
  seedParticle_->SetShape(I3Particle::InfiniteTrack);
  seedParticle_->SetType(I3Particle::unknown);
  
  seedParticle_->SetEnergy(NAN);
  seedParticle_->SetLength(NAN);

  // Now we'll create a new Params for storing non-std parameters:
  if (fInTrackName_ != "" && f.Has(fInTrackName_+"Params")) {
    // Make the new one a copy of the one from the frame
    log_debug("Copying existing I3LaputopParams structure.");
    curvParams_ = I3LaputopParamsPtr(new I3LaputopParams(f.Get<I3LaputopParams>(fInTrackName_+"Params")));
    if (fSeedA_ > 0) curvParams_->SetValue(Laputop::Parameter::CurvParabA, fSeedA_);
    if (fSeedD_ > 0) curvParams_->SetValue(Laputop::Parameter::CurvGaussD, fSeedD_);
    if (fSeedN_ > 0) curvParams_->SetValue(Laputop::Parameter::CurvGaussN, fSeedN_);
  
  } else {
    // Make a blank one and fill it with seeds:
    log_debug("Creating a fresh params structure.");
    curvParams_ = I3LaputopParamsPtr(new I3LaputopParams());
    if (fSeedA_ > 0) curvParams_->SetValue(Laputop::Parameter::CurvParabA, fSeedA_);
    if (fSeedD_ > 0) curvParams_->SetValue(Laputop::Parameter::CurvGaussD, fSeedD_);
    if (fSeedN_ > 0) curvParams_->SetValue(Laputop::Parameter::CurvGaussN, fSeedN_);
  }
  
  if (fSeedT_ > 0) seedParticle_->SetTime(fSeedT_);

  // Logging checks: print 'em if you got 'em
  if (curvParams_->Has(Laputop::Parameter::CurvParabA)) 
    log_debug("SetEvent seed A: %f", curvParams_->GetValue(Laputop::Parameter::CurvParabA)); 
  else log_debug("SetEvent seed A: was not set"); 
  if (curvParams_->Has(Laputop::Parameter::CurvGaussD)) 
    log_debug("SetEvent seed D: %f", curvParams_->GetValue(Laputop::Parameter::CurvGaussD)); 
  else log_debug("SetEvent seed D: was not set"); 
  if (curvParams_->Has(Laputop::Parameter::CurvGaussN)) 
    log_debug("SetEvent seed N: %f", curvParams_->GetValue(Laputop::Parameter::CurvGaussN)); 
  else log_debug("SetEvent seed N: was not set"); 
    
  if (seedParticle_ && curvParams_) return 1;
  
  else {
    log_warn("Something went wrong with the seedParticle");
    return 0;
  }

}

// get a seed
I3EventHypothesis
I3CurvatureSeedService::GetSeed( unsigned int iseed ) const {
  log_debug("Entering GetSeed");

  // I'm only going to get ONE seed, no matter what "iseed" is...
  log_debug("Here's my seed particle: (%5.2f %5.2f %6.2f) (%4.2f %4.2f) t=%7.1f",
	    seedParticle_->GetX(), seedParticle_->GetY(), seedParticle_->GetZ(), 
	    seedParticle_->GetZenith(), seedParticle_->GetAzimuth(), 
	    seedParticle_->GetTime());
  
  // Again, logging: print 'em if you got 'em
  if (curvParams_->Has(Laputop::Parameter::CurvParabA)) 
    log_debug("GetSeed A: %f", curvParams_->GetValue(Laputop::Parameter::CurvParabA)); 
  else log_debug("GetSeed A: was not set"); 
  if (curvParams_->Has(Laputop::Parameter::CurvGaussD)) 
    log_debug("GetSeed D: %f", curvParams_->GetValue(Laputop::Parameter::CurvGaussD)); 
  else log_debug("GetSeed D: was not set"); 
  if (curvParams_->Has(Laputop::Parameter::CurvGaussN)) 
    log_debug("GetSeed N: %f", curvParams_->GetValue(Laputop::Parameter::CurvGaussN)); 
  else log_debug("GetSeed N: was not set"); 


  // Return it
  I3EventHypothesis eh(seedParticle_,
  		       curvParams_ );
  
  return eh;
}

// get a dummy seed (useful in case of fg failure)
I3EventHypothesis
I3CurvatureSeedService::GetDummy( ) const {
  log_debug("Entering GetDummy");

  // This one's going to to just be full of NaN's and stuff.
  I3ParticlePtr p = I3ParticlePtr(new I3Particle());
  I3LaputopParamsPtr lp = I3LaputopParamsPtr(new I3LaputopParams());
  
  I3EventHypothesis eh(p, lp);

  return eh;
}

/*
 * Space and time coordinates of the vertex are tweaked, for numerical
 * convenience (minimumizer algorithms).
 * This is actually delegated to the external track seeders.
 */
void
I3CurvatureSeedService::Tweak( I3EventHypothesis &eh ) const {
  log_debug("Entering Tweak: none today");
    
}

/*
 * Filling the blanks in the track is delegated to the external track seeders.
 */
void
I3CurvatureSeedService::FillInTheBlanks( I3EventHypothesis &eh ) const {
  log_debug("Entering FillInTheBlanks: none today");

}

/*
 * This will make a (deep) copy of the I3EventHypothesis.
 */
I3EventHypothesis
I3CurvatureSeedService::GetCopy( const I3EventHypothesis &eh ) const {
  log_debug("Entering seed GetCopy");
  
  I3LaputopParamsConstPtr nonStdPtr = boost::dynamic_pointer_cast<const I3LaputopParams>(eh.nonstd);
  if (!nonStdPtr) log_fatal("Your nonstandard parameters is empty... did not cast correctly?");

  I3EventHypothesis neweh( I3ParticlePtr(new I3Particle(*(eh.particle))),
			   I3LaputopParamsPtr(new I3LaputopParams(*nonStdPtr)) );

  return neweh;
}


typedef I3SingleServiceFactory< I3CurvatureSeedService,I3SeedServiceBase >
I3CurvatureSeedServiceFactory;
I3_SERVICE_FACTORY( I3CurvatureSeedServiceFactory )
