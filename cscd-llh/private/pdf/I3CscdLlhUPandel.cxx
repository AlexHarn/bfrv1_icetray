/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhUPandel.cxx
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 */

#include <iostream>
#include <math.h>

using namespace std;

#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/pdf/I3CscdLlhUPandel.h"
#include <climits>

#include <gsl/gsl_sf_gamma.h>

const int I3CscdLlhUPandel::MINIMIZATION_PARAMS    = 4;
const int I3CscdLlhUPandel::PARAM_INDEX_T          = 0;
const int I3CscdLlhUPandel::PARAM_INDEX_X          = 1;
const int I3CscdLlhUPandel::PARAM_INDEX_Y          = 2;
const int I3CscdLlhUPandel::PARAM_INDEX_Z          = 3;

const int I3CscdLlhUPandel::CONST_NULL             = 0;
const int I3CscdLlhUPandel::CONST_C_ICE            = 1;
const int I3CscdLlhUPandel::CONST_SMALL_PROB       = 2;
const int I3CscdLlhUPandel::CONST_TAU              = 3;
const int I3CscdLlhUPandel::CONST_LAMBDA           = 4;
const int I3CscdLlhUPandel::CONST_LAMBDA_A         = 5;
const int I3CscdLlhUPandel::CONST_SIGMA            = 6;
const int I3CscdLlhUPandel::CONST_MAX_DIST         = 7;

// Unless noted otherwise, all units are meters, nanoseconds, GeV.
const double I3CscdLlhUPandel::DEFAULT_C_ICE       = 2.99792458e-1 / 1.31;
const double I3CscdLlhUPandel::DEFAULT_SMALL_PROB  = 1.0e-10;
const double I3CscdLlhUPandel::DEFAULT_TAU         = 450.0; //nanoseconds
const double I3CscdLlhUPandel::DEFAULT_LAMBDA      = 47.0;  // meters
const double I3CscdLlhUPandel::DEFAULT_LAMBDA_A    = 98.0;  // meters
const double I3CscdLlhUPandel::DEFAULT_SIGMA       = 15.0;  // nanoseconds
const double I3CscdLlhUPandel::DEFAULT_MAX_DIST    = 500.0; // meters

//these are the muon values. These were defined by MgGreene, and fixed by DLR
//They have been left in as comments by DLR, for the sake of continuity.
/*const double I3CscdLlhUPandel::DEFAULT_TAU         = 557.0; //nanoseconds
const double I3CscdLlhUPandel::DEFAULT_LAMBDA      = 33.3;  // meters
const double I3CscdLlhUPandel::DEFAULT_LAMBDA_A    = 98.0;  // meters
const double I3CscdLlhUPandel::DEFAULT_SIGMA       = 15.0;  // nanoseconds*/
I3CscdLlhUPandel::I3CscdLlhUPandel() : I3CscdLlhAbsPdf() 
{
  cIce_ = DEFAULT_C_ICE;
  smallProb_ = DEFAULT_SMALL_PROB;
  tau_ = DEFAULT_TAU;
  lambda_ = DEFAULT_LAMBDA;
  lambdaA_ = DEFAULT_LAMBDA_A;
  sigma_ = DEFAULT_SIGMA;
  maxDist_ = DEFAULT_MAX_DIST;
  SetRho();
  SetPatchTime();
}

