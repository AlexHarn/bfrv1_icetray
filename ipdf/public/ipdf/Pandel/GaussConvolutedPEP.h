#ifndef IPDF_PANDEL_GaussConvolutedPEP_H
#define IPDF_PANDEL_GaussConvolutedPEP_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file GaussConvolutedPEP.h
    @version $Revision$
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
    @author David Boersma <boersma@icecube.wisc.edu>
    @author George Japaridze <george.japaridze@gmail.com>
*/

#include <cmath>
#include <iostream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <gsl/gsl_sf_erf.h>

#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/Simple/PEPBase.h"
#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"
#include "ipdf/Utilities/PdfMath.h"
#include "ipdf/Pandel/HitProbability.h"
#include "ipdf/Pandel/LayeredIce.h"
#include "ipdf/Pandel/ConvolutedHyperg.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <boost/shared_ptr.hpp>

namespace IPDF {
namespace Pandel {

    // default implementation: ignore depth
    template<typename ice>
    double GetAbsorptivity(typename boost::shared_ptr<ice> iceptr, double omrdepth, double emissiondepth){
        return iceptr->Absorptivity();
    }

    // default implementation: ignore depth
    template<typename ice>
    double GetInvEffScattLength(typename boost::shared_ptr<ice> iceptr, double omrdepth, double emissiondepth){ 
        return iceptr->InvEffScattLength();
    }

    // specialization: use depth of both OmReceiver and EmissionHypothesis
    template<typename ice>
    double GetAbsorptivity(typename boost::shared_ptr< LayeredIce<ice> > iceptr, double omr_depth, double emission_depth){ 
        return iceptr->Absorptivity(omr_depth, emission_depth);
    }

    // specialization: use depth of both OmReceiver and EmissionHypothesis
    template<typename ice, typename OmReceiver, typename EmissionGeometry>
    double GetInvEffScattLength(typename boost::shared_ptr< LayeredIce<ice> > iceptr, double omr_depth, double emission_depth){ 
        return iceptr->InvEffScattLength(omr_depth, emission_depth);
    }

  /**
   * The different ways to compute the integral are now configured with an
   * enum. To stay in style, I should probably do this with an extra template
   * parameter, but that is tricky and so before I change the interface
   * I wait for input from gurus.
   */
  enum IntCalcTypes {
      IntFastPlain=0, /// quick approximation using the plain Pandel analytical integral
      IntLookupTable, /// OBSOLETE AND DISABLED
      IntSlowNumeric, /// do the numeric integral over and over again
      IntFastApproximation,  /// fast way dima does it
      NIntCalc        /// number of ways to compute/approximate the cPandel integral
  };

/**
    @brief Implementation of the PDF for the partly Gauss convoluted 
    pandel function - the Pandel PDF convoluted with a Gaussian.
    The integral of the PDF is computed without convolution.

    This implementation uses only the depth of the receiver to 
    determine the ice properties.

    The implementation is described and motivated in detail in
    Astroparticle Physics 28 (2007) 456-462, December 2007, also
    electronically available as http://arxiv.org/abs/0704.1706

    The IcePack implementation was used for reference, comparison and
    more specifics about the definitions of the five regions.

    Implicit copy ctor, op= and dtor are correct (boost::shared_ptr).

  */
template<typename IceModel>
class GaussConvolutedPEP : public PEPBase< GaussConvolutedPEP<IceModel> > {
public:
  typedef GaussConvolutedPEP<IceModel> Self;
  typedef boost::shared_ptr<IceModel> IceModelPtr;
  typedef IceModel IceModelType;

  /**
   * @brief Normal ctor for PEP, taking an IceModel
   * Default: no arguments, new the IceModel, which will be deleted 
   * when the PEP is destroyed.
   * @param[in] ice shared pointer to ice model object (e.g. H2)
   * @param[in] jitter PMT jitter in nanoseconds
   * @param[in] intcalc See the enum IntCalcTypes
   */
  explicit GaussConvolutedPEP(IceModelPtr ice=IceModelPtr(new IceModel()),
			 const double jitter=15., int intcalc=IntFastPlain);

  /// @brief Calculate the PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getPdf(const PEHit& pehit, const EmissionHypothesis& eh) const;

