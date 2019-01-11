#ifndef _TOMFFUNCTION_H_INCLUDED
#define _TOMFFUNCTION_H_INCLUDED

#include <math.h>

/**
 * @brief Class that allows to call an Tom Feusels' function for 
 * a given set of parameters.
 *
 * Used by TomFFcn.h.
 */
class TomFFunction 
{
	public:

		TomFFunction(double N, double b, double gamma) : 
			norm_(N), slope_(b), gamma_(gamma), a_(.24) {}

		~TomFFunction() {}

		double N() const {return norm_;}
		double b() const {return slope_;}
		double gamma() const {return gamma_;}

		/**
		 * @brief Computes the TomF function.

		 * See http://arxiv.org/abs/0912.4668 for reference. Further information at 
		 * http://inwfsun1.ugent.be/~tom/Feusels_0518_ICRC09.pdf, 
		 *  and
		 * in  "Measurement of cosmic ray composition and energy spectrum between 
		 * 1 PeV and 1 EeV with IceTop and IceCube", PhD thesis Tom Feusels 2013, UGent.
		 */
		double operator()(double x) const {
			double emin = (a_/slope_)*(exp(slope_*x)-1);
			return norm_*exp(-slope_*x)*pow(emin,-gamma_)*(a_/gamma_+emin*slope_/(gamma_-1));
		}

	private:

		double norm_;
		double slope_;
		double gamma_;
		double const a_;
};
#endif
