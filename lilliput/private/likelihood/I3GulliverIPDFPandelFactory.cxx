/*
 * class: I3GulliverIPDFPandelFactory
 *
 * Version $Id$
 *
 * Date: 17 Feb 2004
 *
 * (c) IceCube Collaboration
 */

#include "lilliput/likelihood/I3GulliverIPDFPandelFactory.h"
#include "lilliput/likelihood/I3GulliverIPDFPandel.h"
#include "icetray/I3Units.h"
#include "ipdf/Likelihood/MPEAll.h"
#include "ipdf/Likelihood/SPEAll.h"
#include "ipdf/Likelihood/SPEqAll.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Likelihood/MPE.h"
#include "ipdf/Likelihood/PSA.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/LayeredIce.h"
#include "ipdf/Pandel/GaussConvolutedPEP.h"
#include "ipdf/Pandel/BoxConvolutedPEP.h"
#include "ipdf/PhotoSpline/PhotoSplinePEP.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Hypotheses/PointCascade.h"
#include "ipdf/Hypotheses/DirectionalCascade.h"
#include "ipdf/AllOMsLikelihoodWithConstNoise.h"

#include <sstream>
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

static const std::string inputreadout_optionname = "InputReadout";
static const std::string likelihood_optionname = "Likelihood";
static const std::string peprob_optionname = "PEProb";
static const std::string icemodel_optionname = "IceModel";
static const std::string icefile_optionname = "IceFile";
static const std::string absorption_optionname = "AbsorptionLength";
static const std::string jitter_optionname = "JitterTime";
static const std::string mpetimingerror_optionname = "MPETimingError";
static const std::string noiseprob_optionname = "NoiseProbability";
static const std::string lookup_optionname = "GConvoluteLookupFilename";
static const std::string eventtype_optionname = "EventType";

using namespace IPDF::Likelihood;
using namespace IPDF::Pandel;
using IPDF::AllOMsLikelihoodWithConstNoise;
using IPDF::PhotoSplinePEP;
using boost::shared_ptr;

I3_SERVICE_FACTORY(I3GulliverIPDFPandelFactory)
I3_SERVICE_FACTORY(I3GulliverIPDFFactory)

// Constructors

