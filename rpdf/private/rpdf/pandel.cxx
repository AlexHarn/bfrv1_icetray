/**
 * @brief functions for calculating photoelectron probabilities for muon reconstructions
 *
 * @copyright (C) 2018 The Icecube Collaboration
 *
 * @file pandel.cxx
 * @author Kevin Meagher
 * @date January 2018
 *
 */

#include <gsl/gsl_math.h> // ensures BSD math constants, e.g. M_PI
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_hyperg.h>
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_errno.h>
#include <boost/math/distributions/gamma.hpp>

#include "dataclasses/physics/I3Particle.h"
#include "rpdf/geometry.h"
#include "rpdf/pandel.h"

namespace {
  /// The number of coefficients fastConvolutedHyperg should precompute to use
  const uint32_t N_PRECOMP_COEFF=200;
  /// this gets used a lot so precompute this as well
  const double SQRTPI_INV = 1./sqrt(M_PI);

  ///Class to hold the coefficients that fastConvolutedHyperg needs
  //this class was copied from ipdf/private/Pandel/ConvolutedHyperg.cxx:Line 24
  class PrecomputedBCoefficients
  {
  public:
    PrecomputedBCoefficients() {
      b05_[0] = b15_[0] = 1.L;
      for (uint32_t i = 1; i < N_PRECOMP_COEFF; ++i) {
        b05_[i] = 1.L / (i * (0.5L + (i - 1)));
        b15_[i] = 1.L / (i * (1.5L + (i - 1)));
      }
    }
    double b05_[200];
    double b15_[200];
  };
  const PrecomputedBCoefficients cf;

  double fastHyperg1F1(const double a, const double z, const double* b)
  {
    //this function was copied from ipdf/private/Pandel/ConvolutedHyperg.cxx:Line 59
    double val = 1.;
    double term = 1.;
    double an = a;
    const double* end = b + N_PRECOMP_COEFF;
    // We're starting at n = 1 term, so advance b pointer
   for (++b; b < end; ++b) {
      term *= an * z * (*b);
      an += 1.;
      val += term;
      if (fabs(term)*1e17 < val) {
        return val;
      }
    }
    // Out of range.  The series would have converged by now over the
    // specified range.
    return val;
  }

}

double rpdf::pandel_sample(const double eff_distance, const IceModel& ice, I3RandomService &rng)
{
  return gsl_ran_gamma(rng.GSLRng(), eff_distance/ice.scattering_length, 1./ice.rho);
}

double rpdf::pandel_pdf(const double t_res,const double eff_distance, const IceModel& ice)
{
  if (t_res<=0){
    //the the boost call returns NAN for negative values of t_res
    //we need zero if this is going to work
    return 0;
  }
  if (eff_distance<=0){
    //this makes no sense physical sense but the formula for effective distance
    //can very occasionally go netitive so we need to be prepared
    return 0;
  }
  //the pandel function is the same as the gamma distribution so just call
  //the boost library function
  return ice.rho*boost::math::gamma_p_derivative
    (eff_distance/ice.scattering_length, t_res*ice.rho);
}

double rpdf::pandel_sf(const double t_res,const double eff_distance, const IceModel& ice)
{
  if (t_res <= 0){
    //the the gsl call returns NAN for negative values of t_res
    //we need zero if this is going to work
    return 1.0;
  }
  if (eff_distance<=0){
    //this makes no sense physical sense but the formula for effective distance
    //can very occasionally go netitive so we need to be prepared
    return 0;
  }
  return  boost::math::gamma_q(eff_distance/ice.scattering_length,t_res*ice.rho);
}

