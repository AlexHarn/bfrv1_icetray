/**
 * @file I3GulliverMinuit2.cxx
 * @brief implementation of the I3GulliverMinuit2 class
 *
 * (c) 2010 the IceCube Collaboration
 *
 * $Id$
 * @version $Revision$
 * @date $Date$
 * @author Jakob van Santen <vansanten@wisc.edu>
 *
 * See the Minuit2 Manual: http://seal.web.cern.ch/seal/documents/minuit/mnusersguide.pdf
 */

// standard library stuff
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

// Minuit2 stuff
#include "Minuit2/MnUserParameters.h"
#include "Minuit2/MnPrint.h"
#include "Minuit2/MnStrategy.h"
#include "Minuit2/FCNBase.h"
#include "Minuit2/FCNGradientBase.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/ModularFunctionMinimizer.h"
#include "Minuit2/SimplexMinimizer.h"
#include "Minuit2/MinimumError.h"
#include "Minuit2/VariableMetricMinimizer.h"
#include "Minuit2/CombinedMinimizer.h"
#include "Minuit2/FumiliMinimizer.h"
#ifdef I3_USE_ROOT
#include "TError.h"
#endif

#include "lilliput/minimizer/I3GulliverMinuit2.h"
#include "minimizer/I3MinimizerUtils.h"

#include "icetray/I3SingleServiceFactory.h"

// default values for options
static const unsigned int DEFAULT_MAXITERATIONS = 10000;
static const double DEFAULT_TOLERANCE = 0.1;
static const int DEFAULT_PRINTLEVEL = -2;
static const int DEFAULT_STRATEGY = 2;
static const std::string DEFAULT_ALGORITHM = "SIMPLEX";

// option names: a strict superset of those used by I3GulliverMinuit
static const char* tolerance_optionname = "Tolerance";
static const char* maxiterations_optionname = "MaxIterations";
static const char* algorithm_optionname = "Algorithm";
static const char* gradient_optionname = "WithGradients";
static const char* gradientcheck_optionname = "CheckGradient";
static const char* printlevel_optionname = "MinuitPrintLevel";
static const char* strategy_optionname = "MinuitStrategy";
static const char* flatcheck_optionname = "FlatnessCheck";

I3GulliverMinuit2::I3GulliverMinuit2(std::string name, const double tol,
                                     unsigned int imax, int verbose, int strat,
                                     std::string alg, bool flatcheck,
                                     bool gradient, bool gradcheck, bool noedm)
    : I3ServiceBase(name),
    I3MinimizerBase(),
    name_(name),
    tolerance_(tol),
    maxIterations_(imax),
    minuitPrintLevel_(verbose),
    minuitStrategy_(strat),
    algorithmName_(alg),
    flatnessCheck_(flatcheck),
    withGradient_(gradient),
    checkGradient_(gradcheck),
    ignoreEDM_(noedm)
{
    CheckParameters();
}

I3GulliverMinuit2::I3GulliverMinuit2(const I3Context& context)
    : I3ServiceBase(context),
    tolerance_(DEFAULT_TOLERANCE),
    maxIterations_(DEFAULT_MAXITERATIONS),
    minuitPrintLevel_(DEFAULT_PRINTLEVEL),
    minuitStrategy_(DEFAULT_STRATEGY),
    algorithmName_(DEFAULT_ALGORITHM),
    flatnessCheck_(true),
    withGradient_(false),
    checkGradient_(false),
    ignoreEDM_(false)
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
                 "Minimization algorithm (MIGRAD, SIMPLEX, COMBINED, or "
                 "FUMILI)",
                 algorithmName_);
    AddParameter(gradient_optionname,
                 "Use analytic gradients of the likelihood (MIGRAD only).",
                 withGradient_);
    AddParameter(gradientcheck_optionname,
                 "Check analytic gradients against the numerical gradient.",
                 checkGradient_);
    AddParameter("IgnoreEDM",
                 "Treat MIGRAD fits that exceed the maximum "
                 "estimated-distance-to-minimum as having converged. This "
                 "option can result in the minimizer returning truly insane "
                 "results, and should only be used in a context where the "
                 "resulting likelihood value will be used to judge the quality "
                 "of fit (e.g. in iterative fitting).",
                 ignoreEDM_);
    AddParameter(flatcheck_optionname,
                 "Flag to check, after minimization, whether really a minimum "
                 "was found or that TMinuit2 just gave up because it saw no "
                 "variation in the likelihood function. In case of flatness, "
                 "non-convergence will be reported. If you set this flag to "
                 "False, then whatever TMinuit2 claims as a converged "
                 "minimization result will be accepted, no flatness check is "
                 "done.",
                 flatnessCheck_);
}

