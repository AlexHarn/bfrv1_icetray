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

// std library stuff
#include <cmath>
#include <vector>
#include <string>
#include <list>

// icetray/gulliver stuff
#include "lilliput/seedservice/I3BasicSeedService.h"
#include "icetray/I3Frame.h"
#include "icetray/I3FrameObject.h"
#include "icetray/I3Units.h"
#include "phys-services/I3Cuts.h"
#include "phys-services/I3Calculator.h"

I3BasicSeedService::I3BasicSeedService( const std::string &name,
                                        const std::vector<std::string> &fgnames,
                                        const std::string &inputreadout,
                                        double fixedE,
                                        const std::vector<double> &eguesspolynomial,
                                        I3PositionShiftType pstype,
                                        I3TimeShiftType tstype,
                                        I3TimeShiftType alt_tstype,
                                        bool speedpolice,
                                        double maxtresmean,
                                        double frac,
                                        I3SeedAlternatives altmode,
                                        bool onlyalt ) :
    I3SeedServiceBase(),
    name_(name),
    firstGuessNames_(fgnames),
    inputReadout_(inputreadout),
    energyGuessPolynomial_(eguesspolynomial),
    fixedEnergy_(fixedE),
    posShiftType_(pstype),
    timeShiftType_(tstype),
    altTimeShiftType_(alt_tstype),
    speedPolice_(speedpolice),
    maxTResMean_(maxtresmean),
    chargeFraction_(frac),
    altMode_(altmode),
    onlyAlternatives_(onlyalt),
    nMissingFG_(0),
    nBadFG_(0),
    nReadoutMissing_(0),
    nSetEvent_(0){ }


I3BasicSeedService::~I3BasicSeedService(){
    if ( nMissingFG_ > 0 ){
        log_warn( "(%s) In %u SetEvent() calls, "
                  "%u first guess track(s) were AWOL!",
                  name_.c_str(), nSetEvent_, nMissingFG_ );
    }
    if ( nBadFG_ > 0 ){
        log_warn( "(%s) In %u SetEvent() calls, "
                  "%u first guess track(s) were found but had a fit status that was not OK!",
                  name_.c_str(), nSetEvent_, nBadFG_ );
    }
    if ( nReadoutMissing_ > 0 ){
        log_warn( "(%s) For %u out of %u SetEvent() calls, "
                  "there were no hits/pulses named \"%s\" available.",
                  name_.c_str(), nReadoutMissing_, nSetEvent_, inputReadout_.c_str() );
    }
}

bool I3BasicSeedService::SeedOK( const I3Particle &seed ) const {

    const I3Position &pos = seed.GetPos();
    const I3Direction &dir = seed.GetDir();

    if ( I3Particle::OK != seed.GetFitStatus() ){
        log_trace("(%s) bad seed", name_.c_str() );
    } else if ( seed.IsCascade() &&
                !std::isfinite( pos.GetX() + pos.GetY() + pos.GetZ() + seed.GetTime() ) ){
        log_warn( "(%s) cascade seed with OK status but non-finite datamembers",
                  name_.c_str() );

        log_warn( "(%s) bad seed xyz=(%.1f,%.1f,%.1f) t=%.1f zenith=%.1f azimuth=%.1f",
                  name_.c_str(),
                  pos.GetX()/I3Units::m,
                  pos.GetY()/I3Units::m,
                  pos.GetZ()/I3Units::m,
                  seed.GetTime()/I3Units::ns,
                  dir.GetZenith()/I3Units::degree,
                  dir.GetAzimuth()/I3Units::degree );
    } else if ( seed.IsTrack() &&
                !std::isfinite( pos.GetX() + pos.GetY() + pos.GetZ() + seed.GetTime() +
                                dir.GetZenith() + dir.GetAzimuth() ) ){
        log_warn( "(%s) track seed with OK status but non-finite datamembers",
                  name_.c_str() );

        log_warn( "(%s) bad seed xyz=(%.1f,%.1f,%.1f) t=%.1f zenith=%.1f azimuth=%.1f",
                  name_.c_str(),
                  pos.GetX()/I3Units::m,
                  pos.GetY()/I3Units::m,
                  pos.GetZ()/I3Units::m,
                  seed.GetTime()/I3Units::ns,
                  dir.GetZenith()/I3Units::degree,
                  dir.GetAzimuth()/I3Units::degree );
    } else {
        log_debug( "(%s) ok seed xyz=(%.1f,%.1f,%.1f) t=%.1f zenith=%.1f azimuth=%.1f",
                   name_.c_str(),
                   pos.GetX()/I3Units::m,
                   pos.GetY()/I3Units::m,
                   pos.GetZ()/I3Units::m,
                   seed.GetTime()/I3Units::ns,
                   dir.GetZenith()/I3Units::degree,
                   dir.GetAzimuth()/I3Units::degree );

        return true;
    }

    return false;
}


