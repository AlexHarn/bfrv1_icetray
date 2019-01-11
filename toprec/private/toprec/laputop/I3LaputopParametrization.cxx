/**
 * @file I3LaputopParametrization.cxx
 * @brief implementation of the I3LaputopParametrization class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author kath
 *
 */

//////////////////////
// This code was stolen and adapted from double-muon!
//   double-muon/private/double-muon/I3DoubleMuonParametrization.cxx
//////////////////////

#include "toprec/I3LaputopParametrization.h"
#include "icetray/I3SingleServiceFactory.h"
#include "gulliver/I3Gulliver.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Constants.h"
#include <cmath>
#include <recclasses/I3LaputopParams.h>

// User parameter tags, defaults, and descriptions
const std::string I3LaputopParametrization::FIX_CORE_TAG = "fixCore";
const std::string I3LaputopParametrization::FIX_SIZE_TAG = "fixSize";
const std::string I3LaputopParametrization::FIX_TRACKDIR_TAG = "fixTrackDir";
const std::string I3LaputopParametrization::FIT_SNOWCORRECTIONFACTOR_TAG = "fitSnowCorrection";
const std::string I3LaputopParametrization::BETA_TAG = "isBeta";
const std::string I3LaputopParametrization::MINBETA_TAG = "minBeta";
const std::string I3LaputopParametrization::MAXBETA_TAG = "maxBeta";
const std::string I3LaputopParametrization::LIMITCORE_TAG = "limitcoreboxsize";
const std::string I3LaputopParametrization::COREXY_TAG = "coreXYLimits";
const std::string I3LaputopParametrization::MAXLOGS125_TAG = "maxLogS125";

const bool I3LaputopParametrization::DEFAULT_FIX_CORE = false;
const bool I3LaputopParametrization::DEFAULT_FIX_SIZE = false;
const bool I3LaputopParametrization::DEFAULT_FIX_TRACKDIR = false;
const bool I3LaputopParametrization::DEFAULT_FIT_SNOWCORRECTIONFACTOR = false;
const bool I3LaputopParametrization::DEFAULT_ISITBETA = true;
const double I3LaputopParametrization::DEFAULT_LIMITCOREBOXSIZE = -1;  // Numbers greater than zero will use relative parameters
const double I3LaputopParametrization:: DEFAULT_MINBETA = 1.5;
const double I3LaputopParametrization:: DEFAULT_MAXBETA = 5.0;
const double I3LaputopParametrization:: DEFAULT_COREXY = 2000.0;
const double I3LaputopParametrization:: DEFAULT_MAXLOGS125 = 6.0;

const std::string I3LaputopParametrization::FIX_CORE_DESCRIPTION = "Fix the seed Core?";
const std::string I3LaputopParametrization::FIX_SIZE_DESCRIPTION = "Fix S125 and Beta?";
const std::string I3LaputopParametrization::FIX_TRACKDIR_DESCRIPTION = "Fix the seed track direction and time?";
const std::string I3LaputopParametrization::FIT_SNOWCORRECTIONFACTOR_DESCRIPTION
= "THIS PARAMETER IS BECOMING OBSOLETE. You'll have to write your own module to do this functionality.";
//= "Fit the snow correction factor on an event-by-event basis as a free parameter in Minuit";
const std::string I3LaputopParametrization::BETA_DESCRIPTION = "Is it Beta? (set to false if it's Age)";
const std::string I3LaputopParametrization::MINBETA_DESCRIPTION = "Minimum Beta (or age)";
const std::string I3LaputopParametrization::MAXBETA_DESCRIPTION = "Maximum Beta (or age)";
const std::string I3LaputopParametrization::LIMITCORE_DESCRIPTION = "Limit the core location to a square around previous core, of plus OR minus this number in X and Y.";
const std::string I3LaputopParametrization::COREXY_DESCRIPTION = "Maximum/minimum of X and Y of core (default = 2000)";
const std::string I3LaputopParametrization::MAXLOGS125_DESCRIPTION = "Maximum Log10(S125) (default = 6)";

