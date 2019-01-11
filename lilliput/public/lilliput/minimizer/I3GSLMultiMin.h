/**
 *  Copyright (C) 2010
 *  Claudio Kopper <claudio.kopper@nikhef.nl>
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/**
 * @file I3GSLMultiMin.h
 * @version $Revision$
 * @author Claudio Kopper <claudio.kopper@nikhef.nl>
 */

#ifndef I3GSLMultiMin_H_INCLUDED
#define I3GSLMultiMin_H_INCLUDED

// standard library stuff
#include <vector>
#include <string>
#include <algorithm>
#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"

// Gulliver stuff
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3MinimizerResult.h"
#include "gulliver/I3FitParameterInitSpecs.h"

// gsl stuff
#include "gsl/gsl_multimin.h"
#include "gsl/gsl_version.h"


/**
 * @class I3GSLMultiMin
 * @brief A minimizer class which wraps the GSL multideminsional minimization
 * functionality. This one currently ONLY provides minimizers that need the
 * function gradient.
 */
class I3GSLMultiMin : public I3ServiceBase, public I3MinimizerBase 
{
private:
    I3GSLMultiMin();
    I3GSLMultiMin( const I3GSLMultiMin& );
    I3GSLMultiMin operator= (const I3GSLMultiMin& rhs);

    const std::string name_;

    /**
     * Parameter: MaxIterations: Maximum number of iterations (may be overridden by calling module)
     */
    unsigned int maxIterations_;

    /**
     * Parameter: InitialStepsize: initial step size
     */
    double initialStepsize_;

    /**
     * Parameter: LinesearchTolerance: linesearch tolerance
     */
    double linesearchTolerance_;

    /**
     * Parameter: GradnormTolerance: gradient normalization tolerance
     */
    double gradnormTolerance_;

    /**
     * Parameter: Algorithm: GSL multimin algorithm to use
     * (one of: "conjugate_fr", "conjugate_pr", "vector_bfgs",
     * "vector_bfgs2" or "steepest_descent").
     * see http://www.gnu.org/software/gsl/manual/html_node/Multimin-Algorithms-with-Derivatives.html
     */
    std::string algorithm_;

    const gsl_multimin_fdfminimizer_type *gslMinimizerType_;

public:
    /// constructor with full initialization (for unit tests)
    I3GSLMultiMin(std::string name, 
                  const unsigned int maxIterations,
                  const double initialStepsize,
                  const double linesearchTolerance,
                  const double gradnormTolerance,
                  const std::string &algorithm);

    /// constructor for service factory
    I3GSLMultiMin(const I3Context& context);

    /// destructor
    virtual ~I3GSLMultiMin();

    /// configuration
    void Configure();

    /// core method: minimizer a given function with given initial conditions
    I3MinimizerResult Minimize(I3GulliverBase &g,
                               const std::vector<I3FitParameterInitSpecs> &parspecs ) ;

    /// set tolerance (what is "tolerance" supposed to be anyway ?)
    virtual double GetTolerance() const {return linesearchTolerance_;}
    virtual void SetTolerance(double newtol) {linesearchTolerance_ = newtol;}

    virtual unsigned int GetMaxIterations() const {return maxIterations_;}
    virtual void SetMaxIterations(unsigned int newmaxi) {maxIterations_ = newmaxi;}

    virtual const std::string GetName() const
    {
        return I3ServiceBase::GetName();
    }

    // we use gradients, so make this known to gulliver
    virtual bool UsesGradient(){return true;}

    SET_LOGGER( "I3GSLMultiMin" );
};

#endif
