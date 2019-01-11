#ifndef I3POWEXPZENITHWEIGHT_H_INCLUDED
#define I3POWEXPZENITHWEIGHT_H_INCLUDED

/**
 *
 * @brief declaration of the I3PowExpZenithWeight class
 *
 * (c) 2007 * the IceCube Collaboration
 * 
 * $Id$
 *
 * @file I3PowExpZenithWeight.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

// Gulliver utility stuff
#include "icetray/I3ServiceBase.h"
#include "icetray/I3Context.h"


/**
 * @class I3PowExpZenithWeight
 * @brief Zenith weight function, for Bayesian reconstruction
 * 
 * This class defines a weight function which is a three-parameter analytic
 * function of costh=cos(zenith): w=a0*pow(costh,a1)*exp(-a2/costh);
 * The default values for the constants a0, a1, a2 are the same as for
 * AMANDA data (function like_ama_zenwght3(..), as defined in 
 * siegmund/recoos/reco_amanda.c, line 238-257).
 *
 * @sa I3EventLogLikelihoodCombiner
 * @sa I3ZenithWeight
 */
class I3PowExpZenithWeight : public I3ServiceBase  {
private:

    /// parameter a0: scale factor of weight function
    double amplitude_;
    double logAmplitude_;
    /// parameter a1: exponent in in power factor
    double power_;
    /// parameter a2: numerator in in exponent of the exponential factor
    double expFactor_;

    /// minimum cos(zen) (for lower values, always return defWeight_)
    double minCosZenith_;

    /// constant weight, for cos(zen) less than minimum
    double defWeight_;

    /// log of default weight
    double logDefWeight_;
    
    /**
     * recursive function used to approximate from below the cos(zenith) value
     * for which weight==defWeight_.
     */
    double FindMinimumCosZenith(double z, double dz, double tol, int nestlevel );

public:

    /// construct self & declare configuration parameters
    I3PowExpZenithWeight( const I3Context &context );

    /// cleanup
    virtual ~I3PowExpZenithWeight(){}

    /// get configuration parameters
    virtual void Configure();

    /// Get +log(likelihood) for a particular emission hypothesis
    double LogWeight( double coszen );

    SET_LOGGER( "I3PowExpZenithWeight" );

    static const std::string amplOptionName_;
    static const std::string powerOptionName_;
    static const std::string expOptionName_;
    static const std::string defw8OptionName_;

    static const double DEFAULT_AMPLITUDE_;
    static const double DEFAULT_POWER_;
    static const double DEFAULT_EXPFACTOR_;
    static const double DEFAULT_DEFWEIGHT_;

};

#endif /* I3POWEXPZENITHWEIGHT_H_INCLUDED */
