#ifndef I3LOGLIKELIHOODCALCULATOR_H_INCLUDED
#define I3LOGLIKELIHOODCALCULATOR_H_INCLUDED

/**
 * copyright  (C) 2007
 * the icecube collaboration
 * $Id$
 *
 * @file I3LogLikelihoodCalculator.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

// gulliver stuff
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3SeedServiceBase.h"

// icetray stuff
#include "icetray/I3ConditionalModule.h"
#include "icetray/IcetrayFwd.h"
#include "dataclasses/physics/I3Particle.h"

/**
 * @class I3LogLikelihoodCalculator
 * @brief Gulliver-based module to compute the log-likelihood for a given track.
 *
 * This module does not do any reconstruction, it just computes the
 * loglikelihood for a given I3Particle and a likelihood service.
 * 
 * It stores the same set of loglikelihood fitparams that I3SimpleFitter or
 * I3IterativeFitter would store when run with a hypothetical minimizer which
 * would evaluate the likelihood function only once (for the seed track) and then
 * declares convergence.
 * 
 * This module is only for the simplest cases of an I3Particle object, no composite
 * or otherwise complicated tracks which require a seed service and/or additional
 * non-standard objects to specify the physics variables of the event hypothesis.
 * 
 * In a later stage I might add that extra functionality, for now this simple
 * functionality is what I need.
 * 
 * By default the result is an object of type I3LogLikelihoodFitParams
 * (defined in the gulliver project), which is stored in the frame
 * under the modules own name (that is, the second argument of the
 * AddModule(module,name) command in a typical icetray python script). If
 * you set the KeepFitName flag and your input fit has the name "FooBar"
 * then the result will be stored as "FooBar_FitParams" instead, as if
 * the "FooBar" was the result of a I3SimpleFitter or I3IterativeFitter.
 * 
 * There are at least two use cases for this module:
 * - A loglikelihood fit was performed in some previous processing level
 *   (e.g. online filtering, for the Moon) but the loglikelihood fitparams
 *   were not kept in the subsequent reading & writing from/to i3 files.
 *   Only the I3Particle was kept. With this module you can re-compute the
 *   the likelihood without doing a possibly time-costly fit. The number of
 *   minimizer steps (or rather: the number of llh function evaluations) cannot
 *   be retrieved anymore.
 * - If you would like to know the likelihood of some track without doing a
 *   full reconstruction, e.g. to see the difference between the seed and the
 *   full fit.
 * 
 * @todo Extend this module such that you can also use it with nonstandard
 *       event hypotheses (which need an additional object besides the I3Particle
 *       in order to store the relevant physics variables; and/or need a seed
 *       service to tweak and such).
 */
class I3LogLikelihoodCalculator : public I3ConditionalModule {

public:

    /// constructor (define configurables)
    I3LogLikelihoodCalculator(const I3Context& ctx);

    /// destructor
    ~I3LogLikelihoodCalculator(){}

    /// configure (get & check configurables)
    void Configure();

    /// handle geometry
    void Geometry(I3FramePtr frame);

    /// do a reconstruction
    void Physics(I3FramePtr frame);

    /// say bye
    void Finish();

private:

    // inhibit default constructors and assignment by making them private
    I3LogLikelihoodCalculator(); /// inhibited
    I3LogLikelihoodCalculator(const I3LogLikelihoodCalculator& source); /// inhibited
    I3LogLikelihoodCalculator& operator=(const I3LogLikelihoodCalculator& source); /// inhibited

    /// paranoid check that input particle is healthy
    bool CheckParticle( I3ParticleConstPtr p );

    /// log-likelihood service
    I3EventLogLikelihoodBasePtr llhService_;
    I3ParametrizationBasePtr paramService_;

    // configurables (see parameter docs in icetray-inspect)
    std::string llhServiceName_;  /// Name of event loglikelihood calculation service.
    std::string fitName_;         /// fit name
    int nPar_;                    /// pretent there were nPar fitting parameters

    // diagnostics
    unsigned int nEvent_; /// counts physics frames
    unsigned int nFitOK_; /// counts I3Particle with OK fitstatus
    unsigned int nFitBad_; /// counts I3Particle that are absent or with non-OK fitstatus
    unsigned int nSuccess_; /// counts nr of events in which a non-NAN llh was computed

    // thingy for log4cplus
    SET_LOGGER( "I3LogLikelihoodCalculator" );
  
};  // end of the class definition.


#endif /* I3LOGLIKELIHOODCALCULATOR_H_INCLUDED */
