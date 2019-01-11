#ifndef _MUONENERGYFCNBASE_H_
#define _MUONENERGYFCNBASE_H_

#include "Minuit2/FCNBase.h"
#include <vector>

/**
 * @brief Base class for minimizer functions, based on FCNBase. See also
 * http://seal.web.cern.ch/seal/snapshot/work-packages/mathlibs/minuit/
 * for more details.
 *
 * This class adds a method that allows to evaluate the underlying 
 * function used for minimization for a given set of parameters.
 */
class MuonEnergyFCNBase : public ROOT::Minuit2::FCNBase
{
	public:
		MuonEnergyFCNBase(){}
		MuonEnergyFCNBase(const std::vector<double>& meas,
				const std::vector<double>& pos) : measurements_(meas),
		positions_(pos), errorDef_(.5) {}

		~MuonEnergyFCNBase() {}

		virtual double Up() const {return errorDef_;}
		virtual double operator()(const std::vector<double>&) const = 0;

		std::vector<double> measurements() const {return measurements_;}
		std::vector<double> positions() const {return positions_;}

		/// Returns function value for given set of parameters.
		virtual double f(const std::vector<double>&, double) const = 0;

		void setErrorDef(double def) {errorDef_ = def;}

	private:
		std::vector<double> measurements_;
		std::vector<double> positions_;
		double errorDef_;
};
#endif