  /// @brief Calculate the log PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getLogPdf(const PEHit& pehit, const EmissionHypothesis& eh) const {
    double pdf = this->getPdf(pehit,eh);
    return ( (pdf>0) ? log(pdf) : -PdfMath::FINFTY );
  }

  /// @brief Calculate the cumulative PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getIntPdf(const PEHit& pehit, const EmissionHypothesis& eh) const {
    const typename EmissionHypothesis::template EmissionGeometry<IceModel> 
	egeom( eh, pehit.getOmReceiver() );
    double propd = egeom.propagationDistance();
    double delay = pehit.getLeTime()-egeom.geometricalTime();
    if ( intCalc_ == IntFastPlain ){
      if(delay < 0.) { return 0.; }
      double omrdepth = pehit.getOmReceiver().getZ();
      double emdepth = egeom.EmissionZCoordinate();
      double rdist = propd * GetInvEffScattLength(ice_model_,omrdepth,emdepth);
      double rho = 1./ice_model_->TauScale()
                   + IPdfConstants::C_ICE_G * GetAbsorptivity(ice_model_,omrdepth,emdepth);
      return PdfMath::IncGammaP(rdist,delay*rho);
    }
    if ( intCalc_ == IntSlowNumeric ){
      return PEPBase<GaussConvolutedPEP<IceModel> >::getIntPdf(pehit,eh);
    }

    if ( intCalc_ == IntFastApproximation ){
      return GetFastApproximation(pehit,eh);
    }

    AssertX(false,NotImplemented);
    return 0.;
  }


  template<typename PEHit, typename EmissionHypothesis>
    double GetFastApproximation(const PEHit& pehit, const EmissionHypothesis& eh) const;

  /// @brief Calculate the probability of at least one photo-electron hit on an OM
  template<typename OmReceiver, typename EmissionHypothesis>
  double getHitProb(const OmReceiver& omr, const EmissionHypothesis& eh) const;

  /// @brief Mean number of expected photo-electron hits on an OM
  template<typename OmReceiver, typename EmissionHypothesis>
  double expectedNPE(const OmReceiver& omr, const EmissionHypothesis& eh) const;

  /// @brief Change the OmReceiver being used to calculate the PDF
  ///
  /// This is useful, since as default this doesn't need to change if
  /// the PMT parameters (i.e. jitter and sensitivity) aren't being used.
  /// If They are being used then this method allows the code to apply
  /// the correct OM parameters when calculating the PDF.
  template<typename OmReceiver>
  void setOmHardware(const OmReceiver&) const;

private:

  mutable double sigma_;
  double defaultSigma_;
  IceModelPtr ice_model_;
  static const double DEFAULT_SIGMA;
  int intCalc_;
  SET_LOGGER( "ipdf" );

};

template<typename IceModel>
const double GaussConvolutedPEP<IceModel>::DEFAULT_SIGMA=15.;

template<typename IceModel>
inline GaussConvolutedPEP<IceModel>::GaussConvolutedPEP(
		boost::shared_ptr<IceModel> ice,
		const double jitter, int intcalc)
 : sigma_(0.), defaultSigma_(jitter), ice_model_(ice),intCalc_(intcalc)
{
  AssertX(std::isfinite(defaultSigma_),IPDF::ParameterValueIsNotFinite);
  AssertX(intCalc_>=0,IPDF::IllegalInputParameter);
  if (intCalc_ == IntLookupTable ){
    log_fatal("integral lookup table has been disabled "
              "(after a 2 year obsolescence period)");
  }
  AssertX(intCalc_ != IntLookupTable,IPDF::IllegalInputParameter); // now obsolete
  AssertX(intCalc_<NIntCalc,IPDF::IllegalInputParameter);
  if(defaultSigma_<=0.) defaultSigma_=DEFAULT_SIGMA;
}

template<typename IceModel>
template<typename OmReceiver>
inline void GaussConvolutedPEP<IceModel>::setOmHardware(const OmReceiver& omr) const {
  sigma_ = omr.getJitter();
  AssertX(std::isfinite(sigma_),IPDF::ParameterValueIsNotFinite);
  if(sigma_<=0.) sigma_=defaultSigma_;
}

