/**
 *
 * @file I3SnowCorrectionService.cxx
 * @brief implementaration of the I3SnowCorrectionService class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
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
 * I3SimpleSnowCorrectionService
 * class for computing snow corrections
 */

/// default constructor for unit tests        
I3SimpleSnowCorrectionService::I3SimpleSnowCorrectionService(const std::string& name,
                                                             double fLambda) : 
  I3SnowCorrectionServiceBase(name),
  fLambda_(fLambda) {
  log_debug("Hello, I am a nearly empty SnowCorrectionService constructor for unit tests");           
}

/// Regular constructor
I3SimpleSnowCorrectionService::I3SimpleSnowCorrectionService(const I3Context &c):
  I3SnowCorrectionServiceBase(c){
  log_debug("Entering the constructor");
  fLambda_ = 2.1;
  AddParameter ("Lambda", "Snow attenuation length (lambda)", fLambda_);
}

void I3SimpleSnowCorrectionService::Configure(){
  GetParameter ("Lambda", fLambda_);
}

// Setter function for lambda
void I3SimpleSnowCorrectionService::ResetLambda(double newlambda){
  log_trace("Resetting lambda from %f to %f", fLambda_, newlambda);
  fLambda_ = newlambda;
}


/// THIS IS THE MEATY FUNCTION!
double I3SimpleSnowCorrectionService::AttenuationFactor(const I3Position& pos,
                                                        double snowDepth, 
                                                        const I3Particle& hypoth,
                                                        const I3LaputopParams&)
  const
{
  double cosz = fabs(hypoth.GetDir().GetZ());  // Always a positive number (downgoing)
  double slantdepth = snowDepth / cosz;
  return exp(-slantdepth / fLambda_);
}

void I3SimpleSnowCorrectionService::FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr diagnost,
                                                        I3ParticleConstPtr hypoth,
                                                        I3LaputopParamsConstPtr paramPtr)
  const
{
   log_debug("Attempting to fill Simple diagnostics...");

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
   
   diagnost->lambda_EM_30m = fLambda_;
   diagnost->lambda_EM_50m = fLambda_;
   diagnost->lambda_EM_80m = fLambda_;
   diagnost->lambda_EM_100m = fLambda_;
   diagnost->lambda_EM_125m = fLambda_;
   diagnost->lambda_EM_150m = fLambda_;
   diagnost->lambda_EM_200m = fLambda_;
   diagnost->lambda_EM_300m = fLambda_;
   diagnost->lambda_EM_500m = fLambda_;
   diagnost->lambda_EM_1000m = fLambda_;
   diagnost->lambda_EM_30m_restricted = fLambda_;
   diagnost->lambda_EM_50m_restricted = fLambda_;
   diagnost->lambda_EM_80m_restricted = fLambda_;
   diagnost->lambda_EM_100m_restricted = fLambda_;
   diagnost->lambda_EM_125m_restricted = fLambda_;
   diagnost->lambda_EM_150m_restricted = fLambda_;
   diagnost->lambda_EM_200m_restricted = fLambda_;
   diagnost->lambda_EM_300m_restricted = fLambda_;
   diagnost->lambda_EM_500m_restricted = fLambda_;
   diagnost->lambda_EM_1000m_restricted = fLambda_;

   log_debug("Done filling Simple diagnostics...%f", fLambda_);
 }

typedef I3SingleServiceFactory< I3SimpleSnowCorrectionService, I3SnowCorrectionServiceBase > 
I3SimpleSnowCorrectionServiceFactory;
I3_SERVICE_FACTORY( I3SimpleSnowCorrectionServiceFactory )