// More tags
const std::string I3LaputopParametrization::vstep_optionname = "VertexStepsize";
//const std::string I3LaputopParametrization::astep_optionname = "AngleStepsize";
const std::string I3LaputopParametrization::sstep_optionname = "SStepsize";
const std::string I3LaputopParametrization::betastep_optionname = "BetaStepsize";
//const std::string I3LaputopParametrization::snowstep_optionname = "SnowStepsize";



// Other hard-coded defaults
// Defaults from toprec (maybe make these user-adjustable later)
// double step[8]={20.0*I3Units::m, 20.0*I3Units::m, 1., 0.6, .05, .05, 50., snowFactorStep};
const double I3LaputopParametrization:: DEFAULT_CORE_STEPSIZE = 20.0*I3Units::m;
const double I3LaputopParametrization:: DEFAULT_NXY_STEPSIZE = 0.5;
const double I3LaputopParametrization:: DEFAULT_LOGS125_STEPSIZE = 1.0;
const double I3LaputopParametrization:: DEFAULT_BETAAGE_STEPSIZE = 0.6;
const double I3LaputopParametrization:: DEFAULT_T0_STEPSIZE = 50.0*I3Units::ns;
const double I3LaputopParametrization:: DEFAULT_SNOWD0_STEPSIZE = 0.4;

// Curvature A, D, N stuff (as is done in I3CurvatureParametrization)
// Tags
const std::string I3LaputopParametrization::MAX_A_TAG = "MaxA";
const std::string I3LaputopParametrization::MAX_D_TAG = "MaxD";
const std::string I3LaputopParametrization::MAX_N_TAG = "MaxN";
const std::string I3LaputopParametrization::MIN_A_TAG = "MinA";
const std::string I3LaputopParametrization::MIN_D_TAG = "MinD";
const std::string I3LaputopParametrization::MIN_N_TAG = "MinN";
const std::string I3LaputopParametrization::STEPSIZE_A_TAG = "StepsizeA";
const std::string I3LaputopParametrization::STEPSIZE_D_TAG = "StepsizeD";
const std::string I3LaputopParametrization::STEPSIZE_N_TAG = "StepsizeN";
const std::string I3LaputopParametrization::FREE_A_TAG = "FreeA";
const std::string I3LaputopParametrization::FREE_D_TAG = "FreeD";
const std::string I3LaputopParametrization::FREE_N_TAG = "FreeN";
// Stepsizes
const double I3LaputopParametrization:: DEFAULT_A_MIN = 1e-4;  // A=4.823e-4 from top_curv_gausspar
const double I3LaputopParametrization:: DEFAULT_A_MAX = 1e-3;
const double I3LaputopParametrization:: DEFAULT_A_STEPSIZE = 5e-6;
const double I3LaputopParametrization:: DEFAULT_D_MIN = 10;  // D=118.1 from top_curv_gausspar
const double I3LaputopParametrization:: DEFAULT_D_MAX = 300;
const double I3LaputopParametrization:: DEFAULT_D_STEPSIZE = 1.0;
const double I3LaputopParametrization:: DEFAULT_N_MIN = 0.0;  // N=19.41 from top_curv_gausspar
const double I3LaputopParametrization:: DEFAULT_N_MAX = 50.0;
const double I3LaputopParametrization:: DEFAULT_N_STEPSIZE = 1.0;
// Descriptions
const std::string I3LaputopParametrization::MAX_A_DESC = "Maximum A";
const std::string I3LaputopParametrization::MAX_D_DESC = "Maximum D";
const std::string I3LaputopParametrization::MAX_N_DESC = "Maximum N";
const std::string I3LaputopParametrization::MIN_A_DESC = "Minimum A";
const std::string I3LaputopParametrization::MIN_D_DESC = "Minimum D";
const std::string I3LaputopParametrization::MIN_N_DESC = "Minimum N";
const std::string I3LaputopParametrization::STEPSIZE_A_DESC = "Stepsize for A";
const std::string I3LaputopParametrization::STEPSIZE_D_DESC = "Stepsize for D";
const std::string I3LaputopParametrization::STEPSIZE_N_DESC = "Stepsize for N";
const std::string I3LaputopParametrization::FREE_A_DESC = "Let A float as a free parameter?";
const std::string I3LaputopParametrization::FREE_D_DESC = "Let D float as a free parameter?";
const std::string I3LaputopParametrization::FREE_N_DESC = "Let N float as a free parameter?";