/***************************************************************************/
template<typename IceModel>
template<typename PEHit, typename EmissionHypothesis>
inline double GaussConvolutedPEP<IceModel>::getPdf(
    const PEHit& pehit,
    const EmissionHypothesis& emitter) const {

  const typename EmissionHypothesis::template EmissionGeometry<IceModel>
      egeom( emitter, pehit.getOmReceiver() );

  const double propd = egeom.propagationDistance();
  this->setOmHardware( pehit.getOmReceiver() );
  double delay = pehit.getLeTime()-egeom.geometricalTime();
  // const double maxdelay = 3500.;
  // const double mindelay = -sigma_*25;
  // const double maxksi = 50.;
  // const double depth = pehit.getOmReceiver().getZ();
  double omrdepth = pehit.getOmReceiver().getZ();
  double emdepth = egeom.EmissionZCoordinate();
  double ksi = propd*GetInvEffScattLength(ice_model_,omrdepth,emdepth);
  double result = 1.0;

  // if delay or distance is out of bounds,
  // then compute the value at boundary of the valid range
  // and multiply with a dimensionless factor that goes asymptotically
  // to zero.
  // if ( mindelay > delay ){
    // result /= 1.0 + (mindelay - delay)/sigma_;
    // delay = mindelay;
  // }
  // if ( delay > maxdelay ){
    // result /= 1.0 + (delay - maxdelay)/sigma_;
    // delay = maxdelay;
  // }
  // if ( ksi > maxksi ){
    // result /= 1.0 + ksi - maxksi;
    // ksi = maxksi;
  // }

  const double ice_tau_scale = 1./ice_model_->TauScale()+IPdfConstants::C_ICE_G*GetAbsorptivity(ice_model_,omrdepth,emdepth);
  // const double lg_ice_tau_scale(log(ice_tau_scale));
  const double sigma2=sigma_*sigma_;
  const double delay2=delay*delay;
  const double rhosigma = ice_tau_scale * sigma_;
  const double rho2sigma2 = rhosigma * rhosigma;
  // const double rhosigma2 = rhosigma * sigma_;
  const double lnrhosigma = log(rhosigma);
  const double eta = rhosigma-delay/sigma_;
  const double eta2 = eta*eta;

  // const double lngammaksi = PdfMath::LogGamma(ksi);

  if ( ksi <= 0 ){
    // log_debug( "region 0, ksi=%g", ksi );
    // zero distance
    result *= exp(- 0.5*delay2/sigma2 - 0.5*(M_LN2 + M_LNPI))/sigma_;
  } else if ( (-5*sigma_ <= delay) && (delay<=30*sigma_) && (ksi<=5) ){

    /* Region 1 */

    result *=
        exp(-0.5*delay2/sigma2 + ksi*(lnrhosigma-0.5*M_LN2))/(sigma_*M_SQRT2);

    // Evaluate the hypergeometric portion of the PDF
    if (eta > 1.35) {
      // Use Tricomi's function for positive eta to maximize conditioning
      result *= gslConvolutedU(ksi, eta);
    } else if (0.5*eta2 < 100. && ksi > 0.05) {
      // Use the fast power series
      result *= fastConvolutedHyperg(ksi, eta);
    } else {
      // Evaluate using two 1F1 terms
      result *= gslConvoluted1F1Diff(ksi, eta);
    }

  } else if ( ( ksi <= 1 ) && ( delay > 30.*sigma_ ) ) {
      // Approximation in region 2
    // log_debug( "region 2: sigma=%g 1-ksi=%g delay-30sigma=%g",
    //            sigma_, 1.-ksi, delay-30.*sigma_ );
    // Nick:
    // pandel=pow(rho,ksi)*pow(tres,(ksi-1.))*exp(-rho*tres)/TMath::Gamma(ksi);
    // cpandel=exp(rho*rho*sigma*sigma/2.)*pandel;
    // result *= pow(ice_tau_scale,ksi)*pow(delay,(ksi-1.))*exp(-ice_tau_scale*delay)/PdfMath::Gamma(ksi);
    // result *= exp(ice_tau_scale*ice_tau_scale*sigma_*sigma_/2.);
    result *= exp(ksi*log(ice_tau_scale*delay)-ice_tau_scale*delay + 0.5*rho2sigma2 );
    result /= delay*PdfMath::Gamma(ksi);
  } else if ( ( ksi <= 1 ) && ( delay < -5.*sigma_ ) ){
      // Approximation in region 5
    // log_debug( "region 5: 1-ksi=%g -delay-5sigma=%g",
    //            1.-ksi, -delay-5*sigma_ );
    result *= pow(rhosigma/eta,ksi);
    result *= exp(-0.5*delay2/sigma2);
    result /= sigma_*M_SQRT2*M_SQRTPI;
  } else {
    // Approximation in region 3 and 4
    // Note that a term ln(sigma) was taken out of the definition of alpha 
    // (compared to definition (12) in the arXiv paper)
    // instead the final expression is divided by sigma
    // mathematically equivalent, but dimensionally cleaner.
    double ksi21 = 2.*ksi-1;
    double ksi212 = ksi21*ksi21;
    double ksi213 = ksi212*ksi21;
    double z=fabs(eta)/sqrt(2.*ksi21);
    double sqrt1plusz2=sqrt(1+z*z);
    double k=0.5*(z*sqrt1plusz2+log(z+sqrt1plusz2));
    double beta=0.5*(z/sqrt1plusz2-1.);
    double beta2 = beta*beta;
    double beta3 = beta2*beta;
    double beta4 = beta3*beta;
    double beta5 = beta4*beta;
    double beta6 = beta5*beta;
    double n1 = (20.*beta3+30.*beta2+9.*beta)/12.;
    double n2 = (6160.*beta6+18480.*beta5+19404.*beta4+8028.*beta3+945.*beta2)/288.;
    double n3 = 27227200.*beta6+122522400.*beta5+220540320.*beta4;
    n3 += 200166120.*beta3+94064328.*beta2+20546550.*beta+1403325.;
    n3 *= beta3/51840.;

    if ( eta <= 0 ){
      // log_debug( "region 3: eta=%g ksi-5=%g ksi-1=%g -delay-5sigma=%g delay-30*sigma=%g",
      //            eta, ksi-5., ksi-1., -delay-5.*sigma_, delay-30.*sigma_ );
      //
      // t >= rho sigma^2, region 3
      double alpha=-0.5*delay2/sigma2+0.25*eta2-0.5*ksi+0.25+k*ksi21;
      alpha+=-0.5*log(sqrt1plusz2)-0.5*ksi*M_LN2+0.5*(ksi-1.)*log(ksi21)+ksi*log(rhosigma);
      double phi = 1. - n1/ksi21 + n2/ksi212 - n3/ksi213;
      result *= exp(alpha)*phi/PdfMath::Gamma(ksi);
      result /= sigma_;
    } else {
      // log_debug( "region 4: eta=%g ksi-5=%g ksi-1=%g -delay-5sigma=%g delay-30*sigma=%g",
      //            eta, ksi-5., ksi-1., -delay-5.*sigma_, delay-30.*sigma_ );
      //
      // t <= rho sigma^2, region 4
      double u = pow(2.*M_E/ksi21,ksi/2.)*exp(-0.25)/M_SQRT2;
      double psi = 1. + n1/ksi21 + n2/ksi212 + n3/ksi213;
      double cpandel= pow(rhosigma,ksi)/sigma_;
      cpandel *= exp(-0.5*delay2/sigma2+0.25*eta2)/(M_SQRT2*M_SQRTPI);
      result *= cpandel * u * exp(-k*ksi21) * psi / sqrt(sqrt1plusz2);
    }
  }
  if ( !std::isnormal(result) || (result < FLT_MIN) ){
      log_debug("replacing result=%g by %g", result, FLT_MIN );
      result = FLT_MIN;
  }
  log_trace("delay=%f propd=%g ksi=%f sigma=%f pdf=%g",
            delay, propd, ksi, sigma_, result );
  return result;
}