I3EventHypothesis I3BasicSeedService::GetDummy() const {
    return I3EventHypothesis();
}

void I3BasicSeedService::GetFirstGuesses( const I3Frame &f ){
    seeds_.resize(0);
    for ( std::vector<std::string>::iterator ifg = firstGuessNames_.begin();
          ifg != firstGuessNames_.end(); ++ifg ){
        I3ParticleConstPtr single = f.Get<I3ParticleConstPtr>( *ifg );
        I3VectorI3ParticleConstPtr many = f.Get<I3VectorI3ParticleConstPtr>( *ifg );

        if ( single ){
            log_debug( "(%s) Found single reconstruction result %s",
                       name_.c_str(), ifg->c_str() );
            if ( SeedOK( *single ) ){
                seeds_.push_back( I3EventHypothesis( *single ) );
            } else {
                ++nBadFG_;
            }
        } else if ( many ){
            log_debug( "(%s) Found multiple reconstruction result %s with %zu solutions",
                       name_.c_str(), ifg->c_str(), many->size() );

            I3VectorI3Particle::const_iterator iifg;
            for ( iifg = many->begin(); iifg != many->end(); ++iifg ){
                if ( SeedOK( *iifg ) ){
                    seeds_.push_back( I3EventHypothesis( *iifg ) );
                } else {
                    ++nBadFG_;
                }
            }
        } else {
            ++nMissingFG_;
            if ( nMissingFG_ <= 25 ){
                log_warn( "(%s) COULD NOT FIND RECONSTRUCTION RESULT(S) WITH "
                          "LABEL %s", name_.c_str(), ifg->c_str() );
                if ( nMissingFG_ == 25 ){
                    log_warn( "(%s) (For the rest of the run these warnings "
                              "will be suppressed.)", name_.c_str() );
                }
            }
        }
    }

    std::vector<I3EventHypothesis>::iterator iseed;
    for ( iseed = seeds_.begin(); iseed != seeds_.end(); ++iseed ){
        FillInTheBlanks( *iseed );

        if ( speedPolice_ ){
            I3Particle &p = *(iseed->particle);
            if ( p.GetShape() == I3Particle::InfiniteTrack ){
                p.SetSpeed( I3Constants::c );
            } else if ( p.GetShape() == I3Particle::Cascade ){
                p.SetSpeed( 0. );
            }
        }
    }

    log_debug( "(%s), collected %zu first guesses",
               name_.c_str(), seeds_.size() );
}

I3EventHypothesis I3BasicSeedService::GetCopy( const I3EventHypothesis &eh ) const {
    I3EventHypothesis neweh( *(eh.particle) );
    assert( eh.particle->GetShape() == neweh.particle->GetShape() );

    return neweh;
}

