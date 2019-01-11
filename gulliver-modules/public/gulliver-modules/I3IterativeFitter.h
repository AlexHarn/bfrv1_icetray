/**
 * copyright (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3IterativeFitter.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

#ifndef I3ITERATIVEFITTER_H_INCLUDED
#define I3ITERATIVEFITTER_H_INCLUDED

#include <string>
#include <utility>
#include <vector>

#include "dataclasses/physics/I3Particle.h"

#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3LogLikelihoodFit.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3ParametrizationBase.h"
#include "gulliver/I3SeedServiceBase.h"

#include "icetray/I3ConditionalModule.h"
#include "icetray/IcetrayFwd.h"

#include "phys-services/I3RandomService.h"

/**
 * @class I3IterativeFitter
 *
 * @brief Gulliver-based module to perform iterative reconstructions
 *
 * The I3IterativeFitter is a reconstruction module which specializes in
 * reconstructing the direction of a tracks or showers. The module works very
 * similar to I3SimpleFitter: it uses the Gulliver toolkit to reconstruct e.g.
 * a muon track, using the likelihood, parametrization of fit variables and the
 * minimizer algorithm from external (Gulliver) services.
 *
 * For each seed (as provided by a seed service, see I3SeedServiceBase in
 * gulliver or I3BasicSeedService in lilliput) first regular fit is performed,
 * just like I3SimpleFitter does. This fit is then redone several times, each
 * time seeding the minimizer with the result of the previous iteration except
 * that the direction is replaced with some (quasi-) random other direction. In
 * most iterations the result will be worse than for the fit with the original
 * seed, but for some events the original fit may have been found just a local
 * minimum, while during the iterations the global minimum of -log(L) is found
 * (or at least a deeper local minimum).
 *
 * See the documentation of I3SimpleFitter module for more information about
 * the configuration of the services, and about how the results of the
 * reconstruction are stored in the frame.
 *
 * The number of iterations is set with the "NIterations" option. The
 * generated directions can be restricted to fall within a limited range of
 * cos(zenith), specified with the "CosZenithRange" option. You can either use
 * a random service (specified with the "RandomService" option) purely random
 * directions for the iterations, or use a socalled Sobol/Niederreiter2
 * sequence (see below) to generate a pseudorandom grid of directions. You will
 * get the Sobol sequence if you leave the "RandomService" option empty, or if
 * you fill out "SOBOL". You will get the Niederreiter2 sequence if you fill
 * out "NIEDERREITER2".
 *
 *
 * About the Sobol and Niederreiter2 Grids
 *
 * A 2-dimensional Sobol grid is a "quasirandom" sequence of pairs of numbers
 * between 0 and 1. These numbers are uniformly distributed over the unit
 * square (the square with corners at (0,0), (0,1), (1,0) and (1,1)). The
 * sequence is designed to maximally "cover" the phase space. It's actually a
 * pretty simple sequence, the first few points are:<br>
 * (0.500, 0.500)<br>
 * (0.750, 0.250)<br>
 * (0.250, 0.750)<br>
 * (0.375, 0.375)<br>
 * (0.875, 0.875)<br>
 * (0.625, 0.125)<br>
 * (0.125, 0.625)<br>
 * ....
 *
 * Using the Sobol sequence (instead of a proper random number generator) to
 * generate the directions of the iterations of the iterative likelihood has
 * two advantages: (1) the already mentioned uniform coverage of the phase
 * space (with a true random number generator, it can in principle happen that
 * all generated directions for that event are in the lower half of the phase
 * space) (2) the generated directions are much more easily reproducible (a
 * problem with truely random iterative fit in AMANDA was that redoing the fit
 * sometimes led to vastly different results).
 *
 * We compute the sequence Sobol grid points only once and use the same
 * sequence for every seed in every event. However, the actual directions are
 * chosen to be (kind of) relative to the original seed direction. This is done
 * by rescaling cos(zenith) and azimuth such that their range runs from 0 to 1.
 * For a given seed we add its rescaled cos(zenith) and azimuth to all grid
 * points of the sequence, and if necessary they are moved back into the unit
 * square by taking their coordinates modulo 1.0. These translated grid points
 * are then scaled back to regular cos(zenith) and azimuth values.
 *
 * A "Niederreiter2" sequence another quasirandom number generator, just like
 * Sobol. Both sequences are implemented in GSL (@c gsl_qrng_sobol and
 * @c gsl_qrng_niederreiter_2).
 *
 * @sa <a href="http://www.gnu.org/software/gsl/manual/html_node/Quasi_002drandom-number-generator-algorithms.html">Sobol and Niederreiter_2 in GSL</a>
 *
 */
