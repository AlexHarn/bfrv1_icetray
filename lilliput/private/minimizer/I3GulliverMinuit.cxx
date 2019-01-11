/**
 * @file I3GulliverMinuit.cxx
 * @brief implementation of the I3GulliverMinuit class
 *
 * (c) 2005 the IceCube Collaboration
 *
 * $Id$
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 * The Minuit commands are documented in the Minuit Manual:
 * @sa http://wwwinfo.cern.ch/asdoc/minuit/minuit.ps
 * References of the form "MM p. xyz" refer to page xyz in that document
 *
 */

// standard library stuff
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include "lilliput/minimizer/TMinuit.h"

#include "gulliver/I3GulliverBase.h"
#include "icetray/I3SingleServiceFactory.h"
#include "lilliput/minimizer/I3GulliverMinuit.h"
#include "icetray/IcetrayFwd.h"

// #include "dataclasses/I3Double.h"

#include "minimizer/I3MinimizerUtils.h"

// default values for options
static const unsigned int DEFAULT_MAXITERATIONS = 10000;
static const double DEFAULT_TOLERANCE = 0.1;
static const int DEFAULT_PRINTLEVEL = -2;
static const int DEFAULT_STRATEGY = 2;
static const std::string DEFAULT_ALGORITHM = "SIMPLEX";

// option names
static const char* tolerance_optionname = "Tolerance";
static const char* maxiterations_optionname = "MaxIterations";
static const char* algorithm_optionname = "Algorithm";
static const char* printlevel_optionname = "MinuitPrintLevel";
static const char* strategy_optionname = "MinuitStrategy";
static const char* flatcheck_optionname = "FlatnessCheck";

// formal initialization of static class members
static I3GulliverBase* llhfunctor_ = NULL;
static std::vector<double> static_par_;

static void fcn_(int& npar, double* gin, double& f, double*par, int iflag)
{
    assert(llhfunctor_);
    static_par_.resize(npar);
    copy(par, par+npar, static_par_.begin());

    // calculate function value
    f = (*llhfunctor_)(static_par_);

    return;
}

I3GulliverMinuit::I3GulliverMinuit(std::string name, const double tol,
                                   unsigned int maxi, int mpl, int mstr,
                                   std::string alg, bool flatcheck) :
    I3ServiceBase(name),
    I3MinimizerBase(),
    name_(name),
    tolerance_(tol),
    maxIterations_(maxi),
    minuitPrintLevel_(mpl),
    minuitStrategy_(mstr),
    algorithm_(alg),
    flatnessCheck_(flatcheck) { }

I3GulliverMinuit::I3GulliverMinuit(const I3Context& context) :
    I3ServiceBase(context),
    tolerance_(DEFAULT_TOLERANCE),
    maxIterations_(DEFAULT_MAXITERATIONS),
    minuitPrintLevel_(DEFAULT_PRINTLEVEL),
    minuitStrategy_(DEFAULT_STRATEGY),
    algorithm_(DEFAULT_ALGORITHM),
    flatnessCheck_(true)
{
    // unfortunatelly we can not support a max nr. of iterations of unsigned
    // int since AddParameter only supports int
    AddParameter(maxiterations_optionname,
                 "Maximum number of iterations (may be overridden by calling "
                 "module)",
                 maxIterations_);
    AddParameter(tolerance_optionname,
                 "Tolerance for minimum (may be overridden by calling module)",
                 tolerance_);
    AddParameter(printlevel_optionname,
                 "Configure noisiness of minuit. -2 is as quiet as possible.",
                 minuitPrintLevel_);
    AddParameter(strategy_optionname,
                 "Configure strategy of minuit. Strategy 2 means: optimize "
                 "for accuracy rather than speed.",
                 minuitStrategy_);
    AddParameter(algorithm_optionname,
                 "Minimization algorithm (SIMPLEX or MIGRAD)",
                 algorithm_);
    AddParameter(flatcheck_optionname,
                 "Flag to check, after minimization, whether really a minimum "
                 "was found or that TMinuit just gave up because it saw no "
                 "variation in the likelihood function. In case of flatness, "
                 "non-convergence will be reported. If you set this flag to "
                 "False, then whatever TMinuit claims as a converged "
                 "minimization result will be accepted, no flatness check is "
                 "done.",
                 flatnessCheck_);
}

void I3GulliverMinuit::Configure()
{
    GetParameter(tolerance_optionname, tolerance_);
    GetParameter(maxiterations_optionname, maxIterations_);
    GetParameter(algorithm_optionname, algorithm_ );
    GetParameter(printlevel_optionname, minuitPrintLevel_);
    GetParameter(strategy_optionname, minuitStrategy_);
    GetParameter(flatcheck_optionname, flatnessCheck_);

    log_debug("(%s) %s=%f",
              GetName().c_str(),
              tolerance_optionname,
              tolerance_);
    log_debug("(%s) %s=%d",
              GetName().c_str(),
              maxiterations_optionname,
              maxIterations_);
    log_debug("(%s) %s=%d",
              GetName().c_str(),
              printlevel_optionname,
              minuitPrintLevel_);
    log_debug("(%s) %s=%s",
              GetName().c_str(),
              algorithm_optionname,
              algorithm_.c_str());
    log_debug("(%s) %s=%s",
              GetName().c_str(),
              flatcheck_optionname,
              flatnessCheck_ ? "True" : "False");
}

