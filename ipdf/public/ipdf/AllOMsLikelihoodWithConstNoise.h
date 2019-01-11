#ifndef IPDF_AllOMsLikelihoodWithConstNoise_H
#define IPDF_AllOMsLikelihoodWithConstNoise_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file AllOMsLikelihoodWithConstNoise.h
    @version $Revision$
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
    @author Hacked by David Boersma <boersma@icecube.wisc.edu>
*/

#include <iosfwd>
#include "gulliver/utilities/FastLogSum.h"

namespace IPDF {

  /**
   * @brief Main interface to IPDF
   *
   * Is responsible for looping over all OM responses in
   * the detector response, i.e. loops over all hit OMs in
   * the event.  Sums up all the Likelihoods from each OM.
   *
   * AllOMsLikelihoodWithConstNoise owns its Likelihood (i.e. deletes it);
   * the Likelihood in-turn owns its PEP.
   * 
   * DJB: hacked this, in order to add a constant noise term to each OM's likelihood.
   * TODO: proper normalization (both relative and absolute)
   */
template<typename  Likelihood>
class AllOMsLikelihoodWithConstNoise {
public:
  typedef AllOMsLikelihoodWithConstNoise<Likelihood> Self;
  typedef Likelihood LikelihoodType;
  typedef typename LikelihoodType::PEPType PEPType;
  typedef double like_result;

  /// @brief Default ctor simply "new"s a default LikelihoodType
  AllOMsLikelihoodWithConstNoise(double noiseprob=1.0e-8) :
      like_( *new LikelihoodType()),noiseProb_(noiseprob) {
          assert(noiseProb_>0);
      }
  /// @brief Ctor is passed a Likelihood from the user.
  /// (to be deleted when AllOMsLikelihoodWithConstNoise is destroyed)
  explicit AllOMsLikelihoodWithConstNoise(Likelihood& like,double noiseprob=1.0e-8) :
      like_(like),noiseProb_(noiseprob) {
          assert(noiseProb_>0);
      }
  ~AllOMsLikelihoodWithConstNoise() { delete &like_; }
  
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
  double noiseProb_;
};

#ifndef __CINT__

template<typename Likelihood>
template<typename Response, typename EmissionHypothesis>
inline double AllOMsLikelihoodWithConstNoise<Likelihood>::
    getLikelihood(const Response& response,
		  const EmissionHypothesis& emitter) const {
  double result = 1.;
  typename Response::const_iterator hitom = response.begin();
  typename Response::const_iterator endom = response.end();
  for( ; hitom != endom; ++hitom ) {
    assert((!(*hitom)->empty()) && "ipdf internal error: empty HitOm");
    result *= (noiseProb_+like_.getLikelihood((**hitom),emitter));
  }
  return result;
}

template<typename Likelihood>
template<typename Response, typename EmissionHypothesis>
inline double AllOMsLikelihoodWithConstNoise<Likelihood>::
    getLogLikelihood(const Response& response,
		  const EmissionHypothesis& emitter) const {
  // double result;
  FastLogSum result;
  typename Response::const_iterator hitom = response.begin();
  typename Response::const_iterator endom = response.end();
  for( ; hitom != endom; ++hitom ) {
    assert((!(*hitom)->empty()) && "ipdf internal error: empty HitOm");
    //if((*hitom)->empty()) { continue; }
    // result += like_.getLogLikelihood((**hitom),emitter);
    // result += log(noiseProb_+like_.getLikelihood((**hitom),emitter));
    double pdf = noiseProb_+like_.getLikelihood((**hitom),emitter);
    assert(std::isfinite(pdf) && "ipdf internal error: non-finite likelihood");
    result.FastAdd(pdf);
  }
  return result.GetLogSum();
}

template<typename Likelihood>
template<typename Response, typename EmissionHypothesis>
inline double AllOMsLikelihoodWithConstNoise<Likelihood>::
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

#endif // __CINT__

} // namespace IPDF

#ifndef __CINT__

#include <iostream>

template<typename Likelihood>
inline std::ostream& operator<<(std::ostream& os, 
    const IPDF::AllOMsLikelihoodWithConstNoise<Likelihood>& ipdf) {
  return (os<<"AllOMsLikelihoodWithConstNoise: "<<ipdf.getLikelihood());
}

#endif // __CINT__

#endif // IPDF_AllOMsLikelihoodWithConstNoise_H
