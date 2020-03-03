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
 * I3BORSSnowCorrectionService
 * class for computing Kath's first attempt at an advanced snow correction: the BORS function
 */

/// ----------- BORS SNOW CORRECTION --------------
const std::string I3BORSSnowCorrectionService::EM_ONLY_TAG = "EMOnly";

/// default constructor for unit tests        
I3BORSSnowCorrectionService::I3BORSSnowCorrectionService(const std::string& name,
                                                         bool fEMonly) : 
  I3SnowCorrectionServiceBase(name),        
  fEMonly_(fEMonly),
  t_set_externally_(0) {                       
  log_debug("Hello, I am a nearly empty BORSCorrectionService constructor for unit tests");           
}

/// Regular constructor
I3BORSSnowCorrectionService::I3BORSSnowCorrectionService(const I3Context &c):
  I3SnowCorrectionServiceBase(c){
  log_debug("Entering the constructor");
  fEMonly_ = 0;
  t_set_externally_ = 0;
  AddParameter(EM_ONLY_TAG, "Are you feeding me the EM signal only? (boolean)", fEMonly_);
}

void I3BORSSnowCorrectionService::Configure(){
  GetParameter(EM_ONLY_TAG, fEMonly_);
}


/// THIS IS THE MEATY FUNCTION!
double I3BORSSnowCorrectionService::AttenuationFactor(const I3Position& pos,
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

  // Now, we don't have the true "evolution stage", but hopefully we've got an approximator in beta
  if (!t_set_externally_) {
    t_ = T_from_beta_zenith(beta, dir.GetZenith()); 
    // If we're outside the range of what's been tested, bring it inside and issue a warning
    if (t_> 9.0) { log_debug("t out of range (%f), bring it back to %f\n", t_, 9.0); t_= 9.0; }
    if (t_< -1.0) { log_debug("t out of range (%f), bring it back to %f\n", t_, -1.0); t_= -1.0; }
  }

  // Here are variables developed in scripts/snowtheory:
  const float r_nomorebump = 20;
 
  // Compute "c0"
  double c0 = 1.0;   // c0 = 1 means no production bump                            
  if (r < r_nomorebump) c0 = Turnover_c0(r, t_);

  // Compute "s_nose"
  double snose = -10.0;  // this default will keep things from blowing up
  if (r < r_nomorebump) snose = TurnoverExponentialSlope(r, t_);

  // Compute "s"
  double s = DominantExponentialSlope(r, t_);

  //double exp_of_me_p = S_SMOOTH*(logR-S_X0);
  //double log_of_this_p = 1+exp(exp_of_me_p);
  //double constant1_p = S_0 + (S_S2 - S_S1)*S_X0;
  //s_p = (S_S2 - S_S1)*log(log_of_this_p)/S_SMOOTH + S_S1*logR + constant1_p + S_EFACT*(logE-7.0);

  // Now combine 'em!
  double attfactor = c0 * exp(slantdepth*s) - (c0-1)*exp(slantdepth*snose);

  // Apply the attenuation to the signal
  double fEM;
  if (fEMonly_) fEM = 1.0;
  else fEM = FractionEM(r, log10(s125));

  return fEM * attfactor + (1.0 - fEM);
}