I3LaputopParametrization::I3LaputopParametrization(std::string name):
    I3ServiceBase(name),
    I3ParametrizationBase(I3EventHypothesisPtr() ){}

/// constructor I3Tray
I3LaputopParametrization::I3LaputopParametrization(const I3Context &c):
    I3ServiceBase(c),
    I3ParametrizationBase(I3EventHypothesisPtr())
{

  // Booleans
  fFixCore_ = DEFAULT_FIX_CORE;
  fFixSize_ = DEFAULT_FIX_SIZE;
  fFixDir_ = DEFAULT_FIX_TRACKDIR;
  fFitSnowCorrectionFactor_ = DEFAULT_FIT_SNOWCORRECTIONFACTOR;
  fIsItBeta_ = DEFAULT_ISITBETA;
  fLimitCoreBoxSize_ = DEFAULT_LIMITCOREBOXSIZE;

  // Stepsizes
  fCoreStep_ = DEFAULT_CORE_STEPSIZE;
  fNxyStep_ = DEFAULT_NXY_STEPSIZE;
  fLogS125Step_ = DEFAULT_LOGS125_STEPSIZE;
  fBetaAgeStep_ = DEFAULT_BETAAGE_STEPSIZE;
  fT0Step_ = DEFAULT_T0_STEPSIZE;
  fSnowD0Step_ = DEFAULT_SNOWD0_STEPSIZE;

  fBetaMin_ = DEFAULT_MINBETA;
  fBetaMax_ = DEFAULT_MAXBETA;
  fCoreXYLimit_ = DEFAULT_COREXY;
  fLogS125Max_ = DEFAULT_MAXLOGS125;
  
  seedX_ = 0;
  seedY_ = 0;
  seedT_ = 0;

  AddParameter (FIX_CORE_TAG,FIX_CORE_DESCRIPTION, fFixCore_);
  AddParameter (FIX_SIZE_TAG,FIX_SIZE_DESCRIPTION, fFixSize_);
  AddParameter (FIX_TRACKDIR_TAG,FIX_TRACKDIR_DESCRIPTION, fFixDir_);
  AddParameter (FIT_SNOWCORRECTIONFACTOR_TAG,FIT_SNOWCORRECTIONFACTOR_DESCRIPTION, fFitSnowCorrectionFactor_);
  AddParameter (BETA_TAG,BETA_DESCRIPTION, fIsItBeta_);
  AddParameter (MINBETA_TAG,MINBETA_DESCRIPTION, fBetaMin_);
  AddParameter (MAXBETA_TAG,MAXBETA_DESCRIPTION, fBetaMax_);
  AddParameter (LIMITCORE_TAG,LIMITCORE_DESCRIPTION, fLimitCoreBoxSize_);
  AddParameter (COREXY_TAG,COREXY_DESCRIPTION, fCoreXYLimit_);
  AddParameter (MAXLOGS125_TAG,MAXLOGS125_DESCRIPTION, fLogS125Max_);

  AddParameter (vstep_optionname, "Stepsize for the vertices", fCoreStep_ );
  //AddParameter (astep_optionname, "Stepsize for the angles", angleStepsize_ ); 
  AddParameter (sstep_optionname, "Stepsize for the Log(S125)", fLogS125Step_ );
  AddParameter (betastep_optionname, "Stepsize for the beta (or age)", fBetaAgeStep_ );
  //AddParameter (snowstep_optionname, "Stepsize for the snow attn length", fSnowD0Step_ );
 
  //---- CURVATURE PARAMETERS -------
  // Booleans
  fFreeA_ = false;
  fFreeD_ = false;
  fFreeN_ = false;

  // Max, Min, and Stepsize
  fAMin_ = DEFAULT_A_MIN;
  fDMin_ = DEFAULT_D_MIN;
  fNMin_ = DEFAULT_N_MIN;
  fAMax_ = DEFAULT_A_MAX;
  fDMax_ = DEFAULT_D_MAX;
  fNMax_ = DEFAULT_N_MAX;
  fAStep_ = DEFAULT_A_STEPSIZE;
  fDStep_ = DEFAULT_D_STEPSIZE;
  fNStep_ = DEFAULT_N_STEPSIZE;
  
  AddParameter (MIN_A_TAG,MIN_A_DESC, fAMin_);
  AddParameter (MIN_D_TAG,MIN_D_DESC, fDMin_);
  AddParameter (MIN_N_TAG,MIN_N_DESC, fNMin_);
  AddParameter (MAX_A_TAG,MAX_A_DESC, fAMax_);
  AddParameter (MAX_D_TAG,MAX_D_DESC, fDMax_);
  AddParameter (MAX_N_TAG,MAX_N_DESC, fNMax_);
  AddParameter (STEPSIZE_A_TAG,STEPSIZE_A_DESC, fAStep_);
  AddParameter (STEPSIZE_D_TAG,STEPSIZE_D_DESC, fDStep_);
  AddParameter (STEPSIZE_N_TAG,STEPSIZE_N_DESC, fNStep_);
  AddParameter (FREE_A_TAG,FREE_A_DESC, fFreeA_);
  AddParameter (FREE_D_TAG,FREE_D_DESC, fFreeD_);
  AddParameter (FREE_N_TAG,FREE_N_DESC, fFreeN_);

}

