#ifndef I3EVENTLOGLIKELIHOODCOMBINER_H_INCLUDED
#define I3EVENTLOGLIKELIHOODCOMBINER_H_INCLUDED

/**
 *  copyright  (C) 2007
 *  the icecube collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

// std lib stuff
#include <string>
#include <vector>

// icetray stuff
#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3EventHypothesis.h"

/**
 * @class I3EventLogLikelihoodCombiner
 * @brief Auxiliary class for combining event log-likelihood functions (service)
 *
 * This class can be used to combine two or more likelihood services which
 * are Added and configured separately to the tray.  The most obvious
 * (and maybe only) application is the Bayesian likelihood reconstrucion,
 * in which a weight function (which only depends on the event hypothesis,
 * not on the event data) is combined with an arbitrary other likelihood,
 * e.g. convoluted Pandel.
 *
 * Configuration parameters:
 * - @c InputLogLikelihoods List of names of log-likelihood services
 * - @c RelativeWeights List of relative weights to apply 
 * - @c Multiplicity Multiplicity calculation mode
 *
 * For the "multiplicity calculation mode" there are several options:
 * you can give the name of one of the likelihood services, then the
 * multiplicity as calculated by that service will also be used for the
 * combined likelihood. Alternatively, you can specify "Max" (default),
 * "Min" or "Sum"; with those options, the combiner lets all likelihood
 * services compute the multiplicity and then defines the multiplicity
 * of the combined likelihood as the maximum, minimum or sum of those
 * mulitplicities, respectively.
 *
 * @sa I3Gulliver
 * @sa I3GulliverBase
 * @sa I3EventHypothesis
 * @sa I3EventLogLikelihoodBase
 * @sa I3ServiceBase
 *
 */
class I3EventLogLikelihoodCombiner : public I3ServiceBase,
                                     public I3EventLogLikelihoodBase {

public:

    /// constructor
    I3EventLogLikelihoodCombiner(const I3Context&);

    /// destructor
    virtual ~I3EventLogLikelihoodCombiner();

    /**
     * This method is inherited from I3ServiceBase. The factory for
     * this service is constructed using the I3SingleServiceFactory
     * or I3MultiServiceFactory.
     */
    virtual void Configure();

    /**
     * This should be called for every geometry frame
     * The likelihood combiner will not use the geometry itself, it will
     * pass the geometry to the to-be-combined likelihood services.
     */
    virtual void SetGeometry( const I3Geometry &geo );

    /**
     * This will or should be called before doing a new fit.
     * The likelihood combiner will not use the frame itself, it will
     * pass the frame on to the to-be-combined likelihood services.
     */
    virtual void SetEvent( const I3Frame &f );

    /**
     * Get +log(likelihood) for a particular emission hypothesis
     *
     * This returns the weighted sum of the log-likelihood values
     * returned by each of the configured likelihood services.
     */
    virtual double GetLogLikelihood( const I3EventHypothesis &ehypo );

    virtual bool HasGradient();

    virtual double GetLogLikelihoodWithGradient(const I3EventHypothesis &ehypo,
        I3EventHypothesis&, double weight);

    /**
     * Get the multiplicity of the current input event (e.g. number of good hits).
     * The number of degrees of freedom for the fit will be calculated as:
     * multiplicity minus number of fit parameters.
     *
     * Need to think how to define this.
     */
    virtual unsigned int GetMultiplicity();

    virtual I3FrameObjectPtr GetDiagnostics(const I3EventHypothesis &);

    /// tell your name
    virtual const std::string GetName() const {
        return I3ServiceBase::GetName();
    }

private:

    /// the names of likelihoods to combine
    std::vector<std::string> llhNamesVector_;

    /// the pointers to the likelihood services to combine
    std::vector<I3EventLogLikelihoodBasePtr> logLikelihoods_;

    /// the respective weights of likelihoods to combine
    std::vector<double> weightsVector_;

    /// classifier type: how to compute multiplicity of an event
    enum MultiModes {
        MMode_Max=0,   /// max of multiplicities reported by llh services
        MMode_Min,     /// min of multiplicities reported by llh services
        MMode_Sum,     /// sum of multiplicities reported by llh services
        MMode_Favorite /// multiplicity reported by one particular llh service
    } multiMode_;

    /// option string for multiplicity mode
    std::string multiModeString_;

    SET_LOGGER( "I3EventLogLikelihoodCombiner" );
};

I3_POINTER_TYPEDEFS( I3EventLogLikelihoodCombiner );

#endif /* I3EVENTLOGLIKELIHOODCOMBINER_H_INCLUDED */
