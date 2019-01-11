#ifndef IPDF_UnConvolutedPEP_H
#define IPDF_UnConvolutedPEP_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file UnConvolutedPEP.h
    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Pandel/HitProbability.h"
#include "ipdf/Utilities/PdfMath.h"
#include <iosfwd>
#include <boost/shared_ptr.hpp>

namespace IPDF {

namespace Pandel {

template<typename IceModel>
class UnConvolutedPEP;

template<typename IceModel>
std::ostream& operator<< (std::ostream&, const UnConvolutedPEP<IceModel>&);

/**
 * @brief Implementation of the pandel functions for different convolution modes.
 */
template<typename IceModel>
class UnConvolutedPEP {
public:
  typedef UnConvolutedPEP<IceModel> Self;
  typedef boost::shared_ptr<IceModel> IceModelPtr;
  typedef IceModel IceModelType;

  /// @brief Normal ctor for PEP, optionally provide the IceModel instance
  explicit UnConvolutedPEP(IceModelPtr ice=IceModelPtr(new IceModel()), double =0.);
  
  /// @brief Calculate the PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getPdf(const PEHit& pehit, const EmissionHypothesis& eh) const {
    const typename EmissionHypothesis::template EmissionGeometry<IceModel> 
	egeom( eh, pehit.getOmReceiver() );
    //--- do the actual work...
    return this->pandelFunction(pehit.getLeTime()-egeom.geometricalTime(),
	egeom.propagationDistance(),pehit.getOmReceiver().getZ());
  }

  /// @brief Calculate the log PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getLogPdf(const PEHit& pehit, const EmissionHypothesis& eh) const {
    const typename EmissionHypothesis::template EmissionGeometry<IceModel> 
	egeom( eh, pehit.getOmReceiver() );
    //--- do the actual work...
    return this->logPandelFunction(pehit.getLeTime()-egeom.geometricalTime(),
	egeom.propagationDistance(),pehit.getOmReceiver().getZ());
  }

  /// @brief Calculate the cumulative PDF
  template<typename PEHit, typename EmissionHypothesis>
  double getIntPdf(const PEHit& pehit, const EmissionHypothesis& eh) const {
    const typename EmissionHypothesis::template EmissionGeometry<IceModel> 
	egeom( eh, pehit.getOmReceiver() );
    //--- do the actual work...
    return this->intPandelFunction(pehit.getLeTime()-egeom.geometricalTime(),
	egeom.propagationDistance(),pehit.getOmReceiver().getZ());
  }

  /// @brief Calculate the probability of at least one photo-electron hit on an OM
  template<typename OmReceiver, typename EmissionHypothesis>
  double getHitProb(const OmReceiver& omr, const EmissionHypothesis& eh) const;

  /// @brief Mean number of expected photo-electron hits on an OM
  template<typename OmReceiver, typename EmissionHypothesis>
  double expectedNPE(const OmReceiver& omr, const EmissionHypothesis& eh) const;

  friend std::ostream& operator<< <> (std::ostream&, const Self&);
  
  /// @brief Change the OmReceiver being used to calculate the PDF
  ///
  /// This is useful, since as default this doesn't need to change if 
  /// the PMT parameters (i.e. jitter and sensitivity) aren't being used.  
  /// If They are being used then this method allows the code to apply 
  /// the correct OM parameters when calculating the PDF.
  template<typename OmReceiver>
  void setOmHardware(const OmReceiver&) const { }

  /// @brief internal
  double pandelFunction(const double delay, 
			const double propd,
			const double depth) const;
  /// @brief internal
  double logPandelFunction(const double delay, 
			   const double propd,
			   const double depth) const;
  /// @brief internal
  double intPandelFunction(const double delay, 
			   const double propd,
			   const double depth) const;

private:
  /// @brief Assignment operator is disabled (implicit copy ctor is ok)
  Self operator=(const Self&);

  double logPandelNormalisation(const double rdist,
				const double ldist,
				const double depth) const;
  IceModelPtr ice_model_;
};


template<typename IceModel>
inline std::ostream& operator<<(std::ostream& os, 
    const UnConvolutedPEP<IceModel>& pandel) {
  return (os<<"UnConvolutedPEP: "
      <<"( "
      <<pandel.ice_model_->AbsorptionLength()
      <<", "<<IceModel::name()
      <<" )"
      );
}

/********************************************************************/
template<typename IceModel>
inline UnConvolutedPEP<IceModel>::UnConvolutedPEP(IceModelPtr ice, double /* jitter unused */)
 : ice_model_(ice)
{
  AssertX(std::isfinite(ice_model_->AbsorptionLength()),IPDF::ParameterValueIsNotFinite);
}

/********************************************************************/
template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double UnConvolutedPEP<IceModel>::
  getHitProb(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::pHit(omr,eh,*ice_model_);
}

template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double UnConvolutedPEP<IceModel>::
  expectedNPE(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::meanNumPE(omr,eh,*ice_model_);
}

/********************************************************************/
template<typename IceModel>
inline double UnConvolutedPEP<IceModel>::
  pandelFunction(const double delay, 
		 const double propd,
		 const double depth) const {
  double result = this->logPandelFunction(delay,propd,depth);
  return (result>-PdfMath::FINFTY) ? exp(result) : 0.;
}

template<typename IceModel>
inline double UnConvolutedPEP<IceModel>::
  logPandelFunction(const double delay, 
		    const double propd,
		    const double depth) const {
  double result=-PdfMath::FINFTY;

  if(delay <= 0.) { return result; }

  double rdist = propd*ice_model_->InvEffScattLength(depth);
  double ldist = propd*ice_model_->Absorptivity(depth);

  return (
      -delay*(1./ice_model_->TauScale(depth)+IPdfConstants::C_ICE_G*ice_model_->Absorptivity(depth)) -ldist /* exponential*/
      +log(delay)*(rdist-1.0)-log(ice_model_->TauScale(depth))*rdist /* potentials */
      -PdfMath::LogGamma(rdist)          /* Gamma function */
      -this->logPandelNormalisation(rdist,ldist,depth)
      );
}

template<typename IceModel>
inline double UnConvolutedPEP<IceModel>::
  intPandelFunction(const double delay, 
		    const double propd,
		    const double depth) const {
  if(delay < 0.) { return 0.; }
  double rdist = propd*ice_model_->InvEffScattLength(depth);
  return PdfMath::IncGammaP(rdist,delay*
   (1./ice_model_->TauScale(depth)+IPdfConstants::C_ICE_G*ice_model_->Absorptivity(depth)));
}

template<typename IceModel>
inline double UnConvolutedPEP<IceModel>::
  logPandelNormalisation(const double rdist,
			 const double ldist,
			 const double depth) const {
  return ( -ldist -rdist*
   log(1.+ice_model_->TauScale(depth)*IPdfConstants::C_ICE_G*ice_model_->Absorptivity(depth))
   );
}

} // namespace Pandel
} // namespace IPDF

#endif // IPDF_UnConvolutedPEP_H
