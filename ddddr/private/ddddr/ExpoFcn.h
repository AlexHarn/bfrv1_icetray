/**
 * Declaration of ExpoFcn
 *
 * @file ExpoFcn.h
 * @author Hans-Peter Bretz 
 */

#ifndef _EXPOFCN_H_INCLUDED
#define _EXPOFCN_H_INCLUDED

#include "ddddr/MuonEnergyFCNBase.h"
#include <vector>

/**
 * @brief Minimizer function based on an exponential function (ExpoFunction).
 */
class ExpoFcn : public MuonEnergyFCNBase
{
	public:
		/**
		 * @brief Constructs object for given set of measurements
		 *
		 * @param meas	Measured values (y-values)
		 * @param pos	Position of the values (x-values)
		 */
		ExpoFcn(const std::vector<double>& meas,
				const std::vector<double>& pos) : measurements_(meas),
		positions_(pos), errorDef_(.5) {}

		~ExpoFcn() {}

		virtual double Up() const {return errorDef_;}

		/// Returns the log likelihood for given set of parameters.
		virtual double operator()(const std::vector<double>&) const;

		std::vector<double> measurements() const {return measurements_;}

		std::vector<double> positions() const {return positions_;}

		/// Returns function value for given set of parameters.
		virtual double f(const std::vector<double>&, double) const;

		void setErrorDef(double def) {errorDef_ = def;}

	private:
		/// Measured values (energy loss).
		std::vector<double> measurements_;

		/// Position of the values (slant depth).
		std::vector<double> positions_;

		double errorDef_;
};
#endif
