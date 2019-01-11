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
#ifndef I3BASICSEEDSERVICE_H_INCLUDED
#define I3BASICSEEDSERVICE_H_INCLUDED

// standard stuff
#include <vector>
#include <string>
#include "boost/shared_ptr.hpp"


// icetray stuff
#include "icetray/IcetrayFwd.h"
#include "icetray/I3DefaultName.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "gulliver/I3SeedServiceBase.h"

class I3Geometry;
class OMKey;

/**
 * @class I3BasicSeedService
 * @brief Base class seed track collection & preparation services
 *
 * Basic seed services collects first guess tracks and can do a
 * simple vertex correction.  It needs recopulses to do this vertex
 * corrections. Only useful for pure I3Particle seeds, any non-standard
 * topologies won't work with this.
 *
 * @sa I3Gulliver
 * @sa I3GulliverBase
 * @sa I3SeedServiceBase
 * @sa I3EventHypothesis
 * @sa I3BasicSeedServiceFactory
 */
class I3BasicSeedService : public I3SeedServiceBase {

public:
    /**
     * @enum I3TimeShiftType
     * @brief enumerate the different ways to fix the vertex time (of an infinite track)
     * @sa I3BasicSeedServiceFactory
     */
    enum I3TimeShiftType {
        TUndefined=0,
        TNone, /// keep vertex time from first guess
        TMean, /// set vertex time such that the mean time residual is zero
        TFirst,/// set vertex time such that the smallest time residual is zero
        TChargeFraction, /// set vertex time such that a fixed fraction (e.g. 90%) of the charge has positive time residual
        TDirectChargeFraction, /// same, using only first hits/pulses
    };

#define I3BASICSEEDSERVICE_I3TIMESHIFTTYPE_SEQ \
    (TUndefined) \
    (TNone) \
    (TMean) \
    (TFirst) \
    (TChargeFraction) \
    (TDirectChargeFraction)

    /**
     * @enum I3PositionShiftType
     * @brief enumerate the different ways to fix the vertex position (of an infinite track)
     * @sa I3BasicSeedServiceFactory
     */
    enum I3PositionShiftType {
        PUndefined=0,
        PNone, /// keep vertex position from first guess
        PCOG, /// shift the vertex position along the track to the point closest to the COG
    };

#define I3BASICSEEDSERVICE_I3POSITIONSHIFTTYPE_SEQ \
    (PUndefined) \
    (PNone) \
    (PCOG)

    /**
     * The basic behavior of the seed service is to generate only one seed
     * per first guess. We could however also add more seeds, with some
     * trivial transformations. This enum serves to enumerate various ways
     * to generate 1 or more extra seeds.
     */
    enum I3SeedAlternatives {
        SeedAlt_None=0, /// no extra seeds
        SeedAlt_Reverse, /// for each first guess, add 1 extra seed, with opposite direction
        SeedAlt_Tetrahedron, /// add  3 first guesses, with tetrahedric angles
        SeedAlt_Cube, /// add 5 first guesses, one with opposite direction and 4 perpendicular
    };

#define I3BASICSEEDSERVICE_I3SEEDALTERNATIVES_SEQ \
    (SeedAlt_None) \
    (SeedAlt_Reverse) \
    (SeedAlt_Tetrahedron) \
    (SeedAlt_Cube)

    /**
     * @param[in] name name of seed service factory, for log messages
     * @param[in] fgnames Labels of first guess fit results
     * @param[in] inputreadout Name of I3RecoPulseMap (for vertex correction)
     * @param[in] fixedE fixed energy value (overrides FG energy)
     * @param[in] eguesspar coefficients of polynomial for E guess (overrides FG energy)
     * @param[in] pstype position shift type
     * @param[in] tstype time shift type
     * @param[in] alt_tstype 
     * @param[in] speedpolice flag to enable/disable 
                            enforcement of v=c for infinite tracks
     * @param[in] tresmeanmax max mean time residual 
                           (protection with TFirst time shift type)
     * @param[in] frac charge fraction value for "TChargeFraction" vertex time correction
     * @param[in] altMode specifies if alternative seeds 
                            should be created, with which algorithm
     * @param[in] onlyAlt specifies if nonalternative seeds 
                          (based directly on input fg) should be skipped
     */
    I3BasicSeedService( const std::string &name,
                        const std::vector<std::string> &fgnames,
                        const std::string &inputreadout,
                        double fixedE,
                        const std::vector<double> &eguesspar,
                        I3PositionShiftType pstype,
                        I3TimeShiftType tstype,
                        I3TimeShiftType alt_tstype,
                        bool speedpolice,
                        double tresmeanmax,
                        double frac,
                        I3SeedAlternatives altMode,
                        bool onlyAlt );

    /// base class destructor (idle)
    virtual ~I3BasicSeedService();

    /**
     * provide event data
     * - purge old seed tracks
     * - get the first guess tracks, prepare new seeds
     * - do time/space corrections (if wanted)
     * @param[in] f Frame with event data
     * @returns number of available seeds
     */
    virtual unsigned int SetEvent( const I3Frame &f );

