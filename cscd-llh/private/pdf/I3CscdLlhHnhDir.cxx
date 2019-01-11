/**
 * copyright (c) 2006
 * the IceCube collaboration
 * $Id$
 *
 * @file I3CscdLlhHnhDir.cxx
 * @version
 * @author Doug Rutledge
 * @date 1Feb2006
 */

#include "cscd-llh/pdf/I3CscdLlhHnhDir.h"
#include "dataclasses/I3Constants.h"

using namespace std;


const int I3CscdLlhHnhDir::MINIMIZATION_PARAMS       = 6;

const int I3CscdLlhHnhDir::PARAM_INDEX_X             = 1;
const int I3CscdLlhHnhDir::PARAM_INDEX_Y             = 2;
const int I3CscdLlhHnhDir::PARAM_INDEX_Z             = 3;
const int I3CscdLlhHnhDir::PARAM_INDEX_ENERGY        = 4;
const int I3CscdLlhHnhDir::PARAM_INDEX_AZIMUTH       = 5;
const int I3CscdLlhHnhDir::PARAM_INDEX_ZENITH        = 6;

//These constants need to be determined by a fit 
//to MC data. The values here correspond to the earlier values, used
//by Ignacio and Marek. The values are actually different, because
//the old sieglinde code represented the legendre poly differently than
//is done here. The value for the I0 parameter should be changed to 45/1000
//when using this particular hit no hit pdf.
const double I3CscdLlhHnhDir::DEFAULT_LEGENDRE_COEFF_0   = 4.364;
const double I3CscdLlhHnhDir::DEFAULT_LEGENDRE_COEFF_1   = 2.566;
const double I3CscdLlhHnhDir::DEFAULT_LEGENDRE_COEFF_2   = 2;

const double I3CscdLlhHnhDir::DEFAULT_NORM        = 40.0/1000.0;
const double I3CscdLlhHnhDir::DEFAULT_LAMBDA_ATTN = 29.0;
const double I3CscdLlhHnhDir::DEFAULT_NOISE       = 5.0e-3;
const double I3CscdLlhHnhDir::DEFAULT_DIST_CUTOFF = 0.5;
const double I3CscdLlhHnhDir::DEFAULT_DEAD        = 0.05;
const double I3CscdLlhHnhDir::DEFAULT_SMALL_PROB  = 1.0e-40;

const int I3CscdLlhHnhDir :: CONST_L_POLY_0         = 0;
const int I3CscdLlhHnhDir :: CONST_L_POLY_1         = 1;
const int I3CscdLlhHnhDir :: CONST_L_POLY_2         = 2;
const int I3CscdLlhHnhDir ::CONST_HNH_NORM          = 3;
const int I3CscdLlhHnhDir ::CONST_HNH_LAMBDA_ATTN   = 4;
const int I3CscdLlhHnhDir ::CONST_HNH_NOISE         = 5;
const int I3CscdLlhHnhDir ::CONST_HNH_DIST_CUTOFF   = 6;
const int I3CscdLlhHnhDir ::CONST_HNH_DEAD          = 7;
const int I3CscdLlhHnhDir ::CONST_HNH_SMALL_PROB    = 8;


I3CscdLlhHnhDir :: I3CscdLlhHnhDir() : I3CscdLlhAbsPdf()//I3CscdLlhHnhDir()
{
  norm_ = DEFAULT_NORM;
  lambdaAttn_ = DEFAULT_LAMBDA_ATTN;
  noise_ = DEFAULT_NOISE;
  distCutoff_ = DEFAULT_DIST_CUTOFF;
  dead_ = DEFAULT_DEAD;
  smallProb_ = DEFAULT_SMALL_PROB;
  a0_ = DEFAULT_LEGENDRE_COEFF_0;
  a1_ = DEFAULT_LEGENDRE_COEFF_1; 
  a2_ = DEFAULT_LEGENDRE_COEFF_2;;

  //DO NOT CHANGE THIS
  useNoHits_ = true;
}

