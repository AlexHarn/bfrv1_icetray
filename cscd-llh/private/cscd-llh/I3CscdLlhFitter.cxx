/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhFitter.cxx
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 * @author Doug Rutledge 
 */

#include <icetray/I3Configuration.h>

#include "cscd-llh/I3CscdLlhFitter.h"

#include "cscd-llh/minimizer/I3CscdLlhBrent.h"
#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/minimizer/I3CscdLlhPowell.h"
#include "cscd-llh/minimizer/I3CscdLlhSimplex.h"

#include "cscd-llh/pdf/I3CscdLlhHitNoHitMpe.h"
#include "cscd-llh/pdf/I3CscdLlhPndlHnh.h"
#include "cscd-llh/pdf/I3CscdLlhUPandelMpe.h"
#include "cscd-llh/pdf/I3CscdLlhHnhDir.h"

#include <time.h>
#include <iostream>
#include <string>


const int I3CscdLlhFitter::MINIMIZER_NULL     = 0;
const int I3CscdLlhFitter::MINIMIZER_BRENT    = 1;
const int I3CscdLlhFitter::MINIMIZER_POWELL   = 3;
const int I3CscdLlhFitter::MINIMIZER_SIMPLEX  = 4;
const int I3CscdLlhFitter::MINIMIZER_NAG      = 5;

const int I3CscdLlhFitter::PDF_NULL           = 0;
const int I3CscdLlhFitter::PDF_UPANDEL        = 1;
const int I3CscdLlhFitter::PDF_UPANDEL_MPE    = 2;
const int I3CscdLlhFitter::PDF_HIT_NO_HIT     = 3;
const int I3CscdLlhFitter::PDF_HIT_NO_HIT_MPE = 4;
const int I3CscdLlhFitter::PDF_PNDL_HNH       = 5;
const int I3CscdLlhFitter::PDF_HNH_DIR        = 6;

/* ****************************************************** */
/* I3CscdLlhFitter constructor                            */
/* ****************************************************** */
I3CscdLlhFitter::I3CscdLlhFitter() 
{ 
  log_debug("Enter I3CscdLlhFitter::I3CscdLlhFitter()");

  resultingCascade_ = I3ParticlePtr(new I3Particle());
  if (!resultingCascade_) 
  {
    log_error("Unable to create I3Particle!");
  }

  fitParams_ = I3CscdLlhFitParamsPtr(new I3CscdLlhFitParams());
  if(!fitParams_)
  {
    log_error("Unable to create I3CscdLlhFitParams!");
  }	  

  log_debug("Exit I3CscdLlhFitter::I3CscdLlhFitter()");
}

/* ****************************************************** */
/* Slear: Remove all hits and clear variables.            */
/* ****************************************************** */               
void I3CscdLlhFitter::Clear() 
{
  log_trace("Enter I3CscdLlhFitter::Clear()");

  hits_.clear();
  fitParams_->Clear();

  log_trace("Exit I3CscdLlhFitter::Clear()");
  return;
} // end clear

/* ****************************************************** */
/* SetMinimizer                                           */
/* ****************************************************** */               
bool I3CscdLlhFitter::SetMinimizer(int code) 
{
  log_debug("Enter I3CscdLlhFitter::SetMinimizer(code = %d)", code);

  switch (code) 
  {

    case MINIMIZER_BRENT:
      minimizer_ = I3CscdLlhBrentPtr(new I3CscdLlhBrent());
      break;

    case MINIMIZER_POWELL:
      minimizer_ = I3CscdLlhPowellPtr(new I3CscdLlhPowell());
      break;

    case MINIMIZER_SIMPLEX:
      minimizer_ = I3CscdLlhSimplexPtr(new I3CscdLlhSimplex());
      break;

    case MINIMIZER_NAG:
      log_error("NAG minimizers not yet supported");
    default:
      log_error("Unable to create Minimizer -- invalid code: %d.", code);
      return false;  
  } // switch

  if (!minimizer_) 
  {
    log_fatal("Unable to create Minimizer.");
    return false;
  }

  return true;
} // end SetMinimizer

/* ****************************************************** */
/* SetPdf                                                 */
/* ****************************************************** */               
bool I3CscdLlhFitter::SetPdf(int code) 
{
  log_debug("Enter I3CscdLlhFitter::SetPdf(code = %d)", code);
    
  switch (code) 
  {
    case PDF_UPANDEL:
      pdf_ = I3CscdLlhUPandelPtr(new I3CscdLlhUPandel());
      break;

    case PDF_UPANDEL_MPE:
      pdf_ = I3CscdLlhUPandelMpePtr(new I3CscdLlhUPandelMpe());
      break;

    case PDF_HIT_NO_HIT:
      pdf_ = I3CscdLlhHitNoHitPtr(new I3CscdLlhHitNoHit());
      break;

    case PDF_HIT_NO_HIT_MPE:
      pdf_ = I3CscdLlhHitNoHitMpePtr(new I3CscdLlhHitNoHitMpe());
      break;
  
    case PDF_PNDL_HNH:
      pdf_ = I3CscdLlhPndlHnhPtr(new I3CscdLlhPndlHnh());
      break;

    case PDF_HNH_DIR:
      pdf_ = I3CscdLlhHnhDirPtr(new I3CscdLlhHnhDir());
      break;

    default:
      log_fatal("Unable to create PDF -- invalid code: %d.", code);
      return false;  
  } // switch

  if (!pdf_) 
  {
    log_error("Unable to create PDF.");
    return false;  
  }

  int numParams = pdf_->GetNumParams(); 
  {
    if (numParams > I3CscdLlhMinimizer::MAX_MINIMIZATION_PARAMS)
      log_debug("PDF has too many paramters: %d > %d",
        numParams, I3CscdLlhMinimizer::MAX_MINIMIZATION_PARAMS);
  }

  return true;
} // end SetPdf