class I3IterativeFitter : public I3ConditionalModule
{
    public:
        I3IterativeFitter(const I3Context& ctx);

        ~I3IterativeFitter();

        void Configure();
        void Geometry(I3FramePtr frame);
        void Physics(I3FramePtr frame);
        void Finish();

    private:
        // make default constructors and assignment private
        I3IterativeFitter();
        I3IterativeFitter(const I3IterativeFitter& source);
        I3IterativeFitter& operator=(const I3IterativeFitter& source);

        /// The core Gulliver object for basic tracks
        I3GulliverPtr fitterCore_;

        /// Random service to generate random directions.
        I3RandomServicePtr randomService_;

        // configurables
        /// Event loglikelihood calcuation service
        I3EventLogLikelihoodBasePtr likelihood_;
        /// Track/shower/anything parametrization service
        I3ParametrizationBasePtr parametrization_;
        /// Minimizer service
        I3MinimizerBasePtr minimizer_;
        /// Seed collection and preparation service
        I3SeedServiceBasePtr seedService_;
        /// Seed preparation service for the (pseudo) random iteration seeds
        I3SeedServiceBasePtr tweakService_;
        /// Random number service
        std::string randomServiceName_;
        /// Number of random iterations on each seed
        unsigned int nIterations_;
        /// Range of cosine zenith angles within which to generate directions
        std::vector<double> cosZenithRange_;
        /// Fit name; used to store results in the frame.
        std::string fitName_;
        // TODO not implemented yet
        // /// Minimum angular difference between stored solutions
        // double minDifference_;
        // /// Maximum number of stored solutions
        // unsigned int nMaxSolutions_;

        /// Sobol or Niederreiter grid (used to generate pseudo-random
        /// directions)
        std::vector<std::pair<double, double>> gridPoints_;

        /// Number of Physics calls, used in log messages
        unsigned int eventNr_;
        /// Number of Physics calls with at least one good seed
        unsigned int nSeeds_;
        /// Number of seeds resulting in a successful fit
        unsigned int nSuccessFits_;
        /// Number of Physics calls with a successful fit
        unsigned int nSuccessEvents_;

        std::string nonStdName_;

        /**
         * Initialize the Sobol/Niederreiter grid with @c nIterations_ grid
         * points
         * @param[in] sobol Use Sobol (true) or Niederreiter2 (false)
         */
        void InitGridPoints(bool sobol);

        /**
         * Rescale a direction to a point in the unit square (applied to seed
         * direction)
         */
        std::pair<double, double> GetNormDir(const I3Direction &dir);

        /**
         * Set the direction based on a Sobol/Niederreiter grid point and an
         * offset (based on seed direction).
         */
        void SetGridDir(I3ParticlePtr particle,
                        const std::pair<double, double>& offnorm,
                        unsigned int index);

        I3LogLikelihoodFitPtr BestFit(unsigned int nseeds);

        /**
         * Convenience method: do fit, and store it in a vector (if successful)
         */
        void Fit(int seed, int iteration, I3LogLikelihoodFitPtr fit,
                 std::vector<I3LogLikelihoodFit>& goodFits);

        SET_LOGGER("I3IterativeFitter");

}; // end of the class definition

#endif /* I3ITERATIVEFITTER_H_INCLUDED */
