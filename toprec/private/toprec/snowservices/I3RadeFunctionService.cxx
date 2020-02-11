/**
 *
 * @file I3RadeBasicFunctionService.cxx
 * @brief implementaration of the I3RadeBasicFunctionService class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id: I3BORSFunctionService.cxx 143395 2016-03-17 22:10:15Z hdembinski $
 *
 * @version $Revision: 143395 $
 * @date $Date: 2016-03-17 15:10:15 -0700 (Thu, 17 Mar 2016) $
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

namespace {
  // Helper
  double DistToAxis(const I3Particle& part,
                    const I3Position& pos)
  {
    I3Position v = pos - part.GetPos();
    const double d_axis = v * part.GetDir();
    const double ground_r2 = v.Mag2();
    return sqrt(ground_r2 - d_axis * d_axis);
  }
}

/// ----------- KR'S LATEST SNOW CORRECTION --------------
/// default constructor for unit tests
I3RadeBasicSnowCorrectionService::I3RadeBasicSnowCorrectionService(const std::string& name) :
  I3SnowCorrectionServiceBase(name) {
  log_debug("Hello, I am a nearly empty RadeBasicFunctionService constructor for unit tests");
}

/// Regular constructor
I3RadeBasicSnowCorrectionService::I3RadeBasicSnowCorrectionService(const I3Context &c):
  I3SnowCorrectionServiceBase(c){
  log_debug("Entering the constructor");
 }

// Nothing special
//void I3RadeBasicSnowCorrectionService::Configure(){
//}


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
  double l = Lambda(r, s125);
  double attfactor = exp(-slantdepth/l);
  return attfactor;
}

void I3RadeBasicSnowCorrectionService::FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr diagnost,
                                                      I3ParticleConstPtr hypoth,
                                                      I3LaputopParamsConstPtr paramPtr)
  const
{
   log_debug("Attempting to fill BORS diagnostics...");

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


typedef I3SingleServiceFactory< I3RadeBasicSnowCorrectionService, I3SnowCorrectionServiceBase >
I3RadeBasicSnowCorrectionServiceFactory;
I3_SERVICE_FACTORY( I3RadeBasicSnowCorrectionServiceFactory )

