#ifndef IPDF_LIKELIHOOD_MPEAll_H
#define IPDF_LIKELIHOOD_MPEAll_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: MPEAll.h 57329 2009-08-10 18:16:36Z boersma $

    @file MPEAll.h
    @version $Revision: 1.7 $
    @date $Date: 2009-08-10 20:16:36 +0200 (Mon, 10 Aug 2009) $
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Utilities/PdfMath.h"
#include "ipdf/Utilities/IPdfException.h"  /// @todo TEMP
#include <iosfwd>
#include <boost/shared_ptr.hpp>
#include "icetray/I3Logging.h"


namespace IPDF {
namespace Likelihood {

/**
 * @brief Representation of the MPEAll likelihood
 *
 * Implicit dtor, copy ctor, and asignment operator are ok (boost::shared_ptr).
 */
template<typename PEProb>
class MPEAll {
public:
  typedef MPEAll<PEProb> Self;
  typedef PEProb PEPType;
  typedef double like_result;

  /// @brief Default ctor "new"s default PEP
  MPEAll() : spp_(new PEPType) {
    log_warn("MPEAll in ipdf is NEW, EXPERIMENTAL, WITHOUT WARRANTIES");
    log_warn("DO NOT (YET) USE THIS FOR PRODUCTION");
  }
  /// @brief Ctor taking "new" PEP passed by user - will be deleted when MPEAll is destroyed
  explicit MPEAll(boost::shared_ptr<PEProb> spp ) : spp_(spp) {
    log_warn("MPEAll in ipdf is NEW, EXPERIMENTAL, WITHOUT WARRANTIES");
    log_warn("DO NOT (YET) USE THIS FOR PRODUCTION");
  }
  
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
  SET_LOGGER("ipdf");
};

} // namespace Likelihood
} // namespace IPDF

template<typename PEProb>
inline std::ostream& operator<<(std::ostream& os, 
    const IPDF::Likelihood::MPEAll<PEProb>& like) {
  return (os<<"MPEAll: "<<like.getPEP());
}

// Implementation of likelihood mathematics:
template<typename PEProb>
template<typename HitOm, typename EmissionHypothesis>
inline double IPDF::Likelihood::MPEAll<PEProb>::
  getLikelihood(const HitOm& hitom,
		const EmissionHypothesis& emitter) const {

  double result = 1.;

  // NO "box-convolution" (original MPEAll version)
  // const typename HitOm::value_type& pehit1 = hitom[0];
  typename HitOm::const_iterator pehit = hitom.begin();
  for(;pehit!=hitom.end();++pehit){
    double pe     = (*pehit)->getPeAmplitude();
    if (pe < 1.5) {
      result *= spp_->getPdf(**pehit,emitter);
    } else {
      double cdf = spp_->getIntPdf(**pehit,emitter);
      if (cdf > 0.9999){
        cdf = 0.9999;
      } else if (cdf < 0){
        cdf = 0;
      }
      double tint = 1.0 - cdf;
      // AssertX(tint>-0.01,IPDF::NumericalError);  /// @todo TEMP

      if (tint >= 1.0) {
        // this can happen, due to rounding errors
        double tpe = PdfMath::Nint(pe);
        result *= tpe * spp_->getPdf(**pehit,emitter);
      } else { // if (tint>0.0) {}
        unsigned int npe((unsigned int)(pe+0.5));
        result *= spp_->getPdf(**pehit,emitter);
        result *= npe * PdfMath::FastPow( tint, npe-1 );
      // } else {
        // result = 0.;
      }
    }
  }

#if 0
  if ( ! finite(result) ){
      double pdf = spp_->getPdf(pehit1,emitter);
      double tint = 1.0 - spp_->getIntPdf(pehit1,emitter);
      double tpe = PdfMath::Nint(pe);
      log_fatal("pe=%g tpe=%g result=%g tint=%g 1-tint=%g pdf=%g",
              pe, tpe, result, tint, 1.0-tint, pdf );
  }
#endif

  return result;
}

template<typename PEProb>
template<typename HitOm, typename EmissionHypothesis>
inline double IPDF::Likelihood::MPEAll<PEProb>::
  getLogLikelihood(const HitOm& hitom,
		   const EmissionHypothesis& emitter) const {
  double result = 0.;
  // const typename HitOm::value_type& pehit1 = hitom[0];
  typename HitOm::const_iterator pehit = hitom.begin();
  for(;pehit!=hitom.end();++pehit){
    double pe     = (*pehit)->getPeAmplitude();
    if (pe < 1.5) {
      result += spp_->getLogPdf(**pehit,emitter);
    } else {
      double cdf = spp_->getIntPdf(**pehit,emitter);
      if (cdf > 0.9999){
        cdf = 0.9999;
      } else if (cdf < 0){
        cdf = 0;
      }
      double tint = 1.0 - cdf;
      // AssertX(tint<1.0,IPDF::NumericalError);  /// @todo TEMP
      if (tint>=1.0) {
        double tpe = PdfMath::Nint(pe);
        result += log(tpe) + spp_->getLogPdf(**pehit,emitter);
      } else { // if (tint>0.0) {}
        double tpe = PdfMath::Nint(pe);
        result += log(tpe) 
            +  spp_->getLogPdf(**pehit,emitter)
            + (tpe-1) * log (tint);
      // } else {
        // result += -PdfMath::FINFTY;
      }
    }
  }

  return result;
}

template<typename PEProb>
template<typename HitOm, typename EmissionHypothesis>
inline double IPDF::Likelihood::MPEAll<PEProb>::
  getIntLikelihood(const HitOm& hitom,
		   const EmissionHypothesis& emitter) const {

  double result = 1.;
  double pe     = hitom.getTotalPe();
  const typename HitOm::value_type& pehit1 = hitom[0];
  double F1 = spp_->getIntPdf(pehit1,emitter);
  if (pe < 1.5){
    result *= F1;
  } else {
    double tpe = PdfMath::Nint(pe);
    result *= 1. - pow(1.0-F1,tpe);
  }
  return result;
}

#endif // IPDF_LIKELIHOOD_MPEAll_H
