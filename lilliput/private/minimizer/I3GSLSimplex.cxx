/**
 * @file I3GSLSimplex.cxx
 * @brief implementation of the I3GSLSimplex class
 *
 * (c) 2005 the IceCube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

// standard library stuff
#include <cassert>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

// GSL stuff
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>

// icetray stuff
#include "icetray/I3SingleServiceFactory.h"

// Gulliver stuff
#include "gulliver/I3GulliverBase.h"
#include "lilliput/minimizer/I3GSLSimplex.h"
#include "minimizer/I3MinimizerUtils.h"

using I3MinimizerUtils::ParameterWithInclusiveBounds;

// default values for options
static const unsigned int DEFAULT_MAXITERATIONS = 10000;
static const double DEFAULT_TOLERANCE = 0.1;
static const double DEFAULT_SIMPLEX_TOLERANCE = 0.1;
static const int DEFAULT_PATIENCE = 0;

// option names
static const char* tolerance_optionname = "Tolerance";
static const char* simplextolerance_optionname = "SimplexTolerance";
static const char* maxiterations_optionname = "MaxIterations";
static const char* flatnesspatience_optionname = "FlatPatience";
static const char* backward_optionname = "BackwardTMP";

/// helper class
class I3GSLSimplex_ImplHelper {
    private:
        I3GulliverBase& llhFunctor_;
        gsl_vector* gslpar_;
        std::vector<double> par_;
        std::vector<double> steps_;
        std::map<int,ParameterWithInclusiveBounds> bounds_;
    public:
        I3GSLSimplex_ImplHelper(I3GulliverBase &g, const std::vector<I3FitParameterInitSpecs> &parspecs, bool old=false );
        ~I3GSLSimplex_ImplHelper();
        I3GulliverBase& GetFunctor(){ return llhFunctor_; }
        std::vector<double>& SetPar(const gsl_vector *x);
        std::vector<double>& GetSteps(){ return steps_; }
        gsl_vector* GetGSLPar(){ return gslpar_; }
};

// When the GSL simplex minimizer wants to evaluate a function value
// with a new set of (unbounded) parameter values, it delivers these
// as a gsl_vector. This method translates it back to a vector<double>
// (and if needed, transforms between the free/unbounded representation
// and bounded one).
std::vector<double>&
I3GSLSimplex_ImplHelper::SetPar(const gsl_vector *x){
    unsigned int npar = x->size;
    const double *pxdata = x->data;
    assert( x->stride == 1 );
    assert( par_.size() == npar );
    assert( steps_.size() == npar );
    std::transform(pxdata,pxdata+npar,steps_.begin(),par_.begin(),std::multiplies<double>());
    std::map<int,ParameterWithInclusiveBounds>::iterator ib = bounds_.begin();
    for ( ; ib != bounds_.end(); ib++ ){
        int i = ib->first;
        ib->second.SetFreeValue(par_[i]);
        double xnew = ib->second.GetBoundedValue();
        // std::cout << "i=" << i << " xold=" << par_[i];
        par_[i] = xnew;
        // std::cout << " xnew=" << xnew << std::endl;
    }
    return par_;
}

I3GSLSimplex_ImplHelper::I3GSLSimplex_ImplHelper(
        I3GulliverBase &g,
        const std::vector<I3FitParameterInitSpecs> &parspecs,
        bool old ): llhFunctor_(g){
    size_t dim = parspecs.size();
    gslpar_ = gsl_vector_alloc(dim);
    par_.resize(dim);
    steps_.resize(dim);
    for (size_t i=0; i<dim; i++ ){
        const I3FitParameterInitSpecs &spec = parspecs[i];
        par_[i] = spec.initval_;
        if ( (!old) && (spec.minval_< spec.maxval_) ){
            ParameterWithInclusiveBounds pwib(spec);
            steps_[i] = pwib.GetFreeStep(spec.stepsize_);
            bounds_.insert(std::make_pair(i,pwib));
            gsl_vector_set( gslpar_, i, pwib.GetFreeValue() / steps_[i] );
        } else {
            steps_[i] = spec.stepsize_;
            gsl_vector_set( gslpar_, i, par_[i] / steps_[i] );
        }
    }
}

I3GSLSimplex_ImplHelper::~I3GSLSimplex_ImplHelper(){
    gsl_vector_free(gslpar_);
}



I3GSLSimplex::I3GSLSimplex(const I3Context& context) :
        I3ServiceBase(context), I3MinimizerBase(),
        fvalTolerance_(DEFAULT_TOLERANCE),
        simplexTolerance_(DEFAULT_SIMPLEX_TOLERANCE),
        maxIterations_(DEFAULT_MAXITERATIONS),
        flatPatience_(DEFAULT_PATIENCE),
        backwardTMP_(false) {

    AddParameter( tolerance_optionname,
                  "Tolerance for minimum (may be overridden by calling module). "
                  "See also 'SimplexTolerance'",
                  fvalTolerance_ );

    AddParameter( simplextolerance_optionname,
                  "The regular tolerance represents an estimate for the difference between the "
                  "numerically found minimum and the true minimum. The simplex algorithm defines "
                  "another tolerance value, namely the size of the simplex: average distance "
                  "from the geometrical center of the simplex to all its vertices, measured in "
                  "numbers of stepsizes. This is used for an additional convergence criterion; "
                  "with this option you can set the maximum value of the simplex size.",
                  simplexTolerance_ );

    AddParameter( maxiterations_optionname,
                  "Maximum number of iterations (may be overridden by calling module)",
                  maxIterations_);

    AddParameter( flatnesspatience_optionname,
                  "Maximum number of iterations with identical function value. "
                  "In case of a bad seed, simplex might not find any hypothesis were at least "
                  "one of the hits has a likelihood better than the noise likelihood. Instead of "
                  "searching the full number of iterations you can save some time by specifying "
                  "the max number of iterations to try on such a 'flat' likelihood space. If you "
                  "set this to zero, then simplex will search the full number of iterations as "
                  "set with the MaxIterations configuration parameter.",
                  flatPatience_ );

    AddParameter( backward_optionname,
                  "TEMPORARY: the GSL simplex wrapper used to ignore parameter boundaries. "
                  "Now they are actually taken into account. Set this option to True if you "
                  "want the old behavior (for debugging and testing). THIS OPTION WILL BE REMOVED.",
                  backwardTMP_ );

}

void I3GSLSimplex::Configure() {
    GetParameter( tolerance_optionname, fvalTolerance_ );
    GetParameter( simplextolerance_optionname, simplexTolerance_ );
    GetParameter( maxiterations_optionname, maxIterations_ );
    GetParameter( flatnesspatience_optionname, flatPatience_ );
    GetParameter( backward_optionname, backwardTMP_ );

    log_debug( "(%s) %s=%f", GetName().c_str(), tolerance_optionname, fvalTolerance_ );
    log_debug( "(%s) %s=%f", GetName().c_str(), simplextolerance_optionname, simplexTolerance_ );
    log_debug( "(%s) %s=%d", GetName().c_str(), maxiterations_optionname, maxIterations_ );
    log_debug( "(%s) %s=%d", GetName().c_str(), flatnesspatience_optionname, flatPatience_ );
    log_debug( "(%s) %s=%d", GetName().c_str(), flatnesspatience_optionname, backwardTMP_ );
}


I3GSLSimplex::I3GSLSimplex( std::string name,
                            double ftol,
                            double stol,
                            int maxi,
                            int flatpatience,
                            bool tmp ):
        I3ServiceBase(name), I3MinimizerBase(),
        fvalTolerance_(ftol), simplexTolerance_(stol),
        maxIterations_(maxi), flatPatience_(flatpatience),
        backwardTMP_(tmp){ }

I3GSLSimplex::~I3GSLSimplex(){}

// function for GSL simplex to minimize
double I3GSLSimplex::Function(const gsl_vector * x, void * params){

    I3GSLSimplex_ImplHelper *stuff = static_cast<I3GSLSimplex_ImplHelper*>( params );
    assert( stuff );
    I3GulliverBase& g = stuff->GetFunctor();
    std::vector<double> &par = stuff->SetPar(x);

    // calculate function value
    double fval = g(par);
    log_trace( "fval=%g", fval );

    return fval;
}


I3MinimizerResult I3GSLSimplex::Minimize(
        I3GulliverBase &g,
        const std::vector<I3FitParameterInitSpecs> &parspecs ) {

    // we are going to check for errors ourselves, turn GSL error handling off
    gsl_error_handler_t *old_gsl_error_handler = gsl_set_error_handler_off();

    int Dimension = parspecs.size();
    assert( Dimension > 0 );

    I3GSLSimplex_ImplHelper implhelper(g,parspecs,backwardTMP_);

    const gsl_multimin_fminimizer_type *minitype = gsl_multimin_fminimizer_nmsimplex;
    gsl_multimin_fminimizer *simplex = NULL;
    gsl_multimin_function minex_func;
    gsl_vector *stepsizes, *x;

    int iter = 0;
    int status;
    double size;

    // Initial step size vector: all 1
    stepsizes = gsl_vector_alloc (Dimension);
    gsl_vector_set_all (stepsizes, 1.0);

    // Starting point
    x = implhelper.GetGSLPar();

    // Initialize method and iterate
    minex_func.f = &I3GSLSimplex::Function;
    minex_func.params = (void *)&implhelper;
    minex_func.n = Dimension;

    simplex = gsl_multimin_fminimizer_alloc (minitype, Dimension);
    gsl_multimin_fminimizer_set (simplex, &minex_func, x, stepsizes);

    int nflat = 0;
    double lastfval = NAN;
    do
      {
        iter++;
        status = gsl_multimin_fminimizer_iterate(simplex);

        if (status){
            log_trace( "(%s) %dth iteration failed with status %d",
                       GetName().c_str(), iter, status );
            break;
        }

        size = gsl_multimin_fminimizer_size (simplex);
        status = gsl_multimin_test_size (size, simplexTolerance_ );
        log_trace( "iter=%d status=%d size=%g fval=%g",
                   iter, status, size, simplex->fval );

        if (status == GSL_SUCCESS){
            log_trace( "converged to minimum at %g", simplex->fval );
            break;
        }


        if ( flatPatience_ > 0 ){
            if ( lastfval == simplex->fval ){
                ++nflat;
                if ( nflat > flatPatience_ ){
                    status = GSL_ENOPROG; /* no progress */
                    break;
                }
            } else {
                lastfval = simplex->fval;
                nflat = 0;
            }
        }
      }
    while ( status == GSL_CONTINUE && iter <  maxIterations_ );