/* ****************************************************** */
/* Configure                                              */
/* ****************************************************** */               
bool I3CscdLlhFitter::Configure(I3CscdLlhAbsParserPtr parser) 
{
  log_debug("Enter I3CscdLlhFitter::Configure()");
    
  parser->SetFitter(this);
  if (!parser->Configure()) 
  {
    return false;
  }

  log_debug("Exit I3CscdLlhFitter::Configure()");
  return true;
} // end Configure

/* ****************************************************** */
/* SetMaxCalls                                            */
/* ****************************************************** */               
void I3CscdLlhFitter::SetMaxCalls(int maxCalls) 
{
  minimizer_->SetMaxCalls(maxCalls);
}

/* ****************************************************** */
/* SetTolerance                                           */
/* ****************************************************** */               
void I3CscdLlhFitter::SetTolerance(double tolerance) 
{
  minimizer_->SetTolerance(tolerance);
}

/* ****************************************************** */
/* InitParam: Initialize a parameter.                     */
/* ****************************************************** */               
bool I3CscdLlhFitter::InitParam(std::string paramName, double stepSize,
  double lowerLimit, double upperLimit, bool fix) 
{
  log_trace("Enter I3CscdLlhFitter::InitParam()");

  int idx = pdf_->GetParamIndex(paramName);
  if (idx == INT_MIN) 
  {
    log_error("Unable to initialize non-fixed parameter: %s", 
      paramName.c_str());
    return false;
  }

  if((paramName == "energy") && (minimizeInLogE_))
  {
    double logStepSize = 0.0;
    double logUpperLimit = 0.0;
    double logLowerLimit = 0.0;

    //guard against taking log(0)
    if (stepSize != 0.0) 
      logStepSize = log(stepSize);
    else 
      logStepSize = 0.0000001;
    if (lowerLimit != 0.0) 
      logLowerLimit = log(lowerLimit);
    else
      logLowerLimit = 0.0;
    if (upperLimit != 0.0) 
      logUpperLimit = log(upperLimit); 
    else
      logUpperLimit = 0.0;

    minimizer_->InitParam(idx, logStepSize, logLowerLimit, logUpperLimit, fix);
  }
  else
    minimizer_->InitParam(idx, stepSize, lowerLimit, upperLimit, fix);

  log_debug("Initialized parameter: %s %d", paramName.c_str(),fix);
  log_trace("Exit I3CscdLlhFitter::InitParam()");
  return true;
} // end InitParam

/* ****************************************************** */
/* SetSeed: Set the initial value for a parameter.        */
/* ****************************************************** */               
bool I3CscdLlhFitter::SetSeed(std::string paramName, double seed) 
{
  log_debug("Enter I3CscdLlhFitter::SetSeed()");

  int idx = pdf_->GetParamIndex(paramName);
  if (idx == INT_MIN) 
  {
    log_fatal("Unable to set seed for parameter: %s", paramName.c_str());

    return true;
  }

  if (!minimizer_->SetSeed(idx, paramName, seed)) 
  {
    log_error("Unable to Set %s seed.", paramName.c_str());

    return false;
  }

  log_debug("Seed %s = %f.", paramName.c_str(), seed);

  log_debug("Exit I3CscdLlhFitter::SetSeed()");
  return true;
} // end SetSeed

/* ****************************************************** */
/* AddHit: Add an OM hit that contributes to the cog.     */
/* Return false if hit is not valid.                      */
/* ****************************************************** */               
bool I3CscdLlhFitter::AddHit(I3CscdLlhHitPtr& hit) 
{

  if(!hit)
  {
    return false;
    // this will cause a log_fatal in I3CscdLlhModule.
  }
  hits_.push_back(hit);

  return true;
} // end AddHit

