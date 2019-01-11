/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhPndlHnhParser.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include "cscd-llh/I3CscdLlhFitter.h"
#include "cscd-llh/parser/I3CscdLlhPndlHnhParser.h"
#include "cscd-llh/pdf/I3CscdLlhPndlHnh.h"

#include "dataclasses/I3Constants.h"


/* ****************************************************** */
/* AddParameters                                          */
/* ****************************************************** */
void I3CscdLlhPndlHnhParser::AddParameters() 
{
  log_debug("Enter AddParameters().");

  optPndlHnhWeight_ = I3CscdLlhPndlHnh::DEFAULT_WEIGHT;
  AddParameter("PndlHnhWeight",
    "The weight of the Hit/No-hit factor of the PndlHnh PDF.\n"
    "      The weight of the UPandel factor is always 1.0.",
    optPndlHnhWeight_);
    
  log_debug("Exit AddParameters().");
  return;
} // end AddParameters

/* ****************************************************** */
/* Configure                                              */
/* ****************************************************** */
bool I3CscdLlhPndlHnhParser::Configure() 
{
  log_debug("Enter Configure().");

  // PndlHnh
  GetParameter("PndlHnhWeight", optPndlHnhWeight_);

  // UPandel
  GetParameter("PandelSmallProb", optPandelSmallProb_);
  GetParameter("PandelTau", optPandelTau_);
  GetParameter("PandelLambda", optPandelLambda_);
  GetParameter("PandelLambdaA", optPandelLambdaA_);
  GetParameter("PandelSigma", optPandelSigma_);
  GetParameter("PandelLightSpeed", optPandelLightSpeed_);
  GetParameter("PandelMaxDist", optPandelMaxDist_);

  // Hit/No-hit
  GetParameter("HitNoHitNorm", optHitNoHitNorm_);
  GetParameter("HitNoHitLambdaAttn", optHitNoHitLambdaAttn_);
  GetParameter("HitNoHitNoise", optHitNoHitNoise_);
  GetParameter("HitNoHitDistCutoff", optHitNoHitDistCutoff_);
  GetParameter("HitNoHitDead", optHitNoHitDead_);
  GetParameter("HitNoHitSmallProb", optHitNoHitSmallProb_);

  // PndlHnh
  log_info("PndlHnhWeight = %f", optPndlHnhWeight_);

  // UPandel
  log_info("PandelSmallProb = %6.2e", optPandelSmallProb_);
  log_info("PandelTau = %f", optPandelTau_);
  log_info("PandelLambda = %f", optPandelLambda_);
  log_info("PandelLambdaA = %f", optPandelLambdaA_);
  log_info("PandelSigma = %f", optPandelSigma_);
  log_info("PandelLightSpeed = %f", optPandelLightSpeed_);
  log_info("PandelMaxDist = %f", optPandelMaxDist_);

  // Hit/No-hit
  log_info("HitNoHitNorm = %f", optHitNoHitNorm_);
  log_info("HitNoHitLambdaAttn = %f", optHitNoHitLambdaAttn_);
  log_info("HitNoHitNoise = %.2e", optHitNoHitNoise_);
  log_info("HitNoHitDistCutoff = %f", optHitNoHitDistCutoff_);
  log_info("HitNoHitDead = %f", optHitNoHitDead_);
  log_info("HitNoHitSmallProb = %.2e", optHitNoHitSmallProb_);

  // PndlHnh
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_WEIGHT,
      optPndlHnhWeight_)) 
  {
    return false;
  }

  // UPandel
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_PNDL_SMALL_PROB,
      optPandelSmallProb_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_PNDL_TAU,
      optPandelTau_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_PNDL_LAMBDA,
      optPandelLambda_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
    I3CscdLlhPndlHnh::CONST_PNDL_LAMBDA_A,
    optPandelLambdaA_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_PNDL_SIGMA,
      optPandelSigma_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_PNDL_C_ICE,
      optPandelLightSpeed_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_PNDL_MAX_DIST,
      optPandelMaxDist_)) 
  {
    return false;
  }

  // Hit/No-hit
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_HNH_NORM,
      optHitNoHitNorm_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_HNH_LAMBDA_ATTN,
      optHitNoHitLambdaAttn_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_HNH_NOISE,
      optHitNoHitNoise_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_HNH_DIST_CUTOFF,
      optHitNoHitDistCutoff_))
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_HNH_DEAD,
      optHitNoHitDead_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhPndlHnh::CONST_HNH_SMALL_PROB,
      optHitNoHitSmallProb_)) 
  {
    return false;
  }

  log_debug("Exit Configure().");
  return true;
} // end Configure