I3GulliverIPDFPandelFactory::I3GulliverIPDFPandelFactory( const I3Context& context ):
        I3ServiceFactory( context ),
        eventllh_() {

    ncontext_ = 0;

    likelihood_ = "SPE1st";
    AddParameter( likelihood_optionname,
                  "Specify which particular IPDFPandel likelihood function you want:\n"
                  "SPE1st (default), SPEAll, SPEqAll, MPE, MPEAll or PSA.",
                  likelihood_ );

    mpeTimingError_ = 0.;
    AddParameter( mpetimingerror_optionname,
                  "Estimate the calibration error of the ATWD/FADC timing "
                  "information. Only used for MPE likelihood. When you give "
                  "a positive number dt here then MPE(t,n) likelihoods will be "
                  "smeared: MPE_s(t,n) = int_{t-dt}^{t+dt} MPE(t',n) dt'.\n"
                  "In the past this calibration uncertainty was added to the "
                  "jitter time; that is fine for SPE likelihoods, but gives "
                  "too narrow and too negative distributions for MPE.",
                  mpeTimingError_ );

    intCalcType_ = IPDF::Pandel::IntFastPlain;
    peprob_ = "GaussConvoluted";
    AddParameter( peprob_optionname,
        "\nSpecify how jitter and systematic uncertainties\n"
        "are folded in, as well as how the integral is computed/approximated\n"
        "(necessary for MPE and PSA):\n"
        "* \"GaussConvoluted\" (default): new implemenation of the convoluted\n"
        "Pandel function, but it will use plain Pandel (not convoluted) to\n"
        "compute the integral.\n"
        "* \"GaussConvolutedFastApproximation\", same, but it uses a (much)\n"
        "more accurate (but slower) approximation scheme for the integral,\n"
        "thanks to George & Mathieu & Dima;\n"
        "* \"GaussConvolutedWithNumericIntegral\", same, but uses the numeric\n"
        "integral (for MPE). This is VERY slow but (supposedly) accurate;\n"
        "* \"BoxConvoluted\": convolute with a 'box', not with a Gauss;\n"
        "* <name_of_photonics_service>: if you'd like to use a photorec or\n"
        "photospline service to get the PDFs and the expected number of PE.\n",
        peprob_ );

    icefile_ = "";
    AddParameter( icefile_optionname,
                  "Give a file with layer information for I3Medium.\n"
                  "(default: empty string, no layers).",
                  icefile_ );

    icemodel_ = 2;
    AddParameter( icemodel_optionname,
#ifdef ALL_HOLE_ICE_MODELS
                  "Specify the parameter set for ice properties etc.:\n"
                  "0, 1, 2 (default), 3 or 4. You can save compilation time\n"
                  "by removing -DALL_HOLE_ICE_MODELS from the ADD_DEFINITIONS\n"
                  "line in CMakeLists.txt & rebuild. Then this option becomes\n"
                  "dummy, the hole ice is just always 2 (speeds up build time\n"
                  "with a factor of 5).",
#else
                  "Specify the parameter set for ice properties etc.:\n"
                  "0, 1, 2 (default), 3 or 4.\n"
                  "However, it looks like your lilliput project build has been\n"
                  "compiled without support for non-default ice models. If you\n"
                  "would like to use some other ice model than number 2, then\n"
                  "add -DALL_HOLE_ICE_MODELS to the ADD_DEFINITIONS line in\n"
                  "CMakeLists.txt, and rebuild. Warning: this increases the\n"
                  "the build time with a factor of 5!",
#endif
                  icemodel_ );

    absorption_ = 98.0*I3Units::m;
    AddParameter( absorption_optionname,
                  "Average absorption length of Cherenkov light in ice.\n"
                  "Default: 98m",
                  absorption_ );

    jitter_ = 15.0*I3Units::ns;
    AddParameter( jitter_optionname,
                  "Pandel jitter value: accounts for both PMT jitter and\n"
                  "light model uncertainty. Default: 15ns",
                  jitter_ );

    inputreadout_ = "RecoPulse";
    AddParameter( inputreadout_optionname,
                  "which OMResponses to use",
                  inputreadout_ );

    noiseProb_ = 1.e-9*I3Units::hertz;
    AddParameter( noiseprob_optionname,
                  "You should set this to a positive value. For\n"
                  "each hit DOM a noise probability is added to the\n"
                  "likelihood. Note that the default value (1.0e-9Hz)\n"
                  "does not make physical sense, and also that efforts\n"
                  "are underway to achieve a better noise treatment.",
                  noiseProb_ );

    eventTypeString_ = "InfiniteMuon";
    AddParameter( eventtype_optionname,
                  "Type of event: InfiniteMuon (default), PointCascade or\n"
                  "DirectionalCascade.",
                  eventTypeString_ );

}

// Destructors

I3GulliverIPDFPandelFactory::~I3GulliverIPDFPandelFactory(){
    log_trace( "The GulliverIPDFPandel service \"%s\" was installed in %d contexts",
              name_.c_str(), ncontext_ );
}

// Local convenience functions, for instantiation of gazillions of possible
// templated types based on configuration options.



