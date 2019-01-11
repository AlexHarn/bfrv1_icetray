/**
 *
 * @file I3DoubleMuonSeedService.cxx
 * @brief implementaration of the I3DoubleMuonSeedService class
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
#include <cassert>
#include <cmath>
#include <icetray//I3SingleServiceFactory.h>
#include "double-muon/I3DoubleMuonSeedService.h"

/*
 * I3DoubleMuonSeedService
 * class for double muon reconstruction: seeding a coincident muon event
 */

static const std::string seed1_optionname = "Seed1";
static const std::string seed2_optionname = "Seed2";

/// default constructor for unit tests
I3DoubleMuonSeedService::I3DoubleMuonSeedService(
  std::string name,
  I3SeedServiceBasePtr mu1,
  I3SeedServiceBasePtr mu2 ) :
  I3ServiceBase(name), I3SeedServiceBase(),
  trackSeeder1_(mu1), trackSeeder2_(mu2){

    if ( ! (trackSeeder1_ && trackSeeder2_) ){
        log_fatal( "(%s) both input seeders should be non-NULL",
                   GetName().c_str() );
    }

}


/// constructor I3Tray
I3DoubleMuonSeedService::I3DoubleMuonSeedService(const I3Context &c):
  I3ServiceBase(c), I3SeedServiceBase(){

    AddParameter( seed1_optionname,
                  "Seed service providing seed for first track.",
                  trackSeeder1Name_ );
    AddParameter( seed2_optionname,
                  "Seed service providing seed for second track.",
                  trackSeeder2Name_ );

}

// set parameters (in I3Tray)
void
I3DoubleMuonSeedService::Configure(){
    GetParameter( seed1_optionname,
                  trackSeeder1Name_ );
    GetParameter( seed2_optionname,
                  trackSeeder2Name_ );
    if ( trackSeeder1Name_.empty() ){
        log_fatal( "(%s) didn't get a track seeder for first muon...",
                   GetName().c_str() );
    }
    if ( trackSeeder2Name_.empty() ){
        log_fatal( "(%s) didn't get a track seeder for second muon...",
                   GetName().c_str() );
    }
    trackSeeder1_ = context_.Get<I3SeedServiceBasePtr>( trackSeeder1Name_ );
    trackSeeder2_ = context_.Get<I3SeedServiceBasePtr>( trackSeeder2Name_ );
    if ( ! trackSeeder1_ ){
        log_fatal( "(%s) couldn't find a track seeder named %s",
                   GetName().c_str(), trackSeeder1Name_.c_str()  );
    }
    if ( ! trackSeeder2_ ){
        log_fatal( "(%s) couldn't find a track seeder named %s",
                   GetName().c_str(), trackSeeder2Name_.c_str()  );
    }
}

/*
 * provide event data
 * - purge old seed
 * - get the track seed, add decay time
 * @param[in] f Frame with event data
 * @returns number of available seeds
 */
unsigned int
I3DoubleMuonSeedService::SetEvent( const I3Frame &f ){
    nSeeds1_ = trackSeeder1_->SetEvent(f);
    nSeeds2_ = trackSeeder2_->SetEvent(f);
    return nSeeds1_*nSeeds2_;
}

I3ParticlePtr I3DoubleMuonSeedService::CheckEnergy( I3ParticlePtr p, double safe_energy ){
    if ( ! (std::isfinite(safe_energy) && (safe_energy>0) ) ){
        log_fatal("got bad fall-back energy %g "
                  "(should be positive and finite)",
                   safe_energy );
    }
    double energy = p->GetEnergy();
    if ( std::isfinite(energy) && (energy>0) ) return p;
    p->SetEnergy(safe_energy);
    return p;
}

// get a seed
I3EventHypothesis
I3DoubleMuonSeedService::GetSeed( unsigned int iseed ) const {
    if (iseed>=nSeeds1_*nSeeds2_){
        log_fatal("(%s) seed nr. %u requested, only %u available",
                   GetName().c_str(), iseed, nSeeds1_*nSeeds2_ );
    }
    unsigned int iseed1 = iseed % nSeeds1_;
    unsigned int iseed2 = iseed / nSeeds1_;

    I3EventHypothesis eh(
            CheckEnergy(trackSeeder1_->GetSeed( iseed1 ).particle),
            CheckEnergy(trackSeeder2_->GetSeed( iseed2 ).particle) );
    return eh;
}

// get a dummy seed (useful in case of fg failure)
I3EventHypothesis
I3DoubleMuonSeedService::GetDummy( ) const {
    I3EventHypothesis eh(
            CheckEnergy( trackSeeder1_->GetDummy( ).particle ),
            CheckEnergy( trackSeeder2_->GetDummy( ).particle ) );
    return eh;
}

/*
 * Space and time coordinates of the vertex are tweaked, for numerical
 * convenience (minimumizer algorithms).
 * This is actually delegated to the external track seeders.
 */
void
I3DoubleMuonSeedService::Tweak( I3EventHypothesis &eh ) const {
    I3EventHypothesis eh1( eh.particle, I3FrameObjectPtr() );
    I3EventHypothesis eh2(
            boost::dynamic_pointer_cast<I3Particle>(eh.nonstd), I3FrameObjectPtr() );
    if ( ! eh2.particle ){
        log_fatal( "(%s) wrong type or NULL for nonstd part of hypothesis",
                   GetName().c_str() );
    }
    trackSeeder1_->Tweak( eh1 );
    trackSeeder2_->Tweak( eh2 );
}

/*
 * Filling the blanks in the track is delegated to the external track seeders.
 */
void
I3DoubleMuonSeedService::FillInTheBlanks( I3EventHypothesis &eh ) const {
    I3EventHypothesis eh1( eh.particle, I3FrameObjectPtr() );
    I3EventHypothesis eh2(
            boost::dynamic_pointer_cast<I3Particle>(eh.nonstd), I3FrameObjectPtr() );
    if ( ! eh2.particle ){
        log_fatal( "(%s) wrong type or NULL for nonstd part of hypothesis",
                   GetName().c_str() );
    }
    trackSeeder1_->Tweak( eh1 );
    trackSeeder2_->Tweak( eh2 );
    trackSeeder1_->FillInTheBlanks( eh1 );
    trackSeeder2_->FillInTheBlanks( eh2 );
    CheckEnergy( eh1.particle );
    CheckEnergy( eh2.particle );
}

/*
 * This will make a (deep) copy of the I3EventHypothesis.
 */
I3EventHypothesis
I3DoubleMuonSeedService::GetCopy( const I3EventHypothesis &eh ) const {
    I3EventHypothesis eh1( eh.particle, I3FrameObjectPtr() );
    I3EventHypothesis eh2(
            boost::dynamic_pointer_cast<I3Particle>(eh.nonstd), I3FrameObjectPtr() );
    if ( ! eh2.particle ){
        log_fatal( "(%s) wrong type or NULL for nonstd part of hypothesis",
                   GetName().c_str() );
    }
    I3EventHypothesis ehcopy(
            trackSeeder1_->GetCopy( eh1 ).particle,
            trackSeeder2_->GetCopy( eh2 ).particle );
    return ehcopy;
}

typedef I3SingleServiceFactory< I3DoubleMuonSeedService,I3SeedServiceBase >
I3DoubleMuonSeedServiceFactory;
I3_SERVICE_FACTORY( I3DoubleMuonSeedServiceFactory )
