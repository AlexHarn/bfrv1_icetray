#ifndef IPDF_AllOMsLikelihood_H
#define IPDF_AllOMsLikelihood_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file AllOMsLikelihood.h
    @version $Revision: 1.4 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <iosfwd>
#include <cassert>
#include <cmath>

#include "ipdf/Likelihood/MPE.h"
#include "ipdf/Likelihood/MPEAll.h"
#include "ipdf/Likelihood/PSA.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Likelihood/SPEAll.h"
#include "ipdf/Likelihood/SPEqAll.h"

namespace IPDF {

  /**
    @brief Main interface to IPDF

   * Is responsible for looping over all OM responses in
   * the detector response, i.e. loops over all hit OMs in
   * the event.  Sums up all the Likelihoods from each OM.
   *
   * AllOMsLikelihood owns its Likelihood (i.e. deletes it);
   * the Likelihood in-turn owns its PEP.
   */
template<typename  Likelihood>
class AllOMsLikelihood {
public:
  typedef AllOMsLikelihood<Likelihood> Self;
  typedef Likelihood LikelihoodType;
  typedef typename LikelihoodType::PEPType PEPType;
  typedef double like_result;

  /// @brief Default ctor simply "new"s a default LikelihoodType
  AllOMsLikelihood() : like_( *new LikelihoodType() ) { }
  /// @brief Ctor is passed a Likelihood from the user.
  /// (to be deleted when AllOMsLikelihood is destroyed)
  explicit AllOMsLikelihood(Likelihood& like) : like_(like) { }
  ~AllOMsLikelihood() { delete &like_; }
  
  /// @brief Access the object implementing the likelihood
  const Likelihood& getLikelihood() const { return like_; }
  /// @brief Access the object implementing the PEP
  const PEPType& getPEP() const { return like_.getPEP(); }

  /// @brief Access likelihood
  template<typename Response, typename EmissionHypothesis>
  like_result getLikelihood(const Response& response,
			    const EmissionHypothesis& emitter) const;

  /// @brief Access log likelihood
  template<typename Response, typename EmissionHypothesis>
  like_result getLogLikelihood( const Response& response,
			    const EmissionHypothesis& emitter) const;

  /// @brief Access cumulative likelihood
  template<typename Response, typename EmissionHypothesis>
  like_result getIntLikelihood( const Response& response,
			    const EmissionHypothesis& emitter) const;
  
private:
  Likelihood& like_;
};


template<typename Likelihood>
template<typename Response, typename EmissionHypothesis>
inline double AllOMsLikelihood<Likelihood>::
    getLikelihood(const Response& response,
		  const EmissionHypothesis& emitter) const {
  double result = 1.;
  typename Response::const_iterator hitom = response.begin();
  typename Response::const_iterator endom = response.end();
  for( ; hitom != endom; ++hitom ) {
    assert((!(*hitom)->empty()) && "ipdf internal error: empty HitOm");
    result *= like_.getLikelihood((**hitom),emitter);
  }
  return result;
}

template<typename Likelihood>
template<typename Response, typename EmissionHypothesis>
inline double AllOMsLikelihood<Likelihood>::
    getLogLikelihood(const Response& response,
		  const EmissionHypothesis& emitter) const {
  double result = 0.;
  typename Response::const_iterator hitom = response.begin();
  typename Response::const_iterator endom = response.end();
  for( ; hitom != endom; ++hitom ) {
    assert((!(*hitom)->empty()) && "ipdf internal error: empty HitOm");
    //if((*hitom)->empty()) { continue; }
    result += like_.getLogLikelihood((**hitom),emitter);
    assert(std::isfinite(result) && "ipdf internal error: non-finite likelihood");
  }
  return result;
}

template<typename Likelihood>
template<typename Response, typename EmissionHypothesis>
inline double AllOMsLikelihood<Likelihood>::
    getIntLikelihood(const Response& response,
		  const EmissionHypothesis& emitter) const {
  double result = 1.;
  typename Response::const_iterator hitom = response.begin();
  typename Response::const_iterator endom = response.end();
  for( ; hitom != endom; ++hitom ) {
    assert((!(*hitom)->empty()) && "ipdf internal error: empty HitOm");
    result *= like_.getIntLikelihood((**hitom),emitter);
  }
  return result;
}


} // namespace IPDF


#include <iostream>

template<typename Likelihood>
inline std::ostream& operator<<(std::ostream& os, 
    const IPDF::AllOMsLikelihood<Likelihood>& ipdf) {
  return (os<<"AllOMsLikelihood: "<<ipdf.getLikelihood());
}


#endif // IPDF_AllOMsLikelihood_H