// spptype = Single Photon Probability type (aka PDF)
template<typename spptype, typename eventtype>
I3EventLogLikelihoodBasePtr
I3GulliverIPDFPandelFactory::GetNoisyLLH( boost::shared_ptr<spptype> spp ){
    assert(noiseProb_>0);
    if ( likelihood_ == "SPEAll" ){
        boost::shared_ptr< AllOMsLikelihoodWithConstNoise< SPEAll< spptype > > >
            ipdfptr ( new AllOMsLikelihoodWithConstNoise< SPEAll< spptype > >( *new SPEAll<spptype>( spp ), noiseProb_ ) );
        return I3EventLogLikelihoodBasePtr( new I3GulliverIPDFPandel< AllOMsLikelihoodWithConstNoise< SPEAll< spptype > >, eventtype >(name_, ipdfptr, inputreadout_) );
    } else if ( likelihood_ == "SPEqAll" ){
        boost::shared_ptr< AllOMsLikelihoodWithConstNoise< SPEqAll< spptype > > >
            ipdfptr ( new AllOMsLikelihoodWithConstNoise< SPEqAll< spptype > >( *new SPEqAll<spptype>( spp ), noiseProb_ ) );
        return I3EventLogLikelihoodBasePtr( new I3GulliverIPDFPandel< AllOMsLikelihoodWithConstNoise< SPEqAll< spptype > >, eventtype >(name_, ipdfptr, inputreadout_) );
    } else if ( likelihood_ == "SPE1st" ){
        boost::shared_ptr< AllOMsLikelihoodWithConstNoise< SPE1st< spptype > > >
            ipdfptr ( new AllOMsLikelihoodWithConstNoise< SPE1st< spptype > >( *new SPE1st<spptype>( spp ), noiseProb_ ) );
        return I3EventLogLikelihoodBasePtr( new I3GulliverIPDFPandel< AllOMsLikelihoodWithConstNoise< SPE1st< spptype > >, eventtype >(name_, ipdfptr, inputreadout_) );
    } else if ( likelihood_ == "MPE" ){
        boost::shared_ptr< AllOMsLikelihoodWithConstNoise< MPE< spptype > > >
            ipdfptr ( new AllOMsLikelihoodWithConstNoise< MPE< spptype > >( *new MPE<spptype>( spp, mpeTimingError_ ), noiseProb_ ) );
        return I3EventLogLikelihoodBasePtr( new I3GulliverIPDFPandel< AllOMsLikelihoodWithConstNoise< MPE< spptype > >, eventtype >(name_, ipdfptr, inputreadout_) );
    } else if ( likelihood_ == "MPEAll" ){
        boost::shared_ptr< AllOMsLikelihoodWithConstNoise< MPEAll< spptype > > >
            ipdfptr ( new AllOMsLikelihoodWithConstNoise< MPEAll< spptype > >( *new MPEAll<spptype>( spp ), noiseProb_ ) );
        return I3EventLogLikelihoodBasePtr( new I3GulliverIPDFPandel< AllOMsLikelihoodWithConstNoise< MPEAll< spptype > >, eventtype >(name_, ipdfptr, inputreadout_) );
    } else if ( likelihood_ == "PSA" ){
        boost::shared_ptr< AllOMsLikelihoodWithConstNoise< PSA< spptype > > >
            ipdfptr ( new AllOMsLikelihoodWithConstNoise< PSA< spptype > >( *new PSA<spptype>( spp ), noiseProb_ ) );
        return I3EventLogLikelihoodBasePtr( new I3GulliverIPDFPandel< AllOMsLikelihoodWithConstNoise< PSA< spptype > >, eventtype >(name_, ipdfptr, inputreadout_) );
    }

    log_fatal( "(%s) Config error: %s=\"%s\"\n"
               "(should be one of SPEAll, SPEqAll, SPE1st, MPE, MPEAll, PSA)",
               name_.c_str(), likelihood_optionname.c_str(),
               likelihood_.c_str() );

    // obligatory return
    return I3EventLogLikelihoodBasePtr();
}

// spptype = Single Photon Probability type (aka PDF)
template<typename spptype, typename eventtype>
I3EventLogLikelihoodBasePtr
I3GulliverIPDFPandelFactory::GetLLH( boost::shared_ptr<spptype> spp ){
    if ( noiseProb_ > 0 ){
        return this->GetNoisyLLH<spptype, eventtype>( spp );
    }
    log_fatal( "Zero noise setting not enabled. Consider setting the \"%s\" "
               "configuration parameter to a very small number. Alternatively, "
               "you could rebuild this project with the setting "
               "-DREENABLE_OBSOLETE_STUFF=1 on the cmake cmdline)",
               noiseprob_optionname.c_str() );

    // obligatory return
    return I3EventLogLikelihoodBasePtr();
}

