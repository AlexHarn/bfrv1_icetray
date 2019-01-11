/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhPndlHnh.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include <iostream>
#include <math.h>

using namespace std;

#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/pdf/I3CscdLlhPndlHnh.h"
#include <climits>

const int I3CscdLlhPndlHnh::MINIMIZATION_PARAMS        = 5;
const int I3CscdLlhPndlHnh::PARAM_INDEX_T              = 0;
const int I3CscdLlhPndlHnh::PARAM_INDEX_X              = 1;
const int I3CscdLlhPndlHnh::PARAM_INDEX_Y              = 2;
const int I3CscdLlhPndlHnh::PARAM_INDEX_Z              = 3;
const int I3CscdLlhPndlHnh::PARAM_INDEX_ENERGY         = 4;

const int I3CscdLlhPndlHnh::CONST_NULL                 = 0;
const int I3CscdLlhPndlHnh::CONST_WEIGHT               = 1;

// UPandel constants
const int I3CscdLlhPndlHnh::CONST_PNDL_C_ICE           = 2;
const int I3CscdLlhPndlHnh::CONST_PNDL_SMALL_PROB      = 3;
const int I3CscdLlhPndlHnh::CONST_PNDL_TAU             = 4;
const int I3CscdLlhPndlHnh::CONST_PNDL_LAMBDA          = 5;
const int I3CscdLlhPndlHnh::CONST_PNDL_LAMBDA_A        = 6;
const int I3CscdLlhPndlHnh::CONST_PNDL_SIGMA           = 7;
const int I3CscdLlhPndlHnh::CONST_PNDL_MAX_DIST        = 8;

// Hit/No-hit constants
const int I3CscdLlhPndlHnh::CONST_HNH_NORM             = 9;
const int I3CscdLlhPndlHnh::CONST_HNH_LAMBDA_ATTN      = 10;
const int I3CscdLlhPndlHnh::CONST_HNH_NOISE            = 11;
const int I3CscdLlhPndlHnh::CONST_HNH_DIST_CUTOFF      = 12;
const int I3CscdLlhPndlHnh::CONST_HNH_DEAD             = 13;
const int I3CscdLlhPndlHnh::CONST_HNH_SMALL_PROB       = 14;

const double I3CscdLlhPndlHnh::DEFAULT_WEIGHT          = 1.0;

I3CscdLlhPndlHnh::I3CscdLlhPndlHnh() : I3CscdLlhAbsPdf() 
{
  weight_ = DEFAULT_WEIGHT;

  pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_C_ICE, 
    I3CscdLlhUPandel::DEFAULT_C_ICE);
  pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_SMALL_PROB, 
    I3CscdLlhUPandel::DEFAULT_SMALL_PROB);
  pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_TAU, 
    I3CscdLlhUPandel::DEFAULT_TAU);
  pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_LAMBDA, 
    I3CscdLlhUPandel::DEFAULT_LAMBDA);
  pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_LAMBDA_A, 
    I3CscdLlhUPandel::DEFAULT_LAMBDA_A);
  pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_SIGMA, 
    I3CscdLlhUPandel::DEFAULT_SIGMA);
  // MAX_DIST shouldn't be necessary if the Hit/No-hit is working.
  pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_MAX_DIST, 0.0);

  pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_NORM, 
    I3CscdLlhHitNoHit::DEFAULT_NORM);
  pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_LAMBDA_ATTN, 
    I3CscdLlhHitNoHit::DEFAULT_LAMBDA_ATTN);
  pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_NOISE, 
    I3CscdLlhHitNoHit::DEFAULT_NOISE);
  pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_DIST_CUTOFF, 
    I3CscdLlhHitNoHit::DEFAULT_DIST_CUTOFF);
  pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_DEAD, 
    I3CscdLlhHitNoHit::DEFAULT_DEAD);
  pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_SMALL_PROB, 
    I3CscdLlhHitNoHit::DEFAULT_SMALL_PROB);

  //DO NOT CHANGE THIS!!!
  useNoHits_ = true;
}

/* ****************************************************** */
/* Evaluate                                               */
/* ****************************************************** */
void I3CscdLlhPndlHnh::Evaluate(const I3CscdLlhHitPtr& hit,
  const double* param, double& value) const 
{
  value = NAN;

  double paramPndl[pdfPndl_.GetNumParams()];
  paramPndl[pdfPndl_.GetParamIndex("t")] = param[PARAM_INDEX_T];
  paramPndl[pdfPndl_.GetParamIndex("x")] = param[PARAM_INDEX_X];
  paramPndl[pdfPndl_.GetParamIndex("y")] = param[PARAM_INDEX_Y];
  paramPndl[pdfPndl_.GetParamIndex("z")] = param[PARAM_INDEX_Z];

  double paramHnh[pdfHnh_.GetNumParams()];
  paramHnh[pdfHnh_.GetParamIndex("x")] = param[PARAM_INDEX_X];
  paramHnh[pdfHnh_.GetParamIndex("y")] = param[PARAM_INDEX_Y];
  paramHnh[pdfHnh_.GetParamIndex("z")] = param[PARAM_INDEX_Z];
  paramHnh[pdfHnh_.GetParamIndex("energy")] = param[PARAM_INDEX_ENERGY];

  log_debug("Hit #%d of %d.", hit->hitNumber, hit->omHitCount);

  double probPndl = 1.0;
  if (hit->omHitCount != 0) 
  {
    pdfPndl_.Evaluate(hit, paramPndl, probPndl);
  }

  double probHnh = 1.0;
  if (hit->hitNumber == 0) 
  {
    pdfHnh_.Evaluate(hit, paramHnh, probHnh);

    if (weight_ != 1.0) 
    {
      probHnh = pow(probHnh, weight_);
    }
  }

  log_debug("probPndl = %.2e, probHnh = %.2e, weight = %.2f",
    probPndl, probHnh, weight_);

  double tmpValue = probPndl * probHnh;
  if (std::isnan(tmpValue) || std::isinf(tmpValue)) 
  {
    // probPndl and probHnh should never be overflows, so if tmpValue is 
    //an overflow, it's probably because
    // the small probabilities are too small, so make them larger!
    log_error("Overflow in PndlHnh!\n"
      "    You probably need to adjust "
      "I3CscdLlhUPandel::CONST_SMALL_PROB and "
      "I3CscdLlhHitNoHit::CONST_SMALL_PROB.\n"
      "    probPndl = %.2e, probHnh = %.2e, weight = %.2f",
      probPndl, probHnh, weight_);
    return;
  }

  value = tmpValue;

  log_debug("PndlHnh probability: %.4e", value);
  return;
} // end Evaluate