/* ****************************************************** */
/* Evaluate                                               */
/* ****************************************************** */
void I3CscdLlhUPandel::Evaluate(const I3CscdLlhHitPtr& hit,
  const double* param, double& value) const 
{
  log_debug("Call to I3CscdLlhUPandel :: Evaluate");
  value = NAN;

  if (hit->omHitCount == 0) 
  {
    value = 1.0;
    log_error("Ignoring an unhit OM!");
    return;
  }

  double t = hit->t;
  double x = hit->x;
  double y = hit->y;
  double z = hit->z;

  // This is the trial vertex.
  double at = param[PARAM_INDEX_T];
  double ax = param[PARAM_INDEX_X];
  double ay = param[PARAM_INDEX_Y];
  double az = param[PARAM_INDEX_Z];

  if (std::isnan(at) || std::isnan(ax) || std::isnan(ay) || std::isnan(az)) 
  {
    log_error("Bad vertex (at, ax, ay, az): (%f, %f, %f, %f).",
      at, ax, ay, az);
    return;
  }

  log_trace("Evaluate using hit (t, x, y, z): (%f, %f, %f, %f)\n"
    "     and vertex (at, ax, ay, az): (%f, %f, %f, %f).",
    t, x, y, z, at, ax, ay, az);

  double dd = (x-ax)*(x-ax) + (y-ay)*(y-ay) + (z-az)*(z-az);
  double dist = sqrt(dd);
  double tResidual = (t-at) - (dist/cIce_);

  double tmpValue = NAN;
  if (maxDist_ > 0.0 && dist > maxDist_) 
  {
    log_debug("Distance is large: %.2f > %.2f", dist, maxDist_);
    value = smallProb_;
    return;
  }

  if (tResidual <= 0.0) 
  {
    tmpValue = EvaluateGaussian(dist, tResidual);
  }
  else if (tResidual <= patchTime_) 
  {
    tmpValue = EvaluateSpline(dist, tResidual);
  }
  else 
  {
    tmpValue = EvaluatePandel(dist, tResidual);
  }

  if (std::isnan(tmpValue) || std::isinf(tmpValue)) 
  {
    log_debug("Overflow in UPandel!  Evaluated using hit (t, x, y, z):"
      " (%f, %f, %f, %f)\n"
      "     and vertex (at, ax, ay, az): (%f, %f, %f, %f).",
      t, x, y, z, at, ax, ay, az);
    value = smallProb_;
    return;
  }

  if (tmpValue < smallProb_) 
  {
    log_trace("UPandel value [%f] is very small! Setting it"
      " to smallProb.", tmpValue);
      tmpValue = smallProb_;
  }

  value = tmpValue;

  log_debug("UPandel probability: %.4e", value);
  return;
} // end Evaluate

/* ****************************************************** */
/* CalculateGradient                                      */
/* ****************************************************** */
void I3CscdLlhUPandel::CalculateGradient(const I3CscdLlhHitPtr& hit,
  const double* param, double* gradient) const 
{
  log_fatal("CalculateGradient has not been implemented in I3CscdLlhUPandel!");
  return;
} // end CalculateGradient

/* ****************************************************** */
/* EvaluateGaussian                                       */
/* ****************************************************** */
double I3CscdLlhUPandel::EvaluateGaussian(double dist, double tResidual) const 
{
  log_trace("Enter EvaluateGaussian(dist = %f, tResidual = %f).", 
    dist, tResidual);

  double c0 = CalculateC0(dist);
  double arg = -(tResidual * tResidual) / (2.0 * sigma_ * sigma_);
  double value = c0 * exp(arg);

  log_trace("EvaluateGaussian.  c0: %.2e, arg: %.2e, value: %.2e",
	    c0, arg, value);

  return value;
} // end EvaluateGaussian

/* ****************************************************** */
/* EvaluateSpline                                         */
/* ****************************************************** */
double I3CscdLlhUPandel::EvaluateSpline(double dist, double tResidual) const 
{
  log_trace("Enter EvaluateSpline(dist = %f, tResidual = %f).", 
    dist, tResidual);

  double c0 = CalculateC0(dist);
  double c2 = CalculateC2(dist);
  double c3 = CalculateC3(dist);

  double value = c0 +
      c2*tResidual*tResidual +
      c3*tResidual*tResidual*tResidual;

  log_trace("EvaluateGaussian.  c0: %.2e, c2: %.2e, c3: %.2e, value: %.2e",
	    c0, c2, c3, value);

  return value;
} // end EvaluateSpline

/* ****************************************************** */
/* EvaluatePandel                                         */
/* ****************************************************** */
double I3CscdLlhUPandel::EvaluatePandel(double dist, double tResidual) const 
{
  log_trace("Enter EvaluatePandel(dist = %f, tResidual = %f).",
    dist, tResidual);

  if (tResidual < 0.0) 
    return 0.0;

  double xi = dist/lambda_;

  double n1 = pow(rho_, xi);
  double n2 = pow(tResidual, xi - 1.0);
  double n3 = exp(-rho_*tResidual);
  double d1 = gsl_sf_gamma(xi);

  double value = (n1*n2*n3)/d1;

  log_trace("EvaluatePandel.  xi: %.2e, n1: %.2e, n2: %.2e, n3: %.2e, "
    "d1: %.2e, value: %.2e",
    xi, n1, n2, n3, d1, value);

  return value;
} // end EvaluatePandel

/* ****************************************************** */
/* DifferentiatePandel                                    */
/* ****************************************************** */
double I3CscdLlhUPandel::DifferentiatePandel(double dist, 
  double tResidual) const
{
  double xi = dist/lambda_;

  double rhs = ((xi - 1.0)/tResidual) - rho_;
  double value = EvaluatePandel(dist, tResidual) * rhs;

  return value;
} // end DifferentiatePandel

