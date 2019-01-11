/**
 * @file I3DoubleMuonLogLikelihood.cxx
 * @brief implementation of the I3DoubleMuonLogLikelihood class
 *
 * (c) 2005 * the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */
#include <cassert>
#include <icetray/I3Frame.h>
#include <icetray/I3SingleServiceFactory.h>
#include <dataclasses/I3Position.h>
#include <dataclasses/I3Direction.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/geometry/I3Geometry.h>

#include "double-muon/I3DoubleMuonLogLikelihood.h"
#include "ipdf/I3/I3OmReceiver.h"

static const std::string pulses_optionname = "PulseMapName";
static const std::string noiserate_optionname = "NoiseRate";
static const std::string zenithw8_optionname = "ZenithWeight";
static const std::string mpe_optionname = "UseMPE";

I3DoubleMuonLogLikelihood::I3DoubleMuonLogLikelihood(
            std::string name,
            std::string pulses,
            std::string zenithw8,
            double noiserate,
            bool mpe ):
        I3EventLogLikelihoodBase(), I3ServiceBase(name),
        pulseMapName_(pulses),
        zenithWeightName_(zenithw8), noiseRate_(noiserate), do_mpe_(mpe),
        multiplicity_(0){
    if ( noiseRate_ > 0 ){
        // forcing units of 1/ns (does nothing effectively, as ns=1.0)
        logNoiseRate_=log(noiseRate_*I3Units::ns);
    } else {
        log_fatal( "(%s) negative noise rate %g Hz",
                   GetName().c_str(), noiseRate_/I3Units::hertz );
    }
}


// construct self & declare configuration parameters
I3DoubleMuonLogLikelihood::I3DoubleMuonLogLikelihood( const I3Context &context ):
    I3EventLogLikelihoodBase(), I3ServiceBase(context),multiplicity_(0){

    log_debug( "(%s) hey, this is the likelihood function for "
               "two coincident muons", GetName().c_str() );

    pulseMapName_ = "FEPulses";
    AddParameter(
            pulses_optionname,
            "Name of recopulse map.",
            pulseMapName_ );

    noiseRate_ = 10.0*I3Units::ns*I3Units::hertz;
    AddParameter(
            noiserate_optionname,
            "Noise probability (dimensionless).",
            noiseRate_ );

    AddParameter(
            zenithw8_optionname,
            "Name of zenith weight service.",
            zenithWeightName_ );

    do_mpe_ = false;
    AddParameter(
            mpe_optionname,
            "Use MPE instead of SPE.",
            do_mpe_ );

}

// get configuration parameters
void I3DoubleMuonLogLikelihood::Configure(){

    GetParameter( pulses_optionname, pulseMapName_ );
    GetParameter( noiserate_optionname, noiseRate_ );
    GetParameter( zenithw8_optionname, zenithWeightName_ );
    GetParameter( mpe_optionname, do_mpe_ );

    log_info( "(%s) pulses: %s", GetName().c_str(),
              (pulseMapName_.empty()?"(none)":pulseMapName_.c_str() ) );
    log_info( "(%s) noise rate: %gHz",
              GetName().c_str(), noiseRate_/I3Units::hertz );
    if ( noiseRate_ <= 0 ){
        log_fatal( "(%s) Noise rate should be positive!",
                   GetName().c_str() );
    }

    // forcing units of 1/ns (does nothing effectively, as ns=1.0)
    logNoiseRate_ = log(noiseRate_*I3Units::ns);

    if ( zenithWeightName_.empty() ){
        log_info( "(%s) no zenith weight", GetName().c_str() );
    } else {
        zenithWeight_ =
            context_.Get<I3EventLogLikelihoodBasePtr>( zenithWeightName_);
        if ( ! zenithWeight_ ){
            log_fatal( "(%s) could not find zenith weight named \"%s\"",
                       GetName().c_str(), zenithWeightName_.c_str() );
        }
        log_info( "(%s) using zenith weight from %s",
                  GetName().c_str(), zenithWeightName_.c_str() );
    }

    log_info( "(%s) using: %s", GetName().c_str(), (do_mpe_?"MPE":"SPE") );

}

void I3DoubleMuonLogLikelihood::SetGeometry( const I3Geometry &geo ){
    // now make a new IPDF-detectorconfig object
    ipdfGeometry_ =
        boost::shared_ptr<IPDF::I3DetectorConfiguration>(
                new IPDF::I3DetectorConfiguration( geo ) );
}