double rpdf::gslConvoluted1F1Diff(const double xi, const double eta)
{
  //this function was copied from ipdf/private/Pandel/ConvolutedHyperg.cxx:Line 8
  const double xi1 = 0.5*(xi+1);
  const double xi2 = 0.5*xi;
  const double eta2 = 0.5*eta*eta;

  const double f1 = gsl_sf_hyperg_1F1(xi2,0.5,eta2);
  const double f2 = gsl_sf_hyperg_1F1(xi1,1.5,eta2);
  return f1/gsl_sf_gamma(xi1) - eta*M_SQRT2*f2/gsl_sf_gamma(xi2);
}

double rpdf::gslConvolutedU(const double xi, const double eta)
{
  //this function was copied from ipdf/private/Pandel/ConvolutedHyperg.cxx:Line 49
  return SQRTPI_INV*gsl_sf_hyperg_U(0.5*xi, 0.5, 0.5*eta*eta);
}

/**
 *  @brief Compute the hypergeometric portion of the Gaussian convoluted
 *  Pandel PDF
 *
 *  1F1(0.5*xi,0.5,0.5*eta*eta)/gamma(0.5*(xi+1)) -
 *              sqrt2*eta*1F1(0.5*(xi+1),1.5,0.5*eta*eta)/gamma(0.5*xi)
 *  for 0.5*eta*eta < 100 and xi < 5 using the hypergeometric power series
 *  with the denominator precomputed.  This is >10x faster than GSL.
 */
double rpdf::fastConvolutedHyperg(const double xi, const double eta)
{
  //this function was copied from ipdf/private/Pandel/ConvolutedHyperg.cxx: Line 89
  double xi1 = 0.5*(xi+1);
  double xi2 = 0.5*xi;
  const double eta2 = eta*eta*0.5;

  const double f1 = fastHyperg1F1(xi2,eta2,cf.b05_);
  const double f2 = fastHyperg1F1(xi1,eta2,cf.b15_);
  return f1*exp(-lgamma(xi1)) - eta*M_SQRT2*f2*exp(-lgamma(xi2));
}

double rpdf::UnconvolutedPandel::pdf(const double t, const double d)const
{
  return rpdf::pandel_pdf(t,d,ice_model_);
}

double rpdf::UnconvolutedPandel::sf(const double t, const double d)const
{
  return rpdf::pandel_sf(t,d,ice_model_);
}

