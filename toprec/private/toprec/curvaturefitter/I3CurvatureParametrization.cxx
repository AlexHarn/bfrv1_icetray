/**
 * @file I3CurvatureParametrization.cxx
 * @brief implementation of the I3CurvatureParametrization class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id: I3CurvatureParametrization.cxx 143462 2016-03-18 19:24:40Z hdembinski $
 *
 * @version $Revision: 143462 $
 * @date $Date: 2016-03-18 14:24:40 -0500 (Fri, 18 Mar 2016) $
 * @author kath
 *
 */

//////////////////////
// The CurvatureFitter is very simple, and only has (for now) one free parameter: "A"
// This code was stolen and adapted from:
//   toprec/private/toprec/laputop/I3LaputopParametrization.cxx
//////////////////////

#include "toprec/I3CurvatureParametrization.h"
#include "icetray/I3SingleServiceFactory.h"
#include "gulliver/I3Gulliver.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Constants.h"
#include <cmath>
#include <recclasses/I3LaputopParams.h>

// User parameter tags, defaults, and descriptions
const std::string I3CurvatureParametrization::MAX_A_TAG = "MaxA";
const std::string I3CurvatureParametrization::MAX_D_TAG = "MaxD";
const std::string I3CurvatureParametrization::MAX_N_TAG = "MaxN";
const std::string I3CurvatureParametrization::MAX_T_TAG = "MaxT";
const std::string I3CurvatureParametrization::MIN_A_TAG = "MinA";
const std::string I3CurvatureParametrization::MIN_D_TAG = "MinD";
const std::string I3CurvatureParametrization::MIN_N_TAG = "MinN";
const std::string I3CurvatureParametrization::MIN_T_TAG = "MinT";
const std::string I3CurvatureParametrization::STEPSIZE_A_TAG = "StepsizeA";
const std::string I3CurvatureParametrization::STEPSIZE_D_TAG = "StepsizeD";
const std::string I3CurvatureParametrization::STEPSIZE_N_TAG = "StepsizeN";
const std::string I3CurvatureParametrization::STEPSIZE_T_TAG = "StepsizeT";
const std::string I3CurvatureParametrization::FREE_A_TAG = "FreeA";
const std::string I3CurvatureParametrization::FREE_D_TAG = "FreeD";
const std::string I3CurvatureParametrization::FREE_N_TAG = "FreeN";
const std::string I3CurvatureParametrization::FREE_T_TAG = "FreeT";

const double I3CurvatureParametrization:: DEFAULT_A_MIN = 1e-4;  // A=4.823e-4 from top_curv_gausspar
const double I3CurvatureParametrization:: DEFAULT_A_MAX = 1e-3;
const double I3CurvatureParametrization:: DEFAULT_A_STEPSIZE = 5e-6;
const double I3CurvatureParametrization:: DEFAULT_D_MIN = 10;  // D=118.1 from top_curv_gausspar
const double I3CurvatureParametrization:: DEFAULT_D_MAX = 200;
const double I3CurvatureParametrization:: DEFAULT_D_STEPSIZE = 1.0;
const double I3CurvatureParametrization:: DEFAULT_N_MIN = 0.0;  // N=19.41 from top_curv_gausspar
const double I3CurvatureParametrization:: DEFAULT_N_MAX = 50.0;
const double I3CurvatureParametrization:: DEFAULT_N_STEPSIZE = 1.0;
const double I3CurvatureParametrization:: DEFAULT_T_MIN = 9000.0;  // N=19.41 from top_curv_gausspar
const double I3CurvatureParametrization:: DEFAULT_T_MAX = 50000.0;
const double I3CurvatureParametrization:: DEFAULT_T_STEPSIZE = 100.0;

const std::string I3CurvatureParametrization::MAX_A_DESC = "Maximum A";
const std::string I3CurvatureParametrization::MAX_D_DESC = "Maximum D";
const std::string I3CurvatureParametrization::MAX_N_DESC = "Maximum N";
const std::string I3CurvatureParametrization::MAX_T_DESC = "Maximum T";
const std::string I3CurvatureParametrization::MIN_A_DESC = "Minimum A";
const std::string I3CurvatureParametrization::MIN_D_DESC = "Minimum D";
const std::string I3CurvatureParametrization::MIN_N_DESC = "Minimum N";
const std::string I3CurvatureParametrization::MIN_T_DESC = "Minimum T";
const std::string I3CurvatureParametrization::STEPSIZE_A_DESC = "Stepsize for A";
const std::string I3CurvatureParametrization::STEPSIZE_D_DESC = "Stepsize for D";
const std::string I3CurvatureParametrization::STEPSIZE_N_DESC = "Stepsize for N";
const std::string I3CurvatureParametrization::STEPSIZE_T_DESC = "Stepsize for T";

