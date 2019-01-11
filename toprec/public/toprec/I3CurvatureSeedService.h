#ifndef I3CURVATURESEEDSERVICE_H_INCLUDED
#define I3CURVATURESEEDSERVICE_H_INCLUDED

/**
 *
 * @file I3CurvatureSeedService.h
 * @brief declaration of the I3CurvatureSeedService class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id: I3CurvatureSeedService.h 142579 2016-02-26 23:33:46Z kath $
 *
 * @version $Revision: 142579 $
 * @date $Date: 2016-02-26 17:33:46 -0600 (Fri, 26 Feb 2016) $
 * @author kath
 *
 */

#include <string>
#include "icetray/I3Units.h"
#include "icetray/I3ServiceBase.h"
#include "gulliver/I3SeedServiceBase.h"
#include <recclasses/I3LaputopParams.h>
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
/**
 * @class I3CurvatureSeedService
 * @brief Service to seed the simple CurvatureFitter
 *
 */
class I3CurvatureSeedService : public I3ServiceBase,
                                public I3SeedServiceBase {
public:

    /// default constructor for unit tests
  I3CurvatureSeedService( std::string name,
			  double seedA
			  );

    virtual ~I3CurvatureSeedService(){}

    /// constructor I3Tray
    I3CurvatureSeedService(const I3Context &c);

    /// set parameters (in I3Tray)
    void Configure();

    /**
     * Get seeds from external single muon seeders, and creates the combined seed particle.
     *
     * @param[in] f Frame with event data
     * @returns number of available seeds
     */
    virtual unsigned int SetEvent( const I3Frame &f );

    /// get a seed
    virtual I3EventHypothesis GetSeed( unsigned int iseed ) const;

    /// get a dummy seed (useful in case of first guess failure)
    virtual I3EventHypothesis GetDummy( ) const;

    /**
     * Space and time coordinates of the vertices are tweaked, for numerical
     * convenience (minimizer algorithms).
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
        return I3ServiceBase::GetName();
    }

private:

    // This one is super-simple!

    // A, D, and N:
    static const std::string SEED_A_TAG;
    static const std::string SEED_D_TAG;
    static const std::string SEED_N_TAG;
    static const std::string SEED_T_TAG;
    static const std::string SEED_A_DESC;
    static const std::string SEED_D_DESC;
    static const std::string SEED_N_DESC;
    static const std::string SEED_T_DESC;

    // The track:
    static const std::string SEEDTRACK_TAG;
    static const std::string SEEDTRACK_DESC;
    static const std::string DEFAULT_SEEDTRACK;
    
    double fSeedA_;
    double fSeedD_;
    double fSeedN_;
    double fSeedT_;
    std::string fInTrackName_;
    I3ParticlePtr seedParticle_;
    I3LaputopParamsPtr curvParams_;

    /// log4cplus thingy
    SET_LOGGER( "Curvature" );

};

#endif