    /// get a seed
    virtual I3EventHypothesis GetSeed( unsigned int iseed ) const;

    /// get a dummy seed (useful in case of fg failure)
    virtual I3EventHypothesis GetDummy( ) const;

    /**
     * Space and time coordinates of the vertex are tweaked, for numerical
     * convenience (minimumizer algorithms). If no hits/pulses are configured,
     * then no corrections are applied. Vertex time is corrected such that
     * the timeresiduals are either positive or centered around 0, depending
     * on configuration flags; xyz is chosen such that it is closest to the
     * COG of the hits.
     */
    virtual void Tweak( I3EventHypothesis &eh ) const;

    /**
     * If "FixedEnergy" or "NChEnergyGuessPolynomial" is configured for
     * this service, then this service will use that energy instead
     * of the FG energy.
     */
    void FillInTheBlanks( I3EventHypothesis &eh ) const;

    /**
     * This will just make a (deep) copy of the I3Particle datamember of the
     * I3EventHypothesis. The "nonstd" datamember is not copied.
     */
    virtual I3EventHypothesis GetCopy( const I3EventHypothesis &eh ) const;

    /**
     * @brief Make a new first guess with direction opposite to input track
     *
     * This static method was made public method purely for unit test
     * convenience. If other people would find it useful in other
     * classes, then maybe it would better move to some utility namespace,
     * e.g. I3Calculator.
     *
     * @param[in] p input particle
     * @returns a vector with just 1 new particle (pointer) with opposite direction
     */
    static std::vector<I3ParticlePtr> Reverse(I3ParticleConstPtr p);

    /**
     * @brief Make three new first guesses with directions tetrahedric
     *        w.r.t. input track (i.e. the original direction and the three
     *        new directions are arranged like the points of a tetrahedron).
     *
     * This static method was made public method purely for unit test
     * convenience. If other people would find it useful in other
     * classes, then maybe it would better move to some utility namespace
     * e.g. I3Calculator.
     *
     * @param[in] p input particle
     * @returns a vector with 3 new particles (pointers)
     */
    static std::vector<I3ParticlePtr> Tetrahedron(I3ParticleConstPtr p);

    /**
     * @brief Make five new first guesses with directions "cubic"
     *        w.r.t. input track (1 opposite + 4 perpendicular)
     *
     * You should think of the *faces* of the cube, rather than its
     * points, otherwise this method should have been called
     * "Octahedron". Are you confused yet?
     *
     * This static method was made public method purely for unit test
     * convenience. If other people would find it useful in other
     * classes, then maybe it would better move to some utility namespace,
     * e.g. I3Calculator.
     *
     * @param[in] p input particle
     * @returns a vector with 5 new particles (pointers)
     */
    static std::vector<I3ParticlePtr> Cube(I3ParticleConstPtr p);

private:

    /// basic tests: OK fit status, non-NAN datamembers
    bool SeedOK( const I3Particle &seed ) const;

    /// get FG tracks from the frame, using the configured track labels
    void GetFirstGuesses( const I3Frame &f );

    /// shift the vertex position as close as possible to the COG of the hits (only for inf. tracks)
    void ShiftVertex( I3Particle &p ) const;

    /// adjust the vertex time (such that timeresiduals are more or less OK)
    void ShiftTime( I3Particle &p, const I3RecoPulseSeriesMap& hitmap ) const;

    /// guess energy with a polynomial logE = p0+p1*x+p2*x*x+... with x=log(NCh)
    double PolynomialEnergyGuess() const;

    /// extend seed set with reverse/tetrahedric/cubic alternatives (if wanted)
    void GetAlternatives();

    // configurables
    std::string name_;
    std::vector<std::string> firstGuessNames_;
    std::string inputReadout_;
    std::vector<I3EventHypothesis> seeds_;
    I3EventHypothesis dummy_;
    std::vector<double> energyGuessPolynomial_;
    double fixedEnergy_;
    I3PositionShiftType posShiftType_;
    I3TimeShiftType timeShiftType_;
    I3TimeShiftType altTimeShiftType_;
    bool speedPolice_;
    double maxTResMean_;
    double chargeFraction_;
    I3SeedAlternatives altMode_;
    bool onlyAlternatives_;

    // counters
    unsigned int nMissingFG_; /// number of events without FG
    unsigned int nBadFG_; /// number of bad first guesses (fit status not OK), can be larger than nSetEvent_ in case of multiple FG per event
    unsigned int nReadoutMissing_; /// number of events without pulses (while needed)
    unsigned int nSetEvent_; /// number of events

    // cache
    I3Position cog_;
    I3GeometryConstPtr geometry_;
    I3RecoPulseSeriesMapConstPtr pulses_;

    SET_LOGGER( "I3BasicSeedService" );
};

I3_POINTER_TYPEDEFS( I3BasicSeedService );
I3_DEFAULT_NAME( I3BasicSeedService );

#endif /* GULLIVER_I3SEEDSERVICE_H_INCLUDED */