template<typename ice,typename eventtype>
I3EventLogLikelihoodBasePtr
I3GulliverIPDFPandelFactory::GetPeProbLLH(){
    boost::shared_ptr<ice> bulkice;
    boost::shared_ptr<LayeredIce<ice> > layered;
    if ( icefile_.empty() ){
        bulkice = boost::shared_ptr<ice>( new ice );
    } else {
        layered = boost::shared_ptr<LayeredIce<ice> >( new LayeredIce<ice>(icefile_) );
    }

    if (    ( peprob_ == "GaussConvoluted" )
         || ( peprob_ == "GaussConvolutedWithNumericIntegral")
         || ( peprob_ == "GaussConvolutedFastApproximation") ){

        if ( icefile_.empty() ){
            return GetLLH< GaussConvolutedPEP<ice>, eventtype >(
                boost::shared_ptr< GaussConvolutedPEP<ice> >(
                    new GaussConvolutedPEP<ice>(bulkice,jitter_,intCalcType_)));
        } else {
            return GetLLH< GaussConvolutedPEP< LayeredIce<ice> >, eventtype >(
                boost::shared_ptr< GaussConvolutedPEP< LayeredIce<ice> > >(
                    new GaussConvolutedPEP< LayeredIce<ice> >( layered, jitter_, intCalcType_)));
        }
    } else if ( peprob_ == "BoxConvoluted" ){
        if ( icefile_.empty() ){
            return GetLLH< BoxConvolutedPEP<ice>, eventtype >(
                boost::shared_ptr< BoxConvolutedPEP<ice> >(
                    new BoxConvolutedPEP<ice>(bulkice,jitter_)));
        } else {
            return GetLLH< BoxConvolutedPEP< LayeredIce<ice> >, eventtype >(
                boost::shared_ptr< BoxConvolutedPEP< LayeredIce<ice> > >(
                    new BoxConvolutedPEP< LayeredIce<ice> >( layered, jitter_)));
        }
    } else {
        log_fatal( "(%s) undefined Pandel PEProb %s", name_.c_str(), peprob_.c_str() );
    }

    log_fatal( "(%s) Config error: %s=\"%s\", %s=\"%s\"",
               name_.c_str(),
               likelihood_optionname.c_str(), likelihood_.c_str(),
               peprob_optionname.c_str(), peprob_.c_str() );

    // obligatory return
    return I3EventLogLikelihoodBasePtr();
}

template<typename ice>
I3EventLogLikelihoodBasePtr I3GulliverIPDFPandelFactory::GetLLHwithHypothesis(){
    if ( eventTypeString_ == "InfiniteMuon" ){
        return GetPeProbLLH<ice,IPDF::InfiniteMuon>();
    } else if ( eventTypeString_ == "PointCascade" ){
        typedef IPDF::Pandel::CascIce0<ice> casc0ice;
        return GetPeProbLLH<casc0ice,IPDF::PointCascade>();
    } else if ( eventTypeString_ == "DirectionalCascade" ){
        typedef IPDF::Pandel::CascIce0<ice> casc0ice;
        return GetPeProbLLH<casc0ice,IPDF::DirectionalCascade>();
    } else {
        log_fatal( "(%s) Config Error %s=%s "
                   "Should be InfiniteMuon, PointCascade or DirectionalCascade",
                   name_.c_str(),
                   eventtype_optionname.c_str(),
                   eventTypeString_.c_str() );
    }

    // obligatory return
    return I3EventLogLikelihoodBasePtr();
}


// Member functions

