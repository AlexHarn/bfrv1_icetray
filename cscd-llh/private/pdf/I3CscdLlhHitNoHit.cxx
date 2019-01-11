/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhHitNoHit.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include <iostream>
#include <math.h>

using namespace std;

#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/pdf/I3CscdLlhHitNoHit.h"
#include <climits>


const int I3CscdLlhHitNoHit::MINIMIZATION_PARAMS     = 4;//DLR
const int I3CscdLlhHitNoHit::PARAM_INDEX_X           = 0;
const int I3CscdLlhHitNoHit::PARAM_INDEX_Y           = 1;
const int I3CscdLlhHitNoHit::PARAM_INDEX_Z           = 2;
const int I3CscdLlhHitNoHit::PARAM_INDEX_ENERGY      = 3;

const int I3CscdLlhHitNoHit::CONST_NULL              = 0;
const int I3CscdLlhHitNoHit::CONST_NORM              = 1;
const int I3CscdLlhHitNoHit::CONST_LAMBDA_ATTN       = 2;
const int I3CscdLlhHitNoHit::CONST_NOISE             = 3;
const int I3CscdLlhHitNoHit::CONST_DIST_CUTOFF       = 4;
const int I3CscdLlhHitNoHit::CONST_DEAD              = 5;
const int I3CscdLlhHitNoHit::CONST_SMALL_PROB        = 6;

// Unless noted otherwise, all units are meters, nanoseconds, GeV.
const double I3CscdLlhHitNoHit::DEFAULT_NORM           = 1.4;
const double I3CscdLlhHitNoHit::DEFAULT_LAMBDA_ATTN    = 29.0;
const double I3CscdLlhHitNoHit::DEFAULT_NOISE          = 5.0e-3;
const double I3CscdLlhHitNoHit::DEFAULT_DIST_CUTOFF    = 0.5;
const double I3CscdLlhHitNoHit::DEFAULT_DEAD           = 0.05;
const double I3CscdLlhHitNoHit::DEFAULT_SMALL_PROB     = 1.0e-40;

I3CscdLlhHitNoHit::I3CscdLlhHitNoHit() : I3CscdLlhAbsPdf() 
{
  norm_ = DEFAULT_NORM;
  lambdaAttn_ = DEFAULT_LAMBDA_ATTN; noise_ = DEFAULT_NOISE;
  distCutoff_ = DEFAULT_DIST_CUTOFF;
  dead_ = DEFAULT_DEAD;
  smallProb_ = DEFAULT_SMALL_PROB;

  //DO NOT CHANGE THIS!!!
  useNoHits_ = true;
}

/* ****************************************************** */
/* evaluate                                               */
/* ****************************************************** */
void I3CscdLlhHitNoHit::Evaluate(const I3CscdLlhHitPtr& hit,
  const double* param, double& value) const 
{
  value = NAN;

  double x = hit->x;
  double y = hit->y;
  double z = hit->z;
  int hitCount = hit->omHitCount;

  // This is the trial vertex.
  double ax = param[PARAM_INDEX_X];
  double ay = param[PARAM_INDEX_Y];
  double az = param[PARAM_INDEX_Z];
  double aEnergy = param[PARAM_INDEX_ENERGY];

  if (minimizeInLogE_)
  {
    aEnergy = exp(aEnergy);
  }

  if (std::isnan(ax) || std::isnan(ay) || std::isnan(az) || std::isnan(aEnergy)) 
  {
    log_error("Bad vertex (ax, ay, az, aEnergy): (%f, %f, %f, %f).",
       ax, ay, az, aEnergy);
    return;
  }

  if (aEnergy < 0.0) 
  {
    value = smallProb_;
    return;
  }

  log_trace("Evaluate using hit (x, y, z, hitCount): (%f, %f, %f, %d)\n"
     "     and vertex (ax, ay, az, aEnergy): (%f, %f, %f, %f).",
     x, y, z, hitCount, ax, ay, az, aEnergy);

  double dd = (x-ax)*(x-ax) + (y-ay)*(y-ay) + (z-az)*(z-az);
  double dist = sqrt(dd + distCutoff_*distCutoff_);

  double mu = CalculateMu(dist, aEnergy);
  double tmpValue = hitCount == 0 ? ProbNoHit(mu) : ProbHit(mu);

  if (std::isnan(tmpValue) || std::isinf(tmpValue)) 
  {
    log_error("Overflow in HitNoHit.  mu = %f."
      " Evaluated using hit (x, y, z, hitCount): (%f, %f, %f, %d)\n" 
      " and vertex (ax, ay, az, aEnergy): (%f, %f, %f, %f).",
       mu, x, y, z, hitCount, ax, ay, az, aEnergy); 
    value = smallProb_;
    return;
  }

  if (tmpValue < smallProb_) 
  {
    log_trace("HitNoHit value [%f] is very small! Setting it to smallProb.",
      tmpValue);
    tmpValue = smallProb_;
  }

  value = tmpValue;

  log_debug("HitNoHit probability = %.4e", value);
  return;
} // end evaluate

