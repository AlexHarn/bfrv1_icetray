/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id$
 *
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 *  This base class is the medium through which the minimizer talks
 *  with the likelihood function and the "recomethod" aka "strategy"
 *  aka "global minimizer".
 *
 */

#ifndef I3GULLIVERBASE_H_INCLUDED
#define I3GULLIVERBASE_H_INCLUDED

#include <vector>
#include "icetray/IcetrayFwd.h"

/**
 * @class I3GulliverBase
 * @brief Functor interface to likelihood function, for the minimizer algorithms
 *
 * You only need to know about this class if you would like to implement
 * a (Gulliver wrapper of a) minimizer algorithm.  You do *not* need
 * to know about it if you are implementing other Gulliver components,
 * such as a likelihood functions and parametrizations, or if you are
 * writing a new Gulliver-based reconstruction module.
 *
 * This virtual base class is the medium through which the minimizer
 * evaluates the likelihood function. Its only implementation is the
 * I3Gulliver class, which uses the parametrization object to convert
 * a vector of doubles to a physics object (I3EventHypothesis) and
 * evaluates the likelihood function with that. This base class does
 * not depend on any physics-related classes.
 *
 * @sa I3Gulliver, I3MinimizerBase
 */
class I3GulliverBase {

public:

    /// constructor (idle)
    I3GulliverBase(){}

    /// destructor (idle)
    virtual ~I3GulliverBase(){}

    /**
     * Get function value
     * @param[in] par vector with fit parameter values
     * @returns function value
     */
    virtual double operator()(const std::vector<double> &par ) = 0;

    /**
     * Get function value and its gradient
     * @param[in] par vector with fit parameter values
     * @param grad with gradients w.r.t. parameters
     * @returns function value
     */
    virtual double operator()(const std::vector<double> &par, std::vector<double> &grad ) = 0;

};

I3_POINTER_TYPEDEFS( I3GulliverBase );

#endif /* I3GULLIVERBASE_H_INCLUDED */
