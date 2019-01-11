/**
 * @file I3GulliverAnnealing.cxx
 * @brief implementation of the I3GulliverAnnealing class
 * @author Holger Motz <holger.motz@physik.uni-erlangen.de>
 *
 */

// standard library stuff
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>


#include "gulliver/I3GulliverBase.h"
#include "icetray/I3SingleServiceFactory.h"
#include "lilliput/minimizer/I3GulliverAnnealing.h"
#include "DSimulatedAnnealing.h"
#include "icetray/IcetrayFwd.h"


// default values for options
const unsigned int I3GulliverAnnealing::DEFAULT_MAXITERATIONS = 10000;
const double I3GulliverAnnealing::DEFAULT_TOLERANCE=1.e-5;
const bool I3GulliverAnnealing::DEFAULT_SHIFTBOUNDARIES = false;
const double I3GulliverAnnealing::DEFAULT_QUENCHINGFACTOR=0.5;
const double I3GulliverAnnealing::DEFAULT_STARTTEMP=10000.0;
const int I3GulliverAnnealing::DEFAULT_SCYCLE = 20;
const int I3GulliverAnnealing::DEFAULT_TCYCLE = 5;
const int I3GulliverAnnealing::DEFAULT_NCYCLESEPS = 5;


// option names

static const char* shitftboundaries_optionname = "ShiftBoundariesAroundStart";
static const char* maxiterations_optionname = "MaxIterations";
static const char* tolerance_optionname = "Tolerance";
static const char* quenchingfactor_optionname = "Quenchingfactor";
static const char* starttemp_optionname = "StartTemp";
static const char* scycle_optionname = "SCycle";
static const char* tcycle_optionname = "TCycle";
static const char* ncycleseps_optionname = "NCyclesEps";

static I3GulliverBase* llhfunctor_ = NULL;

static double f(const std::vector<double>* par){
    assert(llhfunctor_);

    // calculate and return function value

    return  (*llhfunctor_)(*par);;
}


I3GulliverAnnealing::I3GulliverAnnealing( std::string name,
                                          I3RandomServicePtr rndptr,
                                          unsigned int maxi, double tol,
                                          bool shiftBoundaries,
                                          double quench, double temp0,
                                          int scyc, int tcyc, int ncycleseps ):
    I3ServiceBase(name),
    I3MinimizerBase(),
    name_(name),
    random_(rndptr),
    maxIterations_(maxi),
    tolerance_(tol),
    shiftBoundaries_(shiftBoundaries),
    quenchingfactor_(quench),
    starttemp_(temp0),
    scycle_(scyc),
    tcycle_(tcyc),
    ncycleseps_(ncycleseps){ }

I3GulliverAnnealing::I3GulliverAnnealing(const I3Context& context) :
I3ServiceBase(context)
{
    maxIterations_= DEFAULT_SHIFTBOUNDARIES;
    AddParameter( shitftboundaries_optionname,
                 "Shift the boundaries from the Parameterization to be around the start value.",
                 shiftBoundaries_);

    maxIterations_= DEFAULT_MAXITERATIONS;
    AddParameter( maxiterations_optionname,
                  "Maximum number of iterations (may be overridden by calling module)",
                  maxIterations_);

    tolerance_ = DEFAULT_TOLERANCE;
    AddParameter( tolerance_optionname,
                  "Annealing tolerance (convergence criterion)",
                  tolerance_ );

    quenchingfactor_= DEFAULT_QUENCHINGFACTOR;
    AddParameter( quenchingfactor_optionname,
                 "Quenchingfactor: If you don't know what it is, don't change it",
                 quenchingfactor_);

    starttemp_= DEFAULT_STARTTEMP;
    AddParameter( starttemp_optionname,
                 "Start Temperature: If you don't know what it is, don't change it",
                 starttemp_);

    scycle_= DEFAULT_SCYCLE;
    AddParameter( scycle_optionname,
                 "SCycle: If you don't know what it is, don't change it",
                 scycle_);

    tcycle_= DEFAULT_TCYCLE;
    AddParameter( tcycle_optionname,
                 "TCycle: If you don't know what it is, don't change it",
                 tcycle_);

    ncycleseps_= DEFAULT_NCYCLESEPS;
    AddParameter( ncycleseps_optionname,
                 "NCyclesEps: Number of temperature cycles the function value has to stay within the tolerance limits for termination",
                 ncycleseps_);

}

void I3GulliverAnnealing::Configure() {
    GetParameter(maxiterations_optionname , maxIterations_ );
    GetParameter(tolerance_optionname, tolerance_ );
    GetParameter(shitftboundaries_optionname, shiftBoundaries_);
    GetParameter(quenchingfactor_optionname , quenchingfactor_);
    GetParameter(starttemp_optionname , starttemp_);
    GetParameter(scycle_optionname , scycle_);
    GetParameter(tcycle_optionname , tcycle_);
    GetParameter(ncycleseps_optionname , ncycleseps_);

    // we need random numbers later on, so retrieve a random service instance..
    log_trace("Fetching random number generator interface...");
    random_ = context_.Get<I3RandomServicePtr>();
    if (!random_) log_fatal("You have to install an I3RandomService before I3CMAESFactory!");

    if (std::isnan(tolerance_)){
        log_info( "got NAN for tolerance; setting back to default %g",
                  DEFAULT_TOLERANCE );
        tolerance_ = DEFAULT_TOLERANCE;
    }
}

