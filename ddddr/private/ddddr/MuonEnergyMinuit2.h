#ifndef _I3MUONENERGYMINUIT2_H_INCLUDED
#define _I3MUONENERGYMINUIT2_H_INCLUDED

#include <icetray/I3Logging.h>
#include "ddddr/FitParameterSpecs.h"
#include "ddddr/MinimizerResult.h"
#include "Minuit2/FCNBase.h"

enum Minuit2Algorithm {SIMPLEX, MIGRAD, COMBINED, FUMILI};
enum Minuit2FitFCN {EXPOFCN, TOMFFCN};

/**
 * @brief A class to perform a mimization using Minuit2, based on I3GullerMinuit2. Since 
 * I3GulliverMinuit2 serves a much more general purpose, I adapted it for the simple 
 * minimizations needed inside DDDDR.
 */
class MuonEnergyMinuit2
{
	public:
		MuonEnergyMinuit2(std::string name, const double tol,
				unsigned int maxi, int mpl, int mstr, std::string alg) :
			name_(name), tolerance_(tol), maxIterations_(maxi), 
			minuitStrategy_(mstr) 
		{
				if (alg == "SIMPLEX")
					algorithm_ = SIMPLEX;
				else if (alg == "MIGRAD")
					algorithm_ = MIGRAD;
				else if (alg == "COMBINED")
					algorithm_ = COMBINED;
				else if (alg == "FUMILI")
					algorithm_ = FUMILI;
				else {
					log_warn("Unknown algorithm '%s' requested! Reverting to "
						"'SIMPLEX'.", alg.c_str());
					algorithm_ = SIMPLEX;
				}
		}

		~MuonEnergyMinuit2() {}

		/**
		 * @brief Minimize function func for given parameters.
		 *
		 * @param func
		 * @param parspecs
		 *
		 * @return Result of minimization as MinimizerResult, containing the 
		 * best fit parameters and their uncertainties
		 */
		MinimizerResult Minimize(
				ROOT::Minuit2::FCNBase& func,
				const std::vector<FitParameterSpecs> &parspecs);

	private:
		std::string name_;
		double tolerance_;
		unsigned int maxIterations_;
		int minuitStrategy_;
		Minuit2Algorithm algorithm_;
};

#endif

