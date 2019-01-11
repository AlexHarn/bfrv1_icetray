/**
 * @brief implementation of the I3GulliverFinitePhPnhFactory class
 *
 * @file I3GulliverFinitePhPnhFactory.cxx
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This Service places an I3GulliverFinitePhPnh object in the frame. Various parameters have to be set to use this PDF.
 */

#include "icetray/IcetrayFwd.h"
#include "icetray/I3Context.h"
#include "finiteReco/I3GulliverFinitePhPnh.h"
#include "finiteReco/I3GulliverFinitePhPnhFactory.h"
#include "gulliver/I3PDFBase.h"
#include "finiteReco/probability/PhPnhProbBase.h"
#include "finiteReco/probability/PhPnhParam.h"
#include "finiteReco/probability/PhPnhMCDist.h"
#include "finiteReco/probability/PhPnhPhotorec.h"
#include "finiteReco/probability/PhPnhPhotorecCone.h"
#include "photonics-service/I3PhotonicsServiceFactory.h"

// some icetray thingy, similar to ClassImp in ROOT
I3_SERVICE_FACTORY(I3GulliverFinitePhPnhFactory)

static unsigned int installcounter(0);
static const std::string input_optionname = "InputReadout";
static const std::string photorec_optionname = "PhotorecName";
static const std::string onlyInfinite_optionname = "OnlyInfiniteTables";
static const std::string finiteDefaultLength_optionname = "finiteDefaultLength";
static const std::string noiserate_optionname = "NoiseRate";
static const std::string defaultEventDuration_optionname = "DefaultEventDuration";
static const std::string absorption_optionname = "AbsorptionLength";
static const std::string flagStringLLH_optionname = "StringLLH";
static const std::string rCylinder_optionname = "RCylinder";
static const std::string SelectStrings_optionname = "SelectStrings";
static const std::string Prob_optionname = "ProbName";
static const std::string ProbFile_optionname = "InputProbFile";
static const std::string ProbMultiDet_optionname = "DOMHitProb";
static const std::string useOnlyFirstHit_optionname = "UseOnlyFirstHit";

I3GulliverFinitePhPnhFactory::I3GulliverFinitePhPnhFactory(const I3Context &context):
        I3ServiceFactory(context){

    useSignalsFrom_ = "RecoPulse";
    AddParameter( input_optionname,
                  "Give a label with which I can get I3RecoPulses from an event.",
                  useSignalsFrom_ );

    noiseRate_ = 500.0 * I3Units::hertz;
    AddParameter( noiserate_optionname,
                  "Average noise rate of DOMs (multiply with I3Units::hertz!).",
                  noiseRate_ );

    defaultEventDuration_ = 10000.0 * I3Units::ns;
    AddParameter( defaultEventDuration_optionname,
                  "Default event duration in ns. This value is used if the information can't be read from the event header.",
                  defaultEventDuration_ );

    probName_ = "PhPnhParam";
    AddParameter(Prob_optionname,
                 "How to calculate probabilities: 'PhPnhMCDist' or 'PhPnhParam' or 'PhPnhPhotorec' or 'PhPnhPhotorecCone'. See documentation.",
                 probName_);
    
    inputProbFile_ = "";
    AddParameter(ProbFile_optionname,
                 "File with the probabilities of a hit at certain Cherenkov distances. (Only required with 'PhPnhMCDist').",
                 inputProbFile_);

    namePhotorec_ = "";
    AddParameter( photorec_optionname,
                  "Give the photorec name. (Only required with 'PhPnhPhotorec' and 'PhPnhPhotorecCone').",
                  namePhotorec_ );

    onlyInfiniteTables_ = false;
    AddParameter( onlyInfinite_optionname,
                  "Use only infinite tables. (Only required with 'PhPnhPhotorec and 'PhPnhPhotorecCone').",
                  onlyInfiniteTables_ );

    finiteDefaultLength_ = 2*I3Units::km;
    AddParameter( finiteDefaultLength_optionname,
                  "Default length for finite photonics calls. (Only required with 'PhPnhPhotorec and 'PhPnhPhotorecCone').",
                  finiteDefaultLength_);

    absorptionLength_ = 98.0*I3Units::m;
    AddParameter( absorption_optionname,
                  "Absorption length. (Only required with 'PhPnhParam').",
                  absorptionLength_ );
    
    flagStringLLH_ = "false";
    AddParameter( flagStringLLH_optionname,
                  "Use StringLLH (correction for HLC while calculating probabilities)?",
                  flagStringLLH_ );

    rCylinder_ = 300*I3Units::m;
    AddParameter( rCylinder_optionname,
                  "Radius of the cylinder.",
                  rCylinder_ );

    for(int i=1;i<=80;i++){
      selectedStrings_.push_back(i);
    }
    AddParameter( SelectStrings_optionname,
                  "Strings used.",
                  selectedStrings_ );

    probMultiDet_ = 0.1;
    AddParameter( ProbMultiDet_optionname,
                  "All DOMs with a hit probability higher than this vale are included in the multiplicity.",
                  probMultiDet_ );
    
    useOnlyFirstHit_ = true;
    AddParameter( useOnlyFirstHit_optionname,
                  "If 'True' the result only depends on whether a DOM is hit or not. For 'False' also the number of hits is taken into account",
                  useOnlyFirstHit_ );

}

