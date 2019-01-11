/**
 *  Copyright (C) 2010
 *  Claudio Kopper <claudio.kopper@nikhef.nl>
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/**
 * @file I3GSLMultiMin.cxx
 * @brief implementation of the I3GSLMultiMin class
 * @author Claudio Kopper <claudio.kopper@nikhef.nl>
 *
 */

// standard library stuff
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>


#include "icetray/IcetrayFwd.h"
#include "icetray/I3SingleServiceFactory.h"
#include "gulliver/I3GulliverBase.h"
#include "lilliput/minimizer/I3GSLMultiMin.h"
#include "I3MinimizerUtils.h"


// default values for options
static const unsigned int DEFAULT_MAXITERATIONS = 1000;
static const double DEFAULT_INITIALSETPSIZE = 1.0;
static const double DEFAULT_LINESEARCHTOLERANCE = 0.1;
static const double DEFAULT_GRADNORMTOLERANCE = 0.001;
static const std::string DEFAULT_ALGORITHM = "vector_bfgs2";

// option names
static const char* maxiterations_optionname = "MaxIterations";
static const char* initialstepsize_optionname = "InitialStepsize";
static const char* linesearchtolerance_optionname = "LinesearchTolerance";
static const char* gradnormtolerance_optionname = "GradnormTolerance";
static const char* algorithm_optionname = "Algorithm";

// helpers and callbacks for GSL
namespace {
    struct GslParamsStruct {
        GslParamsStruct(I3GulliverBase &g_, std::vector<double>::size_type dimension_, const std::vector<double> &stepSizes_)
        : g(g_), dimension(dimension_), stepSizes(stepSizes_), par(dimension_), grad(dimension_)
        {
            if (stepSizes.size() != dimension_) log_fatal("Internal error. Dimension != stepSizes.size()");
        }

        I3GulliverBase &g;

        std::vector<double>::size_type dimension;

        std::vector<double> stepSizes;

        // these are re-used and not allocated over and over again
        // (this is our working space, they do not hold the fit result!)
        std::vector<double> par;
        std::vector<double> grad;
    };


    // callbacks for gsl
    void gsl_fdf(const gsl_vector *x, void *params, double *value, gsl_vector *grad)
    {
        assert(x); // this HAS to be a valid pointer

        GslParamsStruct &gParams = *static_cast<GslParamsStruct *>(params);

        // assign the input parameters to our working vector
        gParams.par.assign(x->data,x->data+gParams.dimension);

        // adjust input parameters for step size
        for (std::vector<double>::size_type i=0; i<gParams.dimension; ++i)
            gParams.par[i] *= gParams.stepSizes[i];

        // call the likelihood function and assign the result to *f
        if (!value)
        {
            // note: gParams.grad is a return value!
            gParams.g(gParams.par, gParams.grad); // call I3GulliverBase()
        } else {
            // note: gParams.grad is a return value!
            *value = gParams.g(gParams.par, gParams.grad); // call I3GulliverBase()
        }

        if (grad) // do not assign the gradient if there is no gradient vector
        {
            // assign the gradient (adjusted for step size)
            for (std::vector<double>::size_type i=0; i<gParams.dimension; ++i)
            {
                grad->data[i] = gParams.grad[i]/gParams.stepSizes[i];
            }
        }
    }

    // do not implement again, use gsl_fdf
    double gsl_f(const gsl_vector *x, void *params)
    {
        double f;
        gsl_fdf(x, params, &f, NULL);

        return f;
    }

    void gsl_df(const gsl_vector *x, void *params, gsl_vector* grad)
    {
        gsl_fdf(x, params, NULL, grad);
    }

    inline static const gsl_multimin_fdfminimizer_type *MinimizerFromString(const std::string &miniName)
    {
        if (miniName=="conjugate_fr") {
            return gsl_multimin_fdfminimizer_conjugate_fr;
        } else if (miniName=="conjugate_pr") {
            return gsl_multimin_fdfminimizer_conjugate_pr;
        } else if (miniName=="vector_bfgs") {
            return gsl_multimin_fdfminimizer_vector_bfgs;
        } else if (miniName=="vector_bfgs2") {
#ifndef GSL_MAJOR_VERSION // older versions did not have GSL_VERSION_MAJOR/MINOR
            log_error("The \"vector_bfgs2\" minimizer is not available in your version of GSL. Update to version 1.14 or newer.");
            return NULL;
#else
#if GSL_MAJOR_VERSION==1
#if GSL_MINOR_VERSION<14
            log_error("The \"vector_bfgs2\" minimizer is not available in your version of GSL. Update to version 1.14 or newer.");
            return NULL;
#else //GSL_VERSION_MINOR<14
            return gsl_multimin_fdfminimizer_vector_bfgs2;
#endif
#else //GSL_VERSION_MAJOR==1
            return gsl_multimin_fdfminimizer_vector_bfgs2;
#endif
#endif
        } else if (miniName=="steepest_descent") {
            return gsl_multimin_fdfminimizer_steepest_descent;
        } else {
            return NULL;
        }
    }

}


