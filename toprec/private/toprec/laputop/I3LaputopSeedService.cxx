/**
 *
 * @file I3LaputopSeedService.cxx
 * @brief implementaration of the I3LaputopSeedService class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author kath
 *
 */

#include <string>
#include <cassert>
#include <cmath>
#include "toprec/I3LaputopSeedService.h"
#include "recclasses/I3TopRecoPlaneFitParams.h"
#include "toprec/TTopRecoShower.h"
#include "recclasses/I3TopLateralFitParams.h"
#include "icetray/I3SingleServiceFactory.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Constants.h"
#include "toprec/LateralFitFunctions.h"
/*
 * I3LaputopSeedService
 * class for combining Core/Direction fits into a seed for LateralFit
 */

// User parameters directly from toprec
const std::string I3LaputopSeedService::DEFAULT_INPLANENAME = "ShowerPlane";
const std::string I3LaputopSeedService::DEFAULT_INCORENAME = "ShowerCOG";

// ... and one of my own!
const double I3LaputopSeedService::DEFAULT_BETA = 2.6;
const std::string I3LaputopSeedService::DEFAULT_PULSENAME = "";

const std::string I3LaputopSeedService::INPLANENAME_TAG = "InPlane";
const std::string I3LaputopSeedService::INCORENAME_TAG = "InCore";
const std::string I3LaputopSeedService::INPARAMSNAME_TAG = "InParams";
const std::string I3LaputopSeedService::BETA_TAG = "Beta";
const std::string I3LaputopSeedService::PULSENAME_TAG = "InputPulses";

const std::string I3LaputopSeedService::INPLANENAME_DESCRIPTION = "Particle containing First Guess direction";
const std::string I3LaputopSeedService::INCORENAME_DESCRIPTION = "Particle containing First Guess core position";
const std::string I3LaputopSeedService::INPARAMSNAME_DESCRIPTION = "Params (containing S125, beta, etc.) containing First Guesses for those parameters)";
const std::string I3LaputopSeedService::BETA_DESCRIPTION 
= "Seed value of beta for DLP (or of age if using NKG). Default=2.6.";
const std::string I3LaputopSeedService::PULSENAME_DESCRIPTION 
= "Name of a pulseseries (will be used to estimate seed S125)";


// This seedservice will figure out the best starting S125 (later),
// but here's the default:
const double I3LaputopSeedService::DEFAULT_S125 = 1.0;

// A, D, and N: as is done in I3CurvatureSeedService
const std::string I3LaputopSeedService::SEED_A_TAG = "A";
const std::string I3LaputopSeedService::SEED_D_TAG = "D";
const std::string I3LaputopSeedService::SEED_N_TAG = "N";
const std::string I3LaputopSeedService::SEED_A_DESC = "Parameter describing curvature parabola coefficient A";
const std::string I3LaputopSeedService::SEED_D_DESC = "Parameter describing curvature gaussian parameter D";
const std::string I3LaputopSeedService::SEED_N_DESC = "Parameter describing curvature gaussian normalization N";


// Saturation: Upper bound for HG in PE (same as in topeventcleaning)
const double SAT_HG = 3000.;                  



/// default constructor for unit tests
I3LaputopSeedService::I3LaputopSeedService(std::string name,
					   std::string InPlaneName,
					   std::string InCoreName,
					   std::string ParamsName,
					   double seedBeta,
					   std::string pulses
					   ) : 
  I3ServiceBase(name),
  fInPlaneName_(InPlaneName),
  fInCoreName_(InCoreName),
  fInParamsName_(ParamsName),
  seedBeta_(seedBeta),
  fDataReadoutLabel_(pulses) {

  log_debug("Hello, I am a nearly empty constructor for unit tests");
  fSeedA_ = -1;
  fSeedD_ = -1;
  fSeedN_ = -1;
  seedS125_ = DEFAULT_S125;
  override_beta_ = 0;

}