double I3BasicSeedService::PolynomialEnergyGuess() const {

    // get hitset
    double Nch;
    if ( pulses_ ){
        Nch = pulses_->size();
    } else {
        log_fatal( "(%s) no pulses, no hits???", name_.c_str() );
    }

    if ( Nch <= 0. ) {
        return 0.;
    }

    double log10E = 0.;
    // should be true: Nch > minNch
    assert( Nch > 0. );
    // nHits > minHits
    double x = std::log10( Nch );
    double xpow = 1.0;

    for ( std::vector<double>::const_iterator ifitp = energyGuessPolynomial_.begin();
          ifitp != energyGuessPolynomial_.end(); ++ifitp ){
        log10E += (*ifitp) * xpow;
        xpow *= x;
    }

    double energy = std::pow( 10.0, log10E );

    // sets the energy first guess
    // make sure that your energy first guess is appropriate for your reconstruction...
    // for example in photorec-llh you are reconstructing the energy loss per unit meter
    energy *= I3Units::GeV;

    log_debug( "(%s) energy=%g GeV (log10(E)=%.2f)", name_.c_str(),
               energy/I3Units::GeV, log10E );

    return energy;
}

void I3BasicSeedService::FillInTheBlanks( I3EventHypothesis &eh ) const {
    assert( eh.particle );
    I3Particle &p = *(eh.particle);

    if ( !std::isnan( fixedEnergy_ ) ){
        p.SetEnergy( fixedEnergy_ );
    } else if ( energyGuessPolynomial_.size() > 0 ){
        p.SetEnergy( PolynomialEnergyGuess() );
    }

    if ( p.GetShape() == I3Particle::Null ){
        if ( p.IsCascade() || !p.HasDirection() ){
             p.SetShape( I3Particle::Cascade );
        } else {
             // should be configurable
             p.SetShape( I3Particle::InfiniteTrack );
        }
    }
}

void I3BasicSeedService::Tweak( I3EventHypothesis &eh ) const {
    assert( eh.particle );

    if ( inputReadout_.empty() ) {
        return;
    }

    I3Particle &p = *(eh.particle);
    ShiftVertex( p );

    if (pulses_) {
        ShiftTime( p, *pulses_ );
    }
}

void I3BasicSeedService::ShiftVertex( I3Particle &p ) const {
    if ( posShiftType_ != I3BasicSeedService::PCOG ){
        return;
    }
    if ( p.GetShape() != I3Particle::InfiniteTrack ){
        return;
    }
    if ( !p.HasDirection() ){
        return;
    }

    I3Position vertex = p.GetPos();
    I3Direction dir = p.GetDir();

    double dx = cog_.GetX() - vertex.GetX();
    double dy = cog_.GetY() - vertex.GetY();
    double dz = cog_.GetZ() - vertex.GetZ();

    if ( !std::isfinite( dx + dy + dz ) ){
        log_warn( "(%s) non finite cog-vertex...?!", name_.c_str() );

        log_warn( "(%s) cog=(%f,%f,%f) vertex=(%f,%f,%f)", name_.c_str(),
                  cog_.GetX(), cog_.GetY(), cog_.GetZ(),
                  vertex.GetX(), vertex.GetY(), vertex.GetZ() );
        return;
    }

    double oldd = std::sqrt( dx*dx + dy*dy + dz*dz );

    log_debug( "(%s) old vertex=(%.1f,%.1f,%.1f) d=%.1f", name_.c_str(),
               vertex.GetX(), vertex.GetY(), vertex.GetZ(), oldd );

    double shiftlength = dx*dir.GetX() + dy*dir.GetY() + dz*dir.GetZ();
    I3Position newvertex = p.GetPos() + shiftlength*p.GetDir();
    p.SetPos( newvertex );

    dx = cog_.GetX() - newvertex.GetX();
    dy = cog_.GetY() - newvertex.GetY();
    dz = cog_.GetZ() - newvertex.GetZ();
    double newd = std::sqrt( dx*dx + dy*dy + dz*dz );

    log_debug( "(%s) new vertex=(%.1f,%.1f,%.1f) d=%.1f", name_.c_str(),
               newvertex.GetX(), newvertex.GetY(), newvertex.GetZ(), newd );

    if ( oldd + 0.5 < newd ){
        log_fatal( "(%s) old=%f new=%f", name_.c_str(), oldd, newd );
    }

    double oldtime = p.GetTime();
    assert( p.GetSpeed() > 0. );

    log_debug( "(%s) space correction: vertex %f meters, time %f ns (speed=%fc)",
               name_.c_str(),
               shiftlength / I3Units::m,
               shiftlength / p.GetSpeed(),
               p.GetSpeed() / I3Constants::c );

    p.SetTime( oldtime + shiftlength / p.GetSpeed() );
}

