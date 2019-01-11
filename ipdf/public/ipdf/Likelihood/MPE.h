#ifndef IPDF_LIKELIHOOD_MPE_H
#define IPDF_LIKELIHOOD_MPE_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file MPE.h
    @version $Revision: 1.7 $
    @date $Date$
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
 * @brief Representation of the MPE likelihood
 *
 * Implicit dtor, copy ctor, and asignment operator are ok (boost::shared_ptr).
 */
template<typename PEProb>
class MPE {
public:
  typedef MPE<PEProb> Self;
  typedef PEProb PEPType;
  typedef double like_result;

  /// @brief Default ctor "new"s default PEP
  MPE(double tbox=0.) : t_error_(tbox), spp_(new PEPType) { }
  /// @brief Ctor taking "new" PEP passed by user - will be deleted when MPE is destroyed
  explicit MPE(boost::shared_ptr<PEProb> spp, double tbox=0. ) :
      t_error_(tbox), spp_(spp) { }
  
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
  // timing uncertainty due to calibration error
  // NOTE: this is different from jitter
  // jitter must be taken into account in the spp
  // (each single photoelectron can jitter independently)
  // calibration error is the same for all PE in a DOM.
  // The distribution is modeled as a flat, finite width distribution
  // (a so-called "box distribution").
  double t_error_;
  boost::shared_ptr<PEProb> spp_;
  SET_LOGGER("ipdf");
};

} // namespace Likelihood
} // namespace IPDF

template<typename PEProb>
inline std::ostream& operator<<(std::ostream& os, 
    const IPDF::Likelihood::MPE<PEProb>& like) {
  return (os<<"MPE: "<<like.getPEP());
}