void I3CscdLlhHnhDir :: Evaluate(const I3CscdLlhHitPtr& hit,
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
  double azimuth = param[PARAM_INDEX_AZIMUTH];
  double zenith = param[PARAM_INDEX_ZENITH];
  
  if (azimuth > 2*I3Constants::pi) azimuth = 2*I3Constants::pi;
  if (azimuth < 0) azimuth = 0;
  
  if(std::isnan(zenith)) zenith = 1.570796;
  if(zenith > I3Constants::pi) zenith = I3Constants::pi;
  if(zenith < 0) zenith = 0;

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
    "     and vertex (ax, ay, az, aEnergy, azimuth, zenith): "
    "(%f, %f, %f, %f, %f, %f).",
    x, y, z, hitCount, ax, ay, az, aEnergy,azimuth,zenith);

  double dd = (x-ax)*(x-ax) + (y-ay)*(y-ay) + (z-az)*(z-az);
  double dist = sqrt(dd + distCutoff_*distCutoff_);

  double mu = CalculateMu(dist, aEnergy);

  //calculate the angle between the the DOM, the Vertex, and the
  //zenith, azimuth direction.
  double angle = CalculateAngle(x, y, z, ax, ay, az, azimuth, zenith);

  log_trace("Angle:%f",angle);
  
  double angularCorrection = CalculateLegendrePoly(angle);

  mu *= angularCorrection;
 
  double tmpValue = hitCount == 0 ? ProbNoHit(mu) : ProbHit(mu);

  if (std::isnan(tmpValue) || std::isinf(tmpValue)) 
  {
    log_error("Overflow in HitNoHit.  mu = %f."
      " Evaluated using hit (x, y, z, hitCount): (%f, %f, %f, %d)\n" 
      " and vertex (ax, ay, az, aEnergy): (%f, %f, %f, %f).",
       mu, x, y, z, hitCount, ax, ay, az, aEnergy); 
    log_error("(Angle, correction) : (%f,%f)",angle,angularCorrection);
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
  //value = tmpValue * tan((zenith - 1.570796)/2);

  log_debug("HitNoHit probability = %.4e Hypothesis: (azi,zen) = "
    "(%f,%f) Energy = %f", 
    value,azimuth,zenith,aEnergy);
  return;
}//end Evaluate

double I3CscdLlhHnhDir :: CalculateMu(double dist, double energy) const
{
  log_trace(
    "Enter CalculateMu(dist = %f, energy = %.2e,", 
    dist, energy);

  double numer = norm_ * energy;
  double arg = -dist / lambdaAttn_;
  double mu = numer * exp(arg) / dist;
 
  log_trace("CalculateMu.  numer: %.4e, arg: %.4e, mu: %.4e",
    numer, arg, mu);

  return mu;
}//end CalculateMu

/* ****************************************************** */
/* ProbNoHit                                              */
/* ****************************************************** */
double I3CscdLlhHnhDir::ProbNoHit(const double mu) const 
{
  double prob = 1.0 - ProbHit(mu);

  log_trace("ProbNoHit.  prob: %.4e", prob);
  return prob;
} // end ProbNoHit

/* ****************************************************** */
/* ProbHit                                                */
/* ****************************************************** */
double I3CscdLlhHnhDir::ProbHit(const double mu) const 
{
  double prob1 = (1.0 - dead_) * ProbHitFromCasc(mu);
  double prob2 = noise_ * ProbNoHitFromCasc(mu);
  double prob = prob1 + prob2;

  log_trace("ProbHit. prob1: %.4e, prob2: %.4e, prob: %.4e",
    prob1, prob2, prob);
  return prob;
} // end ProbHit

int I3CscdLlhHnhDir :: GetParamIndex(const string name) const
{
  int index = -1;
  if (name == "x") 
  {
    index = PARAM_INDEX_X;
  }
  else if (name == "y") 
  {
    index = PARAM_INDEX_Y;
  }
  else if (name == "z") 
  {
    index = PARAM_INDEX_Z;
  }
  else if (name == "energy") 
  {
    index = PARAM_INDEX_ENERGY;
  }else if (name == "azimuth")
  {
    index = PARAM_INDEX_AZIMUTH;
  }else if (name == "zenith")
  {
    index = PARAM_INDEX_ZENITH;
  }
  
  return index; 
}

string I3CscdLlhHnhDir :: GetParamName(const int index) const
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

    case PARAM_INDEX_AZIMUTH:
      s = "azimuth";
      break;

    case PARAM_INDEX_ZENITH:
      s = "zenith";
      break;

    default:
      break;
  }

  return s;
}