bool
I3GulliverIPDFPandelFactory::InstallService(I3Context& services){

    ++ncontext_;
    if ( eventllh_ ){
        log_trace( "using existing instantiation for %s", name_.c_str() );
        return services.Put< I3EventLogLikelihoodBase >( eventllh_, name_ );
    }

    if ( context_.Has<I3PhotonicsService>(peprob_) ){
        log_info("found photonics service %s, will use PhotoSplinePEP",peprob_.c_str());
        psp_ = context_.Get< boost::shared_ptr<I3PhotonicsService> >(peprob_);
        if ( ! psp_ ){
            log_fatal( "There is something named \"%s\" in the tray/context, "
                       "but it cannot be casted to an I3PhotonicsService.",
                       peprob_.c_str() );
        }
        boost::shared_ptr< PhotoSplinePEP > pspep(new PhotoSplinePEP(psp_,peprob_));
        if ( eventTypeString_ == "InfiniteMuon" ){
            eventllh_ = GetLLH< PhotoSplinePEP,IPDF::InfiniteMuon>(pspep);
        } else if ( eventTypeString_ == "DirectionalCascade" ){
            eventllh_ = GetLLH< PhotoSplinePEP,IPDF::DirectionalCascade>(pspep);
        } else {
            log_fatal( "event type %s is not supported with PhotoSpline, "
                       "please try \"InfiniteMuon\" or \"DirectionalCascade\".",
                       eventTypeString_.c_str() );
        }
    } else {
        psp_ = boost::shared_ptr<I3PhotonicsService>();
        log_info("no photonics service %s found, will try to use Pandel",peprob_.c_str());

#ifdef ALL_HOLE_ICE_MODELS
        switch( icemodel_ ){
        case 0:
            eventllh_ = GetLLHwithHypothesis<IPDF::Pandel::H0>();
            break;
        case 1:
            eventllh_ = GetLLHwithHypothesis<IPDF::Pandel::H1>();
            break;
        case 2:
            eventllh_ = GetLLHwithHypothesis<IPDF::Pandel::H2>();
            break;
        case 3:
            eventllh_ = GetLLHwithHypothesis<IPDF::Pandel::H3>();
            break;
        case 4:
            eventllh_ = GetLLHwithHypothesis<IPDF::Pandel::H4>();
            break;
        default:
            log_fatal( "(%s) invalid icemodel parameter %d (should be 0..4)",
                       name_.c_str(), icemodel_ );
            break;
        }
#else
        assert( icemodel_ == 2 );
        eventllh_ = GetLLHwithHypothesis<IPDF::Pandel::H2>();
#endif
    }

    assert( eventllh_ );

    log_trace( "PandelIPDF(%s): OK!", name_.c_str() );
    return services.Put< I3EventLogLikelihoodBase >( eventllh_, name_ );
}


