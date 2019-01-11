#include "ddddr/MuonEnergyMinuit2.h"
#include <cmath>
#include <vector>

// Minuit 2 includes
#include "Minuit2/MnStrategy.h"
#include "Minuit2/FCNBase.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/ModularFunctionMinimizer.h"
#include "Minuit2/SimplexMinimizer.h"
#include "Minuit2/VariableMetricMinimizer.h"
#include "Minuit2/CombinedMinimizer.h"
#include "Minuit2/FumiliMinimizer.h"
#include "Minuit2/MnUserParameters.h"


/**
 * @brief Creates ModularFunctionMinimizer needed by Minuit2 from
 * the algorithm name
 *
 * @param algorithm
 *
 * @return 
 */
boost::shared_ptr<ROOT::Minuit2::ModularFunctionMinimizer>
MinimizerFromString(Minuit2Algorithm algorithm)
{
	switch (algorithm) {
		case MIGRAD:
		{
			boost::shared_ptr<ROOT::Minuit2::VariableMetricMinimizer> 
				minimizer(new ROOT::Minuit2::VariableMetricMinimizer);
			return minimizer;
		}
		case COMBINED:
		{
			boost::shared_ptr<ROOT::Minuit2::CombinedMinimizer> 
				minimizer(new ROOT::Minuit2::CombinedMinimizer);
			return minimizer;
		}
		case FUMILI:
		{
			boost::shared_ptr<ROOT::Minuit2::FumiliMinimizer> 
				minimizer(new ROOT::Minuit2::FumiliMinimizer);
			return minimizer;
		}
		default:
		{
			boost::shared_ptr<ROOT::Minuit2::SimplexMinimizer> 
				minimizer(new ROOT::Minuit2::SimplexMinimizer);
			return minimizer;
		}
	}
}

MinimizerResult MuonEnergyMinuit2::Minimize(
		ROOT::Minuit2::FCNBase& func,
		const std::vector<FitParameterSpecs> &parspecs)
{
#ifdef I3_USE_ROOT
	// this does not work with standalone Minuit2
	// // make Minuit2 less verbose
	extern int gErrorIgnoreLevel;
	gErrorIgnoreLevel = 1001;
#endif

	bool success;
	ROOT::Minuit2::MnUserParameters params;
	ROOT::Minuit2::MnStrategy strategy(minuitStrategy_);
	std::vector<FitParameterSpecs>::const_iterator i_par;
	boost::shared_ptr<ROOT::Minuit2::ModularFunctionMinimizer> minimizer;

	for (i_par = parspecs.begin(); i_par != parspecs.end(); i_par++)
	{
		double minval = i_par->minval_;
		double maxval = i_par->maxval_;

		if ( std::isnan(minval) || std::isnan(maxval) )
		{
		  minval = maxval = 0.0;
		}
		if (minval == 0.0 && maxval == 0.0)
			/* Parameter is completely free */
			success = params.Add(i_par->name_, i_par->initval_,
			    i_par->stepsize_);
		else if (minval == maxval)
			/* Parameter is fixed */
			success = params.Add(i_par->name_, minval);
		else
			/* Parameter is constrained on both sides */
			success = params.Add(i_par->name_, i_par->initval_,
			    i_par->stepsize_, minval, maxval);
		if (!success)
		{
			log_warn("(%s) Error setting parameter '%s'",
			    name_.c_str(), i_par->name_.c_str());
		}
		else
		{
			log_debug("(%s) Par %s initval=%f step=%f minval=%f "
			    "max=%f maxiter=%d", name_.c_str(), i_par->name_.c_str(),
			    i_par->initval_, i_par->stepsize_, minval, maxval, maxIterations_);
		}
	}

	minimizer = MinimizerFromString(algorithm_);
	
	assert(minimizer);

	/* Modify tolerance for MIGRAD's definition of the EDM */
	double tol = tolerance_;
	if (algorithm_ == MIGRAD)
		tol /= 0.0002;

	ROOT::Minuit2::FunctionMinimum minimum = ROOT::Minuit2::FunctionMinimum(
			minimizer->Minimize((ROOT::Minuit2::FCNBase &) func,
				params, strategy, maxIterations_, tol));


	MinimizerResult result(parspecs.size());

	// Extract fitted parameter.
	ROOT::Minuit2::MnUserParameters finalparams = minimum.UserParameters();
	for (unsigned int i = 0; i < parspecs.size(); i++) {
		result.par_[i] = finalparams.Value(i);
		result.err_[i] = finalparams.Error(i);
			
		log_debug( "(%s) par[%d] name=%s val=%f err=%f",
		    name_.c_str(), i, parspecs[i].name_.c_str(),
		    result.par_[i], result.err_[i] );
	}

	result.converged_ = minimum.IsValid();
	result.minval_ = minimum.Fval();

	return result;
}