void I3GulliverMinuit2::Configure()
{
    GetParameter(tolerance_optionname, tolerance_);
    GetParameter(maxiterations_optionname, maxIterations_);
    GetParameter(algorithm_optionname, algorithmName_);
    GetParameter(printlevel_optionname, minuitPrintLevel_);
    GetParameter(strategy_optionname, minuitStrategy_);
    GetParameter(flatcheck_optionname, flatnessCheck_);
    GetParameter(gradient_optionname, withGradient_);
    GetParameter(gradientcheck_optionname, checkGradient_);
    GetParameter("IgnoreEDM", ignoreEDM_);

    CheckParameters();
}

void I3GulliverMinuit2::CheckParameters()
{
    if (algorithmName_ == "SIMPLEX")
    {
        algorithm_ = SIMPLEX;
    }
    else if (algorithmName_ == "MIGRAD")
    {
        algorithm_ = MIGRAD;
    }
    else if (algorithmName_ == "COMBINED")
    {
        algorithm_ = COMBINED;
    }
    else if (algorithmName_ == "FUMILI")
    {
        algorithm_ = FUMILI;
    }
    else
    {
        log_warn("(%s) Unknown algorithm '%s' requested! Reverting to "
                 "'SIMPLEX'.", GetName().c_str(), algorithmName_.c_str());
        algorithm_ = SIMPLEX;
        algorithmName_ = "SIMPLEX";
    }

    if (withGradient_ && (algorithm_ == SIMPLEX))
    {
        log_warn("(%s) The 'SIMPLEX' algorithm doesn't do anything with "
                 "gradients! Disabling gradient support.", GetName().c_str());
        withGradient_ = false;
    }

    // quote http://wwwasdoc.web.cern.ch/wwwasdoc/minuit/node20.html
    if (!ignoreEDM_ && (algorithm_ == SIMPLEX))
    {
        log_warn("(%s) Thus spoke the Minuit2 manual of SIMPLEX, \"Its "
                 "estimate of EDM is largely fantasy, so it would not even "
                 "know if it did converge.\" Setting IgnoreEDM=True to avoid "
                 "confusion.", GetName().c_str());
        ignoreEDM_ = true;
    }

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
              algorithmName_.c_str());
    log_debug("(%s) %s=%s",
              GetName().c_str(),
              flatcheck_optionname,
              flatnessCheck_ ? "True" : "False");
}

I3GulliverMinuit2::~I3GulliverMinuit2() { }

/*
 * Since we're not planning on interacting with the minimizers, we use the
 * stateless versions rather than the "application" wrappers (MnMigrad,
 * MnSimplex, and friends).
 */
ROOT::Minuit2::ModularFunctionMinimizer*
MinimizerFromString(I3GulliverMinuit2Algorithm algorithm)
{
    ROOT::Minuit2::ModularFunctionMinimizer *minimizer;

    switch (algorithm)
    {
        case MIGRAD:
            minimizer = new ROOT::Minuit2::VariableMetricMinimizer;
            break;
        case COMBINED:
            minimizer = new ROOT::Minuit2::CombinedMinimizer;
            break;
        case FUMILI:
            minimizer = new ROOT::Minuit2::FumiliMinimizer;
            break;
        default:
            minimizer = new ROOT::Minuit2::SimplexMinimizer;
            break;
    }

    return (minimizer);
}

#ifdef I3_USE_ROOT
namespace
{
    struct set_print_level
    {
        int printLevel_;
        ErrorHandlerFunc_t error_handler_;