/* ****************************************************** */
/* CalculateGradient                                      */
/* ****************************************************** */
void I3CscdLlhHitNoHit::CalculateGradient(const I3CscdLlhHitPtr& hit,
  const double* param, double* gradient) const 
{
  log_fatal("CalculateGradient has not been implemented in I3CscdLlhHitNoHit!");  return;
} // end CalculateGradient

/* ****************************************************** */
/* CalculateMu                                            */
/* ****************************************************** */
double I3CscdLlhHitNoHit::CalculateMu(double dist, double energy) const 
{
  log_trace("Enter CalculateMu(dist = %f, energy = %.2e).", dist, energy);

  double numer = norm_ * energy;
  double arg = -dist/lambdaAttn_;
  double mu = numer * exp(arg) / dist;
  
  log_trace("CalculateMu.  numer: %.4e, arg: %.4e, mu: %.4e",
    numer, arg, mu);

  return mu;
} // end CalculateMu

/* ****************************************************** */
/* ProbNoHit                                              */
/* ****************************************************** */
double I3CscdLlhHitNoHit::ProbNoHit(double mu) const 
{
  double prob = 1.0 - ProbHit(mu);

  log_trace("ProbNoHit.  prob: %.4e", prob);
  return prob;
} // end ProbNoHit

/* ****************************************************** */
/* ProbHit                                                */
/* ****************************************************** */
double I3CscdLlhHitNoHit::ProbHit(double mu) const 
{
  double prob1 = (1.0 - dead_) * ProbHitFromCasc(mu);
  double prob2 = noise_ * ProbNoHitFromCasc(mu);
  double prob = prob1 + prob2;

  log_trace("ProbHit. prob1: %.4e, prob2: %.4e, prob: %.4e",
    prob1, prob2, prob);
  return prob;
} // end ProbHit

/* ****************************************************** */
/* GetParamIndex                                          */
/* ****************************************************** */
int I3CscdLlhHitNoHit::GetParamIndex(const string name) const 
{
  int idx = INT_MIN;
  if (name == "x") 
  {
    idx = PARAM_INDEX_X;
  }
  else if (name == "y") 
  {
    idx = PARAM_INDEX_Y;
  }
  else if (name == "z") 
  {
    idx = PARAM_INDEX_Z;
  }
  else if (name == "energy") 
  {
    idx = PARAM_INDEX_ENERGY;
  }
  
  return idx;
} // end GetParamIndex

/* ****************************************************** */
/* GetParamName                                           */
/* ****************************************************** */
string I3CscdLlhHitNoHit::GetParamName(const int index) const 
{

  string s = "";

  switch (index) 
  {
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
      log_fatal("Attempting to retreive a parameter name "
        "that does not exist: %i.", index);
      break;
  }

  return s;
} // end GetParamName

/* ****************************************************** */
/* SetConstant -- double value                            */
/* ****************************************************** */
bool I3CscdLlhHitNoHit::SetConstant(int id, double value) 
{
  switch(id) 
  {
    case CONST_NORM:
      norm_ = value;
      break;

    case CONST_LAMBDA_ATTN:
      lambdaAttn_ = value;
      break;

    case CONST_NOISE:
      noise_ = value;
      break;

    case CONST_DIST_CUTOFF:
      distCutoff_ = value;
      break;

    case CONST_DEAD:
      dead_ = value;
      break;

    case CONST_SMALL_PROB:
      smallProb_ = value;
      break;

    default:
      log_error("Unable to Set constant -- invalid ID [%d]", id);
      return false;

  } // switch
 
  return true;
} // SetConstant(int, double)

/* ****************************************************** */
/* SetConstant -- int value                               */
/* ****************************************************** */
bool I3CscdLlhHitNoHit::SetConstant(int id, int value) 
{
  log_error("I3CscdLlhHitNoHit has no integer-valued constants.");
  return false;
} // SetConstant(int, int)

/* ****************************************************** */
/* SetConstant -- bool value                              */
/* ****************************************************** */
bool I3CscdLlhHitNoHit::SetConstant(int id, bool value) 
{
  log_error("I3CscdLlhHitNoHit has no boolean-valued constants."); 
  return false;
} // SetConstant(int, bool)

/* ****************************************************** */
/* SetConstant -- string value                            */
/* ****************************************************** */
bool I3CscdLlhHitNoHit::SetConstant(int id, string value) 
{
  log_error("I3CscdLlhHitNoHit has no string constants."); 
  return false;
} // SetConstant(int, string)
