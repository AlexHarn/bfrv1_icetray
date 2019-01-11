#ifndef IPDF_LIKELIHOOD_SPEAll_H
#define IPDF_LIKELIHOOD_SPEAll_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file SPEAll.h
    @version $Revision: 1.5 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <iosfwd>
#include <boost/shared_ptr.hpp>

namespace IPDF {
namespace Likelihood {

/**
 * @brief Representation of the SPE likelihood using all photo-electron hits
 *
 * Implicit dtor, copy ctor, and asignment operator are ok (boost::shared_ptr).
 */
template<typename PEProb>
class SPEAll {
public:
  typedef SPEAll<PEProb> Self;
  typedef PEProb PEPType;
  typedef double like_result;

  /// @brief Default ctor "new"s default PEP
  SPEAll() : spp_(new PEPType) { }
  /// @brief Ctor taking "new" PEP passed by user - will be deleted when SPE is destroyed
  explicit SPEAll(boost::shared_ptr<PEProb> spp) : spp_(spp) { }
  
  /// @brief Access the PEP object used by this likelihood
  const PEProb& getPEP() const { return *spp_; }

  /// @brief Access likelihood
  template<typename HitOm, typename EmissionHypothesis>
  like_result getLikelihood(const HitOm& hitom,
			    const EmissionHypothesis& emitter) const
  {
    double result = 1.;
    typename HitOm::const_iterator pehit = hitom.begin();
    for(;pehit!=hitom.end();++pehit){
      result *= spp_->getPdf((**pehit),emitter);
    }
    return result;
  }

  /// @brief Access log likelihood
  template<typename HitOm, typename EmissionHypothesis>
  like_result getLogLikelihood( const HitOm& hitom,
			    const EmissionHypothesis& emitter) const
  {
    double result = 0.;
    typename HitOm::const_iterator pehit = hitom.begin();
    for(;pehit!=hitom.end();++pehit){
      result += spp_->getLogPdf(**pehit,emitter);
    }
    return result;
  }

  /// @brief Access cumulative likelihood
  template<typename HitOm, typename EmissionHypothesis>
  like_result getIntLikelihood( const HitOm& hitom,
			    const EmissionHypothesis& emitter) const
  {
    double result = 1.;
    typename HitOm::const_iterator pehit = hitom.begin();
    for(;pehit!=hitom.end();++pehit){
      result *= spp_->getIntPdf(**pehit,emitter);
    }
    return result;
  }

private:
  boost::shared_ptr<PEProb> spp_;
};

} // namespace Likelihood
} // namespace IPDF

#include <iostream>

template<typename PEProb>
inline std::ostream& operator<<(std::ostream& os, 
    const IPDF::Likelihood::SPEAll<PEProb>& like) {
  return (os<<"SPEAll: "<<like.getPEP());
}

#endif // IPDF_LIKELIHOOD_SPEAll_H