void I3LaputopParametrization::Configure(){

    GetParameter (FIX_CORE_TAG, fFixCore_);
    GetParameter (FIX_SIZE_TAG, fFixSize_);
    GetParameter (FIX_TRACKDIR_TAG, fFixDir_);
    GetParameter (FIT_SNOWCORRECTIONFACTOR_TAG, fFitSnowCorrectionFactor_);
    GetParameter (BETA_TAG, fIsItBeta_);
    GetParameter (MINBETA_TAG, fBetaMin_);
    GetParameter (MAXBETA_TAG, fBetaMax_);
    GetParameter (LIMITCORE_TAG, fLimitCoreBoxSize_);
    GetParameter (COREXY_TAG, fCoreXYLimit_);
    GetParameter (MAXLOGS125_TAG, fLogS125Max_);

    GetParameter (vstep_optionname, fCoreStep_ );
    GetParameter (sstep_optionname, fLogS125Step_ );
    GetParameter (betastep_optionname, fBetaAgeStep_ );
    //GetParameter (snowstep_optionname, fSnowD0Step_ );

    // --- CURVATURE PARAMETERS ----
    GetParameter (MIN_A_TAG, fAMin_);
    GetParameter (MIN_D_TAG, fDMin_);
    GetParameter (MIN_N_TAG, fNMin_);
    GetParameter (MAX_A_TAG, fAMax_);
    GetParameter (MAX_D_TAG, fDMax_);
    GetParameter (MAX_N_TAG, fNMax_);
    GetParameter (STEPSIZE_A_TAG, fAStep_);
    GetParameter (STEPSIZE_D_TAG, fDStep_);
    GetParameter (STEPSIZE_N_TAG, fNStep_);
    GetParameter (FREE_A_TAG, fFreeA_);
    GetParameter (FREE_D_TAG, fFreeD_);
    GetParameter (FREE_N_TAG, fFreeN_);

    if ( fCoreStep_ <= 0 ){
        log_fatal( "(%s) vertex stepsize should be positive, I got %g m",
                   GetName().c_str(), fCoreStep_/I3Units::m );
    }
    //if ( angleStepsize_ <= 0 ){
    //   log_fatal( "(%s) angle stepsize should be positive, I got %g radians",
    //              GetName().c_str(), angleStepsize_/I3Units::rad );
    //}

    I3FitParameterInitSpecs specs("blah");

    // The core
    if (!fFixCore_) {
      if (fLimitCoreBoxSize_ > 0) {  // relative core location
	// Work with userdefined squares here, or in the future with some gaussian llh around the previous core.
	// Size of the box is user-defined (not related to the stepsize)
	specs.name_ = "RelCoreX";
	specs.stepsize_ = fCoreStep_;
	specs.minval_ = -fLimitCoreBoxSize_;   
	specs.maxval_ =  fLimitCoreBoxSize_;
	parspecs_.push_back(specs);
	specs.name_ = "RelCoreY";
	specs.stepsize_ = fCoreStep_;
	specs.minval_ = -fLimitCoreBoxSize_;
	specs.maxval_ =  fLimitCoreBoxSize_;
	parspecs_.push_back(specs);	

      } else {  // absolute core location
	specs.name_ = "CoreX";
	specs.stepsize_ = fCoreStep_;
	specs.minval_ = -fCoreXYLimit_*I3Units::m; // originally unbounded (0) in toprec
	specs.maxval_ =  fCoreXYLimit_*I3Units::m;  // ... but I like this better
	parspecs_.push_back(specs);
	specs.name_ = "CoreY";
	specs.stepsize_ = fCoreStep_;
	specs.minval_ = -fCoreXYLimit_*I3Units::m;
	specs.maxval_ =  fCoreXYLimit_*I3Units::m;
	parspecs_.push_back(specs);
      }
    }

    // log(S125)
    if (!fFixSize_) {
      specs.name_ = "Log10(S125)";
      specs.stepsize_ = fLogS125Step_;
      specs.minval_ = -3.0;     // These max/mins from toprec code
      specs.maxval_ = fLogS125Max_;  // default = 6, which is from toprec code
      parspecs_.push_back(specs);

      // Beta (or age, depending)
      if (fIsItBeta_) specs.name_ = "Beta";
      else specs.name_ = "Age";
      specs.stepsize_ = fBetaAgeStep_;
      specs.minval_ = fBetaMin_;     // Aw, hell, leave it to the user!
      specs.maxval_ = fBetaMax_;
      parspecs_.push_back(specs);
    }

    // Track direction
    if (!fFixDir_) {
      specs.name_ = "DirNX";
      specs.stepsize_ = fNxyStep_;
      specs.minval_ = -1.0;
      specs.maxval_ =  1.0;
      parspecs_.push_back(specs);
      specs.name_ = "DirNY";
      specs.stepsize_ = fNxyStep_;
      specs.minval_ = -1.0;
      specs.maxval_ =  1.0;
      parspecs_.push_back(specs);
      // Time
      specs.name_ = "Time";
      specs.stepsize_ = fT0Step_;
      specs.minval_ = 0; // unbounded
      specs.maxval_ = 0;
      //specs.minval_ = -10000.0*I3Units::ns;
      //specs.maxval_ = 100000.0*I3Units::ns;
      parspecs_.push_back(specs);

    }
    
    // --- CURVATURE PARAMETERS ----
    if (fFreeA_) {
      specs.name_ = "A";
      specs.stepsize_ = fAStep_;
      specs.minval_ = fAMin_;
      specs.maxval_ = fAMax_;
      parspecs_.push_back(specs);
      log_trace("Setting up Laputop FREE_A: min = %f, max = %f, stepsize = %f", 
		specs.minval_, specs.maxval_, specs.stepsize_);
    }
    if (fFreeD_) {
      specs.name_ = "D";
      specs.stepsize_ = fDStep_;
      specs.minval_ = fDMin_;
      specs.maxval_ = fDMax_;
      parspecs_.push_back(specs);
    }
    if (fFreeN_) {
      specs.name_ = "N";
      specs.stepsize_ = fNStep_;
      specs.minval_ = fNMin_;
      specs.maxval_ = fNMax_;
      parspecs_.push_back(specs);
    }

    // Snow
    if (fFitSnowCorrectionFactor_) {
      log_fatal("This option is obsolete!  You should be using a SnowCorrectionService instead.");
    }

    par_.resize(parspecs_.size(),NAN);
}