I3GSLMultiMin::I3GSLMultiMin(std::string name,
                             const unsigned int maxIterations,
                             const double initialStepsize,
                             const double linesearchTolerance,
                             const double gradnormTolerance,
                             const std::string &algorithm):
I3ServiceBase(name), I3MinimizerBase(),
name_(name),
maxIterations_(maxIterations),
initialStepsize_(initialStepsize),
linesearchTolerance_(linesearchTolerance),
gradnormTolerance_(gradnormTolerance),
algorithm_(algorithm),
gslMinimizerType_(NULL)
{
    gslMinimizerType_ = MinimizerFromString(algorithm_);
    if (!gslMinimizerType_) log_fatal("Unknown minimizer algorithm \"%s\"!", algorithm_.c_str());
}

I3GSLMultiMin::I3GSLMultiMin(const I3Context& context) :
I3ServiceBase(context), I3MinimizerBase(),
gslMinimizerType_(NULL)
{
    maxIterations_ = DEFAULT_MAXITERATIONS;
    AddParameter(maxiterations_optionname,
                 "Maximum number of iterations (may be overridden by calling module)",
                 maxIterations_);

    initialStepsize_ = DEFAULT_INITIALSETPSIZE;
    AddParameter(initialstepsize_optionname,
                 "Correction factor for the step sizes given by the parameterization.\n"
                 "You should probably leave this at 1 and change the parameterization configuration if you want different step sizes.",
                 initialStepsize_);

    linesearchTolerance_ = DEFAULT_LINESEARCHTOLERANCE;
    AddParameter(linesearchtolerance_optionname,
                 "linesearch tolerance",
                 linesearchTolerance_);

    gradnormTolerance_= DEFAULT_GRADNORMTOLERANCE;
    AddParameter(gradnormtolerance_optionname,
                 "gradient normalization tolerance",
                 gradnormTolerance_);

    algorithm_ = DEFAULT_ALGORITHM;
    AddParameter(algorithm_optionname,
                 "GSL multimin algorithm to use (one of: \"conjugate_fr\", \"conjugate_pr\", \"vector_bfgs\", \"vector_bfgs2\" or \"steepest_descent\").",
                 algorithm_);


}

void I3GSLMultiMin::Configure()
{
    GetParameter(maxiterations_optionname, maxIterations_ );
    GetParameter(initialstepsize_optionname, initialStepsize_);
    GetParameter(linesearchtolerance_optionname, linesearchTolerance_);
    GetParameter(gradnormtolerance_optionname, gradnormTolerance_);
    GetParameter(algorithm_optionname, algorithm_);

    gslMinimizerType_ = MinimizerFromString(algorithm_);
    if (!gslMinimizerType_) log_fatal("Unknown minimizer algorithm \"%s\"!", algorithm_.c_str());
}

I3GSLMultiMin::~I3GSLMultiMin()
{

}