namespace {
double MeanTimeResidual( const I3Particle &particle,
                         const I3Geometry &geometry,
                         const I3RecoPulseSeriesMap &hitmap ){
    double sumtres = 0;
    int nsum = 0;
    for ( I3RecoPulseSeriesMap::const_iterator iter = hitmap.begin();
          iter != hitmap.end(); ++iter ){
        if ( geometry.omgeo.count( iter->first ) == 0 ){
            continue;
        }
        if ( iter->second.empty() ){
            continue;
        }

        const I3Position &ompos = geometry.omgeo.find( iter->first )->second.position;
        double thit = iter->second.front().GetTime();

        if ( !std::isfinite( thit ) ){
            continue;
        }

        double tres = I3Calculator::TimeResidual( particle, ompos, thit );

        if ( !std::isfinite( tres ) ){
            continue;
        }

        sumtres += tres;
        ++nsum;
    }

    return ( nsum > 0 ) ? sumtres/nsum : 0.;
}

double MinimumTimeResidual( const I3Particle &particle,
                            const I3Geometry &geometry,
                            const I3RecoPulseSeriesMap &hitmap,
                            double tresmeanmax ){
    double tresmin = NAN;
    bool blockTooEarlyHits = !std::isnan( tresmeanmax );
    double tresmean = MeanTimeResidual( particle, geometry, hitmap );

    for ( I3RecoPulseSeriesMap::const_iterator iter = hitmap.begin();
          iter != hitmap.end(); ++iter ){
        if ( geometry.omgeo.count( iter->first ) == 0 ){
            continue;
        }
        if ( iter->second.empty() ){
            continue;
        }

        const I3Position &ompos = geometry.omgeo.find( iter->first )->second.position;
        double thit = iter->second.front().GetTime();
        double tres = I3Calculator::TimeResidual( particle, ompos, thit );

        if ( tres > tresmin ){
            continue;
        }
        if ( !std::isfinite( tres ) ){
            continue;
        }
        if ( blockTooEarlyHits && ( tres < tresmean-tresmeanmax ) ){
            log_debug( "very early tres=%fns << tresmean=%fns x=%f y=%f z=%f t=%f om(%d,%d)",
                       tres/I3Units::ns, tresmean/I3Units::ns,
                       particle.GetPos().GetX(),
                       particle.GetPos().GetY(),
                       particle.GetPos().GetZ(),
                       particle.GetTime(),
                       iter->first.GetString(),
                       iter->first.GetOM() );
            continue;
        }
        tresmin = tres;
    }

    return tresmin;
}

double ChargeFractionTimeResidual( const I3Particle &particle,
                                   const I3Geometry &geometry,
                                   const I3RecoPulseSeriesMap &hitmap,
                                   double fraction,
                                   bool all,
                                   const std::string &logname ){
    // tq is a sorted list of timeresidual+charge pairs
    std::list<std::pair<double, double> > tq;
    std::list<std::pair<double, double> >::iterator itq;

    // initialize the list with a dummy last element
    // this simplifies the implementation within the loop
    itq = tq.insert( tq.end(), std::pair<double, double>( HUGE_VAL, 0. ) );

    double qtotal = 0.;
    for ( I3RecoPulseSeriesMap::const_iterator idom = hitmap.begin();
          idom != hitmap.end(); ++idom ){
        if ( geometry.omgeo.count( idom->first ) == 0 ){
            continue;
        }
        if ( idom->second.empty() ){
            continue;
        }

        const I3Position &ompos = geometry.omgeo.find( idom->first )->second.position;

        if ( all ){
            for ( I3RecoPulseSeries::const_iterator ihit = idom->second.begin();
                  ihit != idom->second.end(); ++ihit ){
                const I3RecoPulse &hit = *ihit;
                double thit = hit.GetTime();
                double tres = I3Calculator::TimeResidual( particle, ompos, thit );

                if ( !std::isfinite(tres) ){
                    continue;
                }

                double qhit = hit.GetCharge();
                qtotal += qhit;

                while ( ( itq != tq.begin() ) && ( itq->first > tres ) ){
                    --itq;
                }
                while ( ( itq != tq.end() ) && ( itq->first < tres ) ){
                    ++itq;
                }

                tq.insert( itq, std::pair<double, double>( tres, qhit ) );
            }
        } else {
            const I3RecoPulse &firsthit = idom->second.front();
            double thit = firsthit.GetTime();
            double tres = I3Calculator::TimeResidual( particle, ompos, thit );

            if ( !std::isfinite(tres) ){
                continue;
            }

            double qhit = firsthit.GetCharge();
            qtotal += qhit;

            while ( ( itq != tq.begin() ) && ( itq->first > tres ) ){
                --itq;
            }
            while ( ( itq != tq.end() ) && ( itq->first < tres ) ){
                ++itq;
            }

            tq.insert( itq, std::pair<double, double>( tres, qhit ) );
        }
    }
#if 0
    double tprevious = -HUGE_VAL;
    for ( itq = tq.begin(); itq != tq.end(); ++itq ){
        if ( itq->first < tprevious ){
            log_warn("WRONG SORTING, %g < %g !!", itq->first, tprevious);
        }
        tprevious = itq->first;
    }
#endif

    // remove dummy element
    tq.pop_back();

    // silly check
    if ( tq.empty() ){
        log_warn( "(%s) Yikes! ChargeFractionTimeResidual didn't find any hits!",
                  logname.c_str() );

        return 0.;
    }

    // find threshold time
    double qint = 0.;
    double qthreshold = (1. - fraction) * qtotal;

    // qthreshold is the amount of charge that should have a negative time residual.
    for ( itq = tq.begin(); itq != tq.end(); ++itq ){
        qint += itq->second;

        if ( qint >= qthreshold ) {
            // found it
            break;
        }
    }

    // paranoia
    if ( itq == tq.end() ){
        log_warn( "(%s) ChargeFractionTimeResidual failed with %zu hits in %zu doms",
                  logname.c_str(), tq.size(), hitmap.size() );

        return 0.;
    }

    double qtime = itq->first;

    if ( !std::isfinite(qtime) ){
        // this may be overkill, but that's fine :-)
        log_warn( "(%s) URGH: non-finite time %g", logname.c_str(), qtime );
        log_warn( "(%s) This should never happen. Hitmap had %zu DOMs",
                  logname.c_str(), hitmap.size() );
        log_warn( "(%s) Sorted tres-charge list:", logname.c_str() );

        for ( itq = tq.begin(); itq != tq.end(); ++itq ){
            log_warn( "(%s) %14.1fns %8.1fPE", logname.c_str(), itq->first, itq->second );
        }
    }

    return qtime;
}
}