/// compute event hypothesis from minimizer parameters
void I3LaputopParametrization::UpdatePhysicsVariables(){
    log_trace("Entering UpdatePhysicsVariables");
    I3ParticlePtr track = hypothesis_->particle;
    I3LaputopParamsPtr paramsPtr =
        boost::dynamic_pointer_cast<I3LaputopParams>(hypothesis_->nonstd);
    assert(paramsPtr);

    int i = 0;
    // Optional: core
    if (!fFixCore_) {
      if (fLimitCoreBoxSize_ > 0) 
        track->SetPos( seedX_+par_[0], seedY_+par_[1], track->GetZ() );
      else 
        track->SetPos( par_[0], par_[1], track->GetZ() );
      i += 2;
    }
    // optional: S125 and age
    if (!fFixSize_) {
      paramsPtr->SetValue(Laputop::Parameter::Log10_S125, par_[i]);
      paramsPtr->SetValue(Laputop::Parameter::Beta, par_[i+1]);
      i += 2;
    } 
    // optional: track dir and time
    if (!fFixDir_) {
      // Remember, nz is always negative for downgoing particles!
      track->SetDir( par_[i], par_[i+1], -sqrt(1-par_[i]*par_[i]-par_[i+1]*par_[i+1] ));
      track->SetTime(par_[i+2]);
      i += 3;
    } else {
      // The time needs to be updated if the core has moved
      // Take T0, X0, Y0 from "storage" and adjust time to match what  
      // it would be at new position of core
      // (This code borrowed/copied from I3LaputopSeedService.cxx)
      double nx = track->GetDir().GetX();
      double ny = track->GetDir().GetY();
      double T0 = seedT_ +
        (nx * (track->GetX() - seedX_) +
         ny * (track->GetY() - seedY_))/I3Constants::c;
      log_trace("nx, ny: %f %f -- deltax, deltay: %f %f -- old T %f, new T %f",
                nx, ny, track->GetX() - seedX_, track->GetY() - seedY_, seedT_, T0);
      track->SetTime(T0);
    }

    // Set the Curvature Parameters?
    if (fFreeA_) { paramsPtr->SetValue(Laputop::Parameter::CurvParabA, par_[i]); i += 1;}
    if (fFreeD_) { paramsPtr->SetValue(Laputop::Parameter::CurvGaussD, par_[i]); i += 1;}
    if (fFreeN_) { paramsPtr->SetValue(Laputop::Parameter::CurvGaussN, par_[i]); i += 1;}


    I3Gulliver::AnglesInRange(*track,GetName());

    for (int ii=0; ii<i; ii++)
      log_trace( "(%s) update var: p%u=%g ",
    		 GetName().c_str(), ii, par_[ii] );
}

