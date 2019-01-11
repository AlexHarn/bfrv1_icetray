#ifndef I3LAPUTOPSEEDSERVICE_H_INCLUDED
#define I3LAPUTOPSEEDSERVICE_H_INCLUDED

/**
 *
 * @file I3LaputopSeedService.h
 * @brief declaration of the I3LaputopSeedService class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
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
 * @class I3LaputopSeedService
 * @brief Service to seed reconstructions with an hypothesis consisting of one particle and additional parameters (in particular: S125, beta) stored in an I3LaputopParams.
 *
 */
class I3LaputopSeedService : public I3ServiceBase,
                                public I3SeedServiceBase {
public:

    /// default constructor for unit tests
    I3LaputopSeedService( std::string name,
			  std::string InPlaneName,
			  std::string InCoreName,
			  std::string ParamsName,
			  double seedBeta,
			  std::string pulses
			  );

    virtual ~I3LaputopSeedService(){}

    /// constructor I3Tray
    I3LaputopSeedService(const I3Context &c);

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

    static const std::string DEFAULT_INPLANENAME;
    static const std::string DEFAULT_INCORENAME;
    static const double DEFAULT_BETA;
    static const std::string DEFAULT_PULSENAME;
    static const std::string INPLANENAME_TAG;
    static const std::string INCORENAME_TAG;
    static const std::string INPARAMSNAME_TAG;
    static const std::string BETA_TAG;
    static const std::string PULSENAME_TAG;
    static const std::string INPLANENAME_DESCRIPTION;
    static const std::string INCORENAME_DESCRIPTION;
    static const std::string INPARAMSNAME_DESCRIPTION;
    static const std::string BETA_DESCRIPTION;
    static const std::string PULSENAME_DESCRIPTION;

    // This seedservice will figure this one out by itself (later)
    static const double DEFAULT_S125;
    
    // A, D, and N: (as is done in I3CurvatureSeedService)
    static const std::string SEED_A_TAG;
    static const std::string SEED_D_TAG;
    static const std::string SEED_N_TAG;
    static const std::string SEED_A_DESC;
    static const std::string SEED_D_DESC;
    static const std::string SEED_N_DESC;

    
    /// convenience method to guarantee some finite positive energy
    //    static I3ParticlePtr CheckEnergy( I3ParticlePtr p, double safe_energy=1.0*I3Units::TeV );
    
    std::string fInPlaneName_;
    std::string fInCoreName_;
    std::string fInParamsName_;
    I3ParticlePtr seedParticle_;

    double seedS125_;
    double seedBeta_;
    I3LaputopParamsPtr ldfParams_;

    // These will help us decide whether to use default seeds, or something
    // explicitly set by the user.
    bool override_beta_;

    // A, D, and N: (as is done in I3CurvatureSeedService)
    double fSeedA_;
    double fSeedD_;
    double fSeedN_;

    std::string fDataReadoutLabel_;

    //Seeding method from TTopRecoShower :
    double seedLogS125(I3RecoPulseSeriesMapConstPtr rpsm, 
		       I3ParticleConstPtr seedPart, const I3OMGeoMap &om_map);


    /// log4cplus thingy
    SET_LOGGER( "Laputop" );

};

#endif
