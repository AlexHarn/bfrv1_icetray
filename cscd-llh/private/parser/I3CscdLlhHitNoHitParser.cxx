/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhHitNoHitParser.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include "cscd-llh/I3CscdLlhFitter.h"
#include "cscd-llh/parser/I3CscdLlhHitNoHitParser.h"
#include "cscd-llh/pdf/I3CscdLlhHitNoHit.h"

#include "dataclasses/I3Constants.h"

/* ****************************************************** */
/* AddParameters                                          */
/* ****************************************************** */
void I3CscdLlhHitNoHitParser::AddParameters() 
{
  log_debug("Enter AddParameters().");

  optHitNoHitNorm_ = I3CscdLlhHitNoHit::DEFAULT_NORM;
  AddParameter("HitNoHitNorm",
    "The normalization factor for the expected number of photoelectrons. "
    "This is 1.4 (m/GeV) for most reconstructions, but needs to be changed to "
    "40/1000 when reconstructing the direction.",
    optHitNoHitNorm_);
  
  optHitNoHitLambdaAttn_ = I3CscdLlhHitNoHit::DEFAULT_LAMBDA_ATTN;
  AddParameter("HitNoHitLambdaAttn",
    "The attenuation length.",
    optHitNoHitLambdaAttn_);
  
  optHitNoHitNoise_ = I3CscdLlhHitNoHit::DEFAULT_NOISE;
  AddParameter("HitNoHitNoise",
    "The probability that an OM will register a hit due to noise.",
    optHitNoHitNoise_);
  
  optHitNoHitDistCutoff_ = I3CscdLlhHitNoHit::DEFAULT_DIST_CUTOFF;
  AddParameter("HitNoHitDistCutoff",
    "Replace the geometrical factor 1/d\n"
    "with 1/(d + d_c) so that it's well-behaved when d is small.",
    optHitNoHitDistCutoff_);
  
  optHitNoHitDead_ = I3CscdLlhHitNoHit::DEFAULT_DEAD;
  AddParameter("HitNoHitDead",
    "The probability that an OM will not register a hit,\n"
    "regardless of the light intensity.",
    optHitNoHitDead_);
  
  optHitNoHitSmallProb_ = I3CscdLlhHitNoHit::DEFAULT_SMALL_PROB;
  AddParameter("HitNoHitSmallProb",
    "The minimum probability (a small positive definite number.)",
    optHitNoHitSmallProb_);

  log_debug("Exit AddParameters().");
  return;
} // end AddParameters

/* ****************************************************** */
/* Configure                                              */
/* ****************************************************** */
bool I3CscdLlhHitNoHitParser::Configure() {
  log_debug("Enter Configure().");

  GetParameter("HitNoHitNorm", optHitNoHitNorm_);
  GetParameter("HitNoHitLambdaAttn", optHitNoHitLambdaAttn_);
  GetParameter("HitNoHitNoise", optHitNoHitNoise_);
  GetParameter("HitNoHitDistCutoff", optHitNoHitDistCutoff_);
  GetParameter("HitNoHitDead", optHitNoHitDead_);
  GetParameter("HitNoHitSmallProb", optHitNoHitSmallProb_);

  log_info("HitNoHitNorm = %f", optHitNoHitNorm_);
  log_info("HitNoHitLambdaAttn = %f", optHitNoHitLambdaAttn_);
  log_info("HitNoHitNoise = %.2e", optHitNoHitNoise_);
  log_info("HitNoHitDistCutoff = %f", optHitNoHitDistCutoff_);
  log_info("HitNoHitDead = %f", optHitNoHitDead_);
  log_info("HitNoHitSmallProb = %.2e", optHitNoHitSmallProb_);

  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHitNoHit::CONST_NORM,
      optHitNoHitNorm_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHitNoHit::CONST_LAMBDA_ATTN,
      optHitNoHitLambdaAttn_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHitNoHit::CONST_NOISE,
      optHitNoHitNoise_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHitNoHit::CONST_DIST_CUTOFF,
      optHitNoHitDistCutoff_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHitNoHit::CONST_DEAD,
      optHitNoHitDead_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHitNoHit::CONST_SMALL_PROB,
      optHitNoHitSmallProb_)) 
  {
    return false;
  }

  log_debug("Exit Configure().");
  return true;
} // end Configure
