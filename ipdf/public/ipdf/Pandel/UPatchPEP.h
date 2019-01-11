#ifndef IPDF_UPatchPEP_H
#define IPDF_UPatchPEP_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file UPatchPEP.h
    @version $Revision: 1.4 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Simple/PEPBase.h"
#include "ipdf/Pandel/UnConvolutedPEP.h"

#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"
#include "ipdf/Utilities/PdfMath.h"
#include "ipdf/Pandel/PandelConstants.h"
#include "ipdf/Pandel/HitProbability.h"
#include <iosfwd>
#include <cassert>
#include <boost/shared_ptr.hpp>

namespace IPDF {
namespace Pandel {

template<typename IceModel>
class UPatchPEP;

template<typename IceModel>
std::ostream& operator<< (std::ostream&, const UPatchPEP<IceModel>&);

/**
    @brief Implementation of the pandel functions for 
    different convolution modes.

    getIntPdf is implemented by PEPBase using the GSL.

    Implicit copy ctor and dtor are correct (boost::shared_ptr).

    @todo Move remaining common code to patch_it
  */
template<typename IceModel>
class UPatchPEP : public PEPBase< UPatchPEP<IceModel> > {
public:
  typedef UPatchPEP<IceModel> Self;
  typedef boost::shared_ptr<IceModel> IceModelPtr;
  typedef IceModel IceModelType;

  /// @brief Normal ctor for PEP, optionally provide the IceModel instance
  explicit UPatchPEP(IceModelPtr ice=IceModelPtr(new IceModel()), 
		     const double jitter=15.0);
  
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
  void setOmHardware(const OmReceiver&) const;

  /// @brief internal
  double pandelFunction(const double delay, 
			const double propd,
			const double depth) const;
  /// @brief internal
  double logPandelFunction(const double delay, 
			   const double propd,
			   const double depth) const;

private:
  /// @brief Asignment operator is disabled (copy ctor is ok)
  Self& operator=(const Self&);

  static double gsl_td_convolute(double x, void *params);

  double td_int(const double delay, 
			   const double propd) const;
  double patch_it(double t, double df, double pdf, double intpdf) const;
  double log_patch_it(double t, double df, double pdf, double intpdf) const;
  double log_patch_gauss(double t, double df, double pdf, double intpdf) const;
  double patch_p3(double t, double df, double pdf, double intpdf) const;

