#ifndef MINIMIZERRESULT_H_INCLUDED
#define MINIMIZERRESULT_H_INCLUDED

#include <vector>

/**
 * Taken from I3MinimizerResult from gulliver. Holds parameter and 
 * uncertainties resulting from a minimization.
 */

class MinimizerResult {
	public:


		MinimizerResult(int npar):
			converged_(false),
			minval_(NAN),
			par_(std::vector<double>(npar,NAN)),
			err_(std::vector<double>(npar,NAN)){}

		virtual ~MinimizerResult(){}

		/// Success status of minimization
        bool converged_;

        /// Value of the found minimum (NAN if not converged)
        double minval_;

        /// Parameter values which yielded the found minimum
        std::vector<double> par_;

        /// Uncertainties in parameter values which yielded the found minimum
        std::vector<double> err_;

};
#endif // MINIMIZERRESULT_H_INCLUDED
