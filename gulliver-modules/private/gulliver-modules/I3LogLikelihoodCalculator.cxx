/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3LogLikelihoodCalculator.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

// tools, standard lib stuff
#include "icetray/IcetrayFwd.h"
#include <string>

// my header
#include "gulliver-modules/I3LogLikelihoodCalculator.h"

// IceTray/ROOT stuff
#include "icetray/I3Context.h"
#include "icetray/I3Frame.h"
#include "dataclasses/geometry/I3Geometry.h"
I3_MODULE(I3LogLikelihoodCalculator);

// dataclasses & gulliver
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/utilities/ordinal.h"

static const std::string llh_optionname = "LogLikelihoodService";
static const std::string fit_optionname = "FitName";
static const std::string npar_optionname = "NFreeParameters";


//----------------------------------------------------------------------------

I3LogLikelihoodCalculator::I3LogLikelihoodCalculator(const I3Context& ctx) :
        I3ConditionalModule(ctx){

    AddOutBox("OutBox");

    // initialize counters
    nEvent_ = 0;
    nFitOK_ = 0;
    nFitBad_ = 0;
    nSuccess_ = 0;

    // options
    AddParameter( llh_optionname,
                  "LogLikelihood service to use",
                  llhServiceName_ );

    AddParameter("Parametrization",
                 "Parametrization service to use",
                 paramService_ );

    AddParameter( fit_optionname,
                  "Name of a I3Particle object for which you'd like to know "
                  "the likelihood.",
                  fitName_ );

    nPar_ = 0;
    AddParameter( npar_optionname,
                  "Pretend there were npar free fittable parameters. This"
                  "is used in two ways: (1) if the 'multiplicity' reported"
                  "by the llh service is less or equal this number,"
                  "then the likelihood will be NAN; (2) the 'reduced"
                  "likelihood' is computed as the full likelihood divided"
                  "by (multiplicity-npar).",
                  nPar_ );

}

//----------------------------------------------------------------------------
void I3LogLikelihoodCalculator::Configure(){

    // get option values
    GetParameter( fit_optionname, fitName_ );
    GetParameter( npar_optionname, nPar_ );
    GetParameter( llh_optionname, llhServiceName_ );
    GetParameter( "Parametrization", paramService_ );

    // never trust the user
    if ( llhServiceName_.empty() ){
        log_fatal( "(%s) you did not specify a llh service, use the \"%s\" option",
                   GetName().c_str(), llh_optionname.c_str() );
    }

    bool llhOK = context_.Has< I3EventLogLikelihoodBase >( llhServiceName_ );

    if ( ! llhOK ){
        log_fatal( "(%s) problem with getting llh service \"%s\"",
                   GetName().c_str(), llhServiceName_.c_str() );
        return;
    }
    llhService_ = context_.Get< I3EventLogLikelihoodBasePtr >( llhServiceName_ );
    assert( llhService_ );

    if ( fitName_.empty() ){
        log_fatal( "(%s) you should specify the name of a fit "
                   "(an I3Particle object stored in the physics frames)",
                   GetName().c_str() );
    }

    // things seem OK
    log_debug( "(%s) loglikelihood from \"%s\"",
              GetName().c_str(), llhServiceName_.c_str() );
    log_debug( "(%s) input fit \"%s\"",
              GetName().c_str(), fitName_.c_str() );
    log_debug( "(%s) pretending that we have %d fittable parameters.",
              GetName().c_str(), nPar_ );
    assert(nPar_>=0);


}