// Implementation of likelihood mathematics:
template<typename PEProb>
template<typename HitOm, typename EmissionHypothesis>
inline double IPDF::Likelihood::MPE<PEProb>::
  getLikelihood(const HitOm& hitom,
		const EmissionHypothesis& emitter) const {

  double result = 1.;
  double pe     = hitom.getTotalPe();
  if ( t_error_ > 0. ){
    // "box-convoluted" MPE: allow for time calibration error in the DOM
    // with a flat error distribution [t-dt,t+dt]
    // NOTE: this is different from the Gauss convolution on the spp (PDF)
    // The jitter is an independent fluctuation for each PE, while
    // the calibration error is the same for all PEs in the DOM.
    typename HitOm::value_type pehit1 = hitom[0];
    double t = pehit1.getLeTime();
    pehit1.setLeTime(t-t_error_);
    double tintmin = 1.0 - spp_->getIntPdf(pehit1,emitter);
    AssertX(std::isfinite(tintmin),IPDF::NumericalError);
    pehit1.setLeTime(t+t_error_);
    double tintplus = 1.0 - spp_->getIntPdf(pehit1,emitter);
    AssertX(std::isfinite(tintplus),IPDF::NumericalError);
    if (tintmin<tintplus){
      if ( (tintmin<0.999999) || (tintplus>1.) ){
        const typename EmissionHypothesis::template EmissionGeometry<typename PEProb::IceModelType>
          egeom( emitter, pehit1.getOmReceiver() );
        const double tres = pehit1.getLeTime()-egeom.geometricalTime(); //"propagation delay" or "t" in Pandel paper
        const double d = egeom.propagationDistance();                   //"propagation distance" or "R" in Pandel paper
        log_fatal("tres=%g dt=%g d=%g tintmin=%g=1-%g tintplus=%g=1-%g",
                  tres, t_error_, d, tintmin, 1.-tintmin, tintplus, 1.-tintplus );
      } else {
        // allow rounding errors for tintmin and tintmax both almost 1
        // (very negative tres)
        tintmin=tintplus;
      }
    }
    AssertX(tintmin>=tintplus,IPDF::NumericalError);
    if ( pe < 1.5 ){
      result *= tintmin - tintplus;
    } else {
      unsigned int npe((unsigned int)(pe+0.5));
      result *= + PdfMath::FastPow(tintmin,npe)
                - PdfMath::FastPow(tintplus,npe);
    }
    result /= 2.*t_error_;
    return result;
  }

  // NO "box-convolution" (original MPE version)
  const typename HitOm::value_type& pehit1 = hitom[0];
  if (pe < 1.5) {
    result *= spp_->getPdf(pehit1,emitter);
  } else {
    double tint = 1.0 - spp_->getIntPdf(pehit1,emitter);
    // AssertX(tint>-0.01,IPDF::NumericalError);  /// @todo TEMP

    if (tint >= 1.0) {
      // this can happen, due to rounding errors
      double tpe = PdfMath::Nint(pe);
      result *= tpe * spp_->getPdf(pehit1,emitter);
    } else if (tint>0.0) {
      unsigned int npe((unsigned int)(pe+0.5));
      result *= spp_->getPdf(pehit1,emitter);
      result *= npe * PdfMath::FastPow( tint, npe-1 );
    } else {
      result = 0.;
    }
  }

#if 0
  if ( ! std::isfinite(result) ){
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
inline double IPDF::Likelihood::MPE<PEProb>::
  getLogLikelihood(const HitOm& hitom,
		   const EmissionHypothesis& emitter) const {
  double result = 0.;
  double pe     = hitom.getTotalPe();
  if ( t_error_ > 0. ){
    // "box-convoluted" MPE: allow for time calibration error in the DOM
    // with a flat error distribution [t-dt,t+dt]
    // NOTE: this is different from the Gauss convolution on the spp (PDF)
    // The jitter is an independent fluctuation for each PE, while
    // the calibration error is the same for all PEs in the DOM.
    typename HitOm::value_type pehit1 = hitom[0];
    double t = pehit1.getLeTime();
    pehit1.setLeTime(t-t_error_);
    double tintmin = 1.0 - spp_->getIntPdf(pehit1,emitter);
    AssertX(std::isfinite(tintmin),IPDF::NumericalError);
    pehit1.setLeTime(t+t_error_);
    double tintplus = 1.0 - spp_->getIntPdf(pehit1,emitter);
    AssertX(std::isfinite(tintplus),IPDF::NumericalError);
    if (tintmin<tintplus){
      if ( (tintmin<0.999999) || (tintplus>1.) ){
        const typename EmissionHypothesis::template EmissionGeometry<typename PEProb::IceModelType>
          egeom( emitter, pehit1.getOmReceiver() );
        const double tres = pehit1.getLeTime()-egeom.geometricalTime(); //"propagation delay" or "t" in Pandel paper
        const double d = egeom.propagationDistance();                   //"propagation distance" or "R" in Pandel paper
        log_fatal("tres=%g dt=%g d=%g tintmin=%g=1-%g tintplus=%g=1-%g",
                  tres, t_error_, d, tintmin, 1.-tintmin, tintplus, 1.-tintplus );
      } else {
        // allow rounding errors for tintmin and tintmax both almost 1
        // (very negative tres)
        tintmin=tintplus;
      }
    }
    AssertX(tintmin>=tintplus,IPDF::NumericalError);
    if ( pe < 1.5 ){
      result *= tintmin - tintplus;
    } else if (tintmin>tintplus){
      unsigned int npe((unsigned int)(pe+0.5));
      result *= + PdfMath::FastPow(tintmin,npe)
                - PdfMath::FastPow(tintplus,npe);
    }
    if ( result>0. ){
      result /= 2.*t_error_;
      return log(result);
    }

    return -PdfMath::FINFTY;
  }

  const typename HitOm::value_type& pehit1 = hitom[0];
  if (pe < 1.5) {
    result += spp_->getLogPdf(pehit1,emitter);
  } else {
    double tint = 1.0 - spp_->getIntPdf(pehit1,emitter);
    // AssertX(tint<1.0,IPDF::NumericalError);  /// @todo TEMP
    if (tint>=1.0) {
      double tpe = PdfMath::Nint(pe);
      result += log(tpe) + spp_->getLogPdf(pehit1,emitter);
    } else if (tint>0.0) {
      double tpe = PdfMath::Nint(pe);
      result += log(tpe) 
	  +  spp_->getLogPdf(pehit1,emitter)
	  + (tpe-1) * log (tint);
    } else {
      result += -PdfMath::FINFTY;
    }
  }

  return result;
}

template<typename PEProb>
template<typename HitOm, typename EmissionHypothesis>
inline double IPDF::Likelihood::MPE<PEProb>::
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


#endif // IPDF_LIKELIHOOD_MPE_H