/* ****************************************************** */
/* Fit: Perform the actual fitting algorithm.             */
/* ****************************************************** */               
bool I3CscdLlhFitter::Fit() 
{
  log_debug("Enter I3CscdLlhFitter::Fit()");

  time_t fitSec1 = time(NULL);
  log_debug("Number Of hits given to minimizer: %d",(int)hits_.size());
  int fitStartTime = static_cast<int>(fitSec1);
  log_debug("Fit Start Time: %i", fitStartTime);
  if (!minimizer_->Minimize(&hits_, pdf_)) 
  {
    return false;
  }

  time_t fitSec2 = time(NULL);

  int fitEndTime = static_cast<int>(fitSec2);
  log_debug("Fit End Time: %i", fitEndTime);
  int fitSeconds = static_cast<int>(fitSec2 - fitSec1);
  log_debug("Fit time: %d sec.", fitSeconds);

  log_debug("Exit I3CscdLlhFitter::Fit()");
  return true;   
} // end Fit
  
/* ****************************************************** */
/* GetCascade: Get the results of the fit.                */
/* ****************************************************** */               

I3ParticlePtr I3CscdLlhFitter::GetCascade()
{
  log_debug("Enter I3CscdLlhFitter::GetCascade()");

  I3CscdLlhResultPtr cscdLlhResult = minimizer_->GetResult();
  
  if (!std::isnan(cscdLlhResult->t))
    resultingCascade_->SetTime(cscdLlhResult->t);
  
  if ((!std::isnan(cscdLlhResult->x)) && (!std::isnan(cscdLlhResult->y)) && 
    (!std::isnan(cscdLlhResult->z)))
    resultingCascade_->SetPos(cscdLlhResult->x, 
      cscdLlhResult->y, cscdLlhResult->z);
  
  if (!std::isnan(cscdLlhResult->theta) && (!std::isnan(cscdLlhResult->phi)))
    resultingCascade_->SetDir(cscdLlhResult->theta,cscdLlhResult->phi);
  
  if (!minimizeInLogE_ && !std::isnan(cscdLlhResult->energy))
  {
    resultingCascade_->SetEnergy(cscdLlhResult->energy);
  }
  else if( minimizeInLogE_ && !std::isnan(cscdLlhResult->energy)) 
  {
    resultingCascade_->SetEnergy(exp(cscdLlhResult->energy));
  }

  resultingCascade_->SetShape(I3Particle::Cascade);
  resultingCascade_->SetType(I3Particle::EMinus);
  
  log_debug("Result (t, x, y, z, theta, phi, energy) =\n"
    "   (%f, %f, %f, %f, %f, %f, %f)",
    resultingCascade_->GetTime(),
    resultingCascade_->GetPos().GetX(),
    resultingCascade_->GetPos().GetY(),
    resultingCascade_->GetPos().GetZ(),
    resultingCascade_->GetZenith(),
    resultingCascade_->GetAzimuth(),
    resultingCascade_->GetEnergy());

  log_debug("Exit I3CscdLlhFitter::GetCascade()");
  return resultingCascade_;
} // end GetCascade

I3CscdLlhFitParamsPtr I3CscdLlhFitter :: GetFitParams()
{
  log_debug("Enter I3CscdLlhFitter::GetFitParams()");

  I3CscdLlhResultPtr cscdLlhResult = minimizer_->GetResult();
  
  fitParams_->SetStatus(cscdLlhResult->status);

  fitParams_->SetErrT(cscdLlhResult->errT);
  fitParams_->SetErrX(cscdLlhResult->errX);
  fitParams_->SetErrY(cscdLlhResult->errY);
  fitParams_->SetErrZ(cscdLlhResult->errZ);
  fitParams_->SetErrTheta(cscdLlhResult->errTheta);
  fitParams_->SetErrPhi(cscdLlhResult->errPhi);
  fitParams_->SetErrEnergy(cscdLlhResult->errEnergy);
  
  double negLlh = cscdLlhResult->negLlh;
  int nHits = hits_.size();
  int unhitOmCount = fitParams_->GetUnhitOmCount();
  int freeParams = minimizer_->GetNumFreeParams();

  double unHitContribution = pdf_->GetUsesNoHits() ? unhitOmCount : 0.0;

  fitParams_->SetHitCount(nHits);
  fitParams_->SetNegLlh(negLlh);
  fitParams_->SetReducedLlh(negLlh/(nHits + unHitContribution - freeParams));
  
  log_debug("Exit I3CscdLlhFitter::GetFitParams()");
  return fitParams_;
}//end GetFitParams

/* ****************************************************** */
/* InitializeResult: initialize the results of this fit.  */
/* ****************************************************** */               
void I3CscdLlhFitter :: InitializeResult(const I3Particle& priorRecoResult)
{
  resultingCascade_->SetTime(priorRecoResult.GetTime());
  resultingCascade_->SetPos(priorRecoResult.GetPos());
  resultingCascade_->SetDir(priorRecoResult.GetDir());
  resultingCascade_->SetEnergy(priorRecoResult.GetEnergy());

  resultingCascade_->SetType(priorRecoResult.GetType());
  resultingCascade_->SetShape(priorRecoResult.GetShape());
}

void I3CscdLlhFitter :: MinimizeInLogE()
{
  minimizeInLogE_ = true;

  pdf_->SetMinimizeInLogE(true);
}//end MinimizeInLogE