/* ****************************************************** */
/* CalculateGradient                                      */
/* ****************************************************** */
void I3CscdLlhPndlHnh::CalculateGradient(const I3CscdLlhHitPtr& hit,
  const double* param, double* gradient) const 
{
  log_fatal("CalculateGradient has not been implemented in I3CscdLlhPndlHnh!");
  return;
} // end CalculateGradient

/* ****************************************************** */
/* GetParamIndex                                          */
/* ****************************************************** */
int I3CscdLlhPndlHnh::GetParamIndex(const string name) const 
{

  int idx = INT_MIN;
  if (name == "t") {
    idx = PARAM_INDEX_T;
  }
  else if (name == "x") {
    idx = PARAM_INDEX_X;
  }
  else if (name == "y") {
    idx = PARAM_INDEX_Y;
  }
  else if (name == "z") {
    idx = PARAM_INDEX_Z;
  }
  else if (name == "energy") {
    idx = PARAM_INDEX_ENERGY;
  }

  return idx;
} // end GetParamIndex

/* ****************************************************** */
/* GetParamName                                           */
/* ****************************************************** */
string I3CscdLlhPndlHnh::GetParamName(const int index) const 
{
  string s = "";

  switch (index) 
  {
    case PARAM_INDEX_T:
      s = "t";
      break;

    case PARAM_INDEX_X:
      s = "x";
      break;

    case PARAM_INDEX_Y:
      s = "y";
      break;

    case PARAM_INDEX_Z:
      s = "z";
      break;

    case PARAM_INDEX_ENERGY:
      s = "energy";
      break;

    default:
      break;
  }

  return s;
} // end GetParamName

/* ****************************************************** */
/* SetConstant -- double value                            */
/* ****************************************************** */
bool I3CscdLlhPndlHnh::SetConstant(int id, double value) 
{
  bool ret = false;

  switch(id) 
  {
    case CONST_WEIGHT:
      weight_ = value;
      ret = true;
      break;

    case CONST_PNDL_C_ICE:
      ret = pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_C_ICE, value);
      break;

    case CONST_PNDL_SMALL_PROB:
      ret = pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_SMALL_PROB, value);
      break;

    case CONST_PNDL_TAU:
      ret = pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_TAU, value);
      break;

    case CONST_PNDL_LAMBDA:
      ret = pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_LAMBDA, value);
      break;

    case CONST_PNDL_LAMBDA_A:
      ret = pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_LAMBDA_A, value);
      break;

    case CONST_PNDL_SIGMA:
      ret = pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_SIGMA, value);
      break;

    case CONST_PNDL_MAX_DIST:
      ret = pdfPndl_.SetConstant(I3CscdLlhUPandel::CONST_MAX_DIST, value);
      break;

    case CONST_HNH_NORM:
      ret = pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_NORM, value);
      break;

    case CONST_HNH_LAMBDA_ATTN:
      ret = pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_LAMBDA_ATTN, value);
      break;

    case CONST_HNH_NOISE:
      ret = pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_NOISE, value);
      break;

    case CONST_HNH_DIST_CUTOFF:
      ret = pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_DIST_CUTOFF, value);
      break;

    case CONST_HNH_DEAD:
      ret = pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_DEAD, value);
      break;

    case CONST_HNH_SMALL_PROB:
      ret = pdfHnh_.SetConstant(I3CscdLlhHitNoHit::CONST_SMALL_PROB, value);
      break;

    default:
      log_error("Unable to set constant -- invalid ID [%d]", id);
      ret = false;

  } // switch
 
  return ret;
} // SetConstant(int, double)

/* ****************************************************** */
/* SetConstant -- int value                               */
/* ****************************************************** */
bool I3CscdLlhPndlHnh::SetConstant(int id, int value) 
{
  log_error("I3CscdLlhPndlHnh has no integer-valued constants.");
  return false;
} // SetConstant(int, int)


/* ****************************************************** */
/* SetConstant -- bool value                              */
/* ****************************************************** */
bool I3CscdLlhPndlHnh::SetConstant(int id, bool value) 
{
  log_error("I3CscdLlhPndlHnh has no boolean-valued constants.");
  return false;
} // SetConstant(int, bool)

/* ****************************************************** */
/* SetConstant -- string value                             */
/* ****************************************************** */
bool I3CscdLlhPndlHnh::SetConstant(int id, string value) 
{
  log_error("I3CscdLlhPndlHnh has no string-valued constants.");
  return false;
} // SetConstant(int, string)
