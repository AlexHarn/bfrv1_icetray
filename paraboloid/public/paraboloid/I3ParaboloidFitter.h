#ifndef I3PARABOLOIDFITTER_H_INCLUDED
#define I3PARABOLOIDFITTER_H_INCLUDED

/**
 * @file I3ParaboloidFitter.h
 * copyright (C) 2005 the icecube collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 * @author Chad Finley <cfinley@icecube.wisc.edu>
 * @author Jon Dumm <jdumm@icecube.wisc.edu>
 * @author Timo Griesel <timo.griesel@uni-mainz.de>
 */

// std lib stuff
#include <string>

// gulliver stuff
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3SeedServiceBase.h"
#include "lilliput/parametrization/I3SimpleParametrization.h"

// icetray stuff
#include "icetray/I3ConditionalModule.h"
#include "icetray/IcetrayFwd.h"
#include "dataclasses/physics/I3Particle.h"

// details of implementation and output object
#include "paraboloid/ParaboloidImpl.h"
#include "paraboloid/I3ParaboloidFitParams.h"

/**
 * @class I3ParaboloidFitter
 * @brief Evaluate the likelihood on a zenith-azimuth grid near a given track, fit a paraboloid
 *
 * The direction given input track is transformed such that it's on
 * the equator of the direction sphere and the zenith and azimuth are
 * almost Cartesian.  For each direction in a grid of directions close to
 * the input directions, the vertex of the track is refit (reoptimized),
 * preferrably using the same minimizer and likelihood function as used
 * to obtain the input fit. The vertex-optimized likelihood values of the
 * grid points define a "2D likelihood landscape" that can be approximated
 * with a paraboloid surface. The minimum of this paraboloid can be
 * used as a (slightly) improved fit, while the geometrical properties
 * of this paraboloid can be used to define quality cuts.  For example
 * "error ellips" can defined as the curve in zenith-azimuth space on
 * which the (reoptimized) log-likelihood is 0.5 higher than that of
 * the paraboloid minimum.
 *
 * Paraboloid is a Gulliver module, so it uses likelihood functions,
 * minimizers, parametrizations and seed services. Since we do not
 * expect/want the vertex at the grid points to be dramatically different
 * from the vertex of the input track, it's important to configure the
 * parametrization with a small stepsize (default 5m).
 *
 * (A seed service can do seed tweaking before a fit, e.g. setting the
 * vertex time such that the time residuals of all hits are positive. It
 * can do that both with an external first guess event hypotheses and
 * with an event hypothesis generated by the calling code, the module.)
 *
 * You want minimal or no tweaking of the input fit (that fit should
 * already be very good), but for the grid points tweaking of the vertex
 * time is useful. For this reason paraboloid can be configured to use
 * the two different seed services: one to provide the input track,
 * and one to doctor the grid seeds.
 *
 * When using the results of Paraboloid is always very important to
 * use the paraboloid status. In case of a (partial) failure the track
 * variables are not necessarily set to NAN. If you only want to use the
 * results with status 0 (success). The non-zero value of the paraboloid
 * status tells you exactly what went wrong; this can help you to improve
 * the configuration of the fitter modules or to diagnose problems.
 *
 * The original ideas and implementation for the paraboloid fitter were by
 * Till Neunhoeffer.
 *
 * @sa http://icecube.berkeley.edu/manuscripts/20040301xx-tn_phd.pdf (Till's PhD thesis, in German)
 * @sa Astroparticle Physics 25 (2006) 220-225, April 2006:
 *     Estimating the Angular Resolution of Tracks in Neutrino Telescopes
 *     Based on a Likelihood Analysis
 */
class I3ParaboloidFitter : public I3ConditionalModule {

public:

    /// constructor for I3Tray::AddModule (defines configuration parameters)
    I3ParaboloidFitter(const I3Context& ctx);

    /// destructor
    ~I3ParaboloidFitter();

