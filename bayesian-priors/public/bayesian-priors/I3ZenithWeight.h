#ifndef I3ZENITHWEIGHT_H_INCLUDED
#define I3ZENITHWEIGHT_H_INCLUDED

/**
 *
 * @brief declaration of the I3ZenithWeight class
 *
 * (c) 2007 * the IceCube Collaboration
 * $Id$
 *
 * @file I3ZenithWeight.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

// Gulliver stuff
#include "gulliver/I3EventLogLikelihoodBase.h"

// framework stuff
#include "icetray/IcetrayFwd.h"
#include "dataclasses/physics/I3Particle.h"
#include "gulliver/I3EventHypothesis.h"
#include "icetray/I3ServiceBase.h"

/**
 * @class I3ZenithWeight
 * @brief Zenith weight function, for Bayesian reconstruction
 * 
 * This templated class easifies the coding of zenith-weight type Bayesian
 * priors (priors which only depend on cos(zenith)).
 * 
 * @todo Maybe the "penalty" value should not be configurable in this class, but
 *       rather be determined by the actual weight calculator.
 *
 * @sa I3EventLogLikelihoodCombiner
 */
template<class WeightCalc>
class I3ZenithWeight : public I3EventLogLikelihoodBase,
                       public WeightCalc {
private:
    bool flip_;
    double penalty_;
    double penaltySlope_;
    double minCosZenith_;
    double maxCosZenith_;
    double zenithCache_;
    double weightCache_;
public:
    /// construct self & declare configuration parameters
    I3ZenithWeight( const I3Context &context );

    /// cleanup
    virtual ~I3ZenithWeight(){}

    /// get configuration parameters
    virtual void Configure();

    /// provide geometry: the zenith weight function does not use the geometry
    void SetGeometry( const I3Geometry &geo){}

    /// provide event data: the zenith weight function does not use event data.
    void SetEvent( const I3Frame &f ){}

    /// Get +log(likelihood) for a particular emission hypothesis
    double GetLogLikelihood( const I3EventHypothesis &t );

    /// the zenith weight function does not use event data.
    unsigned int GetMultiplicity(){ return 0; }

    /// tell your name
    const std::string GetName() const;

    SET_LOGGER( "I3ZenithWeight" );

    static const std::string flipOptionName_;
    static const std::string penaltyOptionName_;
    static const std::string slopeOptionName_;
    static const std::string coszenrangeOptionName_;

    static const bool DEFAULT_FLIP_;
    static const double DEFAULT_PENALTY_;
    static const double DEFAULT_SLOPE_;
    static const double DEFAULT_MINCOSZENITH_;
    static const double DEFAULT_MAXCOSZENITH_;

};

#endif /* I3ZENITHWEIGHT_H_INCLUDED */
