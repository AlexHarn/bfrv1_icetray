/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id$
 *
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 *  This base class is the base class for the interfaces to minimizer
 *  algorithms used in log-likelihood based reconstruction.
 *
 *  @sa I3GulliverBase
 *  @sa I3Gulliver
 *  @sa I3FitParameterInitSpecs
 */

#ifndef GULLIVER_I3MINIMIZERBASE_H
#define GULLIVER_I3MINIMIZERBASE_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "gulliver/I3FitParameterInitSpecs.h"
#include "gulliver/I3GulliverBase.h"
#include "gulliver/I3MinimizerResult.h"

/**
 * @class I3MinimizerBase
 * @brief base class for minimizer algorithms, for generic loglh reconstruction
 *
 * If you would like to implement a new minimizer algorithm, you basically
 * need to implement the "Minimizer(..)" method.
 *
 * If you write a Gulliver-based reconstruction module then you only
 * need to know that this class exists and that you need to provide (a
 * pointer to) an object of the I3MinimizerBase class to the I3Gulliver
 * core of your module.
 *
 * If you implement a new I3MinimizerBase subclass, then please also
 * make a service factory which creates & configures an object of this
 * new class.
 *
 * @sa I3GulliverBase, I3Gulliver
 *
 * @todo maybe let the minimizer tell how close it thinks it is to the minimum
 */
class I3MinimizerBase {

public:

    I3MinimizerBase() {}

    virtual ~I3MinimizerBase(){}

    virtual void SetTolerance( double newtol ) = 0;
    virtual double GetTolerance( ) const = 0;

    /**
     * Get/Set max number of iterations
     * What constitutes an "iteration" may differ per minimizer algorithm.
     * It should be a measure for how hard the minimizer will try to find 
     * a minimum.
     *
     * We do not actually store the number of iterations; rather the number of
     * times that the minimizer evaluates the likelihood function.
     *
     * @sa I3Gulliver
     */
    virtual unsigned int GetMaxIterations( ) const = 0;
    virtual void SetMaxIterations( unsigned int ) = 0;

    virtual const std::string GetName() const = 0;

    /**
     * Finds a minimum of a function, given initial parameter specs.
     * The function simply takes a vector of doubles.
     *
     * It is the caller's responsibility to make sure that the number
     * of parameters expected by the function is equal to the number of
     * provided parameter specs.
     * (This may be solved a bit more elegantly; but e.g. a template<int npar>
     * won't do here, because then the number of fit parameters can only be
     * some const number in the caller's code. Too restrictive.)
     *
     * @param gullifunctor The interface to the likelihood function
     * @param parinit Specs of number, names, initvalues & bounds of fit parameters
     *
     * @returns an object with convergence info, fit parameters & errors
     *
     * @sa I3MinimizerResult, I3GulliverBase, I3FitParameterInitSpecs
     *
     * @todo is this a convenient enough interface?
     */
    virtual I3MinimizerResult Minimize(
            I3GulliverBase& gullifunctor,
            const std::vector<I3FitParameterInitSpecs>& parinit ) =0;

    /**
     * For a minimizer implementation that uses the gradient w.r.t. the
     * parameters, this method should return true. This method is called
     * just before Minimize(), so it is possible to have a minimizer that
     * only uses the gradient depending on e.g. the configuration by the user.
     */
    virtual bool UsesGradient(){return false;}

};

I3_POINTER_TYPEDEFS( I3MinimizerBase );

#endif /* GULLIVER_I3MINIMIZERBASE_H */