    /// configure (retrieves and checks configuration parameters, initializes the module)
    void Configure();

    /// get the geoemtry from the frame and pass it to the LLH
    void Geometry(I3FramePtr frame);

    /// do the paraboloid fit for a particular physics event
    void Physics(I3FramePtr frame);

private:

    // inhibit unwanted constructors and operator
    I3ParaboloidFitter(); /// inhibited default constructor
    I3ParaboloidFitter(const I3ParaboloidFitter& source); /// inhibited copy constructor
    I3ParaboloidFitter& operator=(const I3ParaboloidFitter& source); /// inhibited assignment operator

    /**
     * the core Gulliver object for basic tracks
     * (a pointer, so that we can postpone object creation to Configure())
     */
    I3GulliverPtr fitterCore_;

    /// seed service for input track
    I3SeedServiceBasePtr seedService_;

    /// seed service for tweaking the grid point seed
    I3SeedServiceBasePtr gridSeedService_;

    /// event likelihood service (should be same as the one used for the input fit)
    I3EventLogLikelihoodBasePtr eventLLH_;
    I3MinimizerBasePtr minimizer_;

    // configurables
    std::string mcName_;              /// name of minimizer service
    int nMaxMissingGridPoints_;       /// max number of grid points for which vertex refit may fail, before we give up
    double vertexStepSize_;           /// initial stepsize for vertex refit (not too big)
    double azimuthReach_;             /// grid scale for azimuth 
    double zenithReach_;              /// grid scale for zenith 
    unsigned int nSteps_;             /// number of grid rings around input direction
    unsigned int nSamplingPoints_;    /// number of grid points on each grid ring

    /// NDF: compute once, use anywhere during physics event
    int ndf_;

    /// event counter, for log_messages
    unsigned int eventNr_;
    unsigned int zollCounter_;

    /// name of minimizer service
    std::string fitName_;

    /**
     * Vertex parametrization
     * In contrast to most other likelihood fitters, paraboloid does
     * not fit arbitrary parameters. For each grid point, the vertex
     * position is reoptimized. In a later stage we might want to rethink
     * this. Maybe in some cases also the energy needs to be reoptimized,
     * for example.
     */
    I3SimpleParametrizationPtr vertexParametrization_;

    /// object generating the grid of directions
    ParaboloidImpl::GridStar grid_;

    /// object containing paraboloid fit details
    ParaboloidImpl::ParabolaFit paraFit_;

    /// object containing paraboloid fit details
    ParaboloidImpl::ParabolaTypePol_2 paraPol_;

    /// object containing paraboloid fit details
    ParaboloidImpl::ParabolaTypeStd_2 paraStd_;

    /// "chi squared" (likelihood)
    double chi2_;

    /// likelihood of input fit (after vertex refit)
    double seedLlh_;

    /// generate grid of directions
    int GetParaboloidDatapoints( const I3Frame& f, const I3EventHypothesis &hypothesis);

    /// store direction error estimate
    int GetErrorsFromCurvature( I3ParaboloidFitParams &parabParam);

    /// store other results
    int StoreResults( I3ParaboloidFitParams &parabParam);

    /// compare with MC track (if given)
    int GetTruth( I3ParaboloidFitParams &parabParam, I3FramePtr f);

    /// define own fit with the direction from the minimum of the parabola, and a refitted vertex
    int GetLogLHFitParams( const I3Frame &f, I3LogLikelihoodFitPtr fitptr);

    /// helper method to check if a particle traverses a given depth range
    bool IsInDepthRange( const I3Particle &p, double zmin, double zmax );

    /// helper method to check NDF 
    bool NDFOK( const I3Frame &f, I3EventHypothesis &h);

    /// macro, setting the "MyClass" name for log4cplus.conf
    SET_LOGGER( "I3ParaboloidFitter" );
  
};  // end of the class definition.


#endif /* I3PARABOLOIDFITTER_H_INCLUDED */