I3GulliverMinuit::~I3GulliverMinuit() { }

I3MinimizerResult I3GulliverMinuit::Minimize(
    I3GulliverBase &g,
    const std::vector<I3FitParameterInitSpecs> &parspecs)
{
    int Dimension = parspecs.size();
    assert(Dimension > 0);
    llhfunctor_ = &g;

    lilliput::TMinuit minuit(Dimension);
    minuit.SetPrintLevel(minuitPrintLevel_);
    minuit.SetFCN(fcn_);
    minuit.SetMaxIterations(maxIterations_);

    int ierflg = 0;

    log_trace("(%s) minuit minimizer starting", name_.c_str());
    for (int i = 0 ; i < Dimension; i++)
    {
        double minval = parspecs[i].minval_;
        double maxval = parspecs[i].maxval_;

        if (std::isnan(minval) || std::isnan(maxval))
        {
            minval = maxval = 0.0;
        }

        log_trace("(%s) par %s initval=%f step=%f min=%f max=%f",
                  name_.c_str(),
                  parspecs[i].name_.c_str(),
                  parspecs[i].initval_,
                  parspecs[i].stepsize_,
                  minval,
                  maxval);

        minuit.mnparm(i,
                      parspecs[i].name_.c_str(),
                      parspecs[i].initval_,
                      parspecs[i].stepsize_,
                      minval,
                      maxval,
                      ierflg);

        if (ierflg == 4)
        {
            log_warn("(%s) Error setting parameter %d", name_.c_str(), i);
            // exit?
        }
    }

    // "SET NOWarnings" (MM p. 23)
    double arglist[10];
    minuit.mnexcm("SET NOW", arglist, 1, ierflg);

    // "SET ERRordef 0.5" (default for log-likelihood searches, MM p. 22)
    minuit.SetErrorDef(tolerance_);

    // "SET STR 2" (optimize for reliability rather than for speed, MM p. 24)
    arglist[0] = minuitStrategy_;
    minuit.mnexcm("SET STR", arglist, 1, ierflg);

    // now do the minimization
    // "SIMPLEX <maxiter> <tol>" (execute simplex, MM p. 24)
    // "MIGRAD <maxiter> <tol>" (execute migrad, MM p. 20)
    // however it seems that the second arg is ignored, maybe because the
    // tolerance is already set with minuit.SetErrorDef(tolerance_);
    arglist[0] = maxIterations_;
    arglist[1] = 0.0;
    minuit.mnexcm(algorithm_.c_str(), arglist, 1, ierflg);

    // harvest results
    I3MinimizerResult zeptognat(Dimension);

    log_trace("(%s) ierflg: %d (%s)", name_.c_str(), ierflg,
              (ierflg == 0) ? "converged" : "failed");

    // zeptognat.converged_ = (ierflg != 4);
    zeptognat.converged_ = (ierflg == 0);

    log_trace("(%s) resulting parameters:", name_.c_str());

    bool checkflat = (zeptognat.converged_ && flatnessCheck_);
    for (int i = 0; i < Dimension; i++ )
    {
        double x, err;
        minuit.GetParameter(i, x, err);
        zeptognat.par_[i] = x;
        zeptognat.err_[i] = err;

        log_trace("(%s) par[%d] name=%s val=%f err=%f",
                  name_.c_str(),
                  i,
                  parspecs[i].name_.c_str(),
                  x,
                  err);

        if (x != parspecs[i].initval_)
        {
            log_trace("(%s) %dth value changed => converged",
                      name_.c_str(), i);
            checkflat = false;
        }
    }

    zeptognat.minval_ = (*llhfunctor_)(zeptognat.par_);

    /**
     * It may happen that the minimizer sees a perfectly constant function. A
     * constant function has a minimum, so the minimizer can give any solution,
     * because any point in the parameter is technically an extreme value, both
     * a minimum and maximum.
     * For our reconstruction purposes we want a different approach; if the
     * likelihood space is flat, then we want the minimizer to report a
     * failure. It seems that Minuit puts the solution back to the seed in
     * such cases.
     * We use this feature to find such events, and then we test for flatness
     * by calculating the function 2*Npar more times, each time varying the
     * parameters by + or - one stepsize, and if we always find the same value,
     * then we change the convergence flag to false.
     */
    if (checkflat)
    {
        I3MinimizerUtils::CheckMinimum(*this, zeptognat, parspecs, g);
    }

    llhfunctor_ = NULL;

    // if you like, you could implement diagnostical output here (Hessian?)
    // zeptognat.diagnostics_ = I3DoublePtr(new I3Double(zeptognat.par_[0]));

    return zeptognat;
};

typedef I3SingleServiceFactory<I3GulliverMinuit, I3MinimizerBase>
I3GulliverMinuitFactory;

I3_SERVICE_FACTORY(I3GulliverMinuitFactory);