double rpdf::FastConvolutedPandel::pdf(const double delay,const double propd)const
{
  //this function was copied from ipdf/public/ipdf/Pandel/GaussConvolutedPEP.h: Line 213
  double sigma=jitter_;
  if (sigma<=0){
    return rpdf::pandel_pdf(delay,propd,ice_model_);
  }

  double result = NAN;

  const double ksi = propd/ice_model_.scattering_length;
  const double sigma2=sigma*sigma;
  const double delay2=delay*delay;
  const double rhosigma = ice_model_.rho * sigma;
  const double rho2sigma2 = rhosigma * rhosigma;
  const double lnrhosigma = log(rhosigma);
  const double eta = rhosigma-delay/sigma;
  const double eta2 = eta*eta;

  if ( ksi <= 0 ){
    // region zero: distance<0
    result = exp(- 0.5*delay2/sigma2 - 0.5*(M_LN2 + M_LNPI))/sigma;
  }
  else if ( (-5*sigma <= delay) && (delay<=30*sigma) && (ksi<=5) ){
    // Region 1: Evaluate the hypergeometric portion of the PDF
    result = exp(-0.5*delay2/sigma2 + ksi*(lnrhosigma-0.5*M_LN2))/(sigma*M_SQRT2);

    if (eta > 1.35) {
      // Region 1A: Use Tricomi's function for positive eta to maximize conditioning
      result *= rpdf::gslConvolutedU(ksi, eta);
    }
    else if ((0.5*eta2 < 100.) && (ksi > 0.05)) {
      //Region 1B: Use the fast power series
      result *= rpdf::fastConvolutedHyperg(ksi, eta);
    }
    else {
      //Region 1C: Evaluate using GSL's implementation of 1F1
      result *= rpdf::gslConvoluted1F1Diff(ksi, eta);
    }
  }
  else if ( ( ksi <= 1 ) && ( delay > 30.*sigma ) ) {
    // Approximation in region 2
    result = (exp(ksi*log(ice_model_.rho*delay)-ice_model_.rho*delay + 0.5*rho2sigma2 )
              /delay/gsl_sf_gamma(ksi));
  }
  else if ( ( ksi <= 1 ) && ( delay < -5.*sigma ) ){
    // Approximation in region 5
    result = pow(rhosigma/eta,ksi)*exp(-0.5*delay2/sigma2)/sigma/M_SQRT2/M_SQRTPI;
  }
  else {
    // Approximation in region 3 and 4
    // Note that a term ln(sigma) was taken out of the definition of alpha
    // (compared to definition (12) in the arXiv paper)
    // instead the final expression is divided by sigma
    // mathematically equivalent, but dimensionally cleaner.
    const double ksi21 = 2.*ksi-1;
    const double ksi212 = ksi21*ksi21;
    const double ksi213 = ksi212*ksi21;
    const double z=fabs(eta)/sqrt(2.*ksi21);
    const double sqrt1plusz2=sqrt(1+z*z);
    const double k=0.5*(z*sqrt1plusz2+log(z+sqrt1plusz2));
    const double beta=0.5*(z/sqrt1plusz2-1.);
    const double beta2 = beta*beta;
    const double beta3 = beta2*beta;
    const double beta4 = beta3*beta;
    const double beta5 = beta4*beta;
    const double beta6 = beta5*beta;
    const double n1 = (20.*beta3+30.*beta2+9.*beta)/12.;
    const double n2 = (6160.*beta6+18480.*beta5+19404.*beta4+8028.*beta3+945.*beta2)/288.;
    const double n3 = (27227200.*beta6+122522400.*beta5+220540320.*beta4
                       + 200166120.*beta3+94064328.*beta2+20546550.*beta+1403325.)*beta3/51840.;

    if ( eta <= 0 ){
      // t >= rho sigma^2, region 3
      const double alpha=(-0.5*delay2/sigma2+0.25*eta2-0.5*ksi+0.25+k*ksi21
                          -0.5*log(sqrt1plusz2)-0.5*ksi*M_LN2+0.5*(ksi-1.)*log(ksi21)+ksi*log(rhosigma));
      const double phi = 1. - n1/ksi21 + n2/ksi212 - n3/ksi213;
      result = exp(alpha)*phi/gsl_sf_gamma(ksi)/sigma;
    } else {
      // t <= rho sigma^2, region 4
      const double u = pow(2.*M_E/ksi21,ksi/2.)*exp(-0.25)/M_SQRT2;
      const double psi = 1. + n1/ksi21 + n2/ksi212 + n3/ksi213;
      const double cpandel= ( pow(rhosigma,ksi)/sigma
                              * exp(-0.5*delay2/sigma2+0.25*eta2)/(M_SQRT2*M_SQRTPI));
      result = cpandel * u * exp(-k*ksi21) * psi / sqrt(sqrt1plusz2);
    }
  }
  return result;
}

