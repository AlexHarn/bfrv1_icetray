#ifndef I3CONSTANTZENITHWEIGHT_H_INCLUDED
#define I3CONSTANTZENITHWEIGHT_H_INCLUDED

/**
 * (c) 2005 * the IceCube Collaboration
 * 
 * $Id$
 *
 * @file I3ConstantZenithWeight.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

// Gulliver utility stuff
#include "icetray/I3ServiceBase.h"
#include "icetray/I3Context.h"


/**
 * @class I3ConstantZenithWeight
 * @brief Trivial zenith weight function, for Bayesian reconstruction
 * 
 * This class defines a trivial weight function.
 * 
 * @sa I3EventLogLikelihoodCombiner
 * @sa I3ZenithWeight
 */
class I3ConstantZenithWeight : public I3ServiceBase  {
private:
public:
    /// construct self & declare configuration parameters
    I3ConstantZenithWeight( const I3Context &context ):I3ServiceBase(context){}

    /// cleanup
    virtual ~I3ConstantZenithWeight(){}

    /// get configuration parameters
    virtual void Configure(){}

    /// Get +log(likelihood) for a particular emission hypothesis
    double LogWeight( double coszen ){return 0;}

    SET_LOGGER( "I3ConstantZenithWeight" );

};

#endif /* I3CONSTANTZENITHWEIGHT_H_INCLUDED */