I3MinimizerResult I3GSLMultiMin::Minimize(I3GulliverBase &g,
                                          const std::vector<I3FitParameterInitSpecs> &parspecs)
{
    if (!gslMinimizerType_) log_fatal("I3GSLMultiMin seems not to be configured correctly!");

    // we are going to check for errors ourselves, turn GSL error handling off
    gsl_error_handler_t *old_gsl_error_handler = gsl_set_error_handler_off();


    unsigned int Dimension = parspecs.size();
    assert(Dimension > 0);

    // our current search position
    gsl_vector *xx = gsl_vector_alloc(Dimension);
    if (!xx) log_fatal("Could not allocate gsl_vector!");

    std::vector<double> Low, High, Start, StepSize;

    for (unsigned int i = 0; i<Dimension; ++i)
    {
        const double stepsize = parspecs[i].stepsize_;
        if (std::isnan(stepsize) || (stepsize <= 0.))
            log_fatal("Invalid step size configured for dimension %u: stepsize=%g",
                      i, stepsize);

        double minval = parspecs[i].minval_; // /stepSize (enable this if boundaries are implemented)
        double maxval = parspecs[i].maxval_; // /stepSize

        // this minimizer does not support parameter constraints
        if (!std::isnan(minval)) {
            if (minval != -HUGE_VAL) {
                log_warn("I3GSLMultiMin does not support boundaries yet, ignoring them. (requested lower bound is %f)", minval);
                minval = -HUGE_VAL;
            }
        }
        if (!std::isnan(maxval)) {
            if (maxval != HUGE_VAL) {
                log_warn("I3GSLMultiMin does not support boundaries yet, ignoring them. (requested upper bound is %f)", maxval);
                maxval = HUGE_VAL;
            }
        }

        if (std::isnan(minval)) minval = -HUGE_VAL;
        if (std::isnan(maxval)) maxval = HUGE_VAL;

        log_trace("(%s) par %s initval=%f step=%f min=%f max=%f",
                  name_.c_str(),
                  parspecs[i].name_.c_str(),
                  parspecs[i].initval_,
                  parspecs[i].stepsize_,
                  minval,
                  maxval );

        Low.push_back(minval);
        High.push_back(maxval);
        Start.push_back(parspecs[i].initval_);
        StepSize.push_back(stepsize);

        xx->data[i] = parspecs[i].initval_/stepsize; // set the GSL position vector (adjusted for step sizes)
    }

    // the currently best point
    const double startBestValue = g(Start);

    log_trace("The starting point yields a value of %f.",
              startBestValue);

    // set up the algorithm

    // the minimizer
    gsl_multimin_fdfminimizer* minimizer =
    gsl_multimin_fdfminimizer_alloc(gslMinimizerType_, Dimension);
    if (!minimizer)
        log_fatal("Could not allocate gsl_multimin_fdfminimizer.");

    // the function
    gsl_multimin_function_fdf gslFunc;

    // we pass a pointer to this struct to the minimizer
    GslParamsStruct gslParamsStruct(g, Dimension, StepSize);

    gslFunc.n   = Dimension;
    gslFunc.f   = gsl_f;
    gslFunc.df  = gsl_df;
    gslFunc.fdf = gsl_fdf;
    gslFunc.params = static_cast<void *>(&gslParamsStruct); // pass a pointer to the I3GulliverBase object


    int ret = gsl_multimin_fdfminimizer_set(minimizer, &gslFunc, xx,
                                            initialStepsize_,
                                            linesearchTolerance_);
    if (ret != GSL_SUCCESS)
        log_fatal("Could not initilaize GSL minimizer using gsl_multimin_fdfminimizer_set.");


    // Minimize:
    int status = GSL_CONTINUE;
    unsigned long iter;
    for (iter=0; iter<maxIterations_; ++iter)
    {
        status = gsl_multimin_fdfminimizer_iterate(minimizer);
        if (status) break;

        status = gsl_multimin_test_gradient(minimizer->gradient, gradnormTolerance_);
        if (status == GSL_SUCCESS) break;

        if (status != GSL_CONTINUE) {
            log_trace("gsl_multimin failed during iteration with %u", status);
            break;
        }
    }

    if (status == GSL_CONTINUE) log_trace("Minimizer has reached its maximum amount of iterations. Aborted after %lu iterations.", iter);
    else if (status == GSL_ENOPROG) log_trace("Minimizer does not seem to be making progress. Aborted after %lu iterations. (GSL_ENOPROG)", iter);
    else if (status == GSL_SUCCESS) log_trace("Minimizer claims to have found an acceptable minimum! (GSL_SUCCESS)");
    else log_warn("The minimizer returned an unknown error code (%i)", status);

    for (unsigned i=0;i<Dimension;++i){
        log_trace("par[%u]=%g grad[%u]=%g",i,minimizer->x->data[i],i,minimizer->gradient->data[i]);
    }

    // That's what we get out of the Fit:
    std::vector<double> fit(minimizer->x->data, minimizer->x->data + Dimension);
    // adjust fit result for step size
    for (unsigned i=0;i<Dimension;++i) fit[i] *= StepSize[i];

    bool converged=(status==GSL_SUCCESS);

    double like = converged ? gsl_multimin_fdfminimizer_minimum(minimizer) : NAN;

    if (converged){
        log_trace("(%s) gsl_multimin found a minimum value of %g, start value was %g.",
                  GetName().c_str(), like, startBestValue);
    } else {
        log_trace("(%s) gsl_multimin DID NOT CONVERGE", GetName().c_str());
    }


    // free the GSL stuff
    gsl_multimin_fdfminimizer_free(minimizer);
    gsl_vector_free(xx);


    // prepare result object and return it
    I3MinimizerResult result(Dimension);

    result.minval_ = like;
    result.converged_ = converged;
    result.par_ = fit;


    // if the fit did not move away from the starting point, we explicitly check for flatness
    bool checkflat = converged || (status==GSL_ENOPROG);
    if (checkflat) {
        for (unsigned i=0;i<Dimension;++i) {
            if (std::fabs(fit[i]-Start[i]) > std::min(linesearchTolerance_,gradnormTolerance_))
            {
                checkflat=false;
                break;
            }
        }
    }

    // so we still should check for flatness?
    if (checkflat){
        I3MinimizerUtils::CheckMinimum(*this, result, parspecs, g);
    }

    // restore the previous GSL error handler
    gsl_set_error_handler(old_gsl_error_handler);


    return result;
};


typedef I3SingleServiceFactory<I3GSLMultiMin,I3MinimizerBase> I3GSLMultiMinFactory;
I3_SERVICE_FACTORY( I3GSLMultiMinFactory );