/***
 * this code was takin from Dima's reconstruction suite:
 * http://icecube.wisc.edu/~dima/work/LBNL/reader/fat-reader/llhreco.cxx
 */

 template<typename IceModel>
   template<typename PEHit, typename EmissionHypothesis>
   inline double GaussConvolutedPEP<IceModel>::GetFastApproximation(const PEHit& pehit, const EmissionHypothesis& emitter) const{
   
   const typename EmissionHypothesis::template EmissionGeometry<IceModel>
     egeom( emitter, pehit.getOmReceiver() );

   const double d = egeom.propagationDistance();               //propagation distance or R in paper
   const double t = pehit.getLeTime()-egeom.geometricalTime(); //propagation delay or t in paper
   this->setOmHardware( pehit.getOmReceiver() );
   const double sig=sigma_;                                    //jitter time or \sigma

   if(d<=0){
     gsl_sf_result er;
     if(gsl_sf_erf_e(t/sig, &er)!=GSL_SUCCESS){
         er.val=(t>0)?1.:0.;
     }
     return (1.+er.val)/2;
   }

   //double l=(lf+ls*d)*this->l;
   //double la=(laf+las*d)*this->la;
   //double xi=d/l;
   //double cm=c/ng;
   //double rho=1/tau+cm/la;
   // const double depth = pehit.getOmReceiver().getZ();          //verticle location in ice
   const double omrdepth = pehit.getOmReceiver().getZ();
   const double emdepth = egeom.EmissionZCoordinate();
   const double rho = 1./ice_model_->TauScale()+IPdfConstants::C_ICE_G*GetAbsorptivity(ice_model_,omrdepth,emdepth); // \rho from the paper
   const double xi = d*GetInvEffScattLength(ice_model_,omrdepth,emdepth);   // \xi or squigma in the paper
   const double highprecision=1;                               //some sort of number to represent the precission of the integration

   if(sig==0){
     if(t>0){
       gsl_sf_result g0;
       if(gsl_sf_gamma_inc_P_e(xi, rho*t, &g0)!=GSL_SUCCESS) g0.val=1;
       return g0.val;
     }
     else return 0;
   }
   //double eta=rho*sig-t/sig;
   
   double a=sqrt(2)*sig;
   double ymin=fmax(0., t-2.5*a);
   double ymax=fmax(0., t+2.5*a);
   
   gsl_sf_result g0, g1, g2;
   if(gsl_sf_gamma_e(xi, &g0)!=GSL_SUCCESS) g0.val=1;
   if(gsl_sf_gamma_inc_Q_e(xi, rho*ymin, &g1)!=GSL_SUCCESS) g1.val=0;
   if(gsl_sf_gamma_inc_Q_e(xi, rho*ymax, &g2)!=GSL_SUCCESS) g2.val=0;
   double result=(g1.val+g2.val)/2;
   
   double Jf=pow(rho, xi)/(2*g0.val);
   double epsi=fmin(0.4/highprecision, ymax); // epsi=0;
   if(xi<1) if(ymin<epsi){
       gsl_sf_result er;
       if(gsl_sf_erf_e(t/a, &er)!=GSL_SUCCESS) er.val=0;
       double epxi=pow(epsi, xi);
       double ymxi=pow(ymin, xi);
       result-=Jf*(er.val*(epxi-ymxi)/xi-((sqrt(2/M_PI)/sig)*exp(-t*t/(2*sig*sig))+rho*er.val)*(epxi*epsi-ymxi*ymin)/(xi+1));
       ymin=epsi;
     }
   
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

   if ( result > 1.0 ) result = 1.0;
   if ( result < 0.0 ) result = 0.0;

   return 1.-result;
 }
															    
/********************************************************************/
template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double GaussConvolutedPEP<IceModel>::
  getHitProb(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::pHit(omr,eh,*ice_model_);
}

template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double GaussConvolutedPEP<IceModel>::
  expectedNPE(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::meanNumPE(omr,eh,*ice_model_);
}
/********************************************************************/


} // namespace Pandel
} // namespace IPDF

template<typename IceModel>
inline std::ostream& operator<<(std::ostream& os, const IPDF::Pandel::GaussConvolutedPEP<IceModel>&) {
  return (os<<"GaussConvolutedPEP<"<<IceModel::name()<<">");
}

#endif // IPDF_PANDEL_GaussConvolutedPEP_H