void I3BasicSeedService::ShiftTime( I3Particle &p,
                                    const I3RecoPulseSeriesMap &hitmap ) const {
    assert( geometry_ );
    const I3Geometry &g = *geometry_;

    for ( I3RecoPulseSeriesMap::const_iterator idom = hitmap.begin();
          idom != hitmap.end(); ++idom ){
        if ( g.omgeo.count( idom->first ) == 0 ) {
            continue;
        }
        if ( idom->second.empty() ) {
            continue;
        }
        if ( !std::isfinite( idom->second.front().GetTime() ) ){
            log_fatal( "(%s) got hit in DOM(%d,%u) with non-finite time %g",
                       name_.c_str(),
                       idom->first.GetString(), 
                       idom->first.GetOM(), 
                       idom->second.front().GetTime() );
        }
    }

    double tshift = 0.;
    switch ( timeShiftType_ ){
        case TMean:
            tshift = MeanTimeResidual( p, g, hitmap );
            break;
        case TFirst:
            tshift = MinimumTimeResidual( p, g, hitmap, maxTResMean_ );
            break;
        case TChargeFraction:
            tshift =
                ChargeFractionTimeResidual( p, g, hitmap, chargeFraction_, true, name_ );
            break;
        case TDirectChargeFraction:
            tshift =
                ChargeFractionTimeResidual( p, g, hitmap, chargeFraction_, false, name_ );
            break;
        case TNone:
            tshift = 0.;
            break;
        default:
            assert( !"kaboom" );
    }

    if ( std::isfinite( tshift ) ){
        double oldtime = p.GetTime();
        double newtime = oldtime + tshift;

        log_debug( "(%s) time correction: from %f ns to %f ns",
                   name_.c_str(),
                   oldtime/I3Units::ns,
                   newtime/I3Units::ns );

        p.SetTime( newtime );

#ifndef NDEBUG
        double checkshift = 0.;
        switch ( timeShiftType_ ){
            case TMean:
                checkshift = MeanTimeResidual( p, g, hitmap );
                break;
            case TFirst:
                checkshift = MinimumTimeResidual( p, g, hitmap,maxTResMean_ );
                break;
            case TChargeFraction:
                checkshift =
                    ChargeFractionTimeResidual( p, g, hitmap, chargeFraction_, true, name_ );
                break;
            case TDirectChargeFraction:
                checkshift =
                    ChargeFractionTimeResidual( p, g, hitmap, chargeFraction_, false, name_ );
                break;
            case TNone:
                checkshift = 0.;
                break;
            default:
                assert( !"kaboom" );
        }
        assert( std::fabs( checkshift ) < 0.1*I3Units::ns );
#endif /* not NDEBUG */
    } else {
        log_warn( "(%s) timecorrection failed with NCH=%zu, got tshift=%gns",
                  name_.c_str(), hitmap.size(), tshift/I3Units::ns );
    }
}

