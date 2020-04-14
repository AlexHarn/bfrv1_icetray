/**
 *
 * @file I3RadeBasicFunctionService.cxx
 * @brief implementaration of the I3RadeBasicFunctionService class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id:  $
 *
 * @version $Revision:  $
 * @date $Date: $
 * @author kath
 *
 */

#include <string>
#include <cassert>
#include <cmath>
#include "icetray/I3SingleServiceFactory.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Constants.h"

#include "toprec/I3SnowCorrectionService.h"

/*
 * I3RadeBasicSnowCorrectionService
 * class for computing Kath's second attempt at a RAdius-DEpendent snow correction
 */

/// ----------- KR'S FIRST RADE SNOW CORRECTION --------------
/// default constructor for unit tests
I3RadeBasicSnowCorrectionService::I3RadeBasicSnowCorrectionService(const std::string& name,
                                                                   double rmin) :
  I3SnowCorrectionServiceBase(name),
  fRminimum_(rmin) {
  log_debug("Hello, I am a nearly empty RadeBasicFunctionService constructor for unit tests");
}

/// Regular constructor
I3RadeBasicSnowCorrectionService::I3RadeBasicSnowCorrectionService(const I3Context &c):
  I3SnowCorrectionServiceBase(c){
  log_debug("Entering the constructor");
  fRminimum_ = 11.0;
  AddParameter("RMinimum", "Minimum radius (should match the DynamicCoreTreatment radius from Laputop)", fRminimum_);

 }

void I3RadeBasicSnowCorrectionService::Configure(){
  GetParameter("RMinimum", fRminimum_);
}


/// THIS IS THE MEATY FUNCTION!
double I3RadeBasicSnowCorrectionService::AttenuationFactor(const I3Position& pos,
                                                      double snowDepth,  
                                                      const I3Particle& hypoth,
                                                      const I3LaputopParams& params)
  const
{
  using namespace Laputop;
  // First, collect track variables:
  //double theta = hypoth->GetZenith();
  // Do it this way to avoid weirdness with straight vertical tracks
  I3Direction dir = hypoth.GetDir();
  I3Position core = hypoth.GetPos();
  double cosz = fabs(dir.GetZ()); // Always a positive number (downgoing)
  double slantdepth = snowDepth / cosz;
  // Also grab relevant info from the "params":
  const double s125 = pow(10.0, params.GetValue(Parameter::Log10_S125));
  const double beta = params.GetValue(Parameter::Beta);

  // Compute the radius (just as is done in LaputopLikelihood
  double r = DistToAxis(hypoth, pos);

  // Attempt #1: a simple function
  double l;
  // Does this land inside the "11-meter cut" from Laputop?  If so, recompute it at the limit:
  if (r < fRminimum_) {
    log_debug("RADE: resetting the radius from %f to %f", r, fRminimum_);
    l = Lambda(fRminimum_, s125);
  } else
    l = Lambda(r, s125);
  
  double attfactor = exp(-slantdepth/l);
  return attfactor;
}

void I3RadeBasicSnowCorrectionService::FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr diagnost,
                                                      I3ParticleConstPtr hypoth,
                                                      I3LaputopParamsConstPtr paramPtr)
  const
{
   log_debug("Attempting to fill RADE-basic diagnostics...");

   // Once again, harvest relevant variables
   const double zenith = hypoth->GetZenith();
   const double logS125 = paramPtr->GetValue(Laputop::Parameter::Log10_S125);
   const double s125 = pow(10.0, logS125);
   const double beta = paramPtr->GetValue(Laputop::Parameter::Beta);
 
   diagnost->tstage = -999;
   diagnost->tstage_restricted = -999;

   diagnost->fEM_30m = 1;
   diagnost->fEM_50m = 1;
   diagnost->fEM_80m = 1;
   diagnost->fEM_100m = 1;
   diagnost->fEM_125m = 1;
   diagnost->fEM_150m = 1;
   diagnost->fEM_200m = 1;
   diagnost->fEM_300m = 1;
   diagnost->fEM_500m = 1;
   diagnost->fEM_1000m = 1;
  
   diagnost->lambda_EM_30m = Lambda(30,s125);
   diagnost->lambda_EM_50m = Lambda(50,s125);
   diagnost->lambda_EM_80m = Lambda(80,s125);
   diagnost->lambda_EM_100m = Lambda(100,s125);
   diagnost->lambda_EM_125m = Lambda(125,s125);
   diagnost->lambda_EM_150m = Lambda(150,s125);
   diagnost->lambda_EM_200m = Lambda(200,s125);
   diagnost->lambda_EM_300m = Lambda(300,s125);
   diagnost->lambda_EM_500m = Lambda(500,s125);
   diagnost->lambda_EM_1000m = Lambda(1000,s125);
  
   diagnost->lambda_EM_30m_restricted = Lambda(30,s125);
   diagnost->lambda_EM_50m_restricted = Lambda(50,s125);
   diagnost->lambda_EM_80m_restricted = Lambda(80,s125);
   diagnost->lambda_EM_100m_restricted = Lambda(100,s125);
   diagnost->lambda_EM_125m_restricted = Lambda(125,s125);
   diagnost->lambda_EM_150m_restricted = Lambda(150,s125);
   diagnost->lambda_EM_200m_restricted = Lambda(200,s125);
   diagnost->lambda_EM_300m_restricted = Lambda(300,s125);
   diagnost->lambda_EM_500m_restricted = Lambda(500,s125);
   diagnost->lambda_EM_1000m_restricted = Lambda(1000,s125);

 }