/// constructor I3Tray
I3LaputopSeedService::I3LaputopSeedService(const I3Context &c):
  I3ServiceBase(c), I3SeedServiceBase(){

  log_debug("Entering the constructor");


  fInPlaneName_ = DEFAULT_INPLANENAME;
  fInCoreName_ = DEFAULT_INCORENAME;
  fInParamsName_ = "";
  // Put off assigning the default on this, until after we know whether the user
  // has specified it
  //seedBeta_ = DEFAULT_BETA;
  seedBeta_ = -1;

  // This one gets computed automatically, or taken from params.  
  // Is not set explicitly by user
  seedS125_ = DEFAULT_S125;

  // Initialize curvature A, D, N
  fSeedA_ = -1;
  fSeedD_ = -1;
  fSeedN_ = -1;

  // This will indicate (in Configure) whether the user has expressly specified
  // a beta.  If so, the user's specifications will override any other
  // beta that the service might pull from somewhere else (such as Params).
  override_beta_ = 0;

  AddParameter (INPLANENAME_TAG, INPLANENAME_DESCRIPTION, fInPlaneName_);
  AddParameter (INCORENAME_TAG, INCORENAME_DESCRIPTION, fInCoreName_);
  AddParameter (INPARAMSNAME_TAG, INPARAMSNAME_DESCRIPTION, fInParamsName_);
  AddParameter (BETA_TAG,BETA_DESCRIPTION, seedBeta_);
  AddParameter (PULSENAME_TAG,PULSENAME_DESCRIPTION, fDataReadoutLabel_);
  AddParameter (SEED_A_TAG, SEED_A_DESC, fSeedA_);
  AddParameter (SEED_D_TAG, SEED_D_DESC, fSeedD_);
  AddParameter (SEED_N_TAG, SEED_N_DESC, fSeedN_);

}

// set parameters (in I3Tray)
void
I3LaputopSeedService::Configure(){
  GetParameter (INPLANENAME_TAG, fInPlaneName_);
  GetParameter (INCORENAME_TAG, fInCoreName_);
  GetParameter (INPARAMSNAME_TAG, fInParamsName_);
  GetParameter (BETA_TAG, seedBeta_);
  GetParameter (PULSENAME_TAG, fDataReadoutLabel_);
  GetParameter (SEED_A_TAG, fSeedA_);
  GetParameter (SEED_D_TAG, fSeedD_);
  GetParameter (SEED_N_TAG, fSeedN_);

  if ( fInPlaneName_.empty() )
    log_fatal( "(%s) You need to specify a particle for direction (InPlane).",
	       GetName().c_str() );
  if ( fInCoreName_.empty() )
    log_fatal( "(%s) You need to specify a particle for the core position (InCore).",
	       GetName().c_str() );
  

  // This line has been moved to SetEvent(), so that it can be filled
  // from a frame object (an existing Params) if the users calls for it,
  // instead of just being created empty.
  //ldfParams_ = I3TopLateralFitParamsPtr(new I3TopLateralFitParams());

  // Here, we need to know whether the user specifically set a Beta
  // If so, we know to USE it (instead of the default,
  // or anything from the Params).  If not, then it goes to the default, and may be 
  // overwritten by something from Params.
  if (seedBeta_ > 0) override_beta_ = 1;
  else seedBeta_ = DEFAULT_BETA;

  
}

/*
 * provide event data
 * - purge old seed
 * - get the track seed, add decay time
 * @param[in] f Frame with event data
 * @returns number of available seeds
 */