/*********************************************************/

    // harvest results
    I3MinimizerResult zeptognat(Dimension);

    // convergence
    log_trace( "status: %d (%s)",
               status, (status == GSL_SUCCESS) ? "converged" :
                       (status == GSL_ENOPROG) ? "llh landscape is FLAT" :
                                                 "failed" );
    zeptognat.converged_ = (status == GSL_SUCCESS);

    // final parameters
    log_trace( "resulting parameters:" );
    bool checkflat = zeptognat.converged_ || (status == GSL_ENOPROG);
    zeptognat.par_ = implhelper.SetPar(simplex->x);
    for ( int i = 0; i < Dimension; i++ ){
    double xi = zeptognat.par_[i];
    zeptognat.err_[i] = NAN;
        log_trace( "par[%d] name=%s val=%g",
                   i, parspecs[i].name_.c_str(), xi );
        double xistep = xi + parspecs[i].stepsize_;
        if ( xi ==  xistep ){
            log_trace("got lost so badly that stepsize of parameter %d is less than "
                      "the floating point accuracy of the final value", i);
            zeptognat.converged_ = false;
            checkflat = false;
        }
        if ( xi !=  parspecs[i].initval_ ) checkflat = false;
    }

    zeptognat.minval_ = g( zeptognat.par_ );

    // gsl_vector_free(x);
    gsl_vector_free(stepsizes);
    gsl_multimin_fminimizer_free (simplex);

    if ( checkflat ){
        I3MinimizerUtils::CheckMinimum(*this, zeptognat,parspecs,g);
    }

    // restore the previous GSL error handler
    gsl_set_error_handler(old_gsl_error_handler);

    return zeptognat;
};

typedef
I3SingleServiceFactory<I3GSLSimplex,I3MinimizerBase>
I3GSLSimplexFactory;
I3_SERVICE_FACTORY( I3GSLSimplexFactory );