// ----------------- HELPER FUNCTIONS -------------------
// -------- RADE 1 ----------
double I3RadeBasicSnowCorrectionService::Lambda(double r, double s125)
  const
{
    // These come from the ROOT analysis of S125 = 1-100, and zenith = 0-30deg
    // Attempt #1: a simple function
    float p0_0 = 4.68368;
    float p0_1 = -0.0278981;
    float p1_0 = -1.22964;
    float p1_1 = 0.0182584;
    double p0, p1;
    // Are we past the point (at large S125) where it should be flat, or not?
    double s125max = -p1_0/p1_1;
    if (s125 < s125max) {
        // Compute p0 and p1 for lambda vs. R
        p0 = p0_0 + p0_1*s125;
        p1 = p1_0 + p1_1*s125;
    } else { // make it flat
        p0 = p0_0 + p0_1*s125max;
        p1 = p1_0 + p1_1*s125max;
    }
    // now compute lambda
    double logr = log10(r);
    double lambda = p0 + p1*logr;
    log_debug("Lambda from RADE: %f, at radius %f and S125 %f", lambda, r, s125);
    return lambda;
}


/// ----------- KR'S SECOND RADE SNOW CORRECTION --------------
/// default constructor for unit tests
I3RadeMuonSnowCorrectionService::I3RadeMuonSnowCorrectionService(const std::string& name,
                                                                   double rmin) :
  I3SnowCorrectionServiceBase(name),
  fRminimum_(rmin) {
  log_debug("Hello, I am a nearly empty RadeMuonFunctionService constructor for unit tests");
}

/// Regular constructor
I3RadeMuonSnowCorrectionService::I3RadeMuonSnowCorrectionService(const I3Context &c):
  I3SnowCorrectionServiceBase(c){
  log_debug("Entering the constructor");
  fRminimum_ = 11.0;
  AddParameter("RMinimum", "Minimum radius (should match the DynamicCoreTreatment radius from Laputop)", fRminimum_);

 }

void I3RadeMuonSnowCorrectionService::Configure(){
  GetParameter("RMinimum", fRminimum_);
}


/// THIS IS THE MEATY FUNCTION!
double I3RadeMuonSnowCorrectionService::AttenuationFactor(const I3Position& pos,
                                                      double snowDepth,
                                                      const I3Particle& hypoth,
                                                      const I3LaputopParams& params)
  const
{
  using namespace Laputop;
  // First, collect track variables:
  //double theta = hypoth->GetZenith();
  // Do it this way to avoid weirdness with straight vertical tracks
  I3Direction dir = hypoth.GetDir();
  I3Position core = hypoth.GetPos();
  double cosz = fabs(dir.GetZ()); // Always a positive number (downgoing)
  double slantdepth = snowDepth / cosz;
  // Also grab relevant info from the "params":
  const double s125 = pow(10.0, params.GetValue(Parameter::Log10_S125));
  const double beta = params.GetValue(Parameter::Beta);

  // Compute the radius (just as is done in LaputopLikelihood
  double r = DistToAxis(hypoth, pos);

  // Attempt #1: a simple function
  double s;
  // Does this land inside the "11-meter cut" from Laputop?  If so, recompute it at the limit:
  if (r < fRminimum_) {
    log_debug("RADE: resetting the radius from %f to %f", r, fRminimum_);
    s = Slope(fRminimum_, s125);
  } else
    s = Slope(r, s125);
  
  double attfactor = exp(s*slantdepth);
  return attfactor;
}

