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

#ifndef GULLIVER_I3GULLIVER_H_INCLUDED
#define GULLIVER_I3GULLIVER_H_INCLUDED

// generic stuff
#include <vector>
#include "icetray/IcetrayFwd.h"
#include "icetray/I3Logging.h"

// loglikelihood stuff
#include "gulliver/I3GulliverBase.h"
#include "gulliver/I3ParametrizationBase.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3LogLikelihoodFit.h"


/**
 * @class I3Gulliver
 * @brief Fitter core of the Gulliver utility
 *
 * You need to know about this class if you would like to implement a
 * new Gulliver-based reconstruction module. You do *not* need to know
 * about it when you are implementing Gulliver components, such as a
 * likelihood functions, parametrizations and minimizers.
 *
 * This class connects the physics-aware log-likelihood function, the
 * physics-ignorant minimizer and the parametrization of the physics track
 * together. Typically your module will have a I3Gulliver datamember,
 * during Configuration you retrieve services for minimization,
 * parametrizaton and likelihood calculation and you plug them into this
 * Gulliver object. Then in the Physics method you call the SetEvent(const
 * I3Frame&) method of the likelihood service, followed by one or more
 * calls to I3Gulliver::Fit(I3LogLikelihoodFitPtr) for various seeds
 * (stored in an I3EventHypothesis), from which you may derive the
 * reconstruction result.
 *
 * The "particle" and "nonstd" members of the I3LogLikelihoodFitPtr
 * which you provide to the Fit method should have been initilialized
 * with a seed track. After the Fit call they contain the physics part
 * of the fit result, while the "fitparams" datamember contains the
 * reconstruction diagnostics such as the (reduced) likelihood, number
 * of degrees of freedom and the number of times that the minimizer
 * evaluated the likelihoodfunction.
 *
 * If you'd like to store additional fit diagnostics (e.g. the
 * paraboloid results), then can e.g. derive a new fit parameter class
 * from I3LogLikelihoodFitParams to add additional variables and fill
 * those after the Fit call.
 *
 * @sa I3LogLikelihoodFit, I3LogLikelihoodFitParams
 * @sa I3EventHypothesis, I3GulliverBase, I3SimpleFitter
 */
class I3Gulliver : public I3GulliverBase {

public:

    /**
     * @brief constructor
     *
     * @param[in] par Translator between physics object and array of doubles
     * @param[in] llh Log-Likelihood calculator (physics model)
     * @param[in] m minimizer (physics ignorant)
     * @param[in] name
     */
    I3Gulliver( I3ParametrizationBasePtr par,
                I3EventLogLikelihoodBasePtr llh,
                I3MinimizerBasePtr m,
                std::string name );

    /**
     * destructor
     */
    ~I3Gulliver();

    /** set/change parametrization
     * If you write a reconstruction module in which you'd like to do
     * the same fit with different parametrizations, then you use this
     * method to swap the parametrization (keeping the event llh service
     * and the minimizer unchanged).
     *
     * @param[in] newpar shared pointer to alternative parametrization service
     */
    void ChangeParametrization( I3ParametrizationBasePtr newpar ){
        log_debug( "switching parametrization to %s", newpar->GetName().c_str() );
        parametrization_ = newpar;
    }

    /**
     * set/change log-likelihood function
     * If you write a reconstruction module in which you'd like to do the
     * same fit with different likelihood functions, then you use this
     * method to swap the eventllh service (keeping the parametrization
     * and the minimizer unchanged).
     *
     * @param[in] newllh shared pointer to alternative eventllh service
     */
    void ChangeFunction( I3EventLogLikelihoodBasePtr newllh ){
        log_debug( "switching event log-likelihood function to %s",
                   newllh->GetName().c_str() );
        eventllh_ = newllh;
    }

    /**
     * set/change minimizer
     * If you write a reconstruction module in which you'd like to do the
     * same fit with different minimizers, then you use this method to swap
     * the minizer service (keeping the parametrization and the likelihood
     * unchanged).
     *
     * @param[in] m shared pointer to alternative minimizer service
     */
    void ChangeMinimizer( I3MinimizerBasePtr m ){
        log_debug( "switching event log-likelihood function to %s", m->GetName().c_str() );
        minimizer_ = m;
    }