bool I3CscdLlhHnhDir :: SetConstant(int id, double value)
{
  switch(id)
  {
    case CONST_L_POLY_0:
      a0_ = value;
      break;
    case CONST_L_POLY_1:
      a1_ = value;
      break;
    case CONST_L_POLY_2:
      a2_ = value;
      break;
    case CONST_HNH_NORM:
      norm_ = value;
      break;

    case CONST_HNH_LAMBDA_ATTN:
      lambdaAttn_ = value;
      break;

    case CONST_HNH_NOISE:
      noise_ = value;
      break;

    case CONST_HNH_DIST_CUTOFF:
      distCutoff_ = value;
      break;

    case CONST_HNH_DEAD:
      dead_ = value;
      break;

    case CONST_HNH_SMALL_PROB:
      smallProb_ = value;
      break;

    default:
      log_error("Unable to Set constant -- invalid ID [%d]", id);
      return false;

  }
  
  return true; 
}

bool I3CscdLlhHnhDir :: SetConstant(int id, int value)
{
  log_fatal("I3CscdLlhHnhDir has no integer-valued constants");
  return false;
}

bool I3CscdLlhHnhDir :: SetConstant(int id, bool value)
{
  log_fatal("I3CscdLlhHnhDir has no boolean-valued constants");
  return false;
}

bool I3CscdLlhHnhDir :: SetConstant(int id, string value)
{
  log_fatal("I3CscdLlhHnhDir has no string-valued constants");
  return false;
}

double I3CscdLlhHnhDir :: CalculateLegendrePoly(const double angle) const
{
  double cosineAngle = cos(angle);

  double legendrePoly = a0_ + a1_ * cosineAngle + 
    (a2_ / 2.0) * (3.0 * cosineAngle * cosineAngle - 1.0);

  return legendrePoly;
}

double I3CscdLlhHnhDir :: CalculateAngle(const double hitX, const double hitY, 
  const double hitZ, const double vertexX, const double vertexY, 
  const double vertexZ, const double hypothesisAzimuth, 
  const double hypothesisZenith) const
{
  double angle = 0.0;

  //define the dot product
  double dotProduct = 0.0;
  //define the norm of the vector from the vertex to the hit
  double segmentHVNorm = 0.0;
  //define the norm of the vertex to angle vector
  //this will be by definition unity, since I am calculating the 
  //coordinates on a unit sphere.
  double segmentVANorm = 1.0;

  double segmentVertexToHitX = hitX - vertexX;
  double segmentVertexToHitY = hitY - vertexY;
  double segmentVertexToHitZ = hitZ - vertexZ;

  double segmentVertexToAngleX =
    segmentVANorm * sin(hypothesisZenith) * cos(hypothesisAzimuth);
  double segmentVertexToAngleY =
    segmentVANorm * sin(hypothesisZenith) * sin (hypothesisAzimuth);;
  double segmentVertexToAngleZ =
    segmentVANorm * cos(hypothesisZenith);
 
  dotProduct +=
    segmentVertexToAngleX * segmentVertexToHitX;
  dotProduct +=
    segmentVertexToAngleY * segmentVertexToHitY;
  dotProduct +=
    segmentVertexToAngleZ * segmentVertexToHitZ; 

  segmentHVNorm +=
    segmentVertexToHitX * segmentVertexToHitX;
  segmentHVNorm +=
    segmentVertexToHitY * segmentVertexToHitY;
  segmentHVNorm +=
    segmentVertexToHitZ * segmentVertexToHitZ;

  segmentHVNorm = sqrt(segmentHVNorm);
  
  //this is the standard way of getting the cosine of the
  //angle between two vectors
  double cosineOfAngle = dotProduct/ (segmentHVNorm * segmentVANorm);

  //apply the acos to get the angle itself
  angle = acos(cosineOfAngle);


  if (std::isnan(angle))
  {
    log_error("Nonsense angle: %f,%f,%f : %f, %f",
      dotProduct,segmentVANorm,segmentHVNorm, hypothesisZenith, hypothesisAzimuth);
  }

  //done!
  return angle;
}

void I3CscdLlhHnhDir::CalculateGradient(const I3CscdLlhHitPtr& hit,
  const double* param, double* gradient) const
{
  log_fatal("CalculateGradient is not implemented in I3CscdLlhHnhDir");
}
