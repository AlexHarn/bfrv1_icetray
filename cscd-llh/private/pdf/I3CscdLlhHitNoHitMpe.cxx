/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhHitNoHitMpe.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#include <iostream>

using namespace std;

#include "cscd-llh/pdf/I3CscdLlhHitNoHitMpe.h"


I3CscdLlhHitNoHitMpe::I3CscdLlhHitNoHitMpe() : I3CscdLlhHitNoHit() 
{
}

/* ****************************************************** */
/* evaluate                                               */
/* ****************************************************** */
void I3CscdLlhHitNoHitMpe::Evaluate(const I3CscdLlhHitPtr& hit,
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
  double dist = sqrt(dd);

  double mu = CalculateMu(dist, aEnergy);

  double tmpValue = ProbHits(hitCount, mu);

  if (std::isnan(tmpValue) || std::isinf(tmpValue)) 
  {
    log_debug("Overflow in HitNoHitMpe.  mu = %f."
      "  Evaluated using hit (x, y, z, hitCount): (%f, %f, %f, %d)\n"
      "     and vertex (ax, ay, az, aEnergy): (%f, %f, %f, %f).",
      mu, x, y, z, hitCount, ax, ay, az, aEnergy);
    value = smallProb_;
    return;
  }

  if (tmpValue < smallProb_) 
  {
    log_trace("HitNoHitMpe value [%f] is very small!"
      " Setting it to smallProb.", tmpValue);
      tmpValue = smallProb_;
  }

  value = tmpValue;

  log_debug("HitNoHitMpe probability = %.4e", value);
  return;
} // end evaluate

/* ****************************************************** */
/* CalculateGradient                                      */
/* ****************************************************** */
void I3CscdLlhHitNoHitMpe::CalculateGradient(const I3CscdLlhHitPtr& hit,
  const double* param, double* gradient) const 
{
  log_fatal("CalculateGradient has not been implemented in"
    " I3CscdLlhHitNoHitMpe!");
  return;
} // end CalculateGradient

/* ****************************************************** */
/* ProbHits                                               */
/* ****************************************************** */
double I3CscdLlhHitNoHitMpe::ProbHits(int n, double mu) const 
{
  log_trace("Enter ProbHits(n = %d, mu = %.4e).", n, mu);

  double probNoNoise = ProbHitsFromCascade(n, mu) * (1 - noise_);

  if (n == 0) return probNoNoise;

  double probNoise = ProbHitsFromCascade(n-1, mu) * noise_;

  double prob =  probNoNoise + probNoise;

  log_trace("ProbHits. probNoNoise: %.4e, probNoise: %.4e, prob: %.4e",
	    probNoNoise, probNoise, prob);
  return prob;
} // end ProbHits;
