/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

#ifndef GULLIVER_I3EVENTLOGLIKELIHOODBASE_H_INCLUDED
#define GULLIVER_I3EVENTLOGLIKELIHOODBASE_H_INCLUDED

// smart pointer definition
#include "boost/shared_ptr.hpp"
#include <string>


// icetray stuff
#include "icetray/IcetrayFwd.h"
#include "icetray/I3DefaultName.h"
#include "icetray/I3FrameObject.h"
class I3EventHypothesis;
class I3Geometry;

/**
 * @class I3EventLogLikelihoodBase
 * @brief Base class for event log-likelihood functions (service)
 *
 * You only need to know about this class if you would like to implement
 * a (wrapper for a) likelihood function, to be used by a Gulliver-based
 * reconstruction module. You do *not* need to know about it when you are
 * implementing any other Gulliver components, such as a parametrizations
 * and minimizers.
 *
 * If you write a Gulliver-based reconstruction module then (in principle)
 * you only need to know that this class exists and that you need to
 * provide (a pointer to) an object of the I3MinimizerBase class to the
 * I3Gulliver core of your module.  This pure base class is the base
 * class for event log-likelihood functions.
 *
 * If you implement a new I3EventLogLikelihoodBase subclass, then please also
 * make a service factory which creates & configures an object of this new class.
 *
 * @sa I3Gulliver
 * @sa I3GulliverBase
 * @sa I3EventHypothesis
 *
 */
class I3EventLogLikelihoodBase {

public:

    /// base class constructor (idle)
    I3EventLogLikelihoodBase(){}

    /// base class destructor (idle)
    virtual ~I3EventLogLikelihoodBase(){}

    /**
     * This should be called on each geometry frame
     * The implementation should get the geometry from the frame
     * and store it in a format which is convenient and efficient for
     * calculating the likelihood
     */
    virtual void SetGeometry( const I3Geometry &f ) = 0;

    /**
     * This will or should be called before doing a new fit.  The
     * implementation should get the relevant event data (hits, pulses,
     * waveforms, geometry) from the frame and store it in a format
     * which is convenient and efficient for calculating the likelihood.
     */
    virtual void SetEvent( const I3Frame &f ) = 0;

    /**
     * Get +log(likelihood) for a particular emission hypothesis
     *
     * The event data should have been provided earlier via the SetEvent method.
     *
     * If you implement a likelihood for some non-standard topology
     * (e.g. double muons) then you should check that the nonstd of
     * @c ehypo indeed contains an object of the class relevant the
     * particular topology for which your likelihood function is intended.
     *
     * For likelihood functions for vanilla hypothesis (muon,
     * cascade,...) implementations should of course check that the
     * particle has indeed the appropriate type/shape.
     *
     * @todo Maybe add a bool TestHypothesisType(const I3EventHypothesis&)
     * method, so that this type checking needs to be done only once
     * per event instead of for each minimizer iteration.
     */
    virtual double GetLogLikelihood( const I3EventHypothesis &ehypo ) = 0;

    /**
     * @brief Same as GetLogLikelihood, *plus* gradient calculation
     *
     * The log(likelihood) is computed just like GetLogLikelihood does,
     * but also the derivatives w.r.t. any requested variables are computed.
     *
     * The "gradient" argument serves both to specify which derivatives should
     * be computed as well as to return the results. E.g. if derivatives w.r.t.
     * x, y and z should be computed, then the calling code should make sure
     * that all (floating point) datamembers of gradient.particle are set to
     * 0. except for x, y and z. The GetLogLikelihoodWithGradient computes
     * the x, y and z derivatives and stores them in gradient.particle.
     *
     * Note that for likelihoods which use the ehypo.nonstd datamember,
     * also gradients w.r.t. those nonstd variables should be computed
     * (when requested).
     *
     * Minor wrinkle: likelihoods should *add* gradients to the already-existing
     * fields in the gradient hypothesis with the given weight.
     *
     * If this functionality is implemented, then make sure to also overload
     * the HasGradient method to return true.
     */
    virtual double GetLogLikelihoodWithGradient( const I3EventHypothesis &ehypo,
                                                 I3EventHypothesis &gradient, double weight=1 ){
        log_fatal("NOT IMPLEMENTED");
        return 0.;
    }

    /**
     * If GetLogLikelihoodWithGradient is implemented, then this method should
     * be overloaded to return "true".
     */
    virtual bool HasGradient(){
        return false;
    }

    /**
     * Some likelihood functions may be able to calculate gradients and/or
     * analytically maximize out extra dimensions. Overload this function
     * if yours is one of the fancy few.
     */
    virtual double GetLogLikelihood (const I3EventHypothesis &ehypo,
                                           I3EventHypothesis *gradient,
                                           bool maximize_extra_dimensions, double weight=1){
        if (gradient && HasGradient()) {
            return GetLogLikelihoodWithGradient(ehypo, *gradient, weight);
        } else {
            return GetLogLikelihood(ehypo);
        }
    }

    /**
     * Get the multiplicity of the current input event (e.g. number of good hits).
     * The number of degrees of freedom for the fit will be calculated as:
     * multiplicity minus number of fit parameters.
     */
    virtual unsigned int GetMultiplicity() = 0;

    /**
     * Get any extra information that might be interesting for the analysis.
     * This is very specific to the particulars of the likelihood implmentation.
     * Gulliver modules will get this frame object through the fit result pointer
     * from the I3Gulliver object, and if it is indeed defined (pointer not NULL)
     * then they should store it into the frame.
     */
    virtual I3FrameObjectPtr GetDiagnostics( const I3EventHypothesis &fitresult ){
        return I3FrameObjectPtr();
    }

    /// tell your name
    virtual const std::string GetName() const = 0;
};

I3_POINTER_TYPEDEFS( I3EventLogLikelihoodBase );

#endif /* GULLIVER_I3EVENTLOGLIKELIHOODBASE_H_INCLUDED */
