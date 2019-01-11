#ifndef IPDF_LIKELIHOOD_SPE1st_H
#define IPDF_LIKELIHOOD_SPE1st_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file SPE1st.h
    @version $Revision: 1.4 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <iosfwd>
#include <boost/shared_ptr.hpp>

namespace IPDF {
namespace Likelihood {

/**
 * @brief Representation of the SPE likelihood using only the first photon hit on an OM
 *
 * Implicit dtor, copy ctor, and asignment operator are ok (boost::shared_ptr).
 */
template<typename PEProb>
class SPE1st {
public:
  typedef SPE1st<PEProb> Self;
  typedef PEProb PEPType;
  typedef double like_result;

  /// @brief Default ctor "new"s default PEP
  SPE1st() : spp_( new PEPType() ) { }
  /// @brief Ctor taking "new" PEP passed by user - will be deleted when SPE is destroyed
  explicit SPE1st(boost::shared_ptr<PEProb> spp) : spp_(spp) { }
  
  /// @brief Access the PEP object used by this likelihood
  const PEProb& getPEP() const { return *spp_; }

  /// @brief Access likelihood
  template<typename HitOm, typename EmissionHypothesis>
  double getLikelihood(const HitOm& hitom,
		       const EmissionHypothesis& emitter) const
  {
    double result = spp_->getPdf((hitom[0]),emitter);
    return result;
  }

  /// @brief Access log likelihood
  template<typename HitOm, typename EmissionHypothesis>
  double getLogLikelihood(const HitOm& hitom,
			  const EmissionHypothesis& emitter) const
  {
    double result = spp_->getLogPdf((hitom[0]),emitter);
    return result;
  }

  /// @brief Access cumulative likelihood
  template<typename HitOm, typename EmissionHypothesis>
  double getIntLikelihood(const HitOm& hitom,
			  const EmissionHypothesis& emitter) const
  {
    return spp_->getIntPdf((hitom[0]),emitter);
  }
  
private:
  boost::shared_ptr<PEProb> spp_;
};

} // namespace Likelihood
} // namespace IPDF


template<typename PEProb>
inline std::ostream& operator<<(std::ostream& os, 
    const IPDF::Likelihood::SPE1st<PEProb>& like) {
  return (os<<"SPE1st: "<<like.getPEP());
}


#endif // IPDF_LIKELIHOOD_SPE1st_H
