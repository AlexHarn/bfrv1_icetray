/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 */

#include "gulliver/I3Gulliver.h"
#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>
#include <boost/numeric/ublas/symmetric.hpp>

I3Gulliver::I3Gulliver( I3ParametrizationBasePtr par,
                        I3EventLogLikelihoodBasePtr llh,
                        I3MinimizerBasePtr mini,
                        std::string name ):
        I3GulliverBase(), parametrization_(par), eventllh_(llh),
        minimizer_(mini), minimizerResult_(par->GetNPar()),
        nFunctionEvaluationsTotal_(0), nNANfromMinimizerTotal_(0),
        withGradient_(false), name_(name) { }

// destructor
I3Gulliver::~I3Gulliver(){
    log_info( "(%s) nr. of proper llh function calls by minimizer: %d",
              name_.c_str(),
              nFunctionEvaluationsTotal_ );
    if ( nNANfromMinimizerTotal_ > 0 ){
        log_error( "(%s) nr. of improper llh fcn calls (NAN parameters)"
                  "by minimizer %s: %d. You should investigate this, "
                  "it is very likely that something is screwed up.",
                  name_.c_str(),
                  minimizer_->GetName().c_str(),
                  nNANfromMinimizerTotal_ );
    }
}

void I3Gulliver::NANParError(const std::vector<double> &par ) {
    std::vector<double>::const_iterator ipar;
    int parindex=0;
    for ( ipar = par.begin(); ipar != par.end(); ++ipar ){
        if ( std::isnan(*ipar) ){
            if ( nNANfromMinimizer_ < 10 ){
                log_warn( "(%s) minimizer attempts llh calculation with par[%d]=nan "
                          "after %d proper calls and %d NAN-ish calls",
                          name_.c_str(),
                          parindex, nFunctionEvaluations_, nNANfromMinimizer_ );
            }
            ++nNANfromMinimizer_;
            if ( nNANfromMinimizer_ == 10 ){
                log_warn( "(%s) (Further NAN attempts will not reported individually)",
                          name_.c_str());
            }
            return;
        }
        ++parindex;
    }
}

// Get the *negative* log-likelihood function value
double I3Gulliver::operator()( const std::vector<double> &par ){

    // checking that the par values are not NAN
    //
    // (It turns out that TMinuit may actually try to compute function
    // values with NAN parameters. We should not waste any CPU cycles on
    // parametrization and likelihood calculations in that case.)
    if ( std::isnan( std::accumulate( par.begin(), par.end(),
                     0., std::plus<double>()) ) ){
        NANParError(par);
        return NAN;

    }

    // Promise the parametrization service that we won't ask it to apply
    // the chain rule to get gradients for the minimizer.
    parametrization_->InitChainRule(false);

    const I3EventHypothesis &h = *( parametrization_->GetHypothesisPtr( par ) );

    double f = -1.0 * eventllh_->GetLogLikelihood(h, NULL, true);

    ++nFunctionEvaluations_;

    log_trace( "(%s) evaluation nr. %d: -log(L)=%f",
               name_.c_str(), nFunctionEvaluations_, f );

    if ( trace_ ){
        log_trace( "(%s) storing trace information", name_.c_str() );
        size_t newsize = trace_->size()+par.size();
        trace_->resize(newsize); // note: we did a "reserve", so "resize" should be cheap.
        copy(par.begin(),par.end(),trace_->end()-par.size());
        trace_->push_back(f);
    }

    return f;
}


// Get the *negative* log-likelihood function value
double I3Gulliver::operator()( const std::vector<double> &par,
                                     std::vector<double> &grad ){

    if ( !  withGradient_ ){
        log_fatal("(%s) Minimizer %s promised that it would not use "
                  "gradients, but apparently it does.",
                  name_.c_str(), minimizer_->GetName().c_str() );
    }

    // checking that the par values are not NAN
    //
    // (It turns out that TMinuit may actually try to compute function
    // values with NAN parameters. We should not waste any CPU cycles on
    // parametrization and likelihood calculations in that case.)
    if ( std::isnan( std::accumulate( par.begin(), par.end(),
                     0., std::plus<double>()) ) ){
        NANParError(par);
        return NAN;

    }

    if ( ! parametrization_->InitChainRule(true) ){
        log_fatal( "(%s) parametrization service %s does not support gradients",
                   name_.c_str(), parametrization_->GetName().c_str() );
    }

    I3EventHypothesisConstPtr h = parametrization_->GetHypothesisPtr( par );
    I3EventHypothesisPtr g = parametrization_->GetGradientPtr();

    if ( !h ){
        log_fatal( "(%s Got NULL pointer for event hypothesis from %s)",
                   name_.c_str(), parametrization_->GetName().c_str() );
    }
    if ( !g ){
        log_fatal( "(%s Got NULL pointer for gradient from %s)",
                   name_.c_str(), parametrization_->GetName().c_str() );
    }

    double f = -1.0 * eventllh_->GetLogLikelihood(*h, g.get(), true);
    parametrization_->GetGradient(grad);

    // now since we actually return -1 times the function value
    // we need to do the same with the gradient. :-)
    for ( vector<double>::iterator ig=grad.begin(); ig!=grad.end(); ++ig ){
        *ig *= -1;
    }

    ++nFunctionEvaluations_;

    log_trace( "(%s) evaluation nr. %d: -log(L)=%f",
               name_.c_str(), nFunctionEvaluations_, f );

    if ( trace_ ){
        assert(par.size()==grad.size());
        log_trace( "(%s) storing trace information (including gradient)", name_.c_str() );
        size_t newsize = trace_->size()+2*par.size();
        trace_->resize(newsize); // note: we did a "reserve", so "resize" should be cheap.
        copy(par.begin(),par.end(),trace_->end()-2*par.size());
        copy(grad.begin(),grad.end(),trace_->end()-par.size());
        trace_->push_back(f);
    }

    return f;
}