unsigned int
I3LaputopSeedService::SetEvent( const I3Frame &f ){
  log_debug("(%s) Entering seed SetEvent", GetName().c_str());

  I3ParticleConstPtr coreptr = f.Get<I3ParticleConstPtr>( fInCoreName_ );
  I3ParticleConstPtr dirptr = f.Get<I3ParticleConstPtr>( fInPlaneName_ );
  // The event time comes from this one:
  I3TopRecoPlaneFitParamsConstPtr inPlaneParams =
    f.Get<I3TopRecoPlaneFitParamsConstPtr>(fInPlaneName_ + "Params");

  seedParticle_ = I3ParticlePtr(new I3Particle());

  if (!coreptr) { 
    log_debug("Didn't find core %s... this'll be bad!",  
	     fInCoreName_.c_str());
    return 0;
  } else if (coreptr->GetFitStatus() != I3Particle::OK){ 
    log_debug("This core %s has a bad fitstatus, probably missing inputdata",
	      fInCoreName_.c_str());
    return 0;
  }
  seedParticle_->SetPos(coreptr->GetX(), coreptr->GetY(), coreptr->GetZ());

  if (!dirptr) { 
    log_debug("Didn't find direction %s... this'll be bad!",
	     fInPlaneName_.c_str());
    return 0;
  } else if (dirptr->GetFitStatus() != I3Particle::OK){ 
    log_debug("This plane %s has a bad fitstatus",
	      fInPlaneName_.c_str());
    return 0;
  }
  seedParticle_->SetDir(dirptr->GetZenith(), dirptr->GetAzimuth());

  if (!inPlaneParams) { 
    log_debug("Didn't find direction Params from %s, but not explicitly necessary...",
	     fInPlaneName_.c_str());
    log_debug("Will get the Time from the InPlane particle directly.");
    seedParticle_->SetTime(dirptr->GetTime());
  } else {

    // Take T0, X0, Y0 from params and adjust time to match what
    // it would be at position of core
    double nx = dirptr->GetDir().GetX();
    double ny = dirptr->GetDir().GetY();
    double T0 = inPlaneParams->T0 +
      (nx * (coreptr->GetX() - inPlaneParams->X0) + 
       ny * (coreptr->GetY() - inPlaneParams->Y0))/I3Constants::c;
    seedParticle_->SetTime(T0);
    if (!(inPlaneParams->Chi2 > 0)) {
      log_warn("Params Chi2 was zero or negative... Bad seed.");
      return 0;
    }
  }
 
  seedParticle_->SetFitStatus(I3Particle::OK);
  seedParticle_->SetShape(I3Particle::InfiniteTrack);
  seedParticle_->SetType(I3Particle::unknown);
  
  seedParticle_->SetEnergy(NAN);
  seedParticle_->SetLength(NAN);

  // The logS125 first guess: this requires PULSES from the frame
  // Which means for now, I will put it here, even though it should
  // probably be in FillInTheBlanks... but I don't know how to get 
  // that function to see stuff in the frame...
  // This code adapted from the Likelihood service, which has to fill 
  // an inputShower_ in a similar way.

  if (fInParamsName_ == "") { // No existing S125 seed... we'll have to compute one.
    if (!(fDataReadoutLabel_=="") && f.Has(fDataReadoutLabel_)) {
      
      I3RecoPulseSeriesMapConstPtr pulse_series_map = 
	f.Get<I3RecoPulseSeriesMapConstPtr> (fDataReadoutLabel_);
      const I3Geometry &geometry = f.Get<I3Geometry>();
      const I3OMGeoMap& om_map = geometry.omgeo;
      
      if(pulse_series_map)
	//// Toprec : already filled inputshower, does the logS125Start loop over the inputs
	//// old laputop : extra filling of inputshower, made laputop slow
	//// here : same as toprec.. only one loop, namely over the recopulses
	seedS125_ = pow(10,seedLogS125(pulse_series_map,seedParticle_,om_map));
      
    } // end first guess of S125
    log_debug("My seed S125: %f", seedS125_);
  }

  // Now we'll create a new Params for storing non-std parameters:
  if (fInParamsName_ != "") {
    // First, check whether it's the old I3TopLateralFitParams:
    boost::shared_ptr<const I3TopLateralFitParams> oldparams = f.Get<boost::shared_ptr<const I3TopLateralFitParams> >(fInParamsName_);
    if (oldparams) {
      // Make a new one and fill variables from the old params
      log_debug("Found an old I3TopLateralFitParams... copying S125 and beta into new structure.");
      ldfParams_ = I3LaputopParamsPtr(new I3LaputopParams());
      ldfParams_->SetValue(Laputop::Parameter::Log10_S125, log10(oldparams->S125));
      ldfParams_->SetValue(Laputop::Parameter::Beta, oldparams->Beta);
    } else {
      // Make the new one a copy of the one from the frame
      log_debug("Copying existing I3LaputopParams structure.");
      ldfParams_ = I3LaputopParamsPtr(new I3LaputopParams(f.Get<I3LaputopParams>(fInParamsName_)));
    }
    // Override what was in the params? (Because user specified 'em?)
    if (override_beta_) ldfParams_->SetValue(Laputop::Parameter::Beta, seedBeta_);
    // If the user wants to see A, D, or N, set those:
    if (fSeedA_ > 0) ldfParams_->SetValue(Laputop::Parameter::CurvParabA, fSeedA_);
    if (fSeedD_ > 0) ldfParams_->SetValue(Laputop::Parameter::CurvGaussD, fSeedD_);
    if (fSeedN_ > 0) ldfParams_->SetValue(Laputop::Parameter::CurvGaussN, fSeedN_);

  } else {
    // Make a blank one and fill it with seeds:
    log_debug("Creating a fresh params structure.");
    ldfParams_ = I3LaputopParamsPtr(new I3LaputopParams());
    ldfParams_->SetValue(Laputop::Parameter::Log10_S125, log10(seedS125_));
    ldfParams_->SetValue(Laputop::Parameter::Beta, seedBeta_);
    // If the user wants to see A, D, or N, set those:
    if (fSeedA_ > 0) ldfParams_->SetValue(Laputop::Parameter::CurvParabA, fSeedA_);
    if (fSeedD_ > 0) ldfParams_->SetValue(Laputop::Parameter::CurvGaussD, fSeedD_);
    if (fSeedN_ > 0) ldfParams_->SetValue(Laputop::Parameter::CurvGaussN, fSeedN_);
  }
  log_debug("My seed log-S125 and Beta: %f %f", 
	    ldfParams_->GetValue(Laputop::Parameter::Log10_S125), 
	    ldfParams_->GetValue(Laputop::Parameter::Beta));

  // Logging checks for Curvature: print 'em if you got 'em
  if (ldfParams_->Has(Laputop::Parameter::CurvParabA)) 
    log_debug("SetEvent seed A: %f", ldfParams_->GetValue(Laputop::Parameter::CurvParabA)); 
  else log_debug("SetEvent seed A: was not set"); 
  if (ldfParams_->Has(Laputop::Parameter::CurvGaussD)) 
    log_debug("SetEvent seed D: %f", ldfParams_->GetValue(Laputop::Parameter::CurvGaussD)); 
  else log_debug("SetEvent seed D: was not set"); 
  if (ldfParams_->Has(Laputop::Parameter::CurvGaussN)) 
    log_debug("SetEvent seed N: %f", ldfParams_->GetValue(Laputop::Parameter::CurvGaussN)); 
  else log_debug("SetEvent seed N: was not set"); 

  if (seedParticle_ && ldfParams_) return 1;
  
  else {
    log_warn("Something went wrong with the seedParticle");
    return 0;
  }

}