void I3LaputopParametrization::PassCovariance(const boost::numeric::ublas::symmetric_matrix<double>& cov) {
    log_trace("Entering PassCovariance");
    I3ParticlePtr track = hypothesis_->particle;
    I3LaputopParamsPtr paramsPtr =
        boost::dynamic_pointer_cast<I3LaputopParams>(hypothesis_->nonstd);
    assert(paramsPtr);

    // This "structure" or ordering of the errors is done identically as above in UpdatePhysicsVariables
    // If you change that one, you gotta change this one accordingly!
    std::vector<Laputop::Parameter::Enum> pars;

    // Optional: core
    if (!fFixCore_) {
      pars.push_back(Laputop::Parameter::Xc);
      pars.push_back(Laputop::Parameter::Yc);
    }

    // optional: S125 and age
    if (!fFixSize_) {
      pars.push_back(Laputop::Parameter::Log10_S125);
      pars.push_back(Laputop::Parameter::Beta);
    }

    // optional: track dir and time
    if (!fFixDir_) {
      pars.push_back(Laputop::Parameter::Nx);
      pars.push_back(Laputop::Parameter::Ny);
      pars.push_back(Laputop::Parameter::Tc);
    }

    // Curvature parameters
    if (fFreeA_) pars.push_back(Laputop::Parameter::CurvParabA);
    if (fFreeD_) pars.push_back(Laputop::Parameter::CurvGaussD);
    if (fFreeN_) pars.push_back(Laputop::Parameter::CurvGaussN);

    // now copy the covariance matrix
    for (unsigned i = 0; i < pars.size(); ++i)
      for (unsigned j = i; j < pars.size(); ++j)
        paramsPtr->SetCovariance(pars[i], pars[j], cov(i, j));
}