        static void log_messages(int level, Bool_t abort, const char *location,
                                  const char *msg)
        {
            switch (level)
            {
                case kUnset:
                    log_trace_stream(location << ": " << msg);
                    break;
                case kPrint:
                    log_debug_stream(location << ": " << msg);
                    break;
                case kInfo:
                    log_info_stream(location << ": " << msg);
                    break;
                case kWarning:
                    log_warn_stream(location << ": " << msg);
                    break;
                default:
                    log_error_stream(location << ": " << msg);
                    break;
            }
        }

        static void silence_messages(int level, Bool_t abort,
                                     const char *location, const char *msg)
        {}

        set_print_level(int printLevel)
        {
            printLevel_ = ROOT::Minuit2::MnPrint::Level();
            ROOT::Minuit2::MnPrint::SetLevel(printLevel);

            error_handler_ = SetErrorHandler(
                (printLevel == DEFAULT_PRINTLEVEL)
                ? &silence_messages : &log_messages);
        }

        ~set_print_level()
        {
            ROOT::Minuit2::MnPrint::SetLevel(printLevel_);
            SetErrorHandler(error_handler_);
        }

        SET_LOGGER("I3GulliverMinuit2");
    };
}
#endif

/* Function-call adapter */
class I3GulliverFCNGradient : public ROOT::Minuit2::FCNGradientBase {
    public:
        I3GulliverFCNGradient(I3GulliverBase &gulliver, bool trust_me = true)
            : gulliver_(gulliver), trust_me_(trust_me) { };

        double operator()(const std::vector<double>& x) const;
        std::vector<double> Gradient(const std::vector<double>& x) const;

        double Up() const {return 0.5;};
        bool CheckGradient() const {return !trust_me_;}

    protected:
        I3GulliverBase &gulliver_;
        bool trust_me_;
};