void I3BasicSeedService::GetAlternatives(){

    if ( altMode_ == SeedAlt_None ){
        return;
    }

    std::vector<I3EventHypothesis> fgseeds;
    fgseeds.swap( seeds_ );
    assert( seeds_.empty() );

    for ( std::vector<I3EventHypothesis>::iterator iseed = fgseeds.begin();
          iseed != fgseeds.end(); ++iseed ){
        if ( !onlyAlternatives_ ){
            seeds_.push_back( *iseed );
        }

        std::vector<I3ParticlePtr> alternatives;
        switch ( altMode_ ){
            case SeedAlt_Reverse:
                alternatives = Reverse( iseed->particle );
                break;
            case SeedAlt_Tetrahedron:
                alternatives = Tetrahedron( iseed->particle );
                break;
            case SeedAlt_Cube:
                alternatives = Cube( iseed->particle );
                break;
            default:
                log_fatal( "(%s) PROGRAMMING ERROR: altMode_=%d",
                           name_.c_str(), altMode_ );
        }

        for ( std::vector<I3ParticlePtr>::iterator ialt = alternatives.begin();
              ialt != alternatives.end(); ++ialt ){
            I3EventHypothesis altseed( *ialt, I3FrameObjectPtr() );

            if ( !inputReadout_.empty() ){
                I3TimeShiftType oriTST = timeShiftType_;
                timeShiftType_ = altTimeShiftType_;
                Tweak( altseed );
                timeShiftType_ = oriTST;
            }
            seeds_.push_back( altseed );
        }
    }
}

