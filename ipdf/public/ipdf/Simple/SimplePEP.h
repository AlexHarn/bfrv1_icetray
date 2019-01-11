#ifndef SimplePEP_H_INCLUDED
#define SimplePEP_H_INCLUDED

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file SimplePEP.h
    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Simple/PEPBase.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Utilities/PdfMath.h"

namespace IPDF {

/**
    @brief Simple demonstration basic PDF implementation.

    This class is an example of the interface to a PDF 
    as provided by ipdf.  For an example of a likelihood see 
    Likelihood/SPEAll.h

    Note: SimplePEP is hard-coded to use InfiniteMuon, 
    most PEPs are also templated for the EmissionHypothesis.
  */
class SimplePEP : public PEPBase<SimplePEP> {
public:
 
  typedef double IceModelType;

  /// @brief Default ctor - no-op
  SimplePEP()  { }
  /// @brief Convert from Base class to me where needed
  SimplePEP(const PEPBase<SimplePEP>&) { }
  ~SimplePEP() { }

  /// @brief Access the PDF.
  ///
  /// Templates allow many different classes to implement 
  /// the methods needed by this method, see for example 
  /// SimplePEHit, SimpleOmReceiver and InfiniteMuon.
  template<typename PEHit>
  double getPdf(const PEHit& pehit, const InfiniteMuon& eh) const;
  /// @brief Access the log PDF.
  template<typename PEHit>
  double getLogPdf(const PEHit& pehit, const InfiniteMuon& eh) const {
    double pdf=this->getPdf(pehit,eh);
    return (
	(pdf>0. && std::isfinite(pdf))
	? log(pdf)
	: -PdfMath::FINFTY
	);
  }

  /// @brief Calculate the probability of at least one photo-electron hit on an OM
  template<typename OmReceiver>
  double getHitProb(const OmReceiver& omr, const InfiniteMuon& eh) const;

  /// @brief Mean number of expected photo-electron hits on an OM
  template<typename OmReceiver>
  double expectedNPE(const OmReceiver& omr, const InfiniteMuon& eh) const;

};

/***************************************************************************/
/// templates - to allow many different implementations:
template<typename PEHit>
inline double SimplePEP::getPdf(
    const PEHit& pehit,
    const InfiniteMuon& emitter) const { 

  // typedef double IceModel; // You should put a sensible model here
  // typename InfiniteMuon::EmissionGeometry<IceModel>
  //     egeom( emitter, pehit.getOmReceiver() );
  //
  // double dperp = egeom.perpendicularDistance();
  // double c_ori = egeom.cosEmissionAngle();
  // double delay = pehit.getLeTime()-egeom.geometricalTime();
  //
  //--- actually do the work...
  // dperp = c_ori = delay;  // no-op

  return 0.;
}

} // namespace IPDF

#endif // SimplePEP_H_INCLUDED