I3GulliverAnnealing::~I3GulliverAnnealing(){}

I3MinimizerResult I3GulliverAnnealing::Minimize(I3GulliverBase &g,
                                                const std::vector<I3FitParameterInitSpecs> &parspecs ){

    int Dimension = parspecs.size();
    assert( Dimension > 0 );
    llhfunctor_ = &g;

    std::vector<double> Low, High, Start, StepSizes;


    for ( int i = 0 ; i<Dimension ; i++){
        double minval = parspecs[i].minval_;
        double maxval = parspecs[i].maxval_;
        if (std::isnan(minval)) minval = -HUGE_VAL;
        if (std::isnan(maxval)) maxval = HUGE_VAL;
        if ((fabs(minval-maxval)<1e-5) && (fabs(minval)<1e-5) && (fabs(maxval)<1e-5))
        {
            minval = -HUGE_VAL;
            maxval = HUGE_VAL;
        }

        if (shiftBoundaries_) {
            // adjust boundaries around prefit
            minval += parspecs[i].initval_;
            maxval += parspecs[i].initval_;
        }

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
        StepSizes.push_back(parspecs[i].stepsize_);
    }

    // the currently best point
    double startBestValue = g(Start);

    log_trace("The starting point yields a value of %g.",
              startBestValue);

    // set up the algorithm
    DSimulatedAnnealing sa(Dimension,f, Low, High, *random_);
    sa.DSimulatedAnnealing::setMaxEvals(maxIterations_);
    sa.DSimulatedAnnealing::setQuenchingFactor(quenchingfactor_);
    sa.DSimulatedAnnealing::setStartTemp(starttemp_);
    sa.DSimulatedAnnealing::setSCycle(scycle_);
    sa.DSimulatedAnnealing::setTCycle(tcycle_);
    sa.DSimulatedAnnealing::setNeps(ncycleseps_);
    sa.DSimulatedAnnealing::setStartPar(Start);
    sa.DSimulatedAnnealing::setStepSizes(StepSizes);
    sa.DSimulatedAnnealing::setEpsTol(tolerance_);


    // Minimize:
    sa.DSimulatedAnnealing::minimise();

    // That's what we get out of the Fit:
    std::vector<double> fit = sa.DSimulatedAnnealing::getOptPar();
    double like = sa.DSimulatedAnnealing::getOptValue();

    log_trace("DSimulatedAnnealing found a minimum value of %g, input value was %g.",
              like, startBestValue);

    // Check the status
    int status = sa.DSimulatedAnnealing::getStatus();
    if (status == 1) {
        // terminated by maximum number of iterations
        log_trace("DSimulatedAnnealing reached its maximum number of iterations.");
        log_trace("It did not find enough close hits.");
    } else if (status == 8) {
        log_trace("DSimulatedAnnealing reached its maximum number of iterations.");
        log_trace("It did find enough close hits, so we'll gamble that it's fine.");
        status = 0;
    }

    // we may still have found a better minimum, re-set the status to 0 anyway.
    if (startBestValue < like) {
        // the start value was better
        log_trace("DSimulatedAnnealing failed to find a better value.");
        like=startBestValue;
        fit.assign(Start.begin(), Start.end());
    }

    bool converged = false;
    if (status==0) converged=true;
    log_trace( "Error Code from DSimulatedAnnealing  = %d" ,sa.DSimulatedAnnealing::getStatus() );

    if ( converged ){
        log_trace("checking against flatness");
        bool flat = true;
        std::vector<double> chkfit = fit;
        for ( int i = 0 ; i<Dimension ; i++){
            for ( int Sign = -1; Sign <= +1; Sign += 2 ){
                chkfit[i] = fit[i] + Sign * StepSizes[i];
                double chklike = g(chkfit);
                if ( chklike > like ){
                    if ( flat ) log_trace("not flat!");
                    flat = false;
                    // we can not stop yet: it could still be a saddle point
                } else if ( chklike < like-tolerance_ ){
                    // oops!
                    log_warn("OOPS! reported minimum is obviously NOT the actual minimum");
                    log_warn("like=%g shifting par %d from %f to %f gives chklike=%g",
                             like, i, fit[i], chkfit[i], chklike );
                    log_warn("flagging this fit as NOT converged");
                    converged = false;
                    // we can stop now: this fit is doomed
                    break;
                }
            }
            // if it's broken, it won't be fixed anymore
            if ( ! converged ) break;
        }
        if ( flat ){
            log_trace("got stuck on a flat part of llh space => NOT converged");
            converged = false;
        }
    }

    llhfunctor_ = NULL;

    I3MinimizerResult result(Dimension);

    result.minval_ = like; 
    result.converged_ = converged;
    result.par_=fit;


    return result;
};


typedef
I3SingleServiceFactory<I3GulliverAnnealing,I3MinimizerBase>
I3GulliverAnnealingFactory;
I3_SERVICE_FACTORY( I3GulliverAnnealingFactory );