    /**
     * Set up gradient support, checking that both the parametrization 
     * and likelihood also support that. If not, then this throws a
     * log_fatal().
     */
    void UseGradients(bool);

    /**
     * Get the *negative* log-likelihood function value (via parameter
     * array) This implements the virtual method/operator inherited from
     * I3GulliverBase. This method is called by the minimizer to evaluate
     * the function varue for a particular set of parameter values.
     *
     * The parameter values are converted into an event hypothesis (using
     * the parametrization service), from which the event loglikelihood
     * service will compute the function value (negative log-likelihood).
     *
     * @param[in] par array of parameters
     * @returns -log(likelihood)
     */
    double operator()( const std::vector<double> &par );

    /**
     * Same as above, but also compute gradient w.r.t. parameters.
     */
    double operator()( const std::vector<double> &par,
                       std::vector<double> &grad );

    /**
     * Get the *negative* log-likelihood function value for a particular
     * event hypothesis, using the actual physics object (instead of
     * the array of parameters, as in @c operator() (see above).
     *
     * This can be useful for gulliver-based modules, to compute @c -log(L)
     * for some specific hypothesis in the current event.
     *
     * @param[in] p const ref to event hypothesis object
     * @returns -log(likelihood)
     */
    double GetFunctionValue( const I3EventHypothesis &p ){
        return -1.0*eventllh_->GetLogLikelihood( p );
    }

    /**
     * Get the *negative* log-likelihood function value for a particular
     * event hypothesis, using the actual physics object (instead of
     * the array of parameters, as in @c operator() (see above).
     *
     * @param[in] ptr shared pointer to event hypothesis object
     * @returns -log(likelihood)
     */
    double GetFunctionValue( I3EventHypothesisConstPtr ptr ){
        return -1.0*eventllh_->GetLogLikelihood( *ptr );
    }

    /**
     * Pass the geometry to the llh service.
     * @param geo geometry
     */
    void SetGeometry(const I3Geometry &geo);

    /**
     * Pass the event & geometry to the llh service.
     * @param f current frame
     * @returns number of degrees of freedom
     */
    int SetEvent(const I3Frame &f);

    /**
     * @brief do fit, starting from specified seed
     * @param fitptr both input and output: it's assumed to be initialized with some seed
     *               and will hold the result of the fit (if it converged)
     * @returns true if converged, false if fit failed
     *
     * Before you call this, either the @c SetEvent(f) or the @c Fit(f,fitptr)
     * method should have been called for the current frame/event.
     * 
     * @sa I3Gulliver::GetMinimizerDetails() to get more diagnostics about fit result
     * @sa I3EventHypothesis
     */
    bool Fit( I3LogLikelihoodFitPtr fitptr );

    /**
     * This convenience method is equivalent to running @c SetEvent(f) and
     * @c Fit(fitptr), respectively.
     */
    bool Fit( const I3Frame &f, I3LogLikelihoodFitPtr fitptr );

    /**
     * @brief get details about fit errors, number of iterations etc., of the latest fit
     * @sa I3MinimizerResult
     *
     * This is for debugging your recomodule. Also, if you'd like to store the fit errors,
     * and the minimizer actually computes them, then you can retrieve them from this
     * object.
     *
     * @todo for simplicity I just return a copy now; maybe it should be a const ref or so.
     */
    I3MinimizerResult GetMinimizationDetails() const {
        return minimizerResult_;
    }

    /**
     * convenience method:
     * set theta between 0 and pi, phi between 0 and 2pi
     * (would be nice to have this as a I3Particle method)
     * DJB Oct 31: return false if zenith/azimuth is pathologically unfixable
     * Current definition of unfixable: fabs(angle)>1e6. It's unfixable because
     * for large floats, 2pi gets smaller than the floating point number precision.
     */
    static bool AnglesInRange( I3Particle &p, const std::string &name );