unsigned int I3BasicSeedService::SetEvent( const I3Frame &f ){

    ++nSetEvent_;

    if ( !inputReadout_.empty() ){
        // get hits
        pulses_ = f.Get<I3RecoPulseSeriesMapConstPtr>( inputReadout_ );

        if ( !pulses_ ){
            const unsigned int Nmissmax = 5;

            if ( nReadoutMissing_ < Nmissmax ){
            log_info( "(%s) Cannot find pulse series with name \"%s\"",
                       name_.c_str(), inputReadout_.c_str() );
            } else if ( nReadoutMissing_ == Nmissmax ){
                log_info( "(%s) Won't report about missing  \"%s\" "
                          "hit/pulse series anymore until the end",
                          name_.c_str(), inputReadout_.c_str() );
            }

            ++nReadoutMissing_;
            return 0;
        }
    }

    GetFirstGuesses(f);

    if ( !inputReadout_.empty() ){
        // get geometry
        geometry_ = f.Get<I3GeometryConstPtr>();

        if ( !geometry_ ){
            log_fatal( "(%s) Cannot find I3Geometry", name_.c_str() );
        }

        const I3Geometry &georef = *geometry_;
        cog_ = I3Cuts::COG( georef, *pulses_ );

        log_debug( "(%s) COG=(%.1f,%.1f,%.1f) ", name_.c_str(),
                   cog_.GetX(), cog_.GetY(), cog_.GetZ() );

        // get good vertex position+time
        for ( std::vector<I3EventHypothesis>::iterator iseed = seeds_.begin();
              iseed != seeds_.end(); ++iseed ){
            Tweak( *iseed );
        }
    }

    GetAlternatives();

    unsigned int nseeds = seeds_.size();
    log_debug( "collected %u seeds", nseeds );

    return nseeds;
}


// get a seed
// returns a deep copy of an element of the internal list of seeds
I3EventHypothesis I3BasicSeedService::GetSeed( unsigned int iseed ) const {
    if( iseed >= seeds_.size() ){
        log_fatal("This seed is not available.");
    }

    return GetCopy( seeds_[iseed] );
}

std::vector<I3ParticlePtr> I3BasicSeedService::Reverse( I3ParticleConstPtr p ){
    std::vector<I3ParticlePtr> v;
    I3ParticlePtr newp( new I3Particle( *p ) );
    newp->SetDir( I3Calculator::GetReverseDirection( p->GetDir() ) );
    v.push_back( newp );
    return v;
}

std::vector<I3ParticlePtr> I3BasicSeedService::Tetrahedron( I3ParticleConstPtr p ){
    // dummy implementation for now
    // (so that I can commit to svn and work further on my laptop)
    std::vector<I3ParticlePtr> v;
    while ( v.size() < 3 ){
        v.push_back( I3ParticlePtr( new I3Particle( *p ) ) );
    }

    // get perpendicular directions
    I3Direction dir = p->GetDir();
    std::pair<I3Direction, I3Direction> perp =
        I3Calculator::GetTransverseDirections( dir );

    // consider a cube with sides 2, 8 points (+/-1,+/-1,+/-1) and center
    // (0,0,0).
    // a tetrahedron can be enclosed in that cube, with points on (1,1,1),
    // (1,-1,-1), (-1,1,-1), (-1,-1,1)
    // so the cosine of the angle between any two legs is cos(angle) = (inner
    // product)/(product of the lengths) = (1-1-1)/(sqrt(3)*sqrt(3)) = -1/3
    double tetra_angle = std::acos( -1./3. );
    I3Calculator::Rotate( perp.first, dir, tetra_angle );
    v[0]->SetDir( dir );

    // once you have two legs, the others can be obtained
    // by rotating one around the other over +120 and +240 degrees.
    I3Calculator::Rotate( p->GetDir(), dir, 120*I3Units::degree );
    v[1]->SetDir( dir );
    I3Calculator::Rotate( p->GetDir(), dir, 120*I3Units::degree );
    v[2]->SetDir( dir );

    return v;
}

std::vector<I3ParticlePtr> I3BasicSeedService::Cube( I3ParticleConstPtr p ){
    // get reverse direction
    std::vector<I3ParticlePtr> v( Reverse( p ) );

    // get perpendicular directions
    std::pair<I3Direction, I3Direction> perp =
        I3Calculator::GetTransverseDirections( p->GetDir() );

    for ( int i = 1; i <= 4; ++i ){
        v.push_back( I3ParticlePtr( new I3Particle( *p ) ) );
    }

    v[1]->SetDir( perp.first );
    v[2]->SetDir( I3Calculator::GetReverseDirection(perp.first) );
    v[3]->SetDir( perp.second );
    v[4]->SetDir( I3Calculator::GetReverseDirection( perp.second ) );

    return v;
}
