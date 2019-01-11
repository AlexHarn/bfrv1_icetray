#ifndef IPDF_LIKELIHOOD_SPEqAll_H
#define IPDF_LIKELIHOOD_SPEqAll_H

/**
 *  copyright  (C) 2011
 *  the icecube collaboration
 *  $Id$
 *
 *  @file SPEqAll.h
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 */

#include <iosfwd>
#include <boost/shared_ptr.hpp>

namespace IPDF {
namespace Likelihood {

/**
 * @brief Representation of the SPE likelihood using all photo-electron hits, weighting the pulses with charge
 *
 * Implicit dtor, copy ctor, and asignment operator are ok (boost::shared_ptr).
 */
template<typename PEProb>
class SPEqAll {
public:
    typedef SPEqAll<PEProb> Self;
    typedef PEProb PEPType;
    typedef double like_result;

    /// @brief Default ctor "new"s default PEP
    SPEqAll() : spp_(new PEPType) { }
    /// @brief Ctor taking "new" PEP passed by user - will be deleted when SPE is destroyed
    explicit SPEqAll(boost::shared_ptr<PEProb> spp) : spp_(spp) { }

    /// @brief Access the PEP object used by this likelihood
    const PEProb& getPEP() const { return *spp_; }

    /// @brief Access likelihood
    template<typename HitOm, typename EmissionHypothesis>
    like_result getLikelihood(const HitOm& hitom,
			    const EmissionHypothesis& emitter) const {
        double result = 1.;
        typename HitOm::const_iterator pehit = hitom.begin();
        for(;pehit!=hitom.end();++pehit){
            double q = (*pehit)->getPeAmplitude();
            result *= std::pow( spp_->getPdf((**pehit),emitter), q);
        }
        return result;
    }

    /// @brief Access log likelihood
    template<typename HitOm, typename EmissionHypothesis>
    like_result getLogLikelihood( const HitOm& hitom,
			    const EmissionHypothesis& emitter) const {
        double result = 0.;
        typename HitOm::const_iterator pehit = hitom.begin();
        for(;pehit!=hitom.end();++pehit){
            double q = (*pehit)->getPeAmplitude();
            result += q * spp_->getLogPdf(**pehit,emitter);
        }
        return result;
    }

    /*
    /// @brief Access cumulative likelihood
    template<typename HitOm, typename EmissionHypothesis>
    like_result getIntLikelihood( const HitOm& hitom,
			    const EmissionHypothesis& emitter) const {
        double result = 1.;
        typename HitOm::const_iterator pehit = hitom.begin();
        for(;pehit!=hitom.end();++pehit){
            result *= spp_->getIntPdf(**pehit,emitter);
        }
        return result;
    }
    */

private:
    boost::shared_ptr<PEProb> spp_;
};

} // namespace Likelihood
} // namespace IPDF


#include <iostream>

template<typename PEProb>
inline std::ostream& operator<<(std::ostream& os,
        const IPDF::Likelihood::SPEqAll<PEProb>& like) {
    return (os<<"SPEqAll: "<<like.getPEP());
}


#endif // IPDF_LIKELIHOOD_SPEqAll_H