//convenience wrapper to pass the geometry to the llh service
void I3Gulliver::SetGeometry(const I3Geometry &geo){
    assert(eventllh_);
    eventllh_->SetGeometry(geo);
}

// convenience wrapper to do pass the event & geometry to the llh service
int I3Gulliver::SetEvent(const I3Frame &f){
    assert(eventllh_);
    assert(parametrization_);
    eventllh_->SetEvent(f);
    parametrization_->SetEvent(f);
    int mult = eventllh_->GetMultiplicity();
    int npar = parametrization_->GetNPar();
    int ndof = mult - npar;
    return ndof;
}


// convenience wrapper to do fit with some event & geometry
bool I3Gulliver::Fit( const I3Frame &f, I3LogLikelihoodFitPtr fitptr ){

    // paranoid
    assert(fitptr);
    // assert(eventllh_);

    // check number of degrees of freedom
    eventllh_->SetEvent(f);
    parametrization_->SetEvent(f);

    // delegate
    return Fit( fitptr );
}

// convenience method:
// set theta between 0 and pi, phi between 0 and 2pi
// (would be nice to have this as a I3Particle method)
bool I3Gulliver::AnglesInRange( I3Particle &p, const std::string &name ){

    double zenith = p.GetZenith();
    double azimuth = p.GetAzimuth();

    zenith = fmod(zenith,2*M_PI);
    if ( zenith < 0 ){
        zenith += 2*M_PI;
    }
    if ( zenith > M_PI ){
        zenith *= -1;
        zenith += 2*M_PI;
        azimuth -= M_PI;
    }
    azimuth = fmod(azimuth,2*M_PI);
    if ( azimuth < 0 ){
        azimuth += 2*M_PI;
    }

    // noise
    log_trace( "(%s) correcting zenith from %f to %f",
               name.c_str(), p.GetZenith(), zenith );
    log_trace( "(%s) correcting azimuth from %f to %f",
               name.c_str(), p.GetAzimuth(), azimuth );

    // do it
    p.SetDir(zenith,azimuth);

    return true;
}

bool I3Gulliver::Fit( I3LogLikelihoodFitPtr fitptr ){

    // paranoia
    assert(fitptr);
    assert(fitptr->fitparams_);
    assert(fitptr->hypothesis_);
    assert(fitptr->hypothesis_->particle);

    // use references as shortcuts/alias
    I3Particle &particle = *(fitptr->hypothesis_->particle);
    I3LogLikelihoodFitParams &fitparams = *(fitptr->fitparams_);
    I3EventHypothesis &hypothesis = *( fitptr->hypothesis_ );

    // more paranoia
    assert(eventllh_);
    assert(parametrization_);
    assert(minimizer_);


    // Check with the likelihood function whether enough hits are there and
    // compute the number of degrees of freedom for the fit.
    //
    // TODO: Maybe the multiplicity also depends on the first guess track
    // or even on the final fit result (e.g. if the llh function makes its
    // own hit selections dependent on track).
    int mult = eventllh_->GetMultiplicity();
    int npar = parametrization_->GetNPar();
    int ndof = mult - npar;
    fitparams.ndof_ = ndof;
    fitparams.nmini_ = 0;
    if ( ndof <= 0 ){
        log_debug( "(%s) insufficient data (%d) for %d-parameter fit",
                   name_.c_str(), mult, npar );
        particle.SetFitStatus( I3Particle::InsufficientHits );
        return false;
    }

    CheckGradientCompatibility();

    // initialize the parametrization with the seed hypothesis, get parspecs
    const std::vector<I3FitParameterInitSpecs>& parspecs =
        parametrization_->GetParInitSpecs(fitptr->hypothesis_);

    // reset counters (number of function calls, number of bad inputs)
    nFunctionEvaluations_ = 0;
    nNANfromMinimizer_ = 0;

    // OK, do the fit!
    log_trace( "(%s) calling minimizer", name_.c_str() );
    minimizerResult_ = minimizer_->Minimize( *this, parspecs );

    // update runwise counters
    nFunctionEvaluationsTotal_ += nFunctionEvaluations_;
    nNANfromMinimizerTotal_ += nNANfromMinimizer_;

    // check the result and fill fit parameters accordingly
    if ( minimizerResult_.converged_ ){
        log_trace( "(%s) minimizer CONVERGED after %d function evaluations",
                   name_.c_str(), nFunctionEvaluations_ );

        // final update of the hypothesis
        parametrization_->GetHypothesisPtr( minimizerResult_.par_ );

        // get covariance matrix, incomplete for now, non-diagonal elements are missing
        const unsigned npar = minimizerResult_.par_.size();
        boost::numeric::ublas::symmetric_matrix<double> cov( npar );
        for ( unsigned i = 0; i < npar; ++i )
            cov( i, i ) = minimizerResult_.err_[i] * minimizerResult_.err_[i];

        // passes the covariance matrix to the user
        parametrization_->PassCovariance( cov );

        // NB: if the likelihood has extra dimensions, this call maximizes
        // the likelihood in those dimensions as a side effect.
        double logL = eventllh_->GetLogLikelihood(hypothesis, NULL, true);
        fitparams.logl_ = -1.0*logL;
        fitparams.rlogl_ = -1.0*logL / ndof;
        fitparams.nmini_ = nFunctionEvaluations_;
        if ( AnglesInRange( particle, name_ ) ){
            particle.SetFitStatus( I3Particle::OK );
        } else {
            particle.SetFitStatus( I3Particle::FailedToConverge );
            minimizerResult_.converged_ = false; // override
        }
    } else {
        log_trace( "(%s) minimizer DID NOT CONVERGE after %d function evaluations",
                   name_.c_str(), nFunctionEvaluations_ );
        fitparams.logl_ = NAN;
        fitparams.rlogl_ = NAN;
        fitparams.nmini_ = nFunctionEvaluations_;
        particle.SetFitStatus( I3Particle::FailedToConverge );
    }
    fitptr->llhdiagnostics_ = eventllh_->GetDiagnostics(*(fitptr->hypothesis_));
    fitptr->minidiagnostics_ = minimizerResult_.diagnostics_;
    fitptr->paradiagnostics_ = parametrization_->GetDiagnostics(*(fitptr->hypothesis_));

    return minimizerResult_.converged_;
}