bool I3LogLikelihoodCalculator::CheckParticle( I3ParticleConstPtr p ){
    bool pass = false;
    if ( p && (p->GetFitStatus() == I3Particle::OK ) ){
        const I3Position &pos = p->GetPos();
        const I3Direction &dir = p->GetDir();
        if ( ! std::isfinite(pos.GetX()+pos.GetY()+pos.GetZ()+p->GetTime() ) ){
            log_warn( "(%s) particle with OK fitstatus has sick vertex",
                      GetName().c_str() );
            log_warn( "(%s) (x,y,z,t)=(%g,%g,%g,%g)",
                      GetName().c_str(),
                      pos.GetX(),pos.GetY(),pos.GetZ(),p->GetTime() );
        } else if ( std::isinf( p->GetEnergy() ) || ( p->GetEnergy() < 0 ) ){
            log_warn( "(%s) track with OK fitstatus has E=%g GeV",
                      GetName().c_str(),p->GetEnergy()/I3Units::GeV );
        } else if (p->IsTrack() && ! p->HasDirection() ){
            log_warn( "(%s) track with OK fitstatus has sick direction",
                      GetName().c_str() );
            log_warn( "(%s) (zenith,azimuth)=(%g rad,%g rad)",
                      GetName().c_str(),
                      dir.GetZenith(),dir.GetAzimuth() );
        } else {
            pass = true;
        }
    }
    return pass;
}

//----------------------------------------------------------------------------
void I3LogLikelihoodCalculator::Geometry(I3FramePtr frame){
    const I3Geometry& geo = frame->Get<I3Geometry>();
    llhService_->SetGeometry(geo);
    PushFrame( frame, "OutBox" );
}

//----------------------------------------------------------------------------
void I3LogLikelihoodCalculator::Physics(I3FramePtr frame){

    ++nEvent_;
    log_debug( "(%s) Welcome to the physics method of LogLikelihoodCalculator!",
               GetName().c_str());
    log_debug( "(%s) This is the %s physics frame.",
               GetName().c_str(), ordinal(nEvent_));

    // output object
    I3LogLikelihoodFitParamsPtr fitparams(new I3LogLikelihoodFitParams);

    // input object
    I3ParticleConstPtr particle( frame->Get<I3ParticleConstPtr>( fitName_ ) );
    if ( CheckParticle(particle) ){
        ++nFitOK_;
    } else {
        ++nFitBad_;
        frame->Put( GetName(), fitparams );
        PushFrame( frame, "OutBox" );
        return;
    }
    I3EventHypothesisPtr hypothesis(new I3EventHypothesis( *particle ));
    // give the parametrization a chance to modify nonstd parts of the hypothesis
    if (paramService_ != NULL) {
        paramService_->SetEvent(*frame);
        paramService_->SetHypothesisPtr(hypothesis);
    }

    // try to get a likelihood with it
    llhService_->SetEvent(*frame);
    int multiplicity = llhService_->GetMultiplicity();
    if ( multiplicity > nPar_ ){
        double llh = llhService_->GetLogLikelihood(*hypothesis);
        if ( !std::isnan(llh) ){
            fitparams->logl_ = -llh;
            fitparams->rlogl_ = -llh/(multiplicity-nPar_);
            fitparams->ndof_ = multiplicity-nPar_;
            fitparams->nmini_ = 1; // dummy here
            ++nSuccess_;
        } else {
            log_warn( "(%s) Got NAN for %s input event with multiplicity=%d",
                      GetName().c_str(), ordinal(nEvent_), multiplicity );
        }
    }

    frame->Put( GetName(), fitparams );
    PushFrame( frame, "OutBox" );
    return;

}

void I3LogLikelihoodCalculator::Finish(){
    if ( (nSuccess_ == 0) || (nFitBad_ > 0) ){
        log_warn( "(%s) Nevent=%u NfitOK=%u NfitBad=%u Nsuccess=%u",
                  GetName().c_str(),
                  nEvent_, nFitOK_, nFitBad_, nSuccess_ );
    } else {
        log_info( "(%s) Nevent=%u NfitOK=%u NfitBad=%u Nsuccess=%u",
                  GetName().c_str(),
                  nEvent_, nFitOK_, nFitBad_, nSuccess_ );
    }
}
