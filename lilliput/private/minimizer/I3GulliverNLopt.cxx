/**
 * @file I3GulliverNLopt.cxx
 * @brief Gulliver interface to the minimizers of the NLopt library
 *
 * Copyright (C) 2016 the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * $Id: I3GSLSimplex.cxx 131565 2015-04-16 14:01:15Z boersma $
 *
 * @version $Revision: 131565 $
 * @date $Date: 2015-04-16 16:01:15 +0200 (Thu, 16 Apr 2015) $
 * @author Kevin Meagher
 *
 */
#include <boost/algorithm/string.hpp>
#include "icetray/I3SingleServiceFactory.h"
#include "lilliput/minimizer/I3GulliverNLopt.hpp"
#include <nlopt.hpp>
#include <iostream>

const std::map<std::string,int> I3GulliverNLopt::algorithms = {
  //Global  Non-Gradient algorithms 
  {"GN_DIRECT",               nlopt::GN_DIRECT},
  {"GN_DIRECT_L",             nlopt::GN_DIRECT_L},
  {"GN_DIRECT_L_RAND",        nlopt::GN_DIRECT_L_RAND},
  {"GN_DIRECT_NOSCAL",        nlopt::GN_DIRECT_NOSCAL},
  {"GN_DIRECT_L_NOSCAL",      nlopt::GN_DIRECT_L_NOSCAL},
  {"GN_DIRECT_L_RAND_NOSCAL", nlopt::GN_DIRECT_L_RAND_NOSCAL},
  {"GN_ORIG_DIRECT",          nlopt::GN_ORIG_DIRECT},
  {"GN_ORIG_DIRECT_L",        nlopt::GN_ORIG_DIRECT_L},
  {"GN_CRS2_LM",              nlopt::GN_CRS2_LM},
  {"GN_MLSL",                 nlopt::GN_MLSL},
  {"GN_MLSL_LDS",             nlopt::GN_MLSL_LDS},
  {"GN_ISRES",                nlopt::GN_ISRES},
  {"GN_ESCH",                 nlopt::GN_ESCH},
  
  //Local No Gradient algorithms 
  {"LN_PRAXIS",       nlopt::LN_PRAXIS},
  {"LN_COBYLA",       nlopt::LN_COBYLA},
  {"LN_NEWUOA",       nlopt::LN_NEWUOA},
  {"LN_NEWUOA_BOUND", nlopt::LN_NEWUOA_BOUND},
  {"LN_NELDERMEAD",   nlopt::LN_NELDERMEAD},
  {"LN_SBPLX",        nlopt::LN_SBPLX},
  {"LN_AUGLAG",       nlopt::LN_AUGLAG},
  {"LN_AUGLAG_EQ",    nlopt::LN_AUGLAG_EQ},
  {"LN_BOBYQA",       nlopt::LN_BOBYQA},

  //Global Gradient algorithms 
  {"GD_STOGO", nlopt::GD_STOGO},
  {"GD_STOGO_RAND", nlopt::GD_STOGO_RAND},
  {"GD_MLSL", nlopt::GD_MLSL},
  {"GD_MLSL_LDS", nlopt::GD_MLSL_LDS},

  //Local Gradient algorithms
  {"LD_LBFGS",                   nlopt::LD_LBFGS},
  {"LD_LBFGS_NOCEDAL",           nlopt::LD_LBFGS_NOCEDAL},
  {"LD_VAR1",                    nlopt::LD_VAR1},
  {"LD_VAR2",                    nlopt::LD_VAR2},
  {"LD_TNEWTON",                 nlopt::LD_TNEWTON},
  {"LD_TNEWTON_RESTART",         nlopt::LD_TNEWTON_RESTART},
  {"LD_TNEWTON_PRECOND",         nlopt::LD_TNEWTON_PRECOND},
  {"LD_TNEWTON_PRECOND_RESTART", nlopt::LD_TNEWTON_PRECOND_RESTART},
  {"LD_MMA",                     nlopt::LD_MMA},
  {"LD_AUGLAG",                  nlopt::LD_AUGLAG},
  {"LD_AUGLAG_EQ",               nlopt::LD_AUGLAG_EQ},
  {"LD_SLSQP",                   nlopt::LD_SLSQP},
  {"LD_CCSAQ",                   nlopt::LD_CCSAQ},
 
  /*
    These don't follow the naming convention
     AUGLAG,
     AUGLAG_EQ,
     G_MLSL,
     G_MLSL_LDS,
  */
};

I3GulliverNLopt::I3GulliverNLopt(const std::string &name,
				 const std::string &algorithm,
				 double tol,
				 int maxi):
  I3ServiceBase(name), I3MinimizerBase(), tolerance_(tol),
  maxIterations_(maxi)
{
  set_algorithm(algorithm);
}

I3GulliverNLopt::I3GulliverNLopt(const I3Context& context) :
  I3ServiceBase(context), I3MinimizerBase(),
  tolerance_(0.001),
  maxIterations_(10000),
  algorithm_(-1)
{
  AddParameter( "Tolerance",
		"Stop iteration when the parameters change by less than this relative amount",
		tolerance_ );
  AddParameter( "MaxIterations",
		"Stop iteration when the number of iterations reaches this number",
		maxIterations_);
  AddParameter( "Algorithm",
		"Name of the minimization algorithm to use.",
		"LN_BOBYQA");
}