I3GulliverFinitePhPnhFactory::~I3GulliverFinitePhPnhFactory(){
    log_trace( "The I3GulliverFinitePhPnh service \"%s\" was installed in %d contexts",
               name_.c_str(), installcounter );
}

bool I3GulliverFinitePhPnhFactory::InstallService(I3Context& services){

    ++installcounter;
    if ( finitePhPnh_ ){
        return services.Put<I3EventLogLikelihoodBase>( finitePhPnh_, name_ );
    }

    if ( useSignalsFrom_.empty() ){
        log_error( "ERROR: you did not specify a provider of I3RecoPulses" );
        log_error( "Please do that with the \"UseSignalsFrom\"" );
        return false;
    }

    // Get the probabilities for hits
    PhPnhProbBasePtr prob;
    if(probName_ == "PhPnhMCDist"){
      if(inputProbFile_ == "") log_fatal("No table with probabilities given!");
      prob = PhPnhMCDistPtr(new PhPnhMCDist(inputProbFile_));
    }
    else if(probName_ == "PhPnhParam"){
      prob = PhPnhParamPtr(new PhPnhParam(557*I3Units::m,absorptionLength_));
    }
    else if(probName_ == "PhPnhPhotorec"){
      if(namePhotorec_ != ""){
        bool prOK = context_.Has< I3PhotonicsService >( namePhotorec_ );
        if (!prOK) log_fatal("Wrong name for photorec");
        I3PhotonicsServicePtr photorecPtr(context_.Get< I3PhotonicsServicePtr >( namePhotorec_ ));
        prob = PhPnhPhotorecPtr(new PhPnhPhotorec(photorecPtr,finiteDefaultLength_,onlyInfiniteTables_));
      }
      else log_fatal("PhotorecServiceName not given!");
    }
    else if(probName_ == "PhPnhPhotorecCone"){
      if(namePhotorec_ != ""){
        bool prOK = context_.Has< I3PhotonicsService >( namePhotorec_ );
        if (!prOK) log_fatal("Wrong name for photorec");
        I3PhotonicsServicePtr photorecPtr(context_.Get< I3PhotonicsServicePtr >( namePhotorec_ ));
        prob = PhPnhPhotorecConePtr(new PhPnhPhotorecCone(photorecPtr,finiteDefaultLength_,onlyInfiniteTables_));
      }
      else log_fatal("PhotorecServiceName not given!");
    }
    else log_fatal("no probabilities given!!");
    

    bool flagStringLLH = false;
    if (flagStringLLH_=="true") flagStringLLH = true;
    finitePhPnh_ = I3GulliverFinitePhPnhPtr(
         new I3GulliverFinitePhPnh( name_,
                                    useSignalsFrom_,
                                    noiseRate_,
                                    defaultEventDuration_,
                                    rCylinder_,
                                    flagStringLLH,
                                    selectedStrings_,
                                    prob,
                                    useOnlyFirstHit_,
                                    probMultiDet_) );
    
    return services.Put<I3EventLogLikelihoodBase>( finitePhPnh_, name_ );

}

void I3GulliverFinitePhPnhFactory::Configure(){

    name_ = GetName();
    GetParameter( input_optionname, useSignalsFrom_ );
    GetParameter( photorec_optionname, namePhotorec_ );
    GetParameter( onlyInfinite_optionname, onlyInfiniteTables_ );
    GetParameter( finiteDefaultLength_optionname, finiteDefaultLength_ );
    GetParameter( noiserate_optionname, noiseRate_ );
    GetParameter( defaultEventDuration_optionname, defaultEventDuration_ );
    GetParameter( absorption_optionname, absorptionLength_ );
    GetParameter( flagStringLLH_optionname, flagStringLLH_ );
    GetParameter( rCylinder_optionname, rCylinder_ );
    GetParameter( SelectStrings_optionname, selectedStrings_ );
    GetParameter( Prob_optionname, probName_ );
    GetParameter( ProbFile_optionname, inputProbFile_ );
    GetParameter( ProbMultiDet_optionname, probMultiDet_ );
    GetParameter( useOnlyFirstHit_optionname, useOnlyFirstHit_ );

    log_info( "input hits (%s): \"%s\"",
              input_optionname.c_str(), useSignalsFrom_.c_str() );
    log_info( "noise rate (%s): %f Hz",
              noiserate_optionname.c_str(), noiseRate_ / I3Units::hertz );
    log_info( "default event duration: %f ns",
              defaultEventDuration_ / I3Units::ns );
    log_info( "Flag for (%s): %s",
              flagStringLLH_optionname.c_str(), flagStringLLH_.c_str());
    log_info( "Radius of the cylinder: %e",
              rCylinder_);
    log_info( "Selected probability for the calculation of the multiplicity is %e",
              probMultiDet_);
    log_info( "Selected probability calculation : %s", probName_.c_str());
    if (probName_ == "PhPnhPhotorec"){
      log_info( "Name of Photonics service: %s", namePhotorec_.c_str());
    }
}
