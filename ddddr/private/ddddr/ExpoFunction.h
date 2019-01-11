/**
 * Implementation of ExpoFunction
 *
 * @file ExpoFunction.h
 * @author Hans-Peter Bretz 
 */

#ifndef _EXPOFUNCTION_H_INCLUDED
#define _EXPOFUNCTION_H_INCLUDED

#include <math.h>

/**
 * @brief Class that allows to call an exponential function for 
 * a given set of parameters.
 *
 * Used by ExpoFcn.h.
 */
class ExpoFunction
{
	public:

		ExpoFunction(double N, double b) : norm_(N), slope_(b) {}

		~ExpoFunction() {}

		double N() const {return norm_;}
		double b() const {return slope_;}

		/**
		 * @brief Computes the exp. function.

		 * @param x
		 *
		 * @return exp(N - b*x)
		 */
		double operator()(double x) const {
			return exp(norm_ - slope_*x);
		}

	private:

		double norm_;
		double slope_;
};
#endif