const std::string I3CurvatureParametrization::FREE_A_DESC = "Let A float as a free parameter?";
const std::string I3CurvatureParametrization::FREE_D_DESC = "Let D float as a free parameter?";
const std::string I3CurvatureParametrization::FREE_N_DESC = "Let N float as a free parameter?";
const std::string I3CurvatureParametrization::FREE_T_DESC = "Let T float as a free parameter?";


I3CurvatureParametrization::I3CurvatureParametrization(std::string name):
    I3ServiceBase(name),
    I3ParametrizationBase(I3EventHypothesisPtr() ){}

/// constructor I3Tray
I3CurvatureParametrization::I3CurvatureParametrization(const I3Context &c):
    I3ServiceBase(c),
    I3ParametrizationBase(I3EventHypothesisPtr())
{

  // Booleans
  fFreeA_ = false;
  fFreeD_ = false;
  fFreeN_ = false;
  fFreeT_ = false;

  // Max, Min, and Stepsize
  fAMin_ = DEFAULT_A_MIN;
  fDMin_ = DEFAULT_D_MIN;
  fNMin_ = DEFAULT_N_MIN;
  fTMin_ = DEFAULT_T_MIN;
  fAMax_ = DEFAULT_A_MAX;
  fDMax_ = DEFAULT_D_MAX;
  fNMax_ = DEFAULT_N_MAX;
  fTMax_ = DEFAULT_T_MAX;
  fAStep_ = DEFAULT_A_STEPSIZE;
  fDStep_ = DEFAULT_D_STEPSIZE;
  fNStep_ = DEFAULT_N_STEPSIZE;
  fTStep_ = DEFAULT_T_STEPSIZE;
  
  AddParameter (MIN_A_TAG,MIN_A_DESC, fAMin_);
  AddParameter (MIN_D_TAG,MIN_D_DESC, fDMin_);
  AddParameter (MIN_N_TAG,MIN_N_DESC, fNMin_);
  AddParameter (MIN_T_TAG,MIN_T_DESC, fTMin_);
  AddParameter (MAX_A_TAG,MAX_A_DESC, fAMax_);
  AddParameter (MAX_D_TAG,MAX_D_DESC, fDMax_);
  AddParameter (MAX_N_TAG,MAX_N_DESC, fNMax_);
  AddParameter (MAX_T_TAG,MAX_T_DESC, fTMax_);
  AddParameter (STEPSIZE_A_TAG,STEPSIZE_A_DESC, fAStep_);
  AddParameter (STEPSIZE_D_TAG,STEPSIZE_D_DESC, fDStep_);
  AddParameter (STEPSIZE_N_TAG,STEPSIZE_N_DESC, fNStep_);
  AddParameter (STEPSIZE_T_TAG,STEPSIZE_T_DESC, fTStep_);

  AddParameter (FREE_A_TAG,FREE_A_DESC, fFreeA_);
  AddParameter (FREE_D_TAG,FREE_D_DESC, fFreeD_);
  AddParameter (FREE_N_TAG,FREE_N_DESC, fFreeN_);
  AddParameter (FREE_T_TAG,FREE_T_DESC, fFreeT_);

  // Indeces
  iparA_ = 0;
  iparD_ = 0;
  iparN_ = 0;
  iparT_ = 0;
}

void I3CurvatureParametrization::Configure(){

  GetParameter (MIN_A_TAG, fAMin_);
  GetParameter (MIN_D_TAG, fDMin_);
  GetParameter (MIN_N_TAG, fNMin_);
  GetParameter (MIN_T_TAG, fTMin_);
  GetParameter (MAX_A_TAG, fAMax_);
  GetParameter (MAX_D_TAG, fDMax_);
  GetParameter (MAX_N_TAG, fNMax_);
  GetParameter (MAX_T_TAG, fTMax_);
  GetParameter (STEPSIZE_A_TAG, fAStep_);
  GetParameter (STEPSIZE_D_TAG, fDStep_);
  GetParameter (STEPSIZE_N_TAG, fNStep_);
  GetParameter (STEPSIZE_T_TAG, fTStep_);
  GetParameter (FREE_A_TAG, fFreeA_);
  GetParameter (FREE_D_TAG, fFreeD_);
  GetParameter (FREE_N_TAG, fFreeN_);
  GetParameter (FREE_T_TAG, fFreeT_);

  I3FitParameterInitSpecs specs("blah");
  if (fFreeA_) {
    specs.name_ = "A";
    specs.stepsize_ = fAStep_;
    specs.minval_ = fAMin_;
    specs.maxval_ = fAMax_;
    parspecs_.push_back(specs);
    iparD_ += 1;
    iparN_ += 1;    
    iparT_ += 1;
    log_trace("Setting up Curvature FREE_A: min = %f, max = %f, stepsize = %f", 
	      specs.minval_, specs.maxval_, specs.stepsize_);
  }
  if (fFreeD_) {
    specs.name_ = "D";
    specs.stepsize_ = fDStep_;
    specs.minval_ = fDMin_;
    specs.maxval_ = fDMax_;
    parspecs_.push_back(specs);
    iparN_ += 1;    
    iparT_ += 1;
  }
  if (fFreeN_) {
    specs.name_ = "N";
    specs.stepsize_ = fNStep_;
    specs.minval_ = fNMin_;
    specs.maxval_ = fNMax_;
    parspecs_.push_back(specs);
    iparT_ += 1;
  }
  if (fFreeT_) {
    specs.name_ = "T";
    specs.stepsize_ = fTStep_;
    specs.minval_ = fTMin_;
    specs.maxval_ = fTMax_;
    parspecs_.push_back(specs);
  }
  par_.resize(parspecs_.size(),NAN);
}

