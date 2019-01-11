/**
 * copyright (c) 2006
 * the IceCube collaboration
 * $Id$
 *
 * @file I3CscdLlhHnhDirParser.cxx
 * @version
 * @author Doug Rutledge
 * @date 08Feb2006
 */

#include "cscd-llh/parser/I3CscdLlhHnhDirParser.h"
#include "cscd-llh/pdf/I3CscdLlhHnhDir.h"
#include "cscd-llh/I3CscdLlhFitter.h"

using namespace std;

void I3CscdLlhHnhDirParser :: AddParameters()
{
  log_debug("Entering HnhDirParser :: AddParameters");

  optLegendrePolyCoeff0_ = I3CscdLlhHnhDir :: DEFAULT_LEGENDRE_COEFF_0;
  AddParameter("Legendre Coefficient 0",
    "The zeroth coefficient for the legendre polynomial",
    optLegendrePolyCoeff0_);

  optLegendrePolyCoeff1_ = I3CscdLlhHnhDir :: DEFAULT_LEGENDRE_COEFF_1;
  AddParameter("Legendre Coefficient 1",
    "The first coefficient for the legendre polynomial",
    optLegendrePolyCoeff1_);

  optLegendrePolyCoeff2_ = I3CscdLlhHnhDir :: DEFAULT_LEGENDRE_COEFF_2;
  AddParameter("Legendre Coefficient 2",
    "The second coefficient for the legendre polynomial",
    optLegendrePolyCoeff2_);

}

bool I3CscdLlhHnhDirParser :: Configure()
{
  log_debug("Entering HnhDirParser :: Configure");

  GetParameter("Legendre Coefficient 0", optLegendrePolyCoeff0_);
  GetParameter("Legendre Coefficient 1", optLegendrePolyCoeff1_);
  GetParameter("Legendre Coefficient 2", optLegendrePolyCoeff2_);

  // Hit/No-hit
  GetParameter("HitNoHitNorm", optHitNoHitNorm_);
  GetParameter("HitNoHitLambdaAttn", optHitNoHitLambdaAttn_);
  GetParameter("HitNoHitNoise", optHitNoHitNoise_);
  GetParameter("HitNoHitDistCutoff", optHitNoHitDistCutoff_);
  GetParameter("HitNoHitDead", optHitNoHitDead_);
  GetParameter("HitNoHitSmallProb", optHitNoHitSmallProb_);

  log_info("Legendre Coefficient 0: %f", optLegendrePolyCoeff0_);
  log_info("Legendre Coefficient 1: %f", optLegendrePolyCoeff1_);
  log_info("Legendre Coefficient 2: %f", optLegendrePolyCoeff2_);

  if(!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir :: CONST_L_POLY_0,
      optLegendrePolyCoeff0_))
  {
    return false;
  }

  if(!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir :: CONST_L_POLY_1,
      optLegendrePolyCoeff1_))
  {
    return false;
  }

  if(!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir :: CONST_L_POLY_2,
      optLegendrePolyCoeff2_))
  {
    return false;
  }

  //Hit-no-hit
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir::CONST_HNH_NORM,
      optHitNoHitNorm_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir::CONST_HNH_LAMBDA_ATTN,
      optHitNoHitLambdaAttn_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir::CONST_HNH_NOISE,
      optHitNoHitNoise_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir::CONST_HNH_DIST_CUTOFF,
      optHitNoHitDistCutoff_))
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir::CONST_HNH_DEAD,
      optHitNoHitDead_)) 
  {
    return false;
  }
  if (!fitter_->
    SetPdfConstant(
      I3CscdLlhHnhDir::CONST_HNH_SMALL_PROB,
      optHitNoHitSmallProb_)) 
  {
    return false;
  }

  return true;
}
