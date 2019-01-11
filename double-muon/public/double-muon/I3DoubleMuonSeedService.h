#ifndef I3DOUBLEMUONSEEDSERVICE_H_INCLUDED
#define I3DOUBLEMUONSEEDSERVICE_H_INCLUDED

/**
 *
 * @file I3DoubleMuonSeedService.h
 * @brief declaration of the I3DoubleMuonSeedService class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#include <string>
#include "icetray/I3Units.h"
#include "icetray/I3ServiceBase.h"
#include "gulliver/I3SeedServiceBase.h"

/**
 * @class I3DoubleMuonSeedService
 * @brief Service to seed reconstructions with an hypothesis consisting of two I3Particles
 *
 * This service has been programmed in a very lazy way: it uses two other seed
 * services to provide regular I3Particle seeds (typically an
 * I3BasicSeedService using a linefit first guess) and then puts one of them in
 * the "particle" datamember of an I3EventHypothesis and the other into the
 * "nonstd" datamember.
 *
 * If the I3Particle seed providers deliver more than one seed, then all
 * possible combinations of seeds are made, except for interchanging two. So if
 * one seed service has 2 and the other 3, then this service delivers 2x3=6
 * difference seeds.
 */
class I3DoubleMuonSeedService : public I3ServiceBase,
                                public I3SeedServiceBase {
public:

    /// default constructor for unit tests
    I3DoubleMuonSeedService( std::string name,
                             I3SeedServiceBasePtr mu1,
                             I3SeedServiceBasePtr mu2 );

    virtual ~I3DoubleMuonSeedService(){}

    /// constructor I3Tray
    I3DoubleMuonSeedService(const I3Context &c);

    /// set parameters (in I3Tray)
    void Configure();

    /**
     * Get seeds from external single muon seeders, then make .
     *
     * @param[in] f Frame with event data
     * @returns number of available seeds
     */
    virtual unsigned int SetEvent( const I3Frame &f );

    /// get a seed
    virtual I3EventHypothesis GetSeed( unsigned int iseed ) const;

    /// get a dummy seed (useful in case of fg failure)
    virtual I3EventHypothesis GetDummy( ) const;

    /**
     * Space and time coordinates of the vertices are tweaked, for numerical
     * convenience (minimumizer algorithms).
     * This is actually delegated to the external track seeders.
     */
    virtual void Tweak( I3EventHypothesis &eh ) const;

    /**
     * Filling the blanks in the track is delegated to the external track
     * seeders.
     */
    void FillInTheBlanks( I3EventHypothesis &eh ) const;

    /**
     * This will make a (deep) copy of the I3EventHypothesis.
     */
    virtual I3EventHypothesis GetCopy( const I3EventHypothesis &eh ) const;

    /// tell your name
    const std::string GetName() const {
        return this->I3ServiceBase::GetName();
    }

private:

    /// convenience method to guarantee some finite positive energy
    static I3ParticlePtr CheckEnergy( I3ParticlePtr p, double safe_energy=1.0*I3Units::TeV );

    /// name of first external track seeder
    std::string trackSeeder1Name_;

    /// name of second external track seeder
    std::string trackSeeder2Name_;

    /// pointer to external track seeder for first muon
    I3SeedServiceBasePtr trackSeeder1_;

    /// pointer to external track seeder for second muon
    I3SeedServiceBasePtr trackSeeder2_;

    /// number of seeds available from trackSeeder1_
    unsigned int nSeeds1_;

    /// number of seeds available from trackSeeder2_
    unsigned int nSeeds2_;

    /// log4cplus thingy
    SET_LOGGER( "I3DoubleMuonSeedService" );

};

#endif