I3MinimizerResult I3GulliverMinuit2::Minimize(
    I3GulliverBase &g,
    const std::vector<I3FitParameterInitSpecs> &parspecs)
{
    unsigned int i;
    bool success;
    bool checkflat = false;
#ifdef I3_USE_ROOT
    set_print_level muzzle(minuitPrintLevel_);
#endif
    ROOT::Minuit2::MnUserParameters params;
    ROOT::Minuit2::MnStrategy strategery(minuitStrategy_);
    I3MinimizerResult result(parspecs.size());
    std::vector<I3FitParameterInitSpecs>::const_iterator i_par;
    ROOT::Minuit2::ModularFunctionMinimizer *minimizer;
    ROOT::Minuit2::FunctionMinimum *minimum;

    for (i_par = parspecs.begin(); i_par != parspecs.end(); i_par++)
    {
        double minval = i_par->minval_;
        double maxval = i_par->maxval_;

        if (std::isnan(minval) || std::isnan(maxval))
        {
            minval = maxval = 0.0;
        }

        if (minval == 0.0 && maxval == 0.0)
        {
            // parameter is completely free
            success = params.Add(i_par->name_, i_par->initval_,
                                 i_par->stepsize_);
        }
        else if (minval == maxval)
        {
            // parameter is fixed
            success = params.Add(i_par->name_, minval);
        }
        else
        {
            // parameter is constrained on both sides
            success = params.Add(i_par->name_, i_par->initval_,
                                 i_par->stepsize_, minval, maxval);
        }

        if (!success)
        {
            log_warn("(%s) Error setting parameter '%s'",
                     GetName().c_str(), i_par->name_.c_str());
        }
        else
        {
            log_trace("(%s) Par %s initval=%f step=%f minval=%f "
                      "max=%f", GetName().c_str(), i_par->name_.c_str(),
                      i_par->initval_, i_par->stepsize_, minval, maxval);
        }
    }

    minimizer = MinimizerFromString(algorithm_);
    assert(minimizer);
    I3GulliverFCNGradient fcn(g, !checkGradient_);

    log_trace("About to minimize using '%s', strategy %u",
              algorithmName_.c_str(), strategery.Strategy());

    // modify tolerance for MIGRAD's definition of the EDM
    double tol = tolerance_;
    if (algorithm_ == MIGRAD)
    {
        tol /= 0.002;
    }

    if (withGradient_)
    {
        // NB: yes, copying this into a new object is stupid
        // however FunctionMinimum doesn't have a default constructor, so we
        // have to jump through hoops to get bog-standard conditional assignment
        minimum = new ROOT::Minuit2::FunctionMinimum(
            minimizer->Minimize(fcn, params, strategery, maxIterations_, tol));
    }
    else
    {
        // pay no attention to the gradient behind the curtain
        minimum = new ROOT::Minuit2::FunctionMinimum(
            minimizer->Minimize((ROOT::Minuit2::FCNBase &) fcn, params,
                                strategery, maxIterations_, tol));
    }

    // figure out if we can trust the minimum
    const ROOT::Minuit2::MinimumError& minerr = minimum->Error();
    success = (std::isfinite(minimum->Fval()) &&
               (ignoreEDM_ ? !(minimum->HasReachedCallLimit()) :
                             (minimum->IsValid() && minerr.IsValid())));

    log_trace("(%s) EDM %s ignored, min %s reached call limit, min %s valid",
              GetName().c_str(),
              ignoreEDM_ ? "is" : "is NOT",
              minimum->HasReachedCallLimit() ? "has" : "has NOT",
              minimum->IsValid() ? "is" : "is NOT");

    // extract fitted parameters, regardless of whether the fit converged
    ROOT::Minuit2::MnUserParameters finalparams = minimum->UserParameters();
    for (i = 0; i < parspecs.size(); i++)
    {
        result.par_[i] = finalparams.Value(i);
        result.err_[i] = finalparams.Error(i);

        // if we've ended up back at the seed, become suspicous
        if (success && flatnessCheck_ &&
                (result.par_[i] == parspecs[i].initval_))
            checkflat = true;

        log_trace("(%s) par[%d] name=%s val=%f err=%f",
                  GetName().c_str(), i, parspecs[i].name_.c_str(),
                  result.par_[i], result.err_[i]);
    }

    result.minval_ = minimum->Fval();
    if (success)
    {
        result.converged_ = true;
    }
    else
    {
        result.converged_ = false;

        if (minimum->HasReachedCallLimit())
        {
            log_trace("Minimizer exhausted call limit!");
        }
        else if (minimum->IsAboveMaxEdm())
        {
            log_trace("The minimizer got itself stuck!");
        }

        log_trace("Minimizer failed! minimum -logL: %f esimated vertical "
                  "distance to minimum: %f", result.minval_, minimum->Edm());
    }

    if (checkflat)
    {
        I3MinimizerUtils::CheckMinimum(*this, result, parspecs, g);
    }

    log_trace("DCovar=%g", minerr.Dcovar());
    log_trace("IsAccurate=%s", minerr.IsAccurate() ? "True" : "False");
    log_trace("IsValid=%s", minerr.IsValid() ? "True" : "False");
    log_trace("IsPosDef=%s", minerr.IsPosDef() ? "True" : "False");
    log_trace("IsMadePosDef=%s", minerr.IsMadePosDef() ? "True" : "False");
    log_trace("HesseFailed=%s", minerr.HesseFailed() ? "True" : "False");
    log_trace("InvertFailed=%s", minerr.InvertFailed() ? "True" : "False");
    log_trace("IsAvailable=%s", minerr.IsAvailable() ? "True" : "False");

    delete minimum;
    delete minimizer;

    return result;
}

double I3GulliverFCNGradient::operator()(const std::vector<double>& x) const
{
    return gulliver_(x);
}

std::vector<double> I3GulliverFCNGradient::Gradient(
    const std::vector<double>& x) const
{
    std::vector<double> gradient(x.size(), 0.0);
    gulliver_(x, gradient);

    return gradient;
}

typedef I3SingleServiceFactory<I3GulliverMinuit2,I3MinimizerBase>
I3GulliverMinuit2Factory;

I3_SERVICE_FACTORY(I3GulliverMinuit2Factory);
