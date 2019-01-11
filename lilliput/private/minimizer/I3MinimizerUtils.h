/**
 * @file I3MinimizerUtils.h
 * @brief shared functions for taming troublesome minimizers
 *
 * (c) 2010 the IceCube Collaboration
 *
 * $Id$
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 * @todo Maybe I3MinimizerUtils should be moved out of lilliput, I think it belongs in gulliver/utilities.
 */

#ifndef I3MINIMIZERUTILS_H_INCLUDED
#define I3MINIMIZERUTILS_H_INCLUDED

#include "icetray/I3ServiceBase.h"
#include "gulliver/I3GulliverBase.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3MinimizerResult.h"
#include "gulliver/I3FitParameterInitSpecs.h"

namespace I3MinimizerUtils {

    /**
     * It may happen that the minimizer sees a perfectly constant function.
     * A constant function has a minimum, so the minimizer can give any solution,
     * because any point in the parameter is technically an extreme value, both
     * a minimum and maximum.
     * For our reconstruction purposes we want a different approach; if the likelihood
     * space is flat, then we want the minimizer to report a failure.
     * It seems that Minuit puts the solution back to the seed in such cases.
     * We use this feature to find such events, and then we test for flatness by
     * calculating the function 2*Npar more times, each time varying the parameters
     * by + or - one stepsize, and if we always find the same value, then we change the
     * convergence flag to false.
     * @param[in] self self-reference to the calling minimizer service
     * @param[in,out] zeptognat minimizer result to be checked
     * @param[in] parspecs initial parameter specifications
     * @param gulliver gulliver base reference to compute function values
     */
    void CheckMinimum(const I3ServiceBase & self, I3MinimizerResult & zeptognat,
        const std::vector<I3FitParameterInitSpecs> &parspecs, I3GulliverBase & gulliver);

    /**
     * @class ParameterWithInclusiveBounds
     * Bounded variables: some minimizers (such as GSL simplex) do not implement
     * bounded parameters.
     * This helper class provides a standard conversion between a bounded
     * variable b (inclusive bounds) and a free parameter f:
     * b = 0.5*(min+max) + 0.5 * (max - min) * sin(f);
     * So the minimizer function becomes cyclical in this variable.
     * I am not aware of methods to implement inclusive bounds without using
     * cyclical functions. Drawback: when the minimizer goes crazy and lets
     * f run till values so large that their floating point accuracy is larger
     * than 2pi, then the parameter conversion becomes ill-defined. So the 
     * "free" parameter is actually not *really* free.
     *
     * @todo: ParameterWithExclusiveBounds, I guess that can be done with tan and atan.
     */
    class ParameterWithInclusiveBounds {
        private:
            double mean_; //! mean of max and min
            double span_; //! half diff between max and min
            double xbounded_; //! bounded parameter value
            double xfree_; //! corresponding free parameter value
            ParameterWithInclusiveBounds();
        public:
            //! simple constructor useful for writing unit tests
            ParameterWithInclusiveBounds( double min, double max, double val);

            //! simple constructor useful in Gulliver minimizer implementations
            ParameterWithInclusiveBounds( const I3FitParameterInitSpecs &spec );

            /**
             * @param[in] boundstep: stepsize for bounded value
             * If the actual bounded value is not exactly the mean of the bounds
             * then the corresponding stepsizes for the free parameter are
             * asymmetric: you get the smaller one (the one for stepping towards
             * the mean of the bounds)
             */
            double GetFreeStep(double boundstep);

            // Reverse of the above: never needed.
            // double GetBoundStep(double freestep);

            /**
             * @param[in] xfree: unbounded value
             * Computes the bounded value
             */
            void SetFreeValue(double xfree);

            double GetFreeValue(){
                return xfree_;
            }

            /**
             * @param[in] xbounded: bounded value
             * Computes the free value
             */
            void SetBoundedValue(double xbounded);

            double GetBoundedValue(){
                return xbounded_;
            }
    };

// SET_LOGGER("I3MinimizerUtils");
}

#endif
