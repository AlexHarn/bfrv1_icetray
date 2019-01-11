/**
 * @file I3GSLSimplex.h
 * @brief declaration of the I3GSLSimplex class
 *
 * (c) 2005 the IceCube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 */

#ifndef I3GSLSIMPLEX_H_INCLUDED
#define I3GSLSIMPLEX_H_INCLUDED

// standard library stuff
#include <vector>
#include <string>
#include <algorithm>
#include <gsl/gsl_vector.h>
#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"

// Gulliver stuff
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3MinimizerResult.h"
#include "gulliver/I3FitParameterInitSpecs.h"

/**
 * @class I3GSLSimplex
 * @brief A minimizer class which wraps the GSL implementation of simplex,
 *        such that it can be used in Gulliver-based reconstruction.
 *
 * GSL is more transparent and cleaner than ROOT. Also the GSL simplex
 * implementation does not spit out useless warnings. Once we get this
 * tested and tweaked such that it outperforms Minuit simplex, then we
 * can maybe get rid of Minuit.
 *
 * This minimizer class has two notions of tolerance: one refers to the
 * acceptable variation of the function value around the minimum, the
 * other refers to the "size of the simplex". With N free parameters,
 * a simplex consists of N+1 points (vertices) in the N-dimensional
 * parameter space.  The size of the simplex is defined as the average
 * distance of the geometrical center to all of the vertices.
 *
 * In this implementation we rescale all parameters with their stepsize
 * before and after interaction with GNU simplex, so that the effective
 * stepsize is 1.0 for all parameters. This makes the "size of the
 * simplex" actually meaningful and avoids randomly mixing units of
 * different kinds of observables with different scales of numerical
 * magnitude.
 *
 * @sa J.A. Nelder and R. Mead, A simplex method for function minimization, Computer Journal vol. 7 (1965), 308–315.
 * @sa GNU Scientific Library Reference Manual (revised and updated edition for version 1.8, section 35.8, page 473.)
 */
class I3GSLSimplex : public I3ServiceBase,
                     public I3MinimizerBase {
public:

    /// constructor for service factory
    I3GSLSimplex(const I3Context& context);

    /// constructor with full initialization
    I3GSLSimplex( std::string name, double tol, double stol, int maxi, int flatmax, bool tmp=false );

    /// destructor
    virtual ~I3GSLSimplex();

    /// configuration
    void Configure();

    /// the meat of the class: do a minimization
    I3MinimizerResult Minimize(
            I3GulliverBase &g,
            const std::vector<I3FitParameterInitSpecs> &parspecs ) ;

    /// get the tolerance (of function value)
    double GetTolerance() const { return fvalTolerance_; }

    /// set the tolerance (of function value)
    void SetTolerance(double newftol ){ fvalTolerance_ = newftol; }

    /// get the tolerance (of simplex size)
    double GetSimplexTolerance() const { return simplexTolerance_; }

    /// set the tolerance (of simplex size)
    void SetSimplexTolerance(double newstol ){ simplexTolerance_ = newstol; }

    /// get the maximum number of simplex iterations
    unsigned int GetMaxIterations() const { return maxIterations_; }

    /// set the maximum number of simplex iterations
    void SetMaxIterations(unsigned int newmaxi ){ maxIterations_ = newmaxi; }

    /// name to use for log_messages
    const std::string GetName() const {
        return I3ServiceBase::GetName();
    }

    /// macro which sets the name to use to configure log4cplus.conf
    SET_LOGGER( "I3GSLSimplex" );

private:

    /// inhibit the default constructor
    I3GSLSimplex();

    /// inhibit the copy constructor
    I3GSLSimplex( const I3GSLSimplex& );

    /// inhibit the assignment operator
    I3GSLSimplex operator= (const I3GSLSimplex& rhs);

    /// tolerance (of function value)
    double fvalTolerance_;

    /// tolerance (of simplex size)
    double simplexTolerance_;

    /// maximum number of simplex iterations
    int maxIterations_;

    /**
     * maximum number iterations spending on a perfectly flat likelihood
     * space before deciding that it will never converge
     */
    int flatPatience_;

    /**
     * TEMPRORARY option to switch on old boundary-ignoring behavior
     */
    bool backwardTMP_;

    static double Function(const gsl_vector * x, void * params);

};

I3_POINTER_TYPEDEFS(I3GSLSimplex);

#endif
