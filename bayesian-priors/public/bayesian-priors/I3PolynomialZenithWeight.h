#ifndef I3POLYNOMIALZENITHWEIGHT_H_INCLUDED
#define I3POLYNOMIALZENITHWEIGHT_H_INCLUDED

/**
 *
 * @brief declaration of the I3PolynomialZenithWeight class
 *
 * (c) 2007 * the IceCube Collaboration
 * 
 * $Id$
 *
 * @file I3PolynomialZenithWeight.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

// Gulliver utility stuff
#include "icetray/I3ServiceBase.h"
#include "icetray/I3Context.h"


/**
 * @class I3PolynomialZenithWeight
 * @brief Zenith weight function, for Bayesian reconstruction
 * 
 * This class defines a weight function which is polynomial of cos(zenith).
 * The default uses the same coefficients as Ty's P7 zenith weight function
 * for AMANDA data (function like_ama_zenwght2(..), defined in 
 * siegmund/recoos/reco_amanda.c, line 205-235).
 *
 * @sa I3EventLogLikelihoodCombiner
 * @sa I3ZenithWeight
 */
class I3PolynomialZenithWeight : public I3ServiceBase  {
private:

    /// polynomial coefficients
    std::vector<double> coefficients_;

    /// minimum cos(zen) for polynomial (otherwise constant)
    double minCosZen_;

    /// constant weight, for cos(zen) less than minimum
    double defWeight_;

public:

    /// construct self & declare configuration parameters
    I3PolynomialZenithWeight( const I3Context &context );

    /// cleanup
    virtual ~I3PolynomialZenithWeight(){}

    /// get configuration parameters
    virtual void Configure();

    /// Get +log(likelihood) for a particular emission hypothesis
    double LogWeight( double coszen );

    SET_LOGGER( "I3PolynomialZenithWeight" );

    static const std::string coefsOptionName_;
    static const std::string mincoszenOptionName_;
    static const std::string defw8OptionName_;
    static const double DEFAULT_DEFWEIGHT_;
    static const double DEFAULT_MINCOSZEN_;
    static const std::vector<double> DEFAULT_COEFFICIENTS_;

    double EvalPolynomial( double coszen );

};

#endif /* I3POLYNOMIALZENITHWEIGHT_H_INCLUDED */
