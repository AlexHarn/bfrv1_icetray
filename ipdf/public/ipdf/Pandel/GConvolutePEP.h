#ifndef IPDF_PANDEL_GConvolutePEP_H
#define IPDF_PANDEL_GConvolutePEP_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file GConvolutePEP.h
    @version $Revision: 1.4 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <cmath>
#include <iostream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/Simple/PEPBase.h"
#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"
#include "ipdf/Utilities/PdfMath.h"
#include "ipdf/Pandel/HitProbability.h"

namespace IPDF {
namespace Pandel {

/**
    @brief Implementation of the PDF for the GConvoluted 
    pandel function - Pandel convoluted with a gaussian.

    This implementation uses only the depth of the receiver to 
    determine the ice properties.

    @note A new implementation has been provided, named GaussConvolutedPEP.
    Please use that implementation instead of this one.

    Implicit copy ctor, op= and dtor are correct (boost::shared_ptr).

    getIntPdf is implemented numerically by PEPBase using the GSL.
  */
template<typename IceModel>
class GConvolutePEP : public PEPBase< GConvolutePEP<IceModel> > {
public:
  typedef GConvolutePEP<IceModel> Self;
  typedef boost::shared_ptr<IceModel> IceModelPtr;
  typedef IceModel IceModelType;

  /**
   * @brief Normal ctor for PEP, taking an IceModel
   * Default: no arguments, new the IceModel, which will be deleted 
   * when the PEP is destroyed.
   * (root-cint doesn't recognize a new in a default parameter)
   */
  explicit GConvolutePEP(IceModelPtr ice=IceModelPtr(new IceModel()),
			 const double jitter=15.);

  /// @brief Calculate the PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getPdf(const PEHit& pehit, const EmissionHypothesis& eh) const {
    return exp(this->getLogPdf(pehit,eh));
  }
  /// @brief Calculate the log PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getLogPdf(const PEHit& pehit, const EmissionHypothesis& eh) const;

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
};


template<typename IceModel>
const double GConvolutePEP<IceModel>::DEFAULT_SIGMA=15.;

template<typename IceModel>
inline GConvolutePEP<IceModel>::GConvolutePEP(
		boost::shared_ptr<IceModel> ice,
		const double jitter)
 : sigma_(0.), defaultSigma_(jitter), ice_model_(ice)
{
  AssertX(std::isfinite(defaultSigma_),IPDF::ParameterValueIsNotFinite);
  if(defaultSigma_<=0.) defaultSigma_=DEFAULT_SIGMA;
}

template<typename IceModel>
template<typename OmReceiver>
inline void GConvolutePEP<IceModel>::setOmHardware(const OmReceiver& omr) const {
  sigma_ = omr.getJitter();
  AssertX(std::isfinite(sigma_),IPDF::ParameterValueIsNotFinite);
  if(sigma_<=0.) sigma_ = defaultSigma_;
}

/***************************************************************************/
template<typename IceModel>
template<typename PEHit, typename EmissionHypothesis>
inline double GConvolutePEP<IceModel>::getLogPdf(
    const PEHit& pehit,
    const EmissionHypothesis& emitter) const {

  const typename EmissionHypothesis::template EmissionGeometry<IceModel>
      egeom( emitter, pehit.getOmReceiver() );

  double propd = egeom.propagationDistance();
  this->setOmHardware( pehit.getOmReceiver() );
  double delay = pehit.getLeTime()-egeom.geometricalTime();
  double depth = pehit.getOmReceiver().getZ();
  double ice_tau_scale(1./ice_model_->TauScale(depth)+IPdfConstants::C_ICE_G*ice_model_->Absorptivity(depth));
  double lg_ice_tau_scale(log(ice_tau_scale));
  double result;

  const double bigtime=sigma_*25;
  const double sigma2=sigma_*sigma_;
  const double rhosigma = ice_tau_scale * sigma_;
  const double rhosigma2 = rhosigma * sigma_;
  const double lnrhosigma = log(rhosigma);
  const double eta=rhosigma-delay/sigma_;
  const double eta2=eta*eta;
  const double ksi = propd*ice_model_->InvEffScattLength(depth);
  const double lngammaksi = PdfMath::LogGamma(ksi);
  const double smalltime=-sigma_*5./(1+ksi); // 10 sigma or less if d>lambda


  if ((delay < smalltime) || (0.5*(ksi+1)>GSL_SF_GAMMA_XMAX) ){
    // too early, too far away: highly unlikely
    result = -PdfMath::FINFTY;
    // log_warn("minus infinity");
  }else if (ksi == 0){
    // OM is exactly on the track
    // avoid singularity in Gamma function
    // (maybe a better, more physical test would be something like:
    //  propd<PMTradius)
    result = - 0.5*delay*delay/sigma2 - 0.5*(M_LN2 + M_LNPI) - log(sigma_);
  }else if ( (delay < bigtime) && (eta2 < 570) ){
    const double lnsigma = log(sigma_);
    const double lnfactor = ksi*lnrhosigma
      + 0.5*(ksi-3)*M_LN2
      -0.5*M_LNPI
      -lngammaksi
      -lnsigma;
    const double ksip1d2=0.5*(ksi+1.);
    const double ksid2=0.5*ksi;
    const double gammaksi1 = PdfMath::Gamma(ksip1d2);
    const double gammaksi2 =PdfMath::Gamma(ksid2);
    const double f11_1=PdfMath::HyperGeometric1F1(ksid2,0.5,0.5*eta2);
    const double f11_2=PdfMath::HyperGeometric1F1(ksip1d2,1.5,0.5*eta2);
    const double argument= gammaksi2*f11_1
      -M_SQRT2*eta*gammaksi1*f11_2;
    if(!(std::isfinite(argument)&&(argument>0))){
        log_warn("arg=%g propd=%f delay=%f depth=%f lnfactor=%f f11_1=%g f11_2=%g gksi1=%g gksi2=%g ksi=%f",
                 argument,propd,delay,depth,lnfactor,f11_1,f11_2,gammaksi1,gammaksi2, ksi);
        result = -PdfMath::FINFTY;
    } else {
        result=lnfactor
          - 0.5*delay*delay/sigma2
          + log(argument);
        if (!std::isfinite(result)){
            log_warn("result=%f propd=%f delay=%f depth=%f lnfactor=%f f11_1=%f f11_2=%f",
                     result,propd,delay,depth,lnfactor,f11_1,f11_2);
            result = -PdfMath::FINFTY;
        }
    }
  }else{
    double lnserie= log(1.- (ksi-1)*rhosigma2/delay + 0.5*(ksi-1)*(ksi-2)*sigma2/(delay*delay) );
    result=ksi*lg_ice_tau_scale
      + (ksi-1)*log(delay)
      -lngammaksi
      -ice_tau_scale*delay
      + lnserie;
    if (!std::isfinite(result)){
        log_warn("result=%f propd=%f delay=%f depth=%f lnserie=%f",
                 result,propd,delay,depth,lnserie);
    }
  }
  return result;
}


/********************************************************************/
template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double GConvolutePEP<IceModel>::
  getHitProb(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::pHit(omr,eh,*ice_model_);
}

template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double GConvolutePEP<IceModel>::
  expectedNPE(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::meanNumPE(omr,eh,*ice_model_);
}
/********************************************************************/


} // namespace Pandel
} // namespace IPDF

template<typename IceModel>
inline std::ostream& operator<<(std::ostream& os, const IPDF::Pandel::GConvolutePEP<IceModel>&) {
  return (os<<"GConvolutePEP<"<<IceModel::name()<<">");
}

#endif // IPDF_PANDEL_GConvolutePEP_H