void I3BORSSnowCorrectionService::FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr diagnost,
                                                      I3ParticleConstPtr hypoth,
                                                      I3LaputopParamsConstPtr paramPtr)
  const
{
   log_debug("Attempting to fill BORS diagnostics...");

   // Once again, harvest relevant variables
   const double zenith = hypoth->GetZenith();
   const double logS125 = paramPtr->GetValue(Laputop::Parameter::Log10_S125);
   const double beta = paramPtr->GetValue(Laputop::Parameter::Beta);
   diagnost->tstage = T_from_beta_zenith(beta, zenith);
   diagnost->tstage_restricted = GetTStage();
   log_debug("Currently: tstage = %f (restricted to %f)", diagnost->tstage, diagnost->tstage_restricted);
  
   diagnost->fEM_30m = FractionEM(30, logS125);
   diagnost->fEM_50m = FractionEM(50, logS125);
   diagnost->fEM_80m = FractionEM(80, logS125);
   diagnost->fEM_100m = FractionEM(100, logS125);
   diagnost->fEM_125m = FractionEM(125, logS125);
   diagnost->fEM_150m = FractionEM(150, logS125);
   diagnost->fEM_200m = FractionEM(200, logS125);
   diagnost->fEM_300m = FractionEM(300, logS125);
   diagnost->fEM_500m = FractionEM(500, logS125);
   diagnost->fEM_1000m = FractionEM(1000, logS125);
  
   diagnost->lambda_EM_30m = -1.0/(DominantExponentialSlope(30, diagnost->tstage));
   diagnost->lambda_EM_50m = -1.0/(DominantExponentialSlope(50, diagnost->tstage));
   diagnost->lambda_EM_80m = -1.0/(DominantExponentialSlope(80, diagnost->tstage));
   diagnost->lambda_EM_100m = -1.0/(DominantExponentialSlope(100, diagnost->tstage));
   diagnost->lambda_EM_125m = -1.0/(DominantExponentialSlope(125, diagnost->tstage));
   diagnost->lambda_EM_150m = -1.0/(DominantExponentialSlope(150, diagnost->tstage));
   diagnost->lambda_EM_200m = -1.0/(DominantExponentialSlope(200, diagnost->tstage));
   diagnost->lambda_EM_300m = -1.0/(DominantExponentialSlope(300, diagnost->tstage));
   diagnost->lambda_EM_500m = -1.0/(DominantExponentialSlope(500, diagnost->tstage));
   diagnost->lambda_EM_1000m = -1.0/(DominantExponentialSlope(1000, diagnost->tstage));
  
   diagnost->lambda_EM_30m_restricted = -1.0/(DominantExponentialSlope(30, diagnost->tstage_restricted));
   diagnost->lambda_EM_50m_restricted = -1.0/(DominantExponentialSlope(50, diagnost->tstage_restricted));
   diagnost->lambda_EM_80m_restricted = -1.0/(DominantExponentialSlope(80, diagnost->tstage_restricted));
   diagnost->lambda_EM_100m_restricted = -1.0/(DominantExponentialSlope(100, diagnost->tstage_restricted));
   diagnost->lambda_EM_125m_restricted = -1.0/(DominantExponentialSlope(125, diagnost->tstage_restricted));
   diagnost->lambda_EM_150m_restricted = -1.0/(DominantExponentialSlope(150, diagnost->tstage_restricted));
   diagnost->lambda_EM_200m_restricted = -1.0/(DominantExponentialSlope(200, diagnost->tstage_restricted));
   diagnost->lambda_EM_300m_restricted = -1.0/(DominantExponentialSlope(300, diagnost->tstage_restricted));
   diagnost->lambda_EM_500m_restricted = -1.0/(DominantExponentialSlope(500, diagnost->tstage_restricted));
   diagnost->lambda_EM_1000m_restricted = -1.0/(DominantExponentialSlope(1000, diagnost->tstage_restricted));
}


// ----------------- HELPER FUNCTIONS -------------------
double I3BORSSnowCorrectionService::DominantExponentialSlope(double r, double t)
  const
{
 // First, the single semilog for largeR slope:                               
  float s_c2 = -0.259064;
  float s_tfactor = 0.0132683;
  float s_slope2 = -0.11575;

  // Compute "s"
  double logR = log10(r);
  if (r < 0.0001) logR = log10(0.0001);  // keep it from blowing up                 
  return s_c2 + s_tfactor*t + s_slope2*logR;
}

double I3BORSSnowCorrectionService::Turnover_c0(double r, double t)
  const
{
  // Next, c0                                                                              
  float c0_intercept = 5.57464;
  float c0_tfactor = 0.489383;
  float c0_slope = 3.62882;
  return c0_intercept * exp(-r/(c0_slope + c0_tfactor*t)) + 1.0;
}