double rpdf::FastConvolutedPandel::sf(const double t, const double d)const
{
  //this function was copied from ipdf/public/ipdf/Pandel/GaussConvolutedPEP.h: Line 366
  //which was taken from from Dima's reconstruction suite:
  //http://icecube.wisc.edu/~dima/work/LBNL/reader/fat-reader/llhreco.cxx

  double sig=jitter_;
  //if no jitter just use the unconvoluted pandel function
  if (sig<=0){
    rpdf::pandel_sf(t,d,ice_model_);
  }

  //if the distance is less than zero (which can happen because of the
  //correction factor just evaluate the normal distribution
  if(d<=0){
    return (1-gsl_sf_erf(t/sig))/2;
  }

  const double rho = ice_model_.rho;
  const double xi  = d/ice_model_.scattering_length;
  //some sort of number to represent the precision of the integration
  const double highprecision=1;

  double a=sqrt(2)*sig;
  //ymin and ymax are the edges of the box with the same first and second
  //moment as the Gaussian
  double ymin=fmax(0., t-2.5*a);
  double ymax=fmax(0., t+2.5*a);

  //calculate the values of the pandel function at the most important places
  gsl_sf_result g0, g1, g2;
  //calculate the gamma distribution at the singularity
  if(gsl_sf_gamma_e(xi, &g0)!=GSL_SUCCESS) g0.val=1;
  //calculate the complementary cumulative gamma distribution at the edges of the box
  if(gsl_sf_gamma_inc_Q_e(xi, rho*ymin, &g1)!=GSL_SUCCESS) g1.val=0;
  if(gsl_sf_gamma_inc_Q_e(xi, rho*ymax, &g2)!=GSL_SUCCESS) g2.val=0;
  double result=(g1.val+g2.val)/2;

  //this is a common value so we calculate it first
  double Jf=pow(rho, xi)/(2*g0.val);

  //if xi is small then the singularity will contribute to the integral
  //and we need to calculate it
  double epsi=fmin(0.4/highprecision, ymax); // epsi=0;
  if(xi<1) if(ymin<epsi){
      gsl_sf_result er;
      if(gsl_sf_erf_e(t/a, &er)!=GSL_SUCCESS) er.val=0;
      double epxi=pow(epsi, xi);
      double ymxi=pow(ymin, xi);
      result-=Jf*(er.val*(epxi-ymxi)/xi-((sqrt(2/M_PI)/sig)*exp(-t*t/(2*sig*sig))+rho*er.val)*(epxi*epsi-ymxi*ymin)/(xi+1));
      ymin=epsi;
    }

  //now calculate the the integral inside the box in even increments
  if(ymin<ymax){
    bool flag=true; // flag=xi<1&&ymin<0.1;
    double zmin=ymin, zmax=ymax;
    if(flag){ zmin=pow(ymin, xi)/xi; zmax=pow(ymax, xi)/xi; }
    double zstp=(zmax-zmin)/(4*highprecision); // zstp/=100;
    for(double z=zmin+zstp/2; z<zmax; z+=zstp){
      double y=flag?pow(z*xi, 1/xi):z;
      gsl_sf_result er;
      if(gsl_sf_erf_e((t-y)/a, &er)!=GSL_SUCCESS) er.val=0;
      double I=(flag?1:pow(y, xi-1))*exp(-rho*y)*er.val;
      result-=Jf*zstp*I;
    }
  }

  //sometimes the result can be slightly above 1 or below 0
  //this will fix that
  if ( result > 1.0 ) result = 1.0;
  if ( result < 0.0 ) result = 0.0;

  return result;
}

double rpdf::SPEfunc::operator()(const rpdf::PhotoElectronProbability& p,
                                 const double t_res, const double d_eff,
                                 const double Npe) const
{
  // SPE is just the probability of the first hit so just return that
  return p.pdf(t_res,d_eff);
}

double rpdf::MPEfunc::operator()(const PhotoElectronProbability& p,
                                 const double t_res,const double d_eff,
                                 const double Npe) const
{
  //first get the probability of the first hit
  const double spepdf = p.pdf(t_res,d_eff);

  if (Npe < 1.5) {
    // if there is less than one and a half hits we know what the second half is
    //going to be 1, so don't bother to calculate it
    return spepdf;
  } else {
    //get the integral from t_res to infinity
    const double survival = p.sf(t_res,d_eff);
    // convert the number of photoelectrons to a whole integer, but still use double to store it
    const double iNpe = floor(Npe+0.5);
    // return the probability of the first hit time the probability of getting
    // N-1 for the rest of the event
    return spepdf*iNpe*std::pow(survival,iNpe-1);
  }
}
