/*
 *  $Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#include <boost/shared_ptr.hpp>
#include "credo/I3PoissonGaussLogLikelihoodFactory.h"
#include "credo/I3PoissonGaussLogLikelihood.h"

using namespace std;
using namespace boost;

I3_SERVICE_FACTORY(I3PoissonGaussLogLikelihoodFactory)

static std::string hypothesis_optname = "EventHypothesis";
static std::string input_optname = "InputPulses";
static std::string pdf_optname = "PDF";
static std::string noiserate_optname = "NoiseRate";
static std::string elength_optname = "EventLength";
static std::string activevolume_optname = "ActiveVolume";
static std::string basecontrib_optname = "UseBaseContributions";
//static std::string skipw8_optname = "SkipWeights";
//static std::string midpulse_optname = "MidPulse";
static std::string gausserror_optname = "GaussianErrorConstant";
static std::string mincharge_optname = "MinChargeFraction";
static std::string saturationLimit_optname = "SaturationLimitIceCube";
static std::string photonicsSaturation_optname = "PhotonicsSaturation";
static std::string badDOMListName_optname = "BadDOMListInFrame";
static std::string badDOMList_optname = "BadDOMList";
static std::string atwdonly_optname = "ATWDOnly";
static std::string useEmptyPulses_optname = "UseEmptyPulses";
static std::string useIC40Correction_optname = "UseIC40Correction";
static std::string light_scale_optname = "LightScale";
//----------------------------------------------------------------------------

I3PoissonGaussLogLikelihoodFactory::I3PoissonGaussLogLikelihoodFactory(const I3Context& ctx) :
        I3ServiceFactory(ctx) {

    ncontext_ = 0;

    AddParameter( pdf_optname.c_str(),
                  "PDF service to use for reconstruction",
                  pdfService_);

    inputPulses_ = "FEPulses";
    AddParameter( input_optname.c_str(),
                  "Name of the RecoPulses you want to use",
                  inputPulses_ );

    noiseRate_      = 700*I3Units::hertz;
    AddParameter( noiserate_optname.c_str(),
                  "The bare noise rate (without LC). "
                  "The old option was to specify noise probability. "
                  "The default for the rate is 700Hz.",
                  noiseRate_ );

    eventLength_   = 10000*I3Units::ns;
    AddParameter( elength_optname.c_str(),
                  "Event length (used to compute total expected noise charge).\n"
                  "If you put this to <=0, then the lenght will be defined as\n"
                  "the difference of the LE time of the latest pulse (plus its\n"
                  "width) and the LE time of the earliest hit.",
                  eventLength_ );

    // how far do you allow the OM to be away from the particle?
    activeVolume_ = 450.0 * I3Units::m;
    AddParameter( activevolume_optname.c_str(),
                  "Active volume; specified as the maximum distance from a\n"
                  "to the particle to calculate the PDF; for a hit further\n"
                  "away a noise probability will be used.",
                  activeVolume_ );
    useBaseContribution_ = true;
    AddParameter( basecontrib_optname.c_str(),
		  "Boolean value in order to use base contribution terms in the likelihood",
		  useBaseContribution_);

    gaussianErrorConstant_ = 1000;
    AddParameter( gausserror_optname.c_str(),
                  "constant defines the error of the gaussian pulses",
		          gaussianErrorConstant_ );
    
    minChargeFraction_ = 0;
    AddParameter( mincharge_optname.c_str(),
                  "fraction of the charge of the brightes DOM that each DOM must have to contribute",
		          minChargeFraction_ );

    saturationLimit_ = 0;
    AddParameter( saturationLimit_optname.c_str(),
                  "IceCube DOMs with a total charge bigger then this limit are saturated",
		          saturationLimit_ );

    photonicsSaturation_ = false;
    AddParameter( photonicsSaturation_optname.c_str(),
                  "restrict photonics prediction to configured saturation limits",
		          photonicsSaturation_ );
  
    badDOMListName_=""; 
    AddParameter(badDOMListName_optname.c_str(), 
                 "Name of the list of bad DOMs in the frame",
                 badDOMListName_);
    
    badDOMListName_.clear();
    AddParameter(badDOMList_optname.c_str(), 
                 "list of bad DOM OMKeys",
                 badDOMList_);
    
   onlyATWD_ = false;
   AddParameter( atwdonly_optname.c_str(),
                 "Boolean value in order to indicate that only pulses from"
                 "the ATWD are in the RecoPulseMap",
		         onlyATWD_ );

   useEmptyPulses_ = false;
   AddParameter( useEmptyPulses_optname.c_str(),
                 "Boolean value that controls whether gaps in the pulseseriesmaps are filled with pulses of charge 0.",
		         useEmptyPulses_ );

   useIC40Correction_ = false;
   AddParameter( useIC40Correction_optname.c_str(),
                 "Boolean value that decides, whether Eike's IC40 photonics amplitude corrections\n"
                 "should be applied",
                         useIC40Correction_);
   
   light_scale_ = 1.;
   AddParameter( light_scale_optname.c_str(),
                 "Modify the photonics charge prediction by this constant factor.\n",
                 light_scale_);
  
}

I3PoissonGaussLogLikelihoodFactory::~I3PoissonGaussLogLikelihoodFactory(){
    log_trace( "The I3PoissonGaussLogLikelihood service \"%s\" was installed in %d contexts",
              GetName().c_str(), ncontext_ );
}

//----------------------------------------------------------------------------
void I3PoissonGaussLogLikelihoodFactory::Configure() {

    GetParameter( input_optname.c_str(), inputPulses_ );
    GetParameter( noiserate_optname.c_str(), noiseRate_ );
    GetParameter( elength_optname.c_str(), eventLength_ );
    GetParameter( pdf_optname.c_str(), pdfService_);
    GetParameter( activevolume_optname.c_str(), activeVolume_ );
    GetParameter( basecontrib_optname.c_str(), useBaseContribution_ );
    GetParameter( gausserror_optname.c_str(), gaussianErrorConstant_);
    GetParameter( mincharge_optname.c_str(), minChargeFraction_);
    GetParameter( saturationLimit_optname.c_str(), saturationLimit_);
    GetParameter( photonicsSaturation_optname.c_str(), photonicsSaturation_);
    GetParameter( badDOMListName_optname.c_str(), badDOMListName_);
    GetParameter( badDOMList_optname.c_str(), badDOMList_);
    GetParameter( atwdonly_optname.c_str(), onlyATWD_);
    GetParameter( useEmptyPulses_optname.c_str(), useEmptyPulses_);
    GetParameter( useIC40Correction_optname.c_str(), useIC40Correction_);
    GetParameter( light_scale_optname.c_str(), light_scale_);

    if(useIC40Correction_){
      log_fatal("This option is no longer supported.");
    }

    if ( noiseRate_ >= 0 ){
        log_info( "will use noise rate = %f Hz", noiseRate_ / I3Units::hertz );
    } else {
        log_fatal( "(%s) Got negative noise rate: %f Hz",
                   GetName().c_str(), noiseRate_ / I3Units::hertz );
    }

}

bool I3PoissonGaussLogLikelihoodFactory::InstallService(I3Context& services){

    // check if the factory already created the service once
    if ( llhservice_ ){
        ++ncontext_;
        return services.Put<I3EventLogLikelihoodBase>( llhservice_, GetName());
    }

    // error checking for input parameters
    if ( inputPulses_.empty() ){
        log_fatal( "(%s) Please specify an input readout with the \"%s\" "
                   "option", GetName().c_str(), input_optname.c_str() );
        return false;
    }

    if (!pdfService_) {
            log_fatal( "(%s) error PDF not available",
                       GetName().c_str());
	    return false;
    }

    // create and configure the likelihood service
    llhservice_ = I3PoissonGaussLogLikelihoodPtr ( 
                     new I3PoissonGaussLogLikelihood(GetName(), pdfService_, inputPulses_,
                         noiseRate_, eventLength_, activeVolume_, 
                         gaussianErrorConstant_, onlyATWD_, useBaseContribution_,
                         useEmptyPulses_, light_scale_) );

    llhservice_->SetSaturation(minChargeFraction_, 
                               saturationLimit_, 
                               photonicsSaturation_);

    llhservice_->SetBadDOMs(badDOMListName_, badDOMList_);
                                                 
    // ... and put it into the context
    bool success = services.Put<I3EventLogLikelihoodBase >( llhservice_, GetName());
    ++ncontext_;
    log_debug( success ? "done" : "failed...?? :-/" );
    return success;

}