double I3BORSSnowCorrectionService::TurnoverExponentialSlope(double r, double t)
  const
{
  // Then, snose                                                               
  float snose_constant = -0.317313;
  float snose_Tlinearfactor = 0.0311667;
  float snose_Rlinearfactor = -0.0456633;
  return snose_constant + (snose_Tlinearfactor*log10(t+1.0) + snose_Rlinearfactor)*r;
}

double I3BORSSnowCorrectionService::T_from_beta_zenith(double beta, double zenith)
  const
{
  // We'll start with something insanely simple:
  // From "find_tstage_humanfunction_fordata.C"
  //TF2 *f2 = new TF2("f2","[0]+[1]*y+[2]*x+[3]*y*y+[4]*x*x",1.0,5.0,0.4,1.0);
  // ... where "x" is beta, and "y" is cos(zenith)
  //1  p0           7.14282e+01   7.31410e-01   8.35995e-04  -2.71980e-09
  //2  p1          -1.11804e+02   1.98609e+00   1.15049e-03  -3.95265e-09
  //3  p2          -4.92277e+00   1.99127e-01   2.61213e-04  -1.74091e-08
  //4  p3           5.36211e+01   1.40033e+00   1.43972e-03  -3.94822e-09
  //5  p4           6.27008e-01   3.24281e-02   6.78315e-05  -6.70407e-09
  double p0 = 7.14282e+01;
  double p1 = -1.11804e+02;
  double p2 = -4.92277;
  double p3 = 5.36211e+01;
  double p4 = 6.27008e-01;
  double x = beta;
  double y = cos(zenith);
  //return p0 + p1*zenith + p2*beta;
  
  // Issue warnings if we're in untested territory
  if (beta<1.0 || beta>5.0) log_debug("This event has a beta out of range of what was fitted (1-5): %f", beta);
  if (zenith>1.2) log_debug("This event has a zenith out of range of what was fitted (0-1.2): %f", zenith);

  return p0 + p1*y + p2*x + p3*y*y + p4*x*x;
}

double I3BORSSnowCorrectionService::FractionEM(double r, double logS125)
  const
{
  // A theory:  EM and mu are both parabolas in log(signal) vs. log(dist).
  // So their ratio has six paramters.
  // Each of these six parameters is a function of S125 (at least, at theta=0 and 25 deg, which are
  // combined to get this function.  Thus function is NOT tested on high zenith angles!)
  
  // First, get the six parameters from S125, each of which is from a linear fit:
  // From "EMfraction/find_S125based_ratiofunction.C"
  //p0                        =      3.02473   +/-   0.161198    
  //p1                        =      1.02027   +/-   0.137401    

  //p0                        =   -0.0996452   +/-   0.0475392   
  //p1                        =     0.203676   +/-   0.0405212   

  //p0                        =    -0.789106   +/-   0.00998644  
  //p1                        =   -0.0293398   +/-   0.00851217  

  //p0                        =     0.322255   +/-   0.0876411   
  //p1                        =     0.840248   +/-   0.0747029   

  //p0                        =    -0.217168   +/-   0.0624597   
  //p1                        =     0.337357   +/-   0.0532389   

  //p0                        =    -0.271973   +/-   0.022922    
  //p1                        =    -0.142145   +/-   0.0195381   
  double par0 = 3.02473 + 1.02027*logS125;
  double par1 = -0.0996452 + 0.203676*logS125;
  double par2 = -0.789106 + -0.0293398*logS125;
  double par3 =  0.322255 + 0.840248*logS125;
  double par4 = -0.217168 + 0.337357*logS125;
  double par5 = -0.271973 + -0.142145*logS125;

  // Now for the signal
  double xx = log10(r);
  double sEM = pow(10, par0 + par1*xx + par2*xx*xx);
  double smu = pow(10, par3 + par4*xx + par5*xx*xx);
  //printf("sEM=%f, smu=%f\n", sEM, smu);
  return sEM/(sEM+smu);
}

typedef I3SingleServiceFactory< I3BORSSnowCorrectionService, I3SnowCorrectionServiceBase > 
I3BORSSnowCorrectionServiceFactory;
I3_SERVICE_FACTORY( I3BORSSnowCorrectionServiceFactory )