void I3RadeMuonSnowCorrectionService::FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr diagnost,
                                                      I3ParticleConstPtr hypoth,
                                                      I3LaputopParamsConstPtr paramPtr)
  const
{
   log_debug("Attempting to fill RADE-muon diagnostics...");

   // Once again, harvest relevant variables
   const double zenith = hypoth->GetZenith();
   const double logS125 = paramPtr->GetValue(Laputop::Parameter::Log10_S125);
   const double s125 = pow(10.0, logS125);
   const double beta = paramPtr->GetValue(Laputop::Parameter::Beta);
 
   diagnost->tstage = -999;
   diagnost->tstage_restricted = -999;

   diagnost->fEM_30m = 1;
   diagnost->fEM_50m = 1;
   diagnost->fEM_80m = 1;
   diagnost->fEM_100m = 1;
   diagnost->fEM_125m = 1;
   diagnost->fEM_150m = 1;
   diagnost->fEM_200m = 1;
   diagnost->fEM_300m = 1;
   diagnost->fEM_500m = 1;
   diagnost->fEM_1000m = 1;
  
   diagnost->lambda_EM_30m = -1.0/Slope(30,s125);
   diagnost->lambda_EM_50m = -1.0/Slope(50,s125);
   diagnost->lambda_EM_80m = -1.0/Slope(80,s125);
   diagnost->lambda_EM_100m = -1.0/Slope(100,s125);
   diagnost->lambda_EM_125m = -1.0/Slope(125,s125);
   diagnost->lambda_EM_150m = -1.0/Slope(150,s125);
   diagnost->lambda_EM_200m = -1.0/Slope(200,s125);
   diagnost->lambda_EM_300m = -1.0/Slope(300,s125);
   diagnost->lambda_EM_500m = -1.0/Slope(500,s125);
   diagnost->lambda_EM_1000m = -1.0/Slope(1000,s125);
  
   diagnost->lambda_EM_30m_restricted = -1.0/Slope(30,s125);
   diagnost->lambda_EM_50m_restricted = -1.0/Slope(50,s125);
   diagnost->lambda_EM_80m_restricted = -1.0/Slope(80,s125);
   diagnost->lambda_EM_100m_restricted = -1.0/Slope(100,s125);
   diagnost->lambda_EM_125m_restricted = -1.0/Slope(125,s125);
   diagnost->lambda_EM_150m_restricted = -1.0/Slope(150,s125);
   diagnost->lambda_EM_200m_restricted = -1.0/Slope(200,s125);
   diagnost->lambda_EM_300m_restricted = -1.0/Slope(300,s125);
   diagnost->lambda_EM_500m_restricted = -1.0/Slope(500,s125);
   diagnost->lambda_EM_1000m_restricted = -1.0/Slope(1000,s125);

 }

// ------- RADE 2, with MUONS --------
double I3RadeMuonSnowCorrectionService::Slope(double r, double s125)
  const
{
    // These came from a second analysis of S125 = 1-100, using the "triplezenith"
    // events from 0-45 degrees all lumped together.
    float fpol0[3] = {-0.04997865, -0.03313576,  0.03236982};
    float fpol1[3] = { 0.03253792,  0.02999731, -0.24494757};
    float fpol2[2] = {0.20433183, 2.24791674};
    float fpol3[2] = {-0.12182074,  0.77739906};
    double logr = log10(r);
    double logs125 = log10(s125);

    // The s125 at which the first slope goes positive; don't let that happen!
    float a = fpol1[0];
    float b = fpol1[1];
    float c = fpol1[2];
    float logs125max1 = (-b + sqrt(b*b - 4*a*c))/(2*a);
    float logs125max2 = (-b - sqrt(b*b - 4*a*c))/(2*a);
    //print ("Quadratic roots = ", logs125max1, logs125max2)
    double y0, s1;
    if (log10(s125) > logs125max1) { // make it flat
        y0 = fpol0[2] + fpol0[1]*logs125max1 + fpol0[0]*logs125max1*logs125max1;
        s1 = 0;
    } else if (log10(s125) < logs125max2) { // again, make it flat
        y0 = fpol0[2] + fpol0[1]*logs125max2 + fpol0[0]*logs125max2*logs125max2;
        s1 = 0;
    } else {  // normal stuff
        y0 = fpol0[2] + fpol0[1]*logs125 + fpol0[0]*logs125*logs125;
        s1 = fpol1[2] + fpol1[1]*logs125 + fpol1[0]*logs125*logs125;
    }
    double x0 = fpol2[1] + fpol2[0]*logs125;
    double s2 = fpol3[1] + fpol3[0]*logs125;

    double x = logr;
    double ans;
    if (x < x0)
        ans = y0 + x*s1;
    else
        ans = (y0 + x0*s1) + (x-x0)*s2;
   
    // Cap the slope at zero!
    if (ans > 0)
        ans = 0;
   
    return ans;
}


typedef I3SingleServiceFactory< I3RadeBasicSnowCorrectionService, I3SnowCorrectionServiceBase >
I3RadeBasicSnowCorrectionServiceFactory;
I3_SERVICE_FACTORY( I3RadeBasicSnowCorrectionServiceFactory )

typedef I3SingleServiceFactory< I3RadeMuonSnowCorrectionService, I3SnowCorrectionServiceBase >
I3RadeMuonSnowCorrectionServiceFactory;
I3_SERVICE_FACTORY( I3RadeMuonSnowCorrectionServiceFactory )