  mutable double sigma_;
  mutable double td_t0_;
  boost::shared_ptr< UnConvolutedPEP<IceModel> > unconvoluted_;
  IceModelPtr ice_model_;
};


template<typename IceModel>
inline std::ostream& operator<<(std::ostream& os, 
    const UPatchPEP<IceModel>& pandel) {
  return (os<<"UPatchPEP: "
      <<"( "
      <<", "<<pandel.sigma_
      <<", "<<IceModel::name()
      <<" )"
      );
}

/********************************************************************/
template<typename IceModel>
inline UPatchPEP<IceModel>::UPatchPEP(IceModelPtr ice, const double jitter)
 : PEPBase< UPatchPEP<IceModel> >(-10.*jitter),
   sigma_(jitter),
   td_t0_(sqrt(2.0*M_PI)*jitter),
   unconvoluted_(new UnConvolutedPEP<IceModel>()),
   ice_model_(ice)
{
  AssertX(std::isfinite(sigma_),IPDF::ParameterValueIsNotFinite);
  AssertX(td_t0_>=0.,IPDF::NumericalError);
}

/********************************************************************/
template<typename IceModel>
template<typename OmReceiver>
inline void UPatchPEP<IceModel>::setOmHardware(const OmReceiver& omr) const {
  sigma_ = omr.getJitter();
  td_t0_=sqrt(2.0*M_PI)*sigma_;
}

/********************************************************************/
template<typename IceModel>
double UPatchPEP<IceModel>::pandelFunction(
				const double delay, 
				const double propd,
				const double depth) const {
  if (propd > PandelConstants::DIST_MAGIC){
    return unconvoluted_->pandelFunction(delay, propd, depth);
  } else if (delay >= td_t0_){ /* pandel */
    return unconvoluted_->pandelFunction(delay, propd, depth);
  }else {
    double pt = unconvoluted_->pandelFunction(td_t0_, propd, depth);
    double rdist = propd*ice_model_->InvEffScattLength(depth);
    double dpt = pt*((rdist-1.)/td_t0_ - (1./ice_model_->TauScale(depth)+IPdfConstants::C_ICE_G*ice_model_->Absorptivity(depth)));
    double ipt = unconvoluted_->intPandelFunction(td_t0_, propd, depth);
    return this->patch_it(delay,dpt,pt,ipt); 
  }
}

template<typename IceModel>
double UPatchPEP<IceModel>::logPandelFunction(
				const double delay, 
				const double propd,
				const double depth) const {
  double result;

  if (propd > PandelConstants::DIST_MAGIC){
    result = unconvoluted_->logPandelFunction(delay, propd, depth);
    if ( !std::isfinite(result) ){
        log_warn( "result=%f not finite, propd>distmagic", result );
    }
  } else if (delay >= td_t0_){ /* pandel */
    result = unconvoluted_->logPandelFunction(delay, propd, depth);
    if ( !std::isfinite(result) ){
        log_warn( "result=%f not finite, delay>td_t0", result );
    }
  } else {
    double pt = unconvoluted_->pandelFunction(td_t0_, propd, depth);
    double rdist = propd*ice_model_->InvEffScattLength(depth);
    double dpt = pt*((rdist-1.)/td_t0_ - (1./ice_model_->TauScale(depth)+IPdfConstants::C_ICE_G*ice_model_->Absorptivity(depth)));
    double ipt = unconvoluted_->intPandelFunction(td_t0_, propd, depth);
    result = this->log_patch_it(delay,dpt,pt,ipt);
    if ( !std::isfinite(result) ){
        log_warn( "result=%f not finite, patch region", result );
    }
  }
  assert(std::isfinite(result) && "ipdf internal error: non-finite logPandelFunction result");
  return result;
}

/********************************************************************/
template<typename IceModel>
double UPatchPEP<IceModel>::patch_it(double t, double df, double pdf, double intpdf) const{

  assert(std::isfinite(pdf) && "ipdf internal error: non-finite int(PDF) passed to patch_it");
  if (t >= td_t0_){ /* pandel */
    return pdf;
  } else if (t <= 0){ /* Gauss */
    return exp(this->log_patch_gauss(t,df,pdf,intpdf));
  } else{ /* P3 */
    return this->patch_p3(t,df,pdf,intpdf);
  }
}

/********************************************************************/
template<typename IceModel>
double UPatchPEP<IceModel>::log_patch_it(double t, double df, double pdf, double intpdf) const{

  assert(std::isfinite(pdf) && "ipdf internal error: non-finite int(PDF) passed to log_patch_it");
  if (t >= td_t0_){ /* pandel */
    return log(pdf);
  } else if (t <= 0){ /* Gauss */
    return this->log_patch_gauss(t,df,pdf,intpdf);
  } else{ /* P3 */
    return log(this->patch_p3(t,df,pdf,intpdf));
  }
}

/********************************************************************/
template<typename IceModel>
double UPatchPEP<IceModel>::log_patch_gauss(double t, double df, double pdf, double intpdf) const{

  const double td_t0_dsq1=1./td_t0_;
  const double norm = intpdf*td_t0_dsq1 - 0.5*pdf + df*td_t0_/12.;
  if ( norm<=0 ) return -PdfMath::FINFTY;
  const double td_sigma_d_2square = 1./(2.*sigma_*sigma_);
  return (log(norm) + (-t*t*td_sigma_d_2square));
}

/********************************************************************/
template<typename IceModel>
double UPatchPEP<IceModel>::patch_p3(double t, double df, double pdf, double intpdf) const{
  const double td_t0_dsq1=1./td_t0_;
  const double td_t0_dsq2=td_t0_dsq1/td_t0_;
  const double td_t0_dsq3=td_t0_dsq2/td_t0_;
  const double td_t0_dsq4=td_t0_dsq3/td_t0_;

  double a0 = intpdf*td_t0_dsq1 - 0.5*pdf + df*td_t0_/12.;
  double a2 = -3.*intpdf*td_t0_dsq3
      - 1.25*df*td_t0_dsq1 + 4.5*pdf*td_t0_dsq2;
  double a3=2.*intpdf*td_t0_dsq4
      +7./6.*df*td_t0_dsq2 - 3. * pdf*td_t0_dsq3;
  return  (a0 + a2*t*t + a3*t*t*t);
}

/********************************************************************/
template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double UPatchPEP<IceModel>::
getHitProb(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::pHit(omr,eh,*ice_model_);
}

template<typename IceModel>
template<typename OmReceiver, typename EmissionHypothesis>
inline double UPatchPEP<IceModel>::
  expectedNPE(const OmReceiver& omr, const EmissionHypothesis& eh) const{
  return HitProbability<EmissionHypothesis,IceModel>::meanNumPE(omr,eh,*ice_model_);
}
/********************************************************************/

} // namespace Pandel
} // namespace IPDF

#endif // IPDF_UPatchPEP_H
