#ifndef I3MINIMIZERRESULT_H_INCLUDED
#define I3MINIMIZERRESULT_H_INCLUDED

#include <cmath>
#include <vector>
#include "icetray/I3FrameObject.h"

/**
 * @class I3MinimizerResult
 * 
 * Holds results of a minimization. Could be a struct, except that
 * I think it's convenient that the result vectors are already intialized
 * to the right length and with a meaningful default result.
 */

class I3MinimizerResult {
    public:

        I3MinimizerResult(int npar):
            converged_(false),
            minval_(NAN),
            par_(std::vector<double>(npar,NAN)),
            err_(std::vector<double>(npar,NAN)){}

        virtual ~I3MinimizerResult(){}

        /// Success status of minimization
        bool converged_;

        /// Value of the found minimum (NAN if not converged)
        double minval_;

        /// Parameter values which yielded the found minimum
        std::vector<double> par_;

        /// Uncertainties in parameter values which yielded the found minimum
        std::vector<double> err_;

        /**
         * Extra diagnostics, specific to the minimizer algorithm,
         * that could possibly be interesting for analysis.
         */
        I3FrameObjectPtr diagnostics_;


    private:
        I3MinimizerResult();
};

#endif /* I3MINIMIZERRESULT_H_INCLUDED */
