#ifndef IPDF_PEPBase_H_INCLUDED
#define IPDF_PEPBase_H_INCLUDED

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file PEPBase.h
    @version $Revision: 1.9 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Pandel/HitProbability.h"
#include "ipdf/Pandel/IceModel.h"
#include <iosfwd>
#include <gsl/gsl_integration.h> // for numrerical integration
#include <gsl/gsl_errno.h> // for error handling

namespace IPDF {

/**
    @brief Base class for PEP classes that only implement getPDF()

    This class provides implementation of getIntPdf() for PDFs,
    which do not implement it themselves.  This is provided 
    using the numerical integration from the GSL library.

    @todo Further optimisation of speed/accuracy of integration.
  */
template<class PEPImpl>
class PEPBase {
public:
  typedef PEPBase Self;
  typedef PEPImpl PEPType;
  typedef double pdf_result;
  
  /// @brief Default ctor (configure GSL integration)
  ///
  /// @param integral_lower_limit is the delay time (in nano-seconds) at which the numerical integration starts.
  PEPBase(const double integral_lower_limit=-100.)
   : integral_lower_limit_(integral_lower_limit) { this->init(); }
  /// @brief Dtor (deallocate GSL resources)
  ~PEPBase() { this->deinit(); }

  /// @brief Copy ctor: implemented by hand due to GSL allocation
  PEPBase(const PEPBase& rhs) 
   : integral_lower_limit_(rhs.integral_lower_limit_) { this->init(); }
  /// @brief Assignment operator: implemented by hand due to GSL allocation
  PEPBase& operator=(const PEPBase&);

  /// @brief Change the OmReceiver being used to calculate the PDF
  ///
  /// This is useful, since as default this doesn't need to change if 
  /// the PMT parameters (i.e. jitter and sensitivity) aren't being used.  
  /// If They are being used then this method allows the code to apply 
  /// the correct OM parameters when calculating the PDF.
  template<typename OmReceiver>
  void setOmHardware(const OmReceiver&) const;
  
  /// @brief Method to access the cumulative PDF (implemented using the GSL):
  template<typename PEHit, typename EmissionHypothesis>
  pdf_result getIntPdf(const PEHit& pehit, const EmissionHypothesis& eh) const;

protected:
  /// @brief GSL allocation
  void init();
  /// @brief GSL deallocation
  void deinit();
private:
  // In order to use a GSL integration:
  //static const double BEGIN_INTEGRAL;
  const double integral_lower_limit_;
  static const size_t gslParSpaceSize_ =10;
  mutable double gslParSpace_[gslParSpaceSize_];
  static const size_t gslWorkspaceSize_  = 2000;
  mutable gsl_integration_workspace * gslWorkspace_;
};

/********************************************************************/
template<class PEPImpl>
inline PEPBase<PEPImpl>& PEPBase<PEPImpl>::operator=(const PEPBase& rhs) {
  if(this!=&rhs) {
    this->deinit();
    this->init();
  }
  return *this;
}

template<class PEPImpl>
inline void PEPBase<PEPImpl>::init() {
  // workspace for integration
  gslWorkspace_  = gsl_integration_workspace_alloc(gslWorkspaceSize_);

  // switch off gsl error handle
  gsl_set_error_handler_off();
}

template<class PEPImpl>
inline void PEPBase<PEPImpl>::deinit() {
  gsl_integration_workspace_free(gslWorkspace_);
  // restor gsl_errorhandle to default
  gsl_set_error_handler(NULL);
}

/********************************************************************/
/** @brief Helper struct to bind to getPdf() member function.
 */
template<class PEPImpl, typename Hit, typename Emitter>
struct PDFunctor {
  typedef PDFunctor<PEPImpl, Hit, Emitter> functor_type;
  PDFunctor(const PEPImpl* impl, const Hit* pehit, const Emitter* emitter)
      : impl_(impl), pehit_(pehit), emitter_(emitter) {}
  static double sample_pdf(double delay, void* params)
  {
    functor_type* f = static_cast<functor_type*>(params);
    Hit pebin(f->pehit_->getOmReceiver(),delay,
	        f->pehit_->getPeAmplitude());
    return f->impl_->getPdf(pebin,*(f->emitter_));
  }
  
  const PEPImpl* impl_;
  const Hit* pehit_;
  const Emitter* emitter_;
};

/********************************************************************/
template<class PEPImpl>
  template<typename PEHit, typename EmissionHypothesis>
inline double PEPBase<PEPImpl>::
	    getIntPdf(
    const PEHit& pehit,
    const EmissionHypothesis& emitter) const {
  double retval=0.;
  double delay = pehit.getLeTime();

  const typename EmissionHypothesis::template EmissionGeometry<typename PEPImpl::IceModelType>
      egeom( emitter, pehit.getOmReceiver() );
  double adj_integral_lower_limit = integral_lower_limit_ + egeom.geometricalTime();

  typedef PDFunctor<PEPImpl,PEHit,EmissionHypothesis> functor_type;
  functor_type functor(static_cast<const PEPImpl*>(this), &pehit, &emitter);
  gsl_function F;
  F.function = &(functor_type::sample_pdf);
  F.params = &functor;

  double error;
  gsl_integration_qag(&F,adj_integral_lower_limit, delay, 0, 1e-7, gslWorkspaceSize_, GSL_INTEG_GAUSS15, gslWorkspace_, &retval, &error);
  return retval;
}

} // namespace IPDF

#endif // IPDF_PEPBase_H_INCLUDED