/// compute event hypothesis from minimizer parameters
void I3CurvatureParametrization::UpdatePhysicsVariables(){
    log_trace("Entering UpdatePhysicsVariables");
    I3ParticlePtr track = hypothesis_->particle;
    I3LaputopParamsPtr paramsPtr =
        boost::dynamic_pointer_cast<I3LaputopParams>(hypothesis_->nonstd);
    assert(paramsPtr);

    // This is the only thing that's going to change from iteration to iteration:
    if (fFreeA_) paramsPtr->SetValue(Laputop::Parameter::CurvParabA, par_[iparA_]);
    if (fFreeD_) paramsPtr->SetValue(Laputop::Parameter::CurvGaussD, par_[iparD_]);
    if (fFreeN_) paramsPtr->SetValue(Laputop::Parameter::CurvGaussN, par_[iparN_]);
    if (fFreeT_) track->SetTime(par_[iparT_]);

		 

}


void I3CurvatureParametrization::PassCovariance(const boost::numeric::ublas::symmetric_matrix<double>& cov) {
    log_trace("Entering PassCovariance");
    I3ParticlePtr track = hypothesis_->particle;
    I3LaputopParamsPtr paramsPtr =
        boost::dynamic_pointer_cast<I3LaputopParams>(hypothesis_->nonstd);
    assert(paramsPtr);

    // This "structure" or ordering of the errors is done identically as above in UpdatePhysicsVariables
    // If you change that one, you gotta change this one accordingly!
    std::vector<Laputop::Parameter::Enum> pars;

    if (fFreeA_) pars.push_back(Laputop::Parameter::CurvParabA);
    if (fFreeD_) pars.push_back(Laputop::Parameter::CurvGaussD);
    if (fFreeN_) pars.push_back(Laputop::Parameter::CurvGaussN);

    /*  Maybe add these too, later?
    // Optional: core
    if (!fFixCore_) {
      pars.push_back(Laputop::Parameter::Xc);
      pars.push_back(Laputop::Parameter::Yc);
    }

    // optional: track dir and time
    if (!fFixDir_) {
      pars.push_back(Laputop::Parameter::Nx);
      pars.push_back(Laputop::Parameter::Ny);
      pars.push_back(Laputop::Parameter::Tc);
    }
    */

    // now copy the covariance matrix
    for (unsigned i = 0; i < pars.size(); ++i)
      for (unsigned j = i; j < pars.size(); ++j)
        paramsPtr->SetCovariance(pars[i], pars[j], cov(i, j));
}


/// compute minimizer parameters from event hypothesis
void I3CurvatureParametrization::UpdateParameters(){
    log_debug("Entering UpdateParameters");
    const I3ParticlePtr track = hypothesis_->particle;
    //const I3Position& pos1 = track->GetPos();
    //const I3Direction& dir1 = track->GetDir();
    I3LaputopParamsPtr paramsPtr =
      boost::dynamic_pointer_cast<I3LaputopParams>(hypothesis_->nonstd);
    if (!paramsPtr){
        log_fatal( "(%s) incomplete/wrong event hypothesis; wrong seed service?",
                   GetName().c_str() );
    }

    if (fFreeA_) par_[iparA_] = paramsPtr->GetValue(Laputop::Parameter::CurvParabA);
    if (fFreeD_) par_[iparD_] = paramsPtr->GetValue(Laputop::Parameter::CurvGaussD);
    if (fFreeN_) par_[iparN_] = paramsPtr->GetValue(Laputop::Parameter::CurvGaussN);
    if (fFreeT_) par_[iparT_] = track->GetTime();

    for (unsigned int ii=0; ii<par_.size(); ii++) 
      log_debug( "(%s) setting var: p%u=%g", GetName().c_str(), ii, par_[ii] );

}

typedef
I3SingleServiceFactory<I3CurvatureParametrization,I3ParametrizationBase>
I3CurvatureParametrizationServiceFactory;
I3_SERVICE_FACTORY( I3CurvatureParametrizationServiceFactory );
