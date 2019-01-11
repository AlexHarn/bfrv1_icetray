/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author David Boersma <boersma@icecube.wisc.edu>

*/

#ifndef GULLIVER_I3LOGLIKELIHOODFIT_H_INCLUDED
#define GULLIVER_I3LOGLIKELIHOODFIT_H_INCLUDED

// stdlib, shared pointers
#include <boost/shared_ptr.hpp>
#include <string>

// result types
#include "dataclasses/physics/I3Particle.h"
#include "icetray/I3FrameObject.h"
#include "gulliver/I3LogLikelihoodFitParams.h"
#include "gulliver/I3EventHypothesis.h"

/**
 * @class I3LogLikelihoodFit
 * @brief complete fit result for an arbitrary event type ("emission hypothesis")
 *
 * You need to know about this class if you would like to implement a
 * new Gulliver-based reconstruction module. You do *not* need to know
 * about it when you are implementing Gulliver components, such as a
 * likelihood functions, parametrizations and minimizers.
 *
 * This class has just two datamembers, one just for the physics result, and
 * one for the details and diagnostics of the likelihood fit.
 *
 * @sa I3EventHypothesis
 * @sa I3LogLikelihoodFitParams
 */
class I3LogLikelihoodFit {
    public:
        /// dummy constructor
        I3LogLikelihoodFit():
            hypothesis_(new I3EventHypothesis),
            fitparams_( new I3LogLikelihoodFitParams )
        { }

        /**
         * handy constructor: initialize with seed pointer
         * @param[in] ehypo Pointer to event hypothesis
         *            (the pointer itself is used, no deep/shallow copy of the object is made)
         * @param[in] params Pointer to fit params
         *            (the pointer itself is used, no deep/shallow copy of the object is made)
         */
        I3LogLikelihoodFit( I3EventHypothesisPtr ehypo,
                            I3LogLikelihoodFitParamsPtr params ):
            hypothesis_( ehypo ),
            fitparams_( params )
        { }

        /**
         * handy constructor: initialize with seed reference
         * @param[in] ehypo Reference to event hypothesis
         *            (NOTE: only a shallow copy of the object is made)
         * @param[in] params Reference to fit params (full copy of the object is made)
         */
        I3LogLikelihoodFit( const I3EventHypothesis &ehypo,
                            const I3LogLikelihoodFitParams &params ):
            hypothesis_(new I3EventHypothesis(ehypo)),
            fitparams_( new I3LogLikelihoodFitParams(params) )
        { }

        /**
         * handy constructor: initialize with seed reference
         * @param[in] ehypo Reference to event hypothesis
         *            (NOTE: only a shallow copy of the object is made)
         */
        I3LogLikelihoodFit( const I3EventHypothesis &ehypo ):
            hypothesis_(new I3EventHypothesis(ehypo)),
            fitparams_( new I3LogLikelihoodFitParams )
        { }

        /**
         * handy constructor for vanilla event types: initialize with seed reference
         * @param[in] p Reference to particle
         *              (full copy of the particle object is made)
         * @param[in] params Reference to fit params (full copy of the params is made)
         */
        I3LogLikelihoodFit( const I3Particle &p,
                            const I3LogLikelihoodFitParams &params ):
            hypothesis_(new I3EventHypothesis(p)),
            fitparams_( new I3LogLikelihoodFitParams(params) )
        { }

        /// the fit (to be)
        I3EventHypothesisPtr hypothesis_;

        /// object holding statistics and reconstruction info: likelihood etc.
        I3LogLikelihoodFitParamsPtr fitparams_;

        /// object holding specific diagnostics from the llh service
        I3FrameObjectPtr llhdiagnostics_;

        /// object holding specific diagnostics from the minimizer service
        I3FrameObjectPtr minidiagnostics_;

        /// object holding specific diagnostics from the parametrization service
        I3FrameObjectPtr paradiagnostics_;

        // suffixes for storage in the I3Frame
        static const std::string PARTICLE_SUFFIX;
        static const std::string NONSTD_SUFFIX;
        static const std::string FITPARAMS_SUFFIX;
        static const std::string PARTICLEVECT_SUFFIX;
        static const std::string FITPARAMSVECT_SUFFIX;
        static const std::string NONSTDVECT_SUFFIX;
};

/// "less than" comparison operator, for sorting solutions
bool operator<(const I3LogLikelihoodFit& lhs, const I3LogLikelihoodFit& rhs);

/// "greater than" comparison operator, for sorting solutions
bool operator>(const I3LogLikelihoodFit& lhs, const I3LogLikelihoodFit& rhs);


I3_POINTER_TYPEDEFS( I3LogLikelihoodFit );

#endif /* GULLIVER_I3LOGLIKELIHOODFIT_H_INCLUDED */