/// compute minimizer parameters from event hypothesis
void I3LaputopParametrization::UpdateParameters(){
    log_debug("Entering UpdateParameters");
    const I3ParticlePtr track = hypothesis_->particle;
    const I3Position& pos1 = track->GetPos();
    const I3Direction& dir1 = track->GetDir();
    I3LaputopParamsPtr paramsPtr =
      boost::dynamic_pointer_cast<I3LaputopParams>(hypothesis_->nonstd);
    if (!paramsPtr){
        log_fatal( "(%s) incomplete/wrong event hypothesis; wrong seed service?",
                   GetName().c_str() );
    }

    // This function is being called ONCE per physics event, so here
    // we'll set the seed core position for use throughout
    // the process.
    seedX_ = pos1.GetX();
    seedY_ = pos1.GetY();
    seedT_ = track->GetTime();
    log_debug("Setting seedX and seedY, seedT: %f %f, %f", seedX_, seedY_, seedT_);

    int i=0;
    if (!fFixCore_) {
      if (fLimitCoreBoxSize_ > 0) {
	par_[0] = pos1.GetX()-seedX_;
	par_[1] = pos1.GetY()-seedY_;
      } else {
	par_[0] = pos1.GetX();
	par_[1] = pos1.GetY();
      }	
      i += 2;
    }
    if (!fFixSize_) {
      log_debug("Filling par[i] and par[i+1] with %f and %f\n", 
		paramsPtr->GetValue(Laputop::Parameter::Log10_S125),
		paramsPtr->GetValue(Laputop::Parameter::Beta));
      par_[i] = paramsPtr->GetValue(Laputop::Parameter::Log10_S125);   // this time, the LOG is the one stored!
      par_[i+1] = paramsPtr->GetValue(Laputop::Parameter::Beta);   // beta
      i += 2;
    }
    if (!fFixDir_) {
      par_[i] = dir1.GetX();
      par_[i+1] = dir1.GetY();
      par_[i+2] = track->GetTime();
      i += 3;
    }

    if (fFreeA_) { par_[i] = paramsPtr->GetValue(Laputop::Parameter::CurvParabA); i += 1; }
    if (fFreeD_) { par_[i] = paramsPtr->GetValue(Laputop::Parameter::CurvGaussD); i += 1; }
    if (fFreeN_) { par_[i] = paramsPtr->GetValue(Laputop::Parameter::CurvGaussN); i += 1; }

    if (fFitSnowCorrectionFactor_) {
      log_fatal("This option is obsolete!  You should be using a SnowCorrectionService instead.");
    }

    for (int ii=0; ii<i; ii++)
      log_debug( "(%s) setting var: p%u=%g",
                 GetName().c_str(), ii, par_[ii] );

}

typedef
I3SingleServiceFactory<I3LaputopParametrization,I3ParametrizationBase>
I3LaputopParametrizationServiceFactory;
I3_SERVICE_FACTORY( I3LaputopParametrizationServiceFactory );