void I3GulliverIPDFPandelFactory::Configure() {

    name_ = GetName();
    GetParameter( likelihood_optionname, likelihood_ );
    GetParameter( mpetimingerror_optionname, mpeTimingError_ );
    GetParameter( peprob_optionname, peprob_ );
    GetParameter( icefile_optionname, icefile_ );
    GetParameter( icemodel_optionname, icemodel_ );
    GetParameter( absorption_optionname, absorption_ );
    GetParameter( jitter_optionname, jitter_ );
    GetParameter( inputreadout_optionname, inputreadout_ );
    GetParameter( noiseprob_optionname, noiseProb_ );
    GetParameter( eventtype_optionname, eventTypeString_ );

    log_debug( "(%s) configuring IPDF Pandel:", name_.c_str() );
    log_debug( "likelihood model (%s): \"%s\"",
              likelihood_optionname.c_str(), likelihood_.c_str() );
    log_debug( "MPE timing error (%s) (ns): %f",
              mpetimingerror_optionname.c_str(), mpeTimingError_ / I3Units::ns );
    log_debug( "PEProb algorithm (%s): \"%s\"",
              peprob_optionname.c_str(), peprob_.c_str() );
    log_debug( "icefile (ice layers) (%s): \"%s\"",
              icefile_optionname.c_str(), ( icefile_.empty() ? "" : icefile_.c_str() ) );
    log_debug( "ice model (%s): H%d",
              icemodel_optionname.c_str(), icemodel_ );
    log_debug( "absorption length (%s) (m): %f",
              absorption_optionname.c_str(), absorption_ / I3Units::m );
    log_debug( "jitter (%s) (ns): %f",
              jitter_optionname.c_str(), jitter_ / I3Units::m );
    log_debug( "input readout (%s): \"%s\"",
              inputreadout_optionname.c_str(), inputreadout_.c_str() );
    log_debug( "noise probability (%s): %g",
              noiseprob_optionname.c_str(), noiseProb_ );
    log_debug( "InfiniteMuon/{Point|Directional}Cascade (%s): %s",
              eventtype_optionname.c_str(), eventTypeString_.c_str() );

#ifndef ALL_HOLE_ICE_MODELS
    if ( icemodel_ != 2 ){
        log_fatal( "(%s) if you want to use hole ice model %d\n"
                   "then change CMakeLists.txt, add -DALL_HOLE_ICE_MODELS\n"
                   "to the ADD_DEFINITIONS line and recompile.",
                   name_.c_str(), icemodel_);
    }
#endif

    // silly checks

    bool layered = !icefile_.empty();
    if ( layered && ( peprob_ != "GConvolute")
                 && ( peprob_ != "GaussConvoluted" )
                 && ( peprob_ != "GaussConvolutedWithNumericIntegral") 
                 && ( peprob_ != "GaussConvolutedFastApproximation") ){
        log_error( "(%s): you provided a filename %s for layered ice",
                   name_.c_str(), icefile_.c_str() );
        log_fatal( "(%s) layers are only defined for PEProb \"GConvolute\", "
                   "\"GaussConvoluted\", "
                   "\"GaussConvolutedWithNumericIntegral\" or "
                   "\"GaussConvolutedFastApproximation\" "
                   "but I got %s=\"%s\"",
                   name_.c_str(), peprob_optionname.c_str(), peprob_.c_str() );
    }
    if ( absorption_ <= 0 ){
        log_fatal( "(%s) absorption length must be positive, but I got %f m",
                   name_.c_str(), absorption_ / I3Units::m  );
    }
    if ( jitter_ <= 0 ){
        log_fatal( "(%s) jitter should be positive, but I got %f",
                   name_.c_str(), jitter_ / I3Units::ns );
    }
    if ( (absorption_ < 10.0 * I3Units::m )
            || (absorption_ > 1000.0 * I3Units::m ) ){
        log_warn( "WARNING(%s) : got unlikely value for absorption length: %f m",
                  name_.c_str(), absorption_ / I3Units::m  );
        log_warn( "WARNING(%s): Please check the units in your runscript!",
                  name_.c_str() );
    }
    if (   (jitter_ < 1.0 * I3Units::ns )
        || (jitter_ > 100.0 * I3Units::ns ) ){
        log_warn( "WARNING(%s): got unlikely value for jitter: %f ns",
                  name_.c_str(), jitter_ / I3Units::ns );
        log_warn( "WARNING(%s): Please check the units in your runscript!",
                  name_.c_str() );
    }
    if ( inputreadout_.empty() ){
        log_fatal( "(%s) hey! you should set the %s option",
                   name_.c_str(), inputreadout_optionname.c_str() );
    }
    if ( ! ((noiseProb_ >= 0.0) && (noiseProb_<=1.0) ) ){
        log_fatal( "(%s) option %s: illegal noise prob value %g (must be >=0 and <=1)",
                   name_.c_str(), noiseprob_optionname.c_str(), noiseProb_ );
    }

    if ( peprob_ == "GaussConvolutedWithNumericIntegral" ){
        intCalcType_ = IPDF::Pandel::IntSlowNumeric;
    }

    if ( peprob_ == "GaussConvolutedFastApproximation"){
      intCalcType_ = IPDF::Pandel::IntFastApproximation; 
    }

    if ( mpeTimingError_ > 0.) {
        if (likelihood_ != "MPE" ){
            log_fatal("Got nonzero MPE timing error %g but likelihood %s != MPE",
                mpeTimingError_, likelihood_.c_str() );
        }
    } else {
        mpeTimingError_ = 0.;
    }


    if ( ( eventTypeString_ != "PointCascade" ) &&
         ( eventTypeString_ != "DirectionalCascade" ) &&
         ( eventTypeString_ != "InfiniteMuon" ) ){
        log_fatal( "(%s) Implemented event types are \"InfiniteMuon\" (default) "
                   "or \"PointCascade\" or \"DirectionalCascade\"; "
                   "you gave me \"%s\"",
                   GetName().c_str(), eventTypeString_.c_str() );
    }

    // the rest is checked later.

}