/* ****************************************************** */
/* CalculateC0                                            */
/* ****************************************************** */
double I3CscdLlhUPandel::CalculateC0(double dist) const 
{   
  double term1 = IntegratePandel(dist, patchTime_) / patchTime_;
  double term2 = -EvaluatePandel(dist, patchTime_) / 2.0;
  double term3 = DifferentiatePandel(dist, patchTime_) * patchTime_ / 12.0;
  double value = term1 + term2 + term3;

  // if c0 is negative, it's probably a rounding error.
  if (value < 0.0) 
  {
    log_trace("c0 [%.2e] is negative! Setting it to zero.  dist: %f,"
      " term1: %.2e, term2: %.2e, term3: %.2e",
      value, dist, term1, term2, term3);
    value = 0.0;
  }

  return value;
} // end CalculateC0

/* ****************************************************** */
/* CalculateC2                                            */
/* ****************************************************** */
double I3CscdLlhUPandel::CalculateC2(double dist) const 
{
  double term1 = -(3.0 * IntegratePandel(dist, patchTime_)) /
    (patchTime_ * patchTime_ * patchTime_);
  double term2 = (9.0 * EvaluatePandel(dist, patchTime_))/
    (2.0 * patchTime_ * patchTime_);
  double term3 = -(5.0 * DifferentiatePandel(dist, patchTime_)) /
    (4.0 * patchTime_);
  double value = term1 + term2 + term3;

  return value;
} // end CalculateC2

/* ****************************************************** */
/* CalculateC3                                            */
/* ****************************************************** */
double I3CscdLlhUPandel::CalculateC3(double dist) const 
{
  double term1 = (2.0 * IntegratePandel(dist, patchTime_)) /
    (patchTime_ * patchTime_ * patchTime_ * patchTime_);
  double term2 = -(3.0 * EvaluatePandel(dist, patchTime_))/
    (patchTime_ * patchTime_ * patchTime_);
  double term3 = (7.0 * DifferentiatePandel(dist, patchTime_)) /
    (6.0 * patchTime_ * patchTime_);
  double value = term1 + term2 + term3;

  return value;
} // end CalculateC3

/* ****************************************************** */
/* IntegratePandel                                        */
/* ****************************************************** */
double I3CscdLlhUPandel::IntegratePandel(double dist, double tResidual) const 
{
  double xi = dist/lambda_;

  // This is the upper incomplete gamma function P(a,x),
  // normalized such that gsl_sf_gamma_inc_P(a, +infinity) = 1.
  double value = gsl_sf_gamma_inc_P(xi, tResidual * rho_);

  return value;
} // end IntegratePandel

/* ****************************************************** */
/* GetParamIndex                                          */
/* ****************************************************** */
int I3CscdLlhUPandel::GetParamIndex(const string name) const 
{
  int idx = INT_MIN;
  if (name == "t") 
  {
    idx = PARAM_INDEX_T;
  }
  else if (name == "x") 
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

  return idx;
} // end GetParamIndex

/* ****************************************************** */
/* GetParamName                                           */
/* ****************************************************** */
string I3CscdLlhUPandel::GetParamName(const int index) const 
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

    default:
      break;
  }

  return s;
} // end GetParamName

/* ****************************************************** */
/* SetConstant -- double value                            */
/* ****************************************************** */
bool I3CscdLlhUPandel::SetConstant(int id, double value) 
{
  switch(id) 
  {
    case CONST_C_ICE:
      cIce_ = value;
      SetRho();
      break;

    case CONST_SMALL_PROB:
      smallProb_ = value;
      break;

    case CONST_TAU:
      tau_ = value;
      SetRho();
      break;

    case CONST_LAMBDA:
      lambda_ = value;
      break;

    case CONST_LAMBDA_A:
      lambdaA_ = value;
      SetRho();
      break;

    case CONST_SIGMA:
      sigma_ = value;
      SetPatchTime();
      break;

    case CONST_MAX_DIST:
      maxDist_ = value;
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
bool I3CscdLlhUPandel::SetConstant(int id, int value) 
{
  log_error("I3CscdLlhUPandel has no integer-valued constants.");
  return false;
} // SetConstant(int, int)


/* ****************************************************** */
/* SetConstant -- bool value                              */
/* ****************************************************** */
bool I3CscdLlhUPandel::SetConstant(int id, bool value) 
{
  log_error("I3CscdLlhUPandel has no boolean-valued constants."); 
  return false;
} // SetConstant(int, bool)

/* ****************************************************** */
/* SetConstant -- string value                             */
/* ****************************************************** */
bool I3CscdLlhUPandel::SetConstant(int id, string value) 
{
  log_error("I3CscdLlhUPandel has no string-valued constants."); 
  return false;
} // SetConstant(int, string)