// get a seed
I3EventHypothesis
I3LaputopSeedService::GetSeed( unsigned int iseed ) const {
  log_debug("Entering GetSeed");

  // I'm only going to get ONE seed, no matter what "iseed" is...
  log_debug("Here's my seed: (%5.2f %5.2f %6.2f) (%4.2f %4.2f) t=%7.1f",
	    seedParticle_->GetX(), seedParticle_->GetY(), seedParticle_->GetZ(), 
	    seedParticle_->GetZenith(), seedParticle_->GetAzimuth(), 
	    seedParticle_->GetTime());
  
  log_debug("Here's my param seed: (%5.2f in the log, %5.2f)",
	    ldfParams_->GetValue(Laputop::Parameter::Log10_S125),
	    ldfParams_->GetValue(Laputop::Parameter::Beta));

  // Return it
  I3EventHypothesis eh(seedParticle_,
  		       ldfParams_ );
  
  return eh;
}

// get a dummy seed (useful in case of fg failure)
I3EventHypothesis
I3LaputopSeedService::GetDummy( ) const {
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
I3LaputopSeedService::Tweak( I3EventHypothesis &eh ) const {
  log_debug("Entering Tweak: none today");
    
}

/*
 * Filling the blanks in the track is delegated to the external track seeders.
 */
void
I3LaputopSeedService::FillInTheBlanks( I3EventHypothesis &eh ) const {
  log_debug("Entering FillInTheBlanks: none today");

}

/*
 * This will make a (deep) copy of the I3EventHypothesis.
 */
I3EventHypothesis
I3LaputopSeedService::GetCopy( const I3EventHypothesis &eh ) const {
  log_debug("Entering seed GetCopy");
  
  I3LaputopParamsConstPtr nonStdPtr = boost::dynamic_pointer_cast<const I3LaputopParams>(eh.nonstd);
  if (!nonStdPtr) log_fatal("Your nonstandard parameters is empty... did not cast correctly?");

  I3EventHypothesis neweh( I3ParticlePtr(new I3Particle(*(eh.particle))),
			   I3LaputopParamsPtr(new I3LaputopParams(*nonStdPtr)) );

  return neweh;
}

// in TTopRecoShower : remove badstations, bad pulses, etc...
// Some remarks : for seeding it's not that important that all tanks, pulses,etc... are very well cleaned
// It's just for an approximate, but reasonable First Guess S125 anyway!
double I3LaputopSeedService::seedLogS125( I3RecoPulseSeriesMapConstPtr rpsm, 
					  I3ParticleConstPtr seedPart,
					  const I3OMGeoMap &om_map){
  const double beta = 3.0;  // assume average beta
  double sumLogS125 = 0;
  int nUsed = 0;

  for (I3RecoPulseSeriesMap::const_iterator it = rpsm->begin(); it != rpsm->end(); ++it) {
    OMKey om_key = it->first; // need OMGeo for DistToAxis
    I3OMGeo om = om_map.find(om_key)->second;
    double abs_x_sq = (om.position.GetX() - seedPart->GetX())*(om.position.GetX() - seedPart->GetX()) 
      + (om.position.GetY() - seedPart->GetY()) * (om.position.GetY() - seedPart->GetY()) 
      + (om.position.GetZ() - seedPart->GetZ()) * (om.position.GetZ() - seedPart->GetZ());
    double n_prod_x = seedPart->GetDir().GetX() * (om.position.GetX() - seedPart->GetX()) 
      + seedPart->GetDir().GetY() * (om.position.GetY() - seedPart->GetY()) 
      + seedPart->GetDir().GetZ() * (om.position.GetZ() - seedPart->GetZ());
    double R_om_axis = sqrt(abs_x_sq - n_prod_x * n_prod_x);
    log_debug("R DOM to axis : %lf",R_om_axis);
    double logR = log10(R_om_axis/LateralFitFunctions::R0_PARAM);
    const I3RecoPulseSeries pulse_series = it->second;
    for (I3RecoPulseSeries::const_iterator i_series = pulse_series.begin ();
         i_series != pulse_series.end (); ++i_series) {

      if ((logR > log10(125./LateralFitFunctions::R0_PARAM)) && // pulses too close to the core are unaccurate
	  (i_series->GetCharge() == i_series->GetCharge()) &&  // the same as SortOutBadCharges and Times
	  (i_series->GetTime() == i_series->GetTime()) && 
	  (i_series->GetCharge() <= SAT_HG)) {   
	log_trace("Adding to sumLogS125 : %lf - %lf - %lf",log10(i_series->GetCharge()),beta*logR,LateralFitFunctions::KAPPA*logR*logR);
	sumLogS125 += log10(i_series->GetCharge()) + beta*logR + LateralFitFunctions::KAPPA*logR*logR;
	++nUsed;
      }
    }
  }
  if (nUsed < 5) return 0;

  if ((sumLogS125 != sumLogS125) || (rpsm->size() == 0)) {
    std::cout << sumLogS125 << std::endl;
    std::cout << rpsm->size() << "\n---------------------------------" << std::endl;
  }
  
  double meanLogS125 = sumLogS125/nUsed;
  if (meanLogS125 < -0.5) return 0;
  return meanLogS125;
}


typedef I3SingleServiceFactory< I3LaputopSeedService,I3SeedServiceBase >
I3LaputopSeedServiceFactory;
I3_SERVICE_FACTORY( I3LaputopSeedServiceFactory )
