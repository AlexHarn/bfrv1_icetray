/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhUPandelMpe.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#include <iostream>
#include <math.h>

using namespace std;

#include "cscd-llh/pdf/I3CscdLlhUPandelMpe.h"
#include <gsl/gsl_sf_erf.h>


I3CscdLlhUPandelMpe::I3CscdLlhUPandelMpe() : I3CscdLlhUPandel() 
{
}

/* ****************************************************** */
/* Evaluate                                               */
/* ****************************************************** */
void I3CscdLlhUPandelMpe::Evaluate(const I3CscdLlhHitPtr& hit,
  const double* param, double& value) const 
{
  value = NAN;

  if (hit->omHitCount == 0) 
  {
    value = 1.0;
    log_error("Ignoring an unhit OM!");
    return;
  }

  double tmpValue = NAN;
  I3CscdLlhUPandel::Evaluate(hit, param, tmpValue);
  if (std::isnan(tmpValue)) 
  {
    return;
  }

  int omHitCount = hit->omHitCount;

  log_trace("hitCount = %d", omHitCount);
  if (omHitCount <= 1) 
  {
    value = tmpValue;
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

  double dd = (x-ax)*(x-ax) + (y-ay)*(y-ay) + (z-az)*(z-az);
  double dist = sqrt(dd);
  double tResidual = (t-at) - (dist/cIce_);

  double integral = IntegrateUPandel(dist, tResidual);
  if (std::isnan(integral) || std::isinf(integral) || integral < 0.0) 
  {
    log_debug("Undefined integral over UPandel: %.2e." 
      " Evaluated using hit (t, x, y, z): (%f, %f, %f, %f)\n"
      "     and vertex (at, ax, ay, az): (%f, %f, %f, %f).",
      value, t, x, y, z, at, ax, ay, az);
    value = smallProb_;
    return;
  }

  tmpValue *= omHitCount * pow(integral, omHitCount-1);

  if (tmpValue < smallProb_) 
  {
    log_trace("UPandelMpe value [%f] is very small! Setting it to smallProb.", 
      tmpValue);
    tmpValue = smallProb_;
  }

  value = tmpValue;

  log_debug("UPandelMpe probability: %.4e", value);
  return;
} // end Evaluate

/* ****************************************************** */
/* CalculateGradient                                      */
/* ****************************************************** */
void I3CscdLlhUPandelMpe::CalculateGradient(const I3CscdLlhHitPtr& hit,
  const double* param, double* gradient) const 
{
  log_fatal("CalculateGradient has not been implemented in"
    " I3CscdLlhUPandelMpeMpe!");
  return;
} // end CalculateGradient

/* ****************************************************** */
/* IntegrateUPandel                                       */
/* ****************************************************** */
double I3CscdLlhUPandelMpe::IntegrateUPandel(double dist, 
  double tResidual) const 
{
  double integral;
  if (tResidual <= 0.0) 
  {
    integral = IntegrateGaussian(dist, tResidual);
  }
  else if (tResidual <= patchTime_) 
  {
    // The integral over the Gaussian from negative infinity to zero:
    integral = CalculateC0(dist) * patchTime_ / 2.0;
    integral += IntegrateSpline(dist, tResidual);
  }
  else 
  {
    // The integral over the Gaussian from negative infinity to zero:
    integral = CalculateC0(dist) * patchTime_ / 2.0;
    // The integral over the spline from zero to the patch time:
    integral += IntegrateSpline(dist, patchTime_);
    integral += IntegratePandel(dist, tResidual);
  }

  return integral;
} // end IntegrateUPandel

/* ****************************************************** */
/* IntegrateGaussian                                      */
/* ****************************************************** */
double I3CscdLlhUPandelMpe::IntegrateGaussian(double dist, 
  double tResidual) const 
{

  double arg = -tResidual/(sigma_ * sqrt(2.0));

  double erf = gsl_sf_erfc(arg);

  double k =  CalculateC0(dist) * sigma_ * sqrt(M_PI / 2.0);

  double integral = k*erf;

  if (integral < 0.0) 
  {
    log_error("Integral over Gaussian [%f] is negative!", integral);
  }

  return integral;
} // end IntegrateGaussian

/* ****************************************************** */
/* IntegrateSpline                                        */
/* ****************************************************** */
double I3CscdLlhUPandelMpe::IntegrateSpline(double dist, 
  double tResidual) const 
{
  double c0 = CalculateC0(dist);
  double c2 = CalculateC2(dist);
  double c3 = CalculateC3(dist);

  double integral = c0*tResidual +
      c2*tResidual*tResidual*tResidual/3.0 +
      c3*tResidual*tResidual*tResidual*tResidual/4.0;

  if (integral < 0.0) 
  {
    // Probably just a rounding error.
    log_debug("Integral over spline [%f] is negative!  Setting it to zero!\n"
            "c0: %f, c2: %f, c3: %f",
              integral, c0, c2, c3);
    integral = 0.0;
  }

  return integral;
} // end IntegrateSpline