void I3GulliverNLopt::Configure() {

  std::string algorithm;

  GetParameter( "Tolerance", tolerance_ );
  GetParameter( "MaxIterations", maxIterations_ );
  GetParameter( "Algorithm", algorithm);

  set_algorithm(algorithm);
}

void I3GulliverNLopt::set_algorithm(const std::string &algo)
{

  std::string algorithm(algo);
  boost::to_upper(algorithm);
  auto f = algorithms.find(algorithm);
  
  // if the algorithm string can't be found
  if ( f == algorithms.end() )
    {
      std::string s;
      for (auto a: algorithms)
	{
	  s += a.first + ", ";
	}
      s = s.substr(0,s.size()-2);
      log_fatal ("Algorithm '%s' is not a valid algorithm, "
		 "the following is a list of valid algorithms: %s",
		 algorithm.c_str(),s.c_str());
    }  
  //determine if this is a local or global algorithm
  if (f->first[0]=='G'){
    global_=true;
  } else if (f->first[0]=='L'){
    global_=false;     
  } else {
    log_fatal("Don't know weather '%s'=%d is global or not",algorithm.c_str(),algorithm_);
  }

  //determine if this algorithm uses gradients or not
  if (f->first[1]=='D'){
    uses_gradient_=true;
  } else if (f->first[1]=='N'){
    uses_gradient_=false;     
  } else {
    log_fatal("Don't know weather '%s'=%d has uses gradients or not",algorithm.c_str(),algorithm_);
  }
  
  algorithm_ = f->second;
  log_notice("NLopt is using %s algorithm '%s'=%d %s gradients",
	     global_?"global":"local",
	     algorithm.c_str(),algorithm_,
	     uses_gradient_?"with":"without");
}

namespace {
  double objective_function(const std::vector<double> &params,
                std::vector<double> &gradiant,
                void *gulliver_instance)
  {

    double likelihood_value;
    //convert void pointer to gulliver pointer
    I3GulliverBase *g = (I3GulliverBase *)gulliver_instance;	

    //nlopt uses an empty gradiant vector to indicate no gradient 
    if (gradiant.size() > 0)
      {
	//allocate temporary vector for gulliver to use
	std::vector<double> newgrad(params.size());

	//call gulliver with specified parameters
	likelihood_value = (*g)(params,newgrad);

	//copy gradient into the buffer nlopt wants to use
	for(size_t i =0; i< params.size(); i++){
	  gradiant[i]=newgrad[i];
	}
      }
    else
      {
	//call gulliver with specified parameters and no gradient 
	likelihood_value = (*g)(params);
      }

    return likelihood_value;
  }
}

I3MinimizerResult I3GulliverNLopt::Minimize(I3GulliverBase &gulliver,
					    const std::vector<I3FitParameterInitSpecs> &parspecs )
{
  
  size_t number_of_parameters = parspecs.size();

  //initilize vectors to pass to NLopt
  std::vector<double> initial_values(number_of_parameters);
  std::vector<double> lower_bounds(number_of_parameters);
  std::vector<double> upper_bounds(number_of_parameters);
  std::vector<double> step_sizes(number_of_parameters);

  //loop over the parameters to extract info from gulliver and put it in
  //vectors that NLopt can understand
  for (size_t i = 0; i!=number_of_parameters; i++)
    {
      initial_values[i]  = parspecs[i].initval_;
      step_sizes[i] = parspecs[i].stepsize_;
      
      //gulliver uses minval==maxval to indicate that there are no bounds
      //but NLopt uses that condition to indicate a fixed parameter
      if (parspecs[i].minval_ < parspecs[i].maxval_)
	{
	  lower_bounds[i] = parspecs[i].minval_;
	  upper_bounds[i] = parspecs[i].maxval_;
	}
      else
	{

	  if (global_)
	    {
	      log_fatal("Bounds must be set for all parameters when using global minimizers. "
			"Bounds have not been set with parameter '%s'.",
			parspecs[i].name_.c_str());
	    }	  
	  lower_bounds[i] = -HUGE_VAL;
	  upper_bounds[i] = +HUGE_VAL;
	}

      log_debug("parameter #%lu: Name='%s' initial value=%g step size=%g "
		"lower bound=%f upper bound=%f",
		i,parspecs[i].name_.c_str(),
		initial_values[i],step_sizes[i],
		lower_bounds[i],upper_bounds[i]);
    }
  
  //initilize NLopt object
  nlopt::opt opt(nlopt::algorithm(algorithm_), number_of_parameters);
  opt.set_min_objective(objective_function, &gulliver);
  opt.set_lower_bounds(lower_bounds);
  opt.set_upper_bounds(upper_bounds);
  opt.set_initial_step(step_sizes);
  opt.set_xtol_rel(tolerance_);
  opt.set_maxeval(maxIterations_);
  
  //container for the minimum function value
  double minimum_value;

  //container for the resutl
  std::vector<double> final_values(initial_values);
  nlopt::result nlresult = opt.optimize(final_values, minimum_value);
  
  //translate the result into what  gulliver understands
  I3MinimizerResult result(number_of_parameters);
  result.converged_ = (nlresult > 0);
  result.par_       = final_values;
  result.minval_    = minimum_value;
  result.err_       = std::vector<double>(number_of_parameters,NAN);
    
  return result;
};

typedef I3SingleServiceFactory<I3GulliverNLopt,I3MinimizerBase> I3GulliverNLoptFactory;
I3_SERVICE_FACTORY( I3GulliverNLoptFactory );
