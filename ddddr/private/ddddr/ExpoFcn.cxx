#include "ddddr/ExpoFcn.h"
#include "ddddr/ExpoFunction.h"
#include "ddddr/MuonEnergyFunctions.h"

#include <cassert>

double ExpoFcn::operator()(const std::vector<double>& par) const 
{
	assert(par.size() == 2);
	ExpoFunction expo(par[0], par[1]);

	double llh = 0;
	for(std::vector<double>::size_type i = 0; i != measurements_.size(); i++){
		llh -= MuonEnergyFunctions::ln_poisson(measurements_[i], expo(positions_[i]));
	}
	return llh;
}

/**
 * Not part of the Minuit2 standard, but here used as a convenience method
 * so that the chi squared and the reduced log likelihood can be calculated
 * easier in the I3MuonEnergy and I3TrueMuonEnergy modules.
 *
 * @param par	Vector of parameters
 * @param X		Slant depth
 *
 * @return  ExpoFunction(X) for given parameters.
 */
double ExpoFcn::f(const std::vector<double>& par, double X) const
{
	assert(par.size() == 2);
	ExpoFunction expof(par[0], par[1]);

	return expof(X);
}
