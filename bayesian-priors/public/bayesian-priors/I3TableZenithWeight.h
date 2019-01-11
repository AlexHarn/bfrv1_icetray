#ifndef I3TABLEZENITHWEIGHT_H_INCLUDED
#define I3TABLEZENITHWEIGHT_H_INCLUDED

/**
 *
 * @brief declaration of the I3TableZenithWeight class
 *
 * (c) 2007 * the IceCube Collaboration
 * 
 * $Id$
 *
 * @file I3TableZenithWeight.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

// Gulliver utility stuff
#include <vector>
#include "icetray/I3ServiceBase.h"
#include "icetray/I3Context.h"


/**
 * @class I3TableZenithWeight
 * @brief Zenith weight function, for Bayesian reconstruction
 * 
 * This class defines a weight function which does a table lookup, for 
 * constant-sized bins in cos(zenith), and interpolating.
 * The default uses the same coefficients as the "zenith_weight" function
 * (called by "like_ama_zenwght") for AMANDA data, as defined in
 * siegmund/recoos/reco_amanda.c, line 133-146).
 *
 * @sa I3EventLogLikelihoodCombiner
 * @sa I3ZenithWeight
 */
class I3TableZenithWeight : public I3ServiceBase  {
private:

    /**
     * table values in cos zenith
     * equidistant bins from minCosZenith_ to 1.0
     */
    std::vector<double> table_;

    /// minimum cos(zen) for polynomial (otherwise constant)
    double minCosZen_;

    /// table binsize in cos(zen)
    double dCosZen_;

    /// number of bins in table
    int nBins_;

    /// constant weight, for cos(zen) less than minimum
    double defWeight_;

    /// interpolate in 1D table to get weight factor
    double Interpolate( double coszen );

public:

    /// construct self & declare configuration parameters
    I3TableZenithWeight( const I3Context &context );

    /// cleanup
    virtual ~I3TableZenithWeight(){}

    /// get configuration parameters
    virtual void Configure();

    /// Get +log(likelihood) for a particular emission hypothesis
    double LogWeight( double coszen );

    SET_LOGGER( "I3TableZenithWeight" );

    // options: names and default values
    static const std::string tabvalOptionName_;
    static const std::string mincoszenOptionName_;
    static const std::vector<double> defTable_;
    static const double defMinCosZen_;

};

#endif /* I3TABLEZENITHWEIGHT_H_INCLUDED */
