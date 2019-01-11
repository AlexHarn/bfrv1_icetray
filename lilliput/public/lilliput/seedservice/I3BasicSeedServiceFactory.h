#ifndef I3BASICSEEDSERVICEFACTORY_H_INCLUDED
#define I3BASICSEEDSERVICEFACTORY_H_INCLUDED

/*
 * class: I3BasicSeedServiceFactory
 *
 * Version $Id$
 *
 * Date: Fri Jan 20 21:40:47 CST 2006
 *
 * (c) IceCube Collaboration
 */

// standard library
#include <string>

// forward declarations
#include "icetray/IcetrayFwd.h"

// base classes
#include "icetray/I3ServiceFactory.h"
#include "gulliver/I3SeedServiceBase.h"
#include "lilliput/seedservice/I3BasicSeedService.h"

/**
 * @class I3BasicSeedServiceFactory
 * @brief This class installs a I3BasicSeedService.
 *
 * You can configure the I3BasicSeedService via eight parameters:
 * - <VAR>FirstGuess</VAR> name first guess track/shower (I3Particle or I3Vector<I3Particle>)
 * - <VAR>FirstGuesses</VAR> list of names of first guess tracks/showers (I3Particle or I3Vector<I3Particle>)
 * - <VAR>InputReadout</VAR> name of hit/pulse series map
 * - <VAR>FixedEnergy</VAR> energy value to use in seed (overrides FG energy)
 * - <VAR>NChEnergyGuessPolynomial</VAR> smarter energy guess (overrides FG energy),
 *        specify coefficients @c [c0,c1,c2...] of polynomial
 *        log10(E/GeV) = c0 + c1 * x + c2 *x *x + ... with x=log10(NCh)
 * - <VAR>TimeShiftType</VAR> how to compute the vertex time
 * - <VAR>AltTimeShiftType</VAR> how to compute the vertex time for the "alternative" seeds
 * - <VAR>SpeedPolice</VAR> flag, whether or not to enforce speed=1 for tracks and speed=0 for cascades.
 * - <VAR>MaxMeanTimeResidual</VAR> bound on time shift when using timeshift type TFirst
 * - <VAR>AddAlternatives</VAR> Add simple alternative seeds for each first guess; argument is a
 *        a string; possibilities: "None" (default, no alternatives), "Reverse" (add a track
 *        with the same vertex in the opposite direction), "Cubic" (add 5 more tracks: the
 *        reverse track and four perpendicular tracks).
 *
 * The names of first guesses can refer to either single (I3Particle
 * object in the frame) or multiple results (I3Vector<I3Particle>),
 * both are accepted. By default no energy guess/fix is performed.
 * In case of an NCh-based energy guess you need also to provide a
 * hit/pulse seriesmap (NCh will be set to the size of that map).
 *
 * If you specify a hit/pulse seriesmap, then the vertex of each seed
 * with type "InfiniteTrack" will always be shifted to the point closest to
 * the COG of the hits/pulses, and the time will be adjusted correspondingly.
 *
 * Timeshifts are intended to improve the vertex time. If the input track
 * already comes from a log-likelihood reconstruction, then you probably want
 * to set it to "TNone", no correction. If you set it to "TMean"/"TFirst" then
 * the vertex time is adjusted such that the mean/smallest time residual is zero.
 * For AMANDA analyses in the past TMean was most common (its the default now),
 * for IceCube data it seems that TFirst works better. In order to protect the
 * TFirst shift against anomalous very early hits, there is a
 * maximum on the mean of time residual, which can be configured with
 * MaxMeanTimeResidual.
 *
 * @version $Id$
 * @author boersma
 */
class I3BasicSeedServiceFactory : public I3ServiceFactory {
public:

    /// constructors
    I3BasicSeedServiceFactory(const I3Context& context);

    /// destructor
    virtual ~I3BasicSeedServiceFactory();

    /**
     * Installed this objects service into the specified services object.
     *
     * @param ctx the I3Context into which the service should be installed.
     * @return true if the service is successfully installed.
     */
    virtual bool InstallService(I3Context& ctx);

    /**
     * Configure service prior to installing it.
     */
    virtual void Configure();

   private:

    // stop defaults
    I3BasicSeedServiceFactory( const I3BasicSeedServiceFactory& rhs); /// no copy constructor
    I3BasicSeedServiceFactory operator= (const I3BasicSeedServiceFactory& rhs); /// no assignment operator

    // option values
    std::string fgName_; /// single first guess name (old option)
    std::vector<std::string> fgNames_; /// list of first guess names (new option)
    std::string inputReadout_; /// hits, pulses needed for vertex correction

    // instance data
    std::string name_; /// name of this service
    unsigned int nContext_; /// number contexts into which a service pointer was installed
    I3SeedServiceBasePtr seedService_; /// pointer to the service object
    std::vector<double> energyGuessPolynomial_; /// coefficients of E guess polynomial (if given: overrides FG energy)
    double fixedEnergy_; /// energy to use (if not NAN: overrides FG energy)
    std::string posShiftTypeName_; /// name of position shift type
    std::string timeShiftTypeName_; /// name of time shift type
    std::string altTimeShiftTypeName_; /// name of time shift type for the alternative seeds
    I3BasicSeedService::I3PositionShiftType posShiftType_; /// actual position shift type
    I3BasicSeedService::I3TimeShiftType timeShiftType_; /// actual time shift type
    I3BasicSeedService::I3TimeShiftType altTimeShiftType_; /// actual time shift type for alternative seeds
    bool speedPolice_;   /// whether or not to force the speed of inf tracks to be c
    double maxTResMean_; /// max value of average time residual (protection against very early hits in case of TFirst shift type
    double chargeFraction_; /// charge fraction, for vertex time correction strategy based on tres-ordered hit charges
    std::string addAlternativesString_; /// name of recipe for generating more than one seed per first guess
    I3BasicSeedService::I3SeedAlternatives addAlternatives_; /// number of the recipe for generating more than one seed per first guess
    bool onlyAlternatives_; /// do not use the seed based on the input track at all

    SET_LOGGER("I3BasicSeedServiceFactory");
};

#endif /* I3BASICSEEDSERVICEFACTORY_H_INCLUDED */