    /**
     * @brief Enable tracing, in order to debug the minimization process. This call also resets (reinitializes) the trace.
     * @sa GetTrace, NoTrace
     */
    void Trace();

    /**
     * Disable tracing
     * @sa GetTrace, Trace
     *
     * Discards any trace collected so far, and for upcoming fits no trace information will be stored.
     */
    void NoTrace();

    /**
     * @brief Return the current trace. This returns a (smart pointer to a) simple vector of doubles.
     *
     * With npar fit parameters and nmini function calls, the trace has a length of @c (npar+1)*nmini.
     * For every function call the @c npar parameter values and the @c llh value is stored, so in order to
     * retrieve the function value of the 7th function call in a 5 parameter fit you would check
     * the 41st (41=(7-1)*(5+1)+5) element of the vector.
     *
     * Note that a new @c I3VectorDouble object will be made every time when the trace is reset using
     * the @c Trace() call. So if you are interested in many fits performed with an @c I3Gulliver object,
     * then it is *not* sufficient to retrieve the @c I3VectorDoublePtr only at the beginning and checking
     * its contents many times. Before *every* fit you call @c Trace() to reinitialize the trace vector,
     * and *after* each fit you retrieve it.
     *
     * @sa Trace, NoTrace
     */
    I3VectorDoublePtr GetTrace();

private:

    SET_LOGGER("I3Gulliver");

    /**
     * smart pointer to the parametrization agent for a particle p
     */
    I3ParametrizationBasePtr parametrization_;

    /**
     * smart pointer to the provider of the event log-likelihood for a particle p
     */
    I3EventLogLikelihoodBasePtr eventllh_;

    /**
     * smart pointer to the minimizer
     */
    I3MinimizerBasePtr minimizer_;

    /**
     * minimizer details of latest fit
     */
    I3MinimizerResult minimizerResult_;

    /**
     * Counting how often the minimizer queries the likelihood function
     * with proper parameter values, for the current fit. Will be reset
     * to zero at the beginning of every @c Fit() call.
     */
    unsigned int nFunctionEvaluations_;

    /**
     * Counting how often the minimizer queries the likelihood function
     * with proper parameter values, for the lifetime of this I3Gulliver
     * object (typically the duration of your processing script).
     *
     * If the log-level for I3Gulliver is set to INFO or lower, then this
     * number gets printed when the I3Gulliver object goes out of scope
     * (end of run, typically).
     */
    unsigned int nFunctionEvaluationsTotal_;

    /**
     * Count how often the minimizer queries the likelihood function
     * with NAN parameter values, for the current fit. Will be reset to
     * zero at the beginning of every @c Fit() call.
     */
    unsigned int nNANfromMinimizer_;

    /**
     * Count how often the minimizer queries the likelihood function with
     * NAN parameter values, for the lifetime of this I3Gulliver object
     * (typically the duration of your processing script).
     *
     * If the log-level for I3Gulliver is set to INFO or lower, and
     * there were indeed NAN parameter values, then this number gets
     * printed when the I3Gulliver object goes out of scope (end of run,
     * typically).
     */
    unsigned int nNANfromMinimizerTotal_;

    /**
     * Fitting with or without gradient?
     */
    bool withGradient_;

    /**
     * Identifier/name to use in log messages. E.g. name of module.
     */
    const std::string name_;

    /**
     * Trace information.
     */
    I3VectorDoublePtr trace_;

    /**
     * If the function call with NAN parameters occurs then this function
     * is used to print the error message.
     * (This should never happen, unless there is a critical error in the
     * parametrization, minimizer or likelihood.)
     */
    void NANParError( const std::vector<double> &par );

    /**
     * Checks whether minimizer wants to use gradients, and if so,
     * that the parametrization and likelihood also support that.
     * If not, then this throws a log_fatal().
     *
     * Checked at the start of every fit, *after* checking that the event
     * multiplicity is OK.
     */
    void CheckGradientCompatibility();

};

I3_POINTER_TYPEDEFS( I3Gulliver );

#endif /* GULLIVER_I3GULLIVER_H_INCLUDED */
