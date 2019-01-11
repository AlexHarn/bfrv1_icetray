#ifndef IPDF_PANDEL_BoxConvolutedPEP_H
#define IPDF_PANDEL_BoxConvolutedPEP_H

/**
    copyright  (C) 2008
    the icecube collaboration
    $Id$

    @file BoxConvolutedPEP.h
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

#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/Simple/PEPBase.h"
#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"
#include "ipdf/Utilities/PdfMath.h"
#include "ipdf/Pandel/HitProbability.h"

namespace IPDF {
namespace Pandel {

/**
    @brief Implementation of the PDF for the "box-convoluted"
    pandel function - Pandel convoluted with a rectangular
    jitter function (box function).

    This implementation uses only the depth of the receiver to 
    determine the ice properties. It was copied from GConvolutedPEP,
    written by Simon, then the essential formulae for the PDF and its
    cumulative were replaced by the "box-convoluted" Pandel, derived
    by George.

    The convolution with a box is analytically and computationally much
    easier to do than the Gauss convolution, but it's probably not as
    good. So for now the box convoluted Pandel mostly a study and
    reference PDF.

  */
template<typename IceModel>
class BoxConvolutedPEP : public PEPBase< BoxConvolutedPEP<IceModel> > {
public:
  typedef BoxConvolutedPEP<IceModel> Self;
  typedef boost::shared_ptr<IceModel> IceModelPtr;
  typedef IceModel IceModelType;

  /// @brief Normal ctor for PEP, taking an IceModel
  ///
  /// Default: no arguments, new the IceModel, which will be deleted 
  /// when the PEP is destroyed.
  explicit BoxConvolutedPEP(IceModelPtr ice=IceModelPtr(new IceModel()),
			 const double jitter=15.);

  /// @brief Calculate the PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getPdf(const PEHit& pehit, const EmissionHypothesis& eh) const;

  /// @brief Calculate the log PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getLogPdf(const PEHit& pehit, const EmissionHypothesis& eh) const {
    double result = -PdfMath::FINFTY;
    double pdf = getPdf(pehit,eh);
    if ( pdf > 0 ){
      result = log(pdf);
    }
    return result;
  }

  /// @brief Calculate the integrated PDF via a lookup table
  template<typename PEHit, typename EmissionHypothesis>
  double getIntPdf(const PEHit& pehit, const EmissionHypothesis& eh) const;

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
const double BoxConvolutedPEP<IceModel>::DEFAULT_SIGMA=15.;

template<typename IceModel>
inline BoxConvolutedPEP<IceModel>::BoxConvolutedPEP(
		boost::shared_ptr<IceModel> ice,
		const double jitter)
 : sigma_(0.), defaultSigma_(jitter), ice_model_(ice)
{
  AssertX(std::isfinite(defaultSigma_),IPDF::ParameterValueIsNotFinite);
  if(defaultSigma_<=0.) defaultSigma_=DEFAULT_SIGMA;
}

template<typename IceModel>
template<typename OmReceiver>
inline void BoxConvolutedPEP<IceModel>::setOmHardware(const OmReceiver& omr) const {
  sigma_ = omr.getJitter();
  AssertX(std::isfinite(sigma_),IPDF::ParameterValueIsNotFinite);
  if(sigma_<=0.) sigma_=defaultSigma_;
}

/***************************************************************************/
template<typename IceModel>
template<typename PEHit, typename EmissionHypothesis>
inline double BoxConvolutedPEP<IceModel>::getPdf(
    const PEHit& pehit,
    const EmissionHypothesis& emitter) const {
  const typename EmissionHypothesis::template EmissionGeometry<IceModel>
      egeom( emitter, pehit.getOmReceiver() );
  this->setOmHardware( pehit.getOmReceiver() );
  const double delay = pehit.getLeTime()-egeom.geometricalTime();
  const double halfsigma = sigma_*0.5;
  double result = 0;
  if ( delay + halfsigma <= 0 ){
      return result;
  }
  const double propd = egeom.propagationDistance();
  // if ( 0 == propd) log_warn( "propd==0" );
  const double depth = pehit.getOmReceiver().getZ();
  const double rho(1./ice_model_->TauScale(depth)+IPdfConstants::C_ICE_G*ice_model_->Absorptivity(depth));
  const double ksi = propd*ice_model_->InvEffScattLength(depth);
  const double rhotplus = rho * (delay+halfsigma);
  const double rhotminus = rho * (delay-halfsigma);
  if ((rhotplus > 0) && (0.5*(ksi+1)<GSL_SF_GAMMA_XMAX) ){
    result = PdfMath::IncGammaP(ksi, rhotplus);
    if ( rhotminus > 0 ){
      result -= PdfMath::IncGammaP(ksi, rhotminus); 
    }
    result /= sigma_;
  } // else: too early, too far away: highly unlikely
  return result;
}

/********************************************************************/
template<typename IceModel>
  template<typename PEHit, typename EmissionHypothesis>
inline double BoxConvolutedPEP<IceModel>::getIntPdf(
        const PEHit& pehit, const EmissionHypothesis& eh) const {
  const typename EmissionHypothesis::template EmissionGeometry<IceModel>
      egeom( eh, pehit.getOmReceiver() );
  this->setOmHardware( pehit.getOmReceiver() );
  double delay = pehit.getLeTime()-egeom.geometricalTime();
  const double halfsigma = sigma_ * 0.5;
  double result = 0;
  if ( delay + halfsigma <= 0 ){
      return result;
  }

  double propd = egeom.propagationDistance();
  // if ( 0 == propd) log_warn( "propd==0" );
  const double depth = pehit.getOmReceiver().getZ();
  const double ksi = propd * ice_model_->InvEffScattLength(depth);
  const double rho(1./ice_model_->TauScale(depth)+IPdfConstants::C_ICE_G*ice_model_->Absorptivity(depth));
  const double rhosigma = rho * sigma_;
  const double rhotplus = rho * (delay+halfsigma);
  const double rhotminus = rho * (delay-halfsigma);
  if ((rhotplus > 0) && (0.5*(ksi+1)<GSL_SF_GAMMA_XMAX) ){
    result = rhotplus * PdfMath::IncGammaP(ksi, rhotplus )
                - ksi * PdfMath::IncGammaP(ksi+1, rhotplus );
    if ( rhotminus > 0 ){
      result -= rhotminus * PdfMath::IncGammaP(ksi, rhotminus )
                    - ksi * PdfMath::IncGammaP(ksi+1, rhotminus );
    }
    result /= rhosigma;
  } // else: too early, too far away: highly unlikely

  return result;

}


/********************************************************************/
template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double BoxConvolutedPEP<IceModel>::
  getHitProb(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::pHit(omr,eh,*ice_model_);
}

template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double BoxConvolutedPEP<IceModel>::
  expectedNPE(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::meanNumPE(omr,eh,*ice_model_);
}
/********************************************************************/


} // namespace Pandel
} // namespace IPDF

template<typename IceModel>
inline std::ostream& operator<<(std::ostream& os, const IPDF::Pandel::BoxConvolutedPEP<IceModel>&) {
  return (os<<"BoxConvolutedPEP<"<<IceModel::name()<<">");
}

#endif // IPDF_PANDEL_BoxConvolutedPEP_H
