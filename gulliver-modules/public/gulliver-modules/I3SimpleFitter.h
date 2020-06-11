/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3SimpleFitter.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

#ifndef I3SIMPLEFITTER_H_INCLUDED
#define I3SIMPLEFITTER_H_INCLUDED

#include <string>
#include <vector>

#include "dataclasses/I3Vector.h"
#include "dataclasses/physics/I3Particle.h"

#include "gulliver/I3Gulliver.h"
#include "gulliver/I3SeedServiceBase.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3ParametrizationBase.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3LogLikelihoodFit.h"

#include "icetray/I3ConditionalModule.h"
#include "icetray/IcetrayFwd.h"

/**
 * @class I3SimpleFitter
 * @brief Gulliver-based module to perform simple generic log-likelihood
 * reconstructions.
 *
 * This module obtains seeds from a seed service (specified with the
 * "SeedService" option), then performs reconstruction using a minimizer
 * service (specified with the "Minimizer" option), a loglikelihood service
 * (specified with the "LogLikelihood" option) to find a best fit, with the
 * fittable variables determined by a parametrization service (specified with
 * the "Parametrization" option).
 *
 * This module is very generic, it does not make any assumptions about what
 * kind of event you are trying to reconstruct. The actual physics is taken of
 * by the Gulliver services.
 *
 * For more general information about Gulliver services, read the documentation
 * of of the @c gulliver project, which defines the interfaces for gulliver
 * services. The @c lilliput project contains a collection of actual
 * implementations.
 *
 * The result of the fit consists of two or three objects. First: an I3Particle
 * object, stored under the module name. Second: *if* the kind of events to be
 * reconstructed required "nonstandard" variables, organized in some class
 * deriving from I3FrameObject, then an object of that class stored under the
 * name composed as the module name plus the "Params" suffix. Third: the
 * details of the log-likelihood reconstruction, such as the final (proper and
 * reduced) likelihood, the number of times the likelihood function was
 * evaluated and the number of degrees of freedom; these are stored as a
 * I3LogLikelihoodFitParams object, under the name composed as the module name
 * plus the "FitParams" suffix.
 *
 */
class I3SimpleFitter : public I3ConditionalModule
{
    public:
        I3SimpleFitter(const I3Context& ctx);

        ~I3SimpleFitter(){}

        void Configure();
        void Geometry(I3FramePtr frame);
        void Physics(I3FramePtr frame);
        void Finish();

    private:
        // make default constructors and assignment private
        I3SimpleFitter();
        I3SimpleFitter(const I3SimpleFitter& source);
        I3SimpleFitter& operator=(const I3SimpleFitter& source);

        I3LogLikelihoodFitPtr Fit(I3FramePtr frame,
                                  const I3EventHypothesis& seed);

        I3LogLikelihoodFitPtr Fit(I3FramePtr frame, unsigned int nseeds,
                                  I3VectorI3ParticlePtr allFits,
                                  I3LogLikelihoodFitParamsVectPtr params,
                                  std::vector<I3VectorDoublePtr>& traces);

        /// The core Gulliver object for basic tracks
        I3GulliverPtr fitterCore_;

        /// Type to specify tracing option
        enum TraceModeType
        {
            TRACE_NONE=0,
            TRACE_ALL,
            TRACE_SINGLE
        };

        /// Type to remember whether to store all or some of the fits
        enum StoragePolicyType
        {
            ONLY_BEST_FIT=0,
            ALL_FITS_AND_FITPARAMS_IN_VECTORS,
            ALL_FITS_AND_FITPARAMS_NOT_IN_VECTORS,
            ALL_RESULTS_IN_VECTORS,
            ALL_RESULTS_NOT_IN_VECTORS
        };

        // configurables (see parameter docs in icetray-inspect)
        /// Seed preparation service
        I3SeedServiceBasePtr seedService_;
        /// Event loglikelihood calcuation service
        I3EventLogLikelihoodBasePtr likelihood_;
        /// Track/shower/anything parametrization service
        I3ParametrizationBasePtr parametrization_;
        /// Minimizer service
        I3MinimizerBasePtr minimizer_;
        /// Option to store single or multiple results
        std::string storagePolicyString_;
        /// Option to store single or multiple results
        StoragePolicyType storagePolicy_;
        /// Name to use when storing non-standard part of hypothesis
        std::string nonStdName_;
        /// Whether to keep trace information and what to store
        std::string traceModeString_;
        /// Option to store fit tracing information (for debugging)
        TraceModeType traceMode_;

        std::string fitName_;
        std::string geometryName_;
        unsigned int eventNr_;
        unsigned int nSeeds_;
        unsigned int nSuccessFits_;
        unsigned int nSuccessEvents_;

        SET_LOGGER("I3SimpleFitter");

}; // end of the class definition

#endif /* I3SIMPLEFITTER_H_INCLUDED */