void I3DoubleMuonLogLikelihood::SetEvent( const I3Frame &f ){
    I3RecoPulseSeriesMapConstPtr pulsemap =
        f.Get<I3RecoPulseSeriesMapConstPtr>( pulseMapName_ );
    multiplicity_ = 0;
    if ( ! pulsemap ){
        log_warn("(%s) no I3RecoPulseSeriesMap with name \"%s\"",
              GetName().c_str(), pulseMapName_.c_str() );
        return;
    }
    I3RecoPulseSeriesMap::const_iterator psmci = pulsemap->begin();
    I3RecoPulseSeriesMap::const_iterator psmciend = pulsemap->end();
    log_debug( "(%s) %zu DOMs in the pulse map",
               GetName().c_str(), pulsemap->size() );
    for ( ; psmci != psmciend; psmci++ ){
        if ( psmci->second.size() > 0 ){
            log_debug( "(%s) DOM(%d,%u) has %zu pulses",
                       GetName().c_str(),
                       psmci->first.GetString(),
                       psmci->first.GetOM(),
                       psmci->second.size() );
            ++multiplicity_;
        }
    }
    log_debug( "(%s) %u DOMs indeed have pulses",
               GetName().c_str(), multiplicity_ );

    ipdfResponseMap_ =
        boost::shared_ptr<IPDF::I3DetectorResponse>(
                new IPDF::I3DetectorResponse(pulsemap,*ipdfGeometry_) );

    // PARANOIA
    unsigned int mcheck=0;
    IPDF::I3DetectorResponse::const_iterator drci = ipdfResponseMap_->begin();
    IPDF::I3DetectorResponse::const_iterator drciend = ipdfResponseMap_->end();
    for ( ; drci != drciend; drci++ ){
        if ( (*drci)->empty() ) continue;
        ++mcheck;
    }
    if ( mcheck != multiplicity_ ){
        log_warn( "(%s) OOPS: %u DOMs indeed have pulses (expected %u)",
                  GetName().c_str(), mcheck, multiplicity_ );
    } else {
        log_debug( "(%s) INDEED: %u DOMs indeed have pulses (expected %u)",
                  GetName().c_str(), mcheck, multiplicity_ );
    }

    return;
}

// compute likelihood
double I3DoubleMuonLogLikelihood::GetLogLikelihood( const I3EventHypothesis &t ){
    double llh=0;
    I3ParticlePtr i3muon1 = t.particle;
    I3ParticlePtr i3muon2 = boost::dynamic_pointer_cast<I3Particle>(t.nonstd);
    if ( ! i3muon2 ){
        log_fatal( "(%s) wrong event hypothesis: missing second muon",
                   GetName().c_str() );
    }

#ifndef NDEBUG
    // this NDEBUG guard avoids compiler warnings in release builds
    double e1 = i3muon1->GetEnergy();
    double e2 = i3muon2->GetEnergy();
    log_debug( "(%s) e1=%g e2=%g",
               GetName().c_str(), e1/I3Units::GeV, e2/I3Units::GeV);
    assert(!std::isnan(e1));
    assert(!std::isnan(e2));
    assert(e1>0);
    assert(e2>0);
#endif
    const I3Position &pos1 = i3muon1->GetPos();
    const I3Position &pos2 = i3muon2->GetPos();
    const I3Direction &dir1 = i3muon1->GetDir();
    const I3Direction &dir2 = i3muon2->GetDir();
    IPDF::InfiniteMuon ipdfmuon1( pos1.GetX(), pos1.GetY(), pos1.GetZ(),
                                  dir1.GetX(), dir1.GetY(), dir1.GetZ(),
                                  i3muon1->GetEnergy(), i3muon1->GetTime() );
    IPDF::InfiniteMuon ipdfmuon2( pos2.GetX(), pos2.GetY(), pos2.GetZ(),
                                  dir2.GetX(), dir2.GetY(), dir2.GetZ(),
                                  i3muon2->GetEnergy(), i3muon2->GetTime() );

    IPDF::I3DetectorResponse::const_iterator drci = ipdfResponseMap_->begin();
    IPDF::I3DetectorResponse::const_iterator drciend = ipdfResponseMap_->end();
    for ( ; drci != drciend; drci++ ){
        if ( (*drci)->empty() ) continue;
        const IPDF::I3OmReceiver &i3om = (*drci)->getOmReceiver();
        double npe1 = pandelGCPDFH2_.expectedNPE(i3om,ipdfmuon1);
        double npe2 = pandelGCPDFH2_.expectedNPE(i3om,ipdfmuon2);
        assert(npe1>=0);
        assert(npe2>=0);
        if ( npe1+npe2 == 0 ){
            if ( do_mpe_ ){
                llh += logNoiseRate_;
            } else {
                llh += (*drci)->size()*logNoiseRate_;
            }
            continue;
        }
        IPDF::I3HitOm::const_iterator omrci = (*drci)->begin();
        IPDF::I3HitOm::const_iterator omrciend = (*drci)->end();
        double prob = 1;

        if ( do_mpe_ ){
            double mpe1 = mpeGCPDFH2_.getLikelihood(**drci,ipdfmuon1);
            double mpe2 = mpeGCPDFH2_.getLikelihood(**drci,ipdfmuon2);
            assert(mpe1>=0);
            assert(mpe2>=0);
            prob *= noiseRate_ + (npe1*mpe1+npe2*mpe2)/(npe1+npe2);
        } else {
            for ( ; omrci != omrciend; omrci++ ){
                double pdf1 = pandelGCPDFH2_.getPdf( **omrci, ipdfmuon1 );
                double pdf2 = pandelGCPDFH2_.getPdf( **omrci, ipdfmuon2 );
                assert(pdf1>=0);
                assert(pdf2>=0);
                prob *= noiseRate_ + (npe1*pdf1+npe2*pdf2)/(npe1+npe2);
            }
        }

        if (prob>0) llh += log(prob);
    }

    // adding zenith weight, if desired
    if ( zenithWeight_ ){
        I3EventHypothesis eh1( i3muon1, I3FrameObjectPtr() );
        I3EventHypothesis eh2( i3muon2, I3FrameObjectPtr() );
        llh += zenithWeight_->GetLogLikelihood(eh1);
        llh += zenithWeight_->GetLogLikelihood(eh2);
    }

    return llh;
}

typedef I3SingleServiceFactory< I3DoubleMuonLogLikelihood,I3EventLogLikelihoodBase >
I3DoubleMuonLogLikelihoodServiceFactory;
I3_SERVICE_FACTORY( I3DoubleMuonLogLikelihoodServiceFactory )
