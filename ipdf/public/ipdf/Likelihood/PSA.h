#ifndef IPDF_LIKELIHOOD_PSA_H
#define IPDF_LIKELIHOOD_PSA_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file PSA.h
    @version $Revision: 1.4 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Utilities/PdfMath.h"
#include "ipdf/Utilities/IPdfException.h"
#include <iosfwd>
#include <cmath>
#include <boost/shared_ptr.hpp>

#include <ipdf/Pandel/BoxConvolutedPEP.h>
#include <ipdf/Pandel/GConvolutePEP.h>
#include <ipdf/Pandel/GSemiConvolutePEP.h>
#include <ipdf/Pandel/GaussConvolutedPEP.h>
#include <ipdf/Pandel/UPatchPEP.h>
#include <ipdf/Pandel/UnConvolutedPEP.h>

namespace IPDF {
namespace Likelihood {

/**
 * @brief Representation of the PSA likelihood
 *
 * Implicit dtor, copy ctor, and asignment operator are ok (boost::shared_ptr).
 */
template<typename PEProb>
class PSA {
public:
  typedef PSA<PEProb> Self;
  typedef PEProb PEPType;
  typedef double like_result;

  /// @brief Default ctor "new"s default PEP
  PSA() : spp_(new PEPType) { }
  /// @brief Ctor taking "new" PEP passed by user - will be deleted when MPE is destroyed
  explicit PSA(boost::shared_ptr<PEProb> spp) : spp_(spp) { }
  
  /// @brief Access the PEP object used by this likelihood
  const PEProb& getPEP() const { return *spp_; }

  /// @brief Access likelihood
  template<typename HitOm, typename EmissionHypothesis>
  like_result getLikelihood(const HitOm& hitom,
			    const EmissionHypothesis& emitter) const;

  /// @brief Access log likelihood
  template<typename HitOm, typename EmissionHypothesis>
  like_result getLogLikelihood( const HitOm& hitom,
			    const EmissionHypothesis& emitter) const;

  /// @brief Access cumulative likelihood
  template<typename HitOm, typename EmissionHypothesis>
  like_result getIntLikelihood( const HitOm& hitom,
			    const EmissionHypothesis& emitter) const;
  
private:
  boost::shared_ptr<PEProb> spp_;
};

} // namespace Likelihood
} // namespace IPDF

template<typename PEProb>
inline std::ostream& operator<<(std::ostream& os, 
    const IPDF::Likelihood::PSA<PEProb>& like) {
  return (os<<"PSA: "<<like.getPEP());
}


template<typename PEProb>
template<typename HitOm, typename EmissionHypothesis>
inline double IPDF::Likelihood::PSA<PEProb>::
    getLikelihood(const HitOm& hitom,
		  const EmissionHypothesis& emitter) const {
  double result = this->getLogLikelihood(hitom,emitter);
  result = (result<= -PdfMath::FINFTY && std::isfinite(result)) ? 0. : exp(result);
  return result;
}

template<typename PEProb>
template<typename HitOm, typename EmissionHypothesis>
inline double IPDF::Likelihood::PSA<PEProb>::
    getLogLikelihood(const HitOm& hitom,
		  const EmissionHypothesis& emitter) const {
  double result = 0.;

  //double pe=(*hitom).getTotalPe(); /// @todo What is correct?
  double expected_pe = spp_->expectedNPE(hitom.getOmReceiver(),emitter);
  AssertX((expected_pe>=0.),IPDF::NumericalError);

  double tmp = 1 - exp(-expected_pe);
  if(tmp<=0.0) {
    result += spp_->getLogPdf((hitom)[0],emitter);
  } else {
    double ln_pdf = spp_->getLogPdf((hitom)[0],emitter);
    result += log(expected_pe / tmp)
	+ ln_pdf
	- expected_pe*spp_->getIntPdf((hitom)[0],emitter);
  }
  return result;
}

template<typename PEProb>
template<typename HitOm, typename EmissionHypothesis>
inline double IPDF::Likelihood::PSA<PEProb>::
    getIntLikelihood(const HitOm& hitom,
		  const EmissionHypothesis& emitter) const {
  double result = 1.;

  double expected_pe = spp_->expectedNPE(hitom.getOmReceiver(),emitter);
  double F1=spp_->getIntPdf((hitom)[0],emitter);
  AssertX((expected_pe>=0.),IPDF::NumericalError);

  double denominator = 1. - exp (- expected_pe);
  if (denominator == 0.) { /* according to L'Hospital */
    result = F1;
  }else{
    result = (1. - exp(-expected_pe*F1))/denominator;
  }
  return result;
}


#endif // IPDF_LIKELIHOOD_PSA_H
