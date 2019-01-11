#include "ddddr/TomFFcn.h"
#include "ddddr/TomFFunction.h"
#include "ddddr/MuonEnergyFunctions.h"

#include <cassert>

double TomFFcn::operator()(const std::vector<double>& par) const 
{
	assert(par.size() == 3);
	TomFFunction tomf(par[0], par[1], par[2]);

	double llh = 0;
	for(std::vector<double>::size_type i = 0; i != measurements_.size(); i++){
		llh -= MuonEnergyFunctions::ln_poisson(measurements_[i], tomf(positions_[i]));
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
 * @return  TomFFunction(X) for given parameters.
 */
double TomFFcn::f(const std::vector<double>& par, double X) const
{
	assert(par.size() == 3);
	TomFFunction tomf(par[0], par[1], par[2]);

	return tomf(X);
}
