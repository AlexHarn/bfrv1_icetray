/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhUPandelParser.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include <icetray/I3Configuration.h>

#include "cscd-llh/I3CscdLlhFitter.h"
#include "cscd-llh/parser/I3CscdLlhUPandelParser.h"
#include "cscd-llh/pdf/I3CscdLlhUPandel.h"

#include "dataclasses/I3Constants.h"


/* ****************************************************** */
/* AddParameters                                          */
/* ****************************************************** */
void I3CscdLlhUPandelParser::AddParameters() 
{
  log_debug("Enter AddParameters().");

  optPandelSmallProb_ = I3CscdLlhUPandel::DEFAULT_SMALL_PROB;
  AddParameter("PandelSmallProb",
    "The minimum probability (a small positive definite number.)",
    optPandelSmallProb_);
    
  optPandelTau_ = I3CscdLlhUPandel::DEFAULT_TAU;
  AddParameter("PandelTau", 
    "Pandel time parameter.",
    optPandelTau_);
    
  optPandelLambda_ = I3CscdLlhUPandel::DEFAULT_LAMBDA;
  AddParameter("PandelLambda", 
    "Pandel length parameter.",
    optPandelLambda_);
    
  optPandelLambdaA_ = I3CscdLlhUPandel::DEFAULT_LAMBDA_A;
  AddParameter("PandelLambdaA", 
    "Pandel absorption length.",
    optPandelLambdaA_);
    
  optPandelSigma_ = I3CscdLlhUPandel::DEFAULT_SIGMA;
  AddParameter("PandelSigma", 
    "Patched Pandel gaussian width.",
    optPandelSigma_);
  
  optPandelLightSpeed_ = I3Constants::c / I3Constants::n_ice;
  AddParameter("PandelLightSpeed", 
    "The speed of light in ice.",
    optPandelLightSpeed_);

  optPandelMaxDist_ = I3CscdLlhUPandel::DEFAULT_MAX_DIST;
  AddParameter("PandelMaxDist", 
    "If the distance from the hit to the cascade vertex"
    " is greater than \"PandelMaxDist\",\n"
    "    the probability will be Set to \"PandelSmallProb\".\n"
    "    If PandelMaxDist is less than or equal to zero,"
    " it will be ignored.",
    optPandelMaxDist_);
  
  log_debug("Exit AddParameters().");
  return;
} // end AddParameters

/* ****************************************************** */
/* Configure                                              */
/* ****************************************************** */
bool I3CscdLlhUPandelParser::Configure() 
{
  log_debug("Enter Configure().");

  GetParameter("PandelSmallProb", optPandelSmallProb_);
  GetParameter("PandelTau", optPandelTau_);
  GetParameter("PandelLambda", optPandelLambda_);
  GetParameter("PandelLambdaA", optPandelLambdaA_);
  GetParameter("PandelSigma", optPandelSigma_);
  GetParameter("PandelLightSpeed", optPandelLightSpeed_);
  GetParameter("PandelMaxDist", optPandelMaxDist_);

  log_info("PandelSmallProb = %6.2e", optPandelSmallProb_);
  log_info("PandelTau = %f", optPandelTau_);
  log_info("PandelLambda = %f", optPandelLambda_);
  log_info("PandelLambdaA = %f", optPandelLambdaA_);
  log_info("PandelSigma = %f", optPandelSigma_);
  log_info("PandelLightSpeed = %f", optPandelLightSpeed_);
  log_info("PandelMaxDist = %f", optPandelMaxDist_);

  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhUPandel::CONST_SMALL_PROB,
      optPandelSmallProb_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhUPandel::CONST_TAU,
      optPandelTau_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhUPandel::CONST_LAMBDA,
      optPandelLambda_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhUPandel::CONST_LAMBDA_A,
      optPandelLambdaA_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhUPandel::CONST_SIGMA,
      optPandelSigma_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhUPandel::CONST_C_ICE,
      optPandelLightSpeed_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhUPandel::CONST_MAX_DIST,
      optPandelMaxDist_)) 
  {
    return false;
  }

  log_debug("Exit Configure().");
  return true;
} // end Configure