void I3Gulliver::CheckGradientCompatibility(){
    UseGradients(minimizer_->UsesGradient());
}

void I3Gulliver::UseGradients(bool b){
    withGradient_ = b;
    if ( withGradient_ ){
        if ( ! eventllh_->HasGradient() ){
            log_fatal( "(%s) minimizer \"%s\" wants to use gradients, "
                       "the likelihood service \"%s\" does not compute "
                       "gradients. (Or maybe you forgot to implement the "
                       "bool HasGradient() method?).",
                       name_.c_str(),
                       minimizer_->GetName().c_str(),
                       eventllh_->GetName().c_str() );
        }
        if ( ! parametrization_->InitChainRule(true) ){
            log_fatal( "(%s) minimizer \"%s\" wants to use gradients, "
                       "but the parametrization \"%s\" is not able (or not "
                       "configured) to apply the chain rule.",
                       name_.c_str(),
                       minimizer_->GetName().c_str(),
                       parametrization_->GetName().c_str() );
        }
        log_trace( "(%s) going to fit using gradient",
                  name_.c_str() );
    } else {
        parametrization_->InitChainRule(false);
        log_trace( "(%s) going to fit WITHOUT using gradient",
                  name_.c_str() );
    }
}

// enabling & initializing debugging trace
void I3Gulliver::Trace(){
    if (trace_){
        log_trace( "(%s) saying goodbye to an existing trace of length %zu",
                   name_.c_str(), trace_->size() );
    }
    trace_ = I3VectorDoublePtr(new I3VectorDouble);
    // random guess for the maximum trace size:
    // 5 parameter fit with 1000 iterations, 10 function calls per iteration
    size_t size_estimate = 6*1000*10;
    if (parametrization_ && minimizer_){
        // educated guess for the maximum trace size
        // npar parameter values per event, plus a llh value
        // and possibly additionally npar gradient values per event
        // 10: wild guess for the number of function calls per minimizer iteration
        size_estimate = 1 + (withGradient_?2:1)*parametrization_->GetNPar();
        size_estimate *= minimizer_->GetMaxIterations() * 10;
    }
    log_trace( "(%s) initializing new trace, reserving space for %zu elements",
               name_.c_str(), size_estimate);
    trace_->reserve(size_estimate);
}

// disabling & forgetting debugging trace
void I3Gulliver::NoTrace(){
    // set to (the boost::shared_ptr equivalent of a) NULL pointer
    log_trace( "(%s) disabling trace", name_.c_str() );
    if (trace_){
        log_trace( "(%s) saying goodbye to a trace of length %zu",
                   name_.c_str(), trace_->size() );
    }
    trace_ = I3VectorDoublePtr();
}

// delivering debugging trace
I3VectorDoublePtr I3Gulliver::GetTrace(){
    if ( ! trace_ ){
        log_warn("(%s) GetTrace: trace not initialized or already discarded!",
                 name_.c_str());
    } else if ( trace_->empty() ){
        log_warn("(%s) GetTrace: trace is empty?!",
                 name_.c_str());
    } else {
        log_trace( "(%s) returning a trace with length %zu",
                   name_.c_str(), trace_->size() );
    }
    return trace_;
}
