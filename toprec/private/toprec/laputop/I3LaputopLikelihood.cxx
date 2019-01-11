/**
 * @file I3LaputopLikelihood.cxx
 * @brief implementation of the I3LaputopLikelihood class
 *
 * (c) 2005 * the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author kath
 *
 */
#include <cassert>
#include "icetray/I3SingleServiceFactory.h"
#include "toprec/I3LaputopLikelihood.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/status/I3DOMStatus.h"
#include "dataclasses/StationKey.h"
#include "icetray/I3Frame.h"

#include "phys-services/I3Calculator.h"

#include <recclasses/I3LaputopParams.h>
#include "toprec/LateralFitFunctions.h"
#include "toprec/I3SnowCorrectionService.h"
#include "toprec/SnowCorrectionDiagnostics.h"

#include <boost/math/special_functions/erf.hpp>  // Erf is in here too
#include <boost/make_shared.hpp>


// Things that we used to get from ROOT
// #define LOG10E 0.434294481903251817
// #define DBL_MIN 2.22507e-308
#ifndef DBL_MIN
#ifdef  __DBL_MIN__
#define DBL_MIN __DBL_MIN__
#endif
#endif

// A test mode for exploring BORS!  Turn this on to manually set Tstage (from Monte Carlo only)
#define TESTMODE_WITH_TRUE_TSTAGE 0
#if TESTMODE_WITH_TRUE_TSTAGE
#include <simclasses/I3CorsikaShowerInfo.h>
#endif

// Defaults and Descriptors:
const std::string I3LaputopLikelihood::DEFAULT_DATA_READOUT_LABEL = "TopEvent_0";
const std::string I3LaputopLikelihood::DATA_READOUT_DESCRIPTION 
= "Label under which the data is found in DataReadoutDict";
const std::string I3LaputopLikelihood::DATA_READOUT_TAG = "datareadout";

const std::string I3LaputopLikelihood::DEFAULT_BADTANKLABEL = "";
const std::string I3LaputopLikelihood::BADTANKLABEL_DESCRIPTION
= "Name of the list of tanks that have been removed by I3TopEventBuilder";
const std::string I3LaputopLikelihood::BADTANKLABEL_TAG = "badtanks";

const int I3LaputopLikelihood::DEFAULT_TRIGGER = 5; // at least 5 stations
const std::string I3LaputopLikelihood::TRIGGER_DESCRIPTION
= "Minimum number of stations you want to require";
const std::string I3LaputopLikelihood::TRIGGER_TAG = "trigger";

const double I3LaputopLikelihood::DEFAULT_CORECUT = 11.0; // Cut away a whole station
const std::string I3LaputopLikelihood::CORECUT_DESCRIPTION
= "Radius from the core, within which to perform DYNAMIC treatment of tanks in LLH calculation (use llh value as if pulse was at THIS distance, does NOT do any cut on pulses)";
const std::string I3LaputopLikelihood::CORECUT_TAG = "dynamiccoretreatment";

const double I3LaputopLikelihood::DEFAULT_SOFT_THR = -1.; // Minimum VEM
const std::string I3LaputopLikelihood::SOFT_THR_DESCRIPTION
= "Apply a software threshold to all pulses. LC is recalculated.";
const std::string I3LaputopLikelihood::SOFT_THR_TAG = "softwarethreshold";

const std::string I3LaputopLikelihood::DEFAULT_LDF = "dlp";
const std::string I3LaputopLikelihood::LDF_DESCRIPTION 
= "Desired lateral density function (dlp = Double Logarithmic Parabola, nkg = NKG function or power = Powerlaw)";
const std::string I3LaputopLikelihood::LDF_TAG = "ldf";

const std::string I3LaputopLikelihood::DEFAULT_CURV = "gausspar";
const std::string I3LaputopLikelihood::CURV_DESCRIPTION 
= "Use also the timing information of defined funtion to fit core & direction in one go.\n\
                   Available options: gausspar, kislat or an empty string to turn off the combined fit.\n\
                   NOTE: \"kislat\" is experimental. Don't use.";
const std::string I3LaputopLikelihood::CURV_TAG = "curvature";

const std::string I3LaputopLikelihood::DEFAULT_SNOWSERVICENAME = "SimpleSnow";
const std::string I3LaputopLikelihood::SNOWSERVICENAME_DESCRIPTION 
= "Name of the SnowCorrection Service to use (Simple, BORS, etc.)";
const std::string I3LaputopLikelihood::SNOWSERVICENAME_TAG = "SnowServiceName";




// names of the fitting functions:
const std::string NKG_NAME = "nkg";
const std::string DLP_NAME = "dlp";
const std::string POWERLAW_NAME = "power";

// names of curvatures:
const std::string GAUSSPAR_NAME = "gausspar"; 
const std::string KISLAT_NAME = "kislat";
const std::string EMILY_NAME = "gaussparemily";
const std::string RAWLINS_NAME = "gaussparfree";

// constants for the llh
const double VEM_THRESHOLD = 0.1657;          // VEM threshold for p_nohit calculation
                                              // big source of uncertainty!! magic number (30PE / 181VEM/PE)

// and for the saturation llh
const double SAT_LG = 90000.;                 // Upper bound for LG in PE
const double SAT_HG = 3000.;                  // Upper bound for HG in PE (same as in topeventcleaning)



I3LaputopLikelihood::I3LaputopLikelihood( std::string name ):
  I3EventLogLikelihoodBase(), I3ServiceBase(name), multiplicity_(0) {

  // In this constructor, set defaults that will be helpful for running unit tests:
  // (Where one of these gets constructed without a context)
  fDataReadoutLabel = "IceTopVEMPulses_0";
  fMaxIntraStaTimeDiff_ = 80.0;  // This is used in the standard traysegment 
  fSaturation_ = true;
  fBadTankLabel = "BadTankList";

  fTrigger = DEFAULT_TRIGGER;
  fCoreCut = DEFAULT_CORECUT;
  fSoftwareThreshold = DEFAULT_SOFT_THR;
  fLDF = DEFAULT_LDF;
  fCurv = DEFAULT_CURV;
  fCurvType = Laputop::FrontDelay::None;
  fSnowServiceName = DEFAULT_SNOWSERVICENAME;
  fCorrectAtm_ = false;

}


// construct self & declare configuration parameters
I3LaputopLikelihood::I3LaputopLikelihood( const I3Context &context ):
    I3EventLogLikelihoodBase(), I3ServiceBase(context),multiplicity_(0){

    log_debug( "(%s) hey, this is the likelihood function for "
               "ICETOP", GetName().c_str() );

  fDataReadoutLabel = DEFAULT_DATA_READOUT_LABEL; // for the input
  fBadTankLabel = DEFAULT_BADTANKLABEL;
  fTrigger = DEFAULT_TRIGGER;  
  fCoreCut = DEFAULT_CORECUT;
  fSoftwareThreshold = DEFAULT_SOFT_THR;

  fLDF = DEFAULT_LDF;
  fCurv = DEFAULT_CURV;
  fCurvType = Laputop::FrontDelay::None;
  fSnowServiceName = DEFAULT_SNOWSERVICENAME;

  AddParameter (DATA_READOUT_TAG,
		DATA_READOUT_DESCRIPTION,
		fDataReadoutLabel);
  AddParameter (TRIGGER_TAG, TRIGGER_DESCRIPTION, fTrigger);
  AddParameter (BADTANKLABEL_TAG, BADTANKLABEL_DESCRIPTION, fBadTankLabel);
  AddParameter (CORECUT_TAG, CORECUT_DESCRIPTION, fCoreCut);
  AddParameter (SOFT_THR_TAG, SOFT_THR_DESCRIPTION, fSoftwareThreshold);
  AddParameter (LDF_TAG, LDF_DESCRIPTION, fLDF);
  AddParameter (CURV_TAG, CURV_DESCRIPTION, fCurv);
  AddParameter (SNOWSERVICENAME_TAG, SNOWSERVICENAME_DESCRIPTION, fSnowServiceName);

  fCorrectAtm_ = false;
  AddParameter("CorrectEnvironment",
	       "Correct expected signal for pressure (and temp)",
	       fCorrectAtm_);

  fSaturation_ = true;
  AddParameter("SaturationLikelihood",
	       "Use likelihood term for saturated signals",
	       fSaturation_);

  fMaxIntraStaTimeDiff_ = -1;
  AddParameter("MaxIntraStationTimeDiff",
	       "Don't use stations for timing likelihood when the difference in time is larger "
	       "than this value, all stations used when <0.",
	       fMaxIntraStaTimeDiff_);

}

// get configuration parameters
void I3LaputopLikelihood::Configure(){
  GetParameter (DATA_READOUT_TAG, fDataReadoutLabel);
  GetParameter (BADTANKLABEL_TAG, fBadTankLabel);
  GetParameter (TRIGGER_TAG, fTrigger);
  GetParameter (CORECUT_TAG, fCoreCut);
  GetParameter (SOFT_THR_TAG, fSoftwareThreshold);
  GetParameter (LDF_TAG, fLDF);
  GetParameter (CURV_TAG, fCurv);
  GetParameter (SNOWSERVICENAME_TAG, fSnowServiceName);
  GetParameter("CorrectEnvironment", fCorrectAtm_);
  GetParameter("SaturationLikelihood", fSaturation_);
  GetParameter("MaxIntraStationTimeDiff",fMaxIntraStaTimeDiff_);

  //if ((fLDF != NKG_NAME) && (fLDF != DLP_NAME) && (fLDF != POWERLAW_NAME))
  //  log_fatal("Lateral distribution function '%s' not supported!", fLDF.c_str());


  if (fTrigger < 3)
    log_fatal("Trigger should be set to >=3 stations");
  else if ((fTrigger < 5) && (fCurv != ""))
    log_fatal("When fitting the curvature at least 5 stations are needed.");

  // Set up the Snow Service (if it exists)
  bool snowserviceOK_ = context_.Has< I3SnowCorrectionServiceBase >( fSnowServiceName );
  if (snowserviceOK_) {
    log_info("I found the SnowCorrectionService %s and I'm loading it now.", fSnowServiceName.c_str());
    snowservice_ = context_.Get< I3SnowCorrectionServiceBasePtr >( fSnowServiceName );
  } else {
    log_warn("The SnowCorrectionService %s does not exist.", fSnowServiceName.c_str());
    // Install a service that does nothing to the signal
    snowservice_ = boost::shared_ptr<I3SnowCorrectionServiceBase>(new I3SnowCorrectionServiceBase("dummysnow"));   
  }

  // Convert the Curvature function name (string) into a Curvature function type
  // Maybe someday we can get rid of the string entirely and use ONLY the type?
  SetCurvature(fCurv);

}

void I3LaputopLikelihood::SetCurvature(std::string newCurv){ 
  fCurv=newCurv;
  if (fCurv == "" || fCurv == KISLAT_NAME) fCurvType = Laputop::FrontDelay::None;
  else if (fCurv == GAUSSPAR_NAME) fCurvType = Laputop::FrontDelay::GaussParabola;
  else if (fCurv == EMILY_NAME) fCurvType = Laputop::FrontDelay::GaussParabolaEmily;
  else if (fCurv == RAWLINS_NAME) fCurvType = Laputop::FrontDelay::GaussParabolaFromParams;
  else log_fatal("What kind of curvature function name is %s?", fCurv.c_str());
}

// APPLYING PERMANENT PULSE CUTS:
// This needs to be done after the pulses have been loaded, and after we know
// the SEED.  But NOT for every call of the likelihood function.
// These functions can only be called by the Fitter Module, since it has the event hypothesis.

/// OK, time to adapt some old functions to the modern world... 
/// Cut out tanks with bad timing:
/// Hey Tom, what exactly should we cut on?  Time Residual in usec?  Timing-LLH?
#if 0
int I3LaputopLikelihood::CutBadTimingPulses( const I3EventHypothesis &t, double t_res_cut ) {

  if (t_res_cut <= 0) {
    log_debug("Exiting CutBadTimingPulses without doing anything; %f", t_res_cut);
    return 0;
  }

  log_debug("Entering CutBadTimingPulses: %f", t_res_cut);
  int nremoved = 0;

  // Get track info (necessary for computing radius to track) 
  I3ParticleConstPtr track = t.particle;
  if ( ! track ){
    log_fatal( "(%s) I failed to get the I3Particle hypothesis",
	       GetName().c_str() );
  }
  const I3Direction &dir = track->GetDir();
  const I3Position &core = track->GetPos();
  double time = track->GetTime();
  
  // The regular hits
  std::vector<tankPulse>::iterator it;
  for (it = inputData_.begin(); it != inputData_.end(); /* BLANK... I will increment this myself */){
    double local_r = GetDistToAxis(core,dir,it);
    double planewave_delta_t = GetDistToPlane(time,core,dir,it);
    //double curved_delta_t = planewave_delta_t - LateralFitFunctions::top_curv_gausspar(local_r, NULL);

    double curved_delta_t;
    if (fCurv == ""){
        curved_delta_t = planewave_delta_t - LateralFitFunctions::top_curv_gausspar(local_r, NULL);
    }
    if(GAUSSPAR_NAME.compare(fCurv) == 0){
        curved_delta_t = planewave_delta_t - LateralFitFunctions::top_curv_gausspar(local_r, NULL);
    }
    else{
        if (EMILY_NAME.compare(fCurv) == 0){
                curved_delta_t = planewave_delta_t - LateralFitFunctions::top_curv_gausspar_emily(local_r, NULL);
         }
    }

    log_trace("(regular) R=%f, PW_tres=%f, gausspar=%f, C_tres=%f",
	      local_r, planewave_delta_t,  LateralFitFunctions::top_curv_gausspar(local_r, NULL), curved_delta_t);
    // Or likelihood instead?
    //double par[4] = {core.GetX(),core.GetY(),logS125,beta};
    //double curved_llh =  LateralFitFunctions::top_curv_gausspar_llh(local_r, planewave_delta_t, par);
    if (fabs(curved_delta_t) > t_res_cut) { // remove this one
      it = inputData_.erase(it);
      nremoved++;
      log_trace("Remove me! %d", nremoved);
    } else { ++it; }
  }
  // The saturated hits
  for (it = saturatedData_.begin(); it != saturatedData_.end(); /* BLANK... I will increment this myself */){
    double local_r = GetDistToAxis(core,dir,it);
    double planewave_delta_t = GetDistToPlane(time,core,dir,it);
    //double curved_delta_t = planewave_delta_t - LateralFitFunctions::top_curv_gausspar(local_r, NULL);
    double curved_delta_t;
    if (fCurv == ""){
        curved_delta_t = planewave_delta_t - LateralFitFunctions::top_curv_gausspar_emily(local_r, NULL);
    }
    if(GAUSSPAR_NAME.compare(fCurv) == 0){
        curved_delta_t = planewave_delta_t - LateralFitFunctions::top_curv_gausspar(local_r, NULL);
    }
    else{
        if (EMILY_NAME.compare(fCurv) == 0){
                curved_delta_t = planewave_delta_t - LateralFitFunctions::top_curv_gausspar_emily(local_r, NULL);
         }
    }


    log_trace("(saturated) R=%f, PW_tres=%f, gausspar=%f, C_tres=%f", 
	      local_r, planewave_delta_t,  LateralFitFunctions::top_curv_gausspar(local_r, NULL), curved_delta_t);
    if (fabs(curved_delta_t) > t_res_cut) { // remove this one
      it = saturatedData_.erase(it);
      nremoved++;
      log_trace("Remove me! %d", nremoved);
    } else { ++it; }
  }
  // The no-hits: leave them be, they have no timing likelihood...

  log_debug("BadTimingCut removed %d pulses total", nremoved);
  return nremoved;
}  

// This one does the "static 11-meter cut", and does it permanently.
// We can have it return the number of pulses removed.
int I3LaputopLikelihood::CutCorePulses( const I3EventHypothesis &t, double rcut ) {

  if (rcut <= 0) {
    log_debug("Exiting CutCorePulses without doing anything; %f", rcut);
    return 0;
  }
  
  log_debug("Entering CutCorePulses: %f", rcut);
  int nremoved = 0;

  // Get track info (necessary for computing radius to track)
  I3ParticleConstPtr track = t.particle;
  if ( ! track ){
    log_fatal( "(%s) I failed to get the I3Particle hypothesis",
	       GetName().c_str() );
  }
  const I3Direction &dir = track->GetDir();
  const I3Position &core = track->GetPos();

  // I don't understand why this is here
  // (It was part of I3TopLateralFit)... Changes the corecut based on Beta.
  /*
  I3TopLateralFitParamsConstPtr paramPtr = boost::dynamic_pointer_cast<I3TopLateralFitParams>(t.nonstd);
  if ( ! paramPtr ){
    log_fatal( "(%s) wrong event hypothesis: missing I3TopLateralFitParams",
	       GetName().c_str() );
  }
  const double beta = paramPtr->Beta;

  // I still don't quite understand what the point of this line is...
  rcut = std::max(rcut*I3Units::m,
		  LateralFitFunctions::R0_PARAM * pow(10, -beta/(2*LateralFitFunctions::KAPPA)));
  */

  // The regular hits
  std::vector<tankPulse>::iterator it;
  for (it = inputData_.begin(); it != inputData_.end(); /* BLANK... I will increment this myself */){
    double local_r = GetDistToAxis(core,dir,it);
    log_trace("This (regular) r: %f", local_r);
    if (local_r < rcut) { // remove this one
      it = inputData_.erase(it);
      nremoved++;
      log_trace("Remove me! %d", nremoved);
    } else { ++it; }
  }
  // The saturated hits
  for (it = saturatedData_.begin(); it != saturatedData_.end(); /* BLANK... I will increment this myself */){
    double local_r = GetDistToAxis(core,dir,it);
    log_trace("This (saturated) r: %f", local_r);
    if (local_r < rcut) { // remove this one
      it = saturatedData_.erase(it);
      nremoved++;
      log_trace("Remove me! %d", nremoved);
    } else { ++it; }
  }
  // The no-hits
  for (it = inputEmptyData_.begin(); it != inputEmptyData_.end(); /* BLANK... I will increment this myself */){
    double local_r = GetDistToAxis(core,dir,it);
    log_trace("This (empty) r: %f", local_r);
    if (local_r < rcut) { // remove this one
      it = inputEmptyData_.erase(it);
      nremoved++;
      log_trace("Remove me! %d", nremoved);
    } else { ++it; }
  }

  log_debug("CoreCut removed %d tanks total", nremoved);
  return nremoved;

}  
#endif


void I3LaputopLikelihood::SetEvent( const I3Frame &f ){
  log_debug("Entering likelihood: SetEvent");

  if(inputData_.size() == 0){ // this means new event, because I reset!
    nStation_ = FillInput(f);
  }


#if TESTMODE_WITH_TRUE_TSTAGE
  // If it's Monte Carlo,
  // Grab the CORSIKA info from the frame, and calculate the true tstage
  double true_tstage_ = -999;
  const I3CorsikaShowerInfoConstPtr corsikainfo = f.Get<I3CorsikaShowerInfoConstPtr>("MCPrimaryInfo"); 
  const I3ParticleConstPtr corsikaparticle = f.Get<I3ParticleConstPtr>("MCPrimary"); 
  if (corsikainfo && corsikaparticle) {
    double zenith = corsikaparticle->GetDir().GetZenith();
    // this bit copied from "ringMC_unisnow.C" 
    double X1stInt = corsikainfo->firstIntDepth/cos(zenith);  
    double Xmax = corsikainfo->ghMaxDepth/cos(zenith); 
    double Xobslevel = 6.92850355E+02/cos(zenith);  
    double slantX = Xobslevel - X1stInt;
    true_tstage_ = (slantX - Xmax)/36.7;
  } 
  log_info("In SetEvent: setting this event's true tstage: %f", true_tstage_);

  // This only works if it's BORS.  Make this more flexible someday?
  I3BORSSnowCorrectionServicePtr borsservice = boost::dynamic_pointer_cast<I3BORSSnowCorrectionService>(snowservice_);
  if (borsservice) borsservice->SetTStage(true_tstage_);
  else log_warn("Hey, you're using an experimental testmode that only works with BORS!  Not setting anything.");
#endif


 

  // Moved the multiplicity calculation here, so that it'll always get done at every step.
  // (Filling is still only done once)
  // As parameters (i.e. curvature) change from step to step, we want to compute this separately each time.
  multiplicity_ = inputData_.size();
 
  if(fSaturation_)
    multiplicity_ += saturatedData_.size();
  log_debug("I found multiplicity_ of %u", multiplicity_);
  log_debug("inputData_ still there? %zu", inputData_.size());
  log_debug("inputEmptyData_ still there? %zu", inputEmptyData_.size());
  if (multiplicity_ <= 0){
    log_debug("input pulses are missing.");
    return;
  }

  // How many pulses are going to be used in this likelihood? Set multiplicity_ correctly.
  // (It should be the "number of pieces of information" for fitting...)

  // "fTrigger" is a station-related trigger which will override gulliver's decision if
  // the event is below a certain number of stations hit. (optional for the user, default is 5 stations)

  // NStations with charge:
  //if(inputData_.size()<fTrigger){  // must be checked again foundPulseInStation! NOT the size of the inputData...
  if(nStation_ < fTrigger){
    log_debug("Too few stations with charge to make a Lateral Fit! %d", nStation_);
    multiplicity_ = 0;
    return;
    }

  // But also unhit tanks contribute to the llh (the no-hit llh is part of the total llh) :
  //multiplicity_ += inputEmptyData_.size(); // necessary for rlogl, but not for FitStatus and Gulliver checks, how to deal with this??

  // Depending on whether Curvature is used or not, multiplicity (pieces of information) changes
  if(fCurv !=""){
    // IF either Charge is bad or Time is bad, pulse is NOT used 
    // => Already checked once with nst_q in FillInput
    multiplicity_ += inputData_.size() - timeFluctuatingTanks_;
    
    // Also add the ones where we'll use the T but not the Q
    if(fSaturation_)
      multiplicity_ += saturatedData_.size();
    
    log_debug("Me curvy! (%s)", fCurv.c_str());
  }
  else log_debug("No curvature this time, yo. empty:(%s)", fCurv.c_str());
    
  log_debug("Sending this multiplicity_ to gulliver: %d", multiplicity_);
  log_debug("This number should be <= the number of available stations: %zu",
	    inputData_.size() + inputEmptyData_.size() + saturatedData_.size());

  return;
}


// A helper function used for updating "foundPulseInStation" and "foundLCInStation"
void I3LaputopLikelihood::UpdateFillCounts(std::set<StationKey> &setA, std::set<StationKey> &setB, OMKey key) {
   if(setA.find(key.GetString())!=setA.end()) {
    setB.insert(key.GetString());
  }
  else {
    setA.insert(key.GetString());
  }
}

// Another helper function... just print stuff.
void I3LaputopLikelihood::PrintPulseFill(tankPulse pulse, std::string description) {
  log_trace("At (%d %d) ------", pulse.omkey.GetString(), pulse.omkey.GetOM());
  log_trace("Adding a %s pulse....with snowdepth %f", description.c_str(), pulse.snowdepth);
  log_trace("Got a pulse time/pos of %f (%f, %f, %f)", pulse.t, pulse.x, pulse.y, pulse.z);
  log_trace("    PE: %f, Width: %f)", pow(10,pulse.logvem), pulse.width);
  log_trace("Using the timing? %d", pulse.usepulsetime);
}

//------------------------------------------------
unsigned int I3LaputopLikelihood::FillInput(const I3Frame &f){
  // This is the part where we fill an "inputShower" with pulses
  // and track info.
  
  if(!f.Has(fDataReadoutLabel)) {
    log_debug("No Pulses in Frame! (%s)", fDataReadoutLabel.c_str()); 
    return 0;
  }

  timeFluctuatingTanks_ = 0;
  // ----- get what is there
  const I3DetectorStatus &status = f.Get<I3DetectorStatus>();
  const I3Geometry &geometry = f.Get<I3Geometry>();
  const I3Calibration &calibration = f.Get<I3Calibration>(); // for saturation
  const I3OMGeoMap& om_map = geometry.omgeo;
  const std::map<OMKey, I3DOMStatus> &status_map = status.domStatus;
  const std::map<OMKey, I3VEMCalibration> &vemcal_map = calibration.vemCal;

  I3RecoPulseSeriesMapConstPtr pulse_series_map =
    f.Get<I3RecoPulseSeriesMapConstPtr> (fDataReadoutLabel);

  // Bad Stations: start with an empty list, and add to it later from the BadTanks list.
  // (Note: should be rewritten more gracefully.)
  I3VectorIntPtr badStations;  // start it null...
  badStations = I3VectorIntPtr(new I3VectorInt());

  // Similarly for bad tanks
  I3VectorTankKeyConstPtr badTanks = f.Get<I3VectorTankKeyConstPtr>(fBadTankLabel);
  if (badTanks) log_debug("%zu Bad Tanks", badTanks->size());
  else log_debug("No Bad Tanks");

  log_trace ("pulse_series_map has %zu entries", pulse_series_map->size ());
  I3StationGeoMap smap = geometry.stationgeo;

  // --- Pressure and temperature will be I3Doubles in the frame (until we know better where to put them)
  I3DoubleConstPtr pressure = f.Get<I3DoubleConstPtr>("Pressure");
  I3DoubleConstPtr temperature = f.Get<I3DoubleConstPtr>("Temperature");

  if(fCorrectAtm_ && !(pressure && temperature)){
    log_warn("No pressure and temperature found in the frame, switching correction OFF!");
    fCorrectAtm_ = false;
  }

  // ----- map to keep track which stations we have
  std::set<StationKey> foundPulseInStation;
  std::set<StationKey> foundLCInStation;

  // ----- loop over the DOMSs of the current detector setup
  log_debug ("Going to loop over DOMs");
  for (std::map<OMKey, I3DOMStatus>::const_iterator i_om = status_map.begin ();
       i_om != status_map.end (); ++i_om) {
    
    // ----- Get and check the OMGeo
    OMKey dom_key = i_om->first;
    I3OMGeo om = om_map.find(dom_key)->second;
    if (om.omtype != I3OMGeo::IceTop) continue; // inice or icetop?
    log_trace("OM is ontop: %d/%d", dom_key.GetString (), dom_key.GetOM ());

    // ----- Get this DOM's snow depth
    double thisdepth = 0;
    // Which station/tank is it in?
    TankKey tk(dom_key);
    unsigned int tankID = tk.tank==TankKey::TankA?0:1;     
    I3StationGeoMap::const_iterator siter = smap.find(dom_key.GetString());
    if(siter==smap.end()) {  
      log_fatal("Station %d doesn't exist in StationGeoMap!", dom_key.GetString()); 
    }
    // Which tank specifically within the station?
    const I3TankGeo& tankGeo = siter->second.at(tankID);
    thisdepth = tankGeo.snowheight;
    log_trace("Station %d, tank %d: depth: %f", dom_key.GetString(), tankID, thisdepth);

    // ----- Get and check its PulseSeries
    I3Map<OMKey, I3RecoPulseSeries>::const_iterator iter = pulse_series_map->find(dom_key);
    if (iter == pulse_series_map->end()) continue; // no series for that om?

    // Make sure this isn't in the BadStations or BadTanks lists
    if (badTanks) 
      if (std::find(badTanks->begin(), badTanks->end(), tk) != badTanks->end()) 
	log_fatal_stream("Pulse series contains a pulse in tank " << tk << ", which is in the BadTanks list!");
    

    // DOM is hit, now check for its calibration
    // ----- Get the vemCalib (for HG saturation and later for LG saturation?)
    if (vemcal_map.find(dom_key) == vemcal_map.end()) {
      log_fatal("No VEM calibration for module %s", dom_key.str().c_str());
    }
    I3VEMCalibration vemCalib = vemcal_map.find(dom_key)->second;
    double pe_per_vem = vemCalib.pePerVEM/vemCalib.corrFactor;
    double hg_sat = vemCalib.hglgCrossOver/pe_per_vem;
    // TODO : Saturation value per DOM, not difficult, just some work needed
    // USE fixed value to start with 
    double lg_sat = SAT_LG/pe_per_vem; 
    // Do it like topeventcleaning: use fallback cross-over value if negative or zero value in DB
    if (hg_sat <= 0) hg_sat = SAT_HG/pe_per_vem; 
    // Also need the gain for saturation :
    I3DOMStatus::DOMGain gain = i_om->second.domGainType;

    const I3RecoPulseSeries pulse_series = iter->second;
    if (0 == pulse_series.size()){ // no pulse in the series?
      log_debug ("pulse_series found, but empty.");
      continue;
    }
    else 
      log_trace("pulse_series succeeded to cast, has %zu entries", pulse_series.size ());

    // ----- Loop over the pulse series to extract the data
    // NOTE: Although this code it set up to loop over all pulses, 
    // there is a "break" at the end which will prevent the loop from being executed more
    // than once!
    // Only FIRST pulses are used.
    for (I3RecoPulseSeries::const_iterator it_pulse = pulse_series.begin ();
         it_pulse != pulse_series.end (); ++it_pulse) {


	// Actually fill the internal inputdata container with HIT tanks
	// This is the "default way of filling".
	tankPulse new_pulse;
	new_pulse.omkey = dom_key; 
	new_pulse.x = (om.position).GetX();
	new_pulse.y = (om.position).GetY();
	new_pulse.z = (om.position).GetZ();
	new_pulse.t = it_pulse->GetTime();
	new_pulse.width = it_pulse->GetWidth();
	new_pulse.logvem = log10(it_pulse->GetCharge());
	new_pulse.snowdepth = thisdepth;
	new_pulse.usepulsetime = true;

	//SortOutBadCharges() : Q > threshold and Q < lg_sat
	//+ SortOutBadTimes() : no NaN times !
	// Default threshold is 0., could be set higher for better threshold description and retriggering
	// This also implies that NO negative pulses are taken ! 
	// Which should anyway already have been cleaned by topeventcleaning/builder !
	if ((it_pulse->GetCharge() > fSoftwareThreshold) &&
	    it_pulse->GetCharge()<= lg_sat &&   
	    it_pulse->GetTime() == it_pulse->GetTime()
	    ){
	  
	  UpdateFillCounts(foundPulseInStation, foundLCInStation, dom_key);
	           

	  /* Decide whether to use the timing or NOT */
	  ////// There are these stations (both in MC and data) with huge timing fluctuations, dt > 100-150ns which we didn't cut with topeventcleaning (because there physics hits)
	  /////  The curvature fit is very confused by these and messes up both the timing and the ldf fit, and step 3 doesn't recover.
	  ///// Tried cutting these pulses before the fit, but the charges are ok, and these are NSta 5 events, so we lose 3% of NSta events that were good.
	  log_trace("MaxIntraStationTimeDiff = %f", fMaxIntraStaTimeDiff_);
	  if(fMaxIntraStaTimeDiff_ > 0){
	    // Look for neighbouring tank and get its pulsetime
	    unsigned int neighbourHG = 63;
	    if (tankID == 1) neighbourHG = 61;
	    // Look for a HG neighbor first:
	    I3Map<OMKey, I3RecoPulseSeries>::const_iterator neighbour_iter = pulse_series_map->find(OMKey(dom_key.GetString(),neighbourHG));
	    if (neighbour_iter == pulse_series_map->end()) // didn't find the HG in the neighbor... look for a LG instead
	      neighbour_iter = pulse_series_map->find(OMKey(dom_key.GetString(),neighbourHG+1));
	    if (neighbour_iter == pulse_series_map->end()) // didn't find a LG neighbor either!
	      log_info("MaxIntraStationTimeDiff check: This pulse does not have any pulses in a neighboring tank! (%d %d)", 
		       dom_key.GetString(), dom_key.GetOM());
	    else {  // Do the time-diff analysis...
	      if(fabs(new_pulse.t - neighbour_iter->second.at(0).GetTime()) > fMaxIntraStaTimeDiff_){
		log_trace("I found a tank outside the MaxIntraStationTimeDiff: (%d,%d)/%f.  Not using for timing.",
			 new_pulse.omkey.GetString(), new_pulse.omkey.GetOM(), new_pulse.t);
		new_pulse.usepulsetime = false;
		timeFluctuatingTanks_++;
	      }
	    }
	    
	  } 

	
	  inputData_.push_back(new_pulse);
	  
	  PrintPulseFill(new_pulse, "NORMAL");
	}
	
	// Fill the saturationList :	
	// A NAN from topeventcleaning (HGq > hglgcrossover, but no matching lg)
	if(it_pulse->GetCharge() != it_pulse->GetCharge() &&
	   it_pulse->GetTime()== it_pulse->GetTime() &&
	   gain == I3DOMStatus::High &&
	   fSaturation_){

	  UpdateFillCounts(foundPulseInStation, foundLCInStation, dom_key);

	  new_pulse.logvem = log10(hg_sat);
	  saturatedData_.push_back(new_pulse);

	  PrintPulseFill(new_pulse, "NAN-CHARGE-SATURATED");
	}
	
	// Saturated LG, could this somehow be merged with the previous one, because lots of code duplication
	if(it_pulse->GetCharge() == it_pulse->GetCharge() &&
	   it_pulse->GetCharge()> lg_sat &&   
	   it_pulse->GetTime() == it_pulse->GetTime() &&
	   gain == I3DOMStatus::Low &&
	   fSaturation_){

	  UpdateFillCounts(foundPulseInStation, foundLCInStation, dom_key);

	  new_pulse.logvem = log10(lg_sat);
	  saturatedData_.push_back(new_pulse);
	  
          PrintPulseFill(new_pulse, "LG-SATURATED");
	}
      
      
      break; // take only the first pulse (could be changed if wanted)
      
    } // loop over single pulses
    
  } // loop over all DOM's in the Detector Status

  //////
  //clean out LC
  if(fSoftwareThreshold > 0.) {
    std::vector<tankPulse> new_pulses;
    for(std::vector<tankPulse>::iterator it = inputData_.begin(); it!=inputData_.end(); it++){
      if(foundLCInStation.find((*it).omkey.GetString()) != foundLCInStation.end()) {
	new_pulses.push_back(*it);
      }
    }
    inputData_ = new_pulses;
  }     
  //////


  // If a bad tank is in a station with no other hits, then treat the whole station as if it were "bad".
  // This is a bit of a hack, and the logic should probably be improved in the future.
  // This should have no effect if the other tank in the station *does* have a hit.
  if (badTanks)
    for (I3VectorTankKey::const_iterator ibad = badTanks->begin(); ibad != badTanks->end(); ibad++) {
      int st = ibad->string;
      if (foundPulseInStation.find(st)==foundPulseInStation.end()) { // there were no pulses in that station
	badStations->push_back(st); // add it to the list of bad stations
      }
    }


  // check which stations do not have a pulse
  for (std::map<OMKey, I3DOMStatus>::const_iterator i_om = status_map.begin ();
       i_om != status_map.end (); ++i_om) {

    // ----- Get and check the OMGeo
    OMKey dom_key = i_om->first;
    if(dom_key.GetOM() != 61 || foundPulseInStation.find(dom_key.GetString())!=foundPulseInStation.end()) continue;
    // ignore stations in the bad station list from TopEventBuilder
    if(badStations && (std::find(badStations->begin(), badStations->end(), dom_key.GetString()) != badStations->end()))
      continue;
    I3OMGeo om = om_map.find(dom_key)->second;

    // ----- Get this station's *average* snow depth (of the two tanks)
    double avgdepth = 0;
    I3StationGeoMap::const_iterator siter = smap.find(dom_key.GetString());
    if(siter==smap.end()) {
      log_fatal("Station %d doesn't exist in StationGeoMap!", dom_key.GetString());
    }
    // Get the depths of both tanks and average them
    const I3TankGeo& tankGeo0 = siter->second.at(0);
    const I3TankGeo& tankGeo1 = siter->second.at(1);
    avgdepth = 0.5*(tankGeo0.snowheight+tankGeo1.snowheight);
    //log_trace("Station %d with no hits: depth: %f", dom_key.GetString(), avgdepth);

    // Actually fill the internal inputdata container with NOT HIT tanks
    tankPulse no_pulse;
    no_pulse.omkey = dom_key;
    //Bugfix 4/17/12: The position of a "not-hit station" should be the average XYZ of the two tanks, not position of DOM 61.
    //no_pulse.x = (om.position).GetX();
    //no_pulse.y = (om.position).GetY();
    //no_pulse.z = (om.position).GetZ();
    no_pulse.x = 0.5*((tankGeo0.position).GetX() + (tankGeo1.position).GetX());
    no_pulse.y = 0.5*((tankGeo0.position).GetY() + (tankGeo1.position).GetY());
    no_pulse.z = 0.5*((tankGeo0.position).GetZ() + (tankGeo1.position).GetZ());
    no_pulse.t = NAN;
    no_pulse.width = NAN;
    no_pulse.logvem = NAN;
    no_pulse.snowdepth = avgdepth;
    no_pulse.usepulsetime = false;
    inputEmptyData_.push_back(no_pulse);

  }
  
  // Fill the "key tank" snowdepths, while we're here:
  if(smap.find(39)==smap.end()) log_fatal("Diagnostics fail: Station 39 doesn't exist in StationGeoMap!");
  else snowdepth_39B_ = smap.find(39)->second.at(1).snowheight;
  if(smap.find(44)==smap.end()) log_fatal("Diagnostics fail: Station 44 doesn't exist in StationGeoMap!");
  else snowdepth_44A_ = smap.find(44)->second.at(0).snowheight;
  if(smap.find(59)==smap.end()) log_fatal("Diagnostics fail: Station 59 doesn't exist in StationGeoMap!");
  else snowdepth_59A_ = smap.find(59)->second.at(0).snowheight;
  if(smap.find(74)==smap.end()) log_fatal("Diagnostics fail: Station 74 doesn't exist in StationGeoMap!");
  else snowdepth_74A_ = smap.find(74)->second.at(0).snowheight;



  return foundPulseInStation.size();
}


void I3LaputopLikelihood::ResetInput(){
  
  //Clean memory of unused mem that's lying around
  inputData_.clear();
  inputEmptyData_.clear();
  saturatedData_.clear();
}

// compute likelihood
double I3LaputopLikelihood::GetLogLikelihood( const I3EventHypothesis &t ){
    log_trace("Entering likelihood: GetLogLikelihood");
    double llh=0;
    I3ParticleConstPtr track = t.particle;
    I3LaputopParamsConstPtr paramPtr = boost::dynamic_pointer_cast<I3LaputopParams>(t.nonstd);
    if ( ! track ){
      log_fatal( "(%s) I failed to get the I3Particle hypothesis",
		 GetName().c_str() );
    }
    if ( ! paramPtr ){
      log_fatal( "(%s) wrong event hypothesis: missing I3LaputopParams",
		 GetName().c_str() );
    }
    const double s125 = pow(10, paramPtr->GetValue(Laputop::Parameter::Log10_S125));
    const double beta = paramPtr->GetValue(Laputop::Parameter::Beta);

    // This option is obsolete
    //const double snowfactor = paramPtr->SnowFactor;

    // This is the only line that's not accounted for from CutCorePulses
    // In the future, core_radius_ can be a dynamic variable
    core_radius_ = std::max(fCoreCut*I3Units::m, 
			    LateralFitFunctions::R0_PARAM * pow(10, -beta/(2*LateralFitFunctions::KAPPA)));
    
    const I3Direction &dir = track->GetDir();
    const I3Position &core = track->GetPos();

    double coreX = core.GetX(); //old par[0]
    double coreY = core.GetY(); //old par[1]
    double logS125 = log10(s125);
    double dirX = dir.GetX();
    double dirY = dir.GetY();
    double time = track->GetTime();

    // Decide: do a curvature fit (with timing), or not?
    bool combined_fit = (fCurv != "");  log_trace("Combined fit? %u", combined_fit);
    // Decide: do an LDF fit (with charge), or not?
    bool do_ldf = (fLDF != "");  log_trace("LDF fit? %u", do_ldf);

    log_debug("Starting llh calculation ...");
    log_debug("X_c=%f, Y_c=%f, log10(S125)=%f, Beta=%f, nx=%f, ny=%f, t0=%f",
	      coreX, coreY, logS125, beta, dirX, dirY, time);
    
    // Here is where the likelihood function(s) is actually called:
    
    /////////////////////////////////////////////////////////////////
    // double cosz = sqrt(1-dirX*dirX-dirY*dirY);
    double logvem_threshold = log10(VEM_THRESHOLD);
    if(fSoftwareThreshold>0) logvem_threshold = log10(fSoftwareThreshold);

    // to use current LateralFitFunctions...
    double par[4] = {coreX,coreY,logS125,beta};    

    // LOOP ONE : OVER THE "NORMAL" PULSES
    for (std::vector<tankPulse>::iterator ii = inputData_.begin(); ii != inputData_.end(); ii++) {
      double local_r = GetDistToAxis(core,dir,ii); 

      //////////////////
      //charge-llh :  //
      //////////////////
      if (do_ldf) {
      double local_ldf;
      if (fLDF == DLP_NAME) {
	if ((fCoreCut <= 0.) || (local_r >= core_radius_)) {
	  // A normal likelihood:
	  local_ldf = LateralFitFunctions::top_ldf_dlp(local_r, par); 

	} else {
	  // Let's try this: keeping LLH constant (rather than zero)
	  // inside the core cut
	  log_debug("I'm using a radius of %f instead of %f", core_radius_, local_r);
	  local_ldf = LateralFitFunctions::top_ldf_dlp(core_radius_, par); 
	}
      } else
	log_fatal("Sorry, no support yet for LDF function %s", fLDF.c_str());

      // Snow-attenuate the expected signal
      if (!snowservice_) log_error("There is no snowservice! Not attenuating!");
      else
	local_ldf = snowservice_->CalculateAttenuatedLogS(local_ldf, *ii, track, paramPtr);


      //// ACTUALLY : a TEMP effect goes here as it could change the slope
      //// Or a pressure if it changes the slope of the LDF 
      /*
      if(fCorrectAtm_){
	// Pressure (and temperature) correction
	// Everything that depends on the signal/tank/distance goes here
      } 
      */


      double local_sigma_q;
      if ((fCoreCut <= 0.) || (local_r >= core_radius_)) {
	local_sigma_q = LateralFitFunctions::top_ldf_sigma(local_r, local_ldf);
      } else {
	// Again, keep the LLH constant by using the core_radius_ instead
	log_debug("Again, I'm using a radius of %f instead of %f", core_radius_, local_r);
	local_sigma_q = LateralFitFunctions::top_ldf_sigma(core_radius_, local_ldf);
      }
      double local_delta_q = (ii->logvem - local_ldf)/local_sigma_q;
      log_trace("(%d,%d): Normal hit: deltaq=%.9f, sigmaq=%.9f, llh = %.9lf",ii->omkey.GetString(),ii->omkey.GetOM(),
		local_delta_q, local_sigma_q,
		local_delta_q*local_delta_q/2.+log(local_sigma_q));
      llh += local_delta_q*local_delta_q/2.+log(local_sigma_q);
      log_trace("(%d,%d): After charge... total llh went up to %lf",ii->omkey.GetString(),ii->omkey.GetOM(),llh);
      }

      //////////////////////
      // time-likelihood  //
      //////////////////////
      if(combined_fit){
	if(ii->usepulsetime){ 
	  double local_delta_t = GetDistToPlane(time,core,dir,ii);
	  // Special case: the Kislat function
	  if (fCurv == KISLAT_NAME) {
	    double dummypar[1] = {0};
	    llh += LateralFitFunctions::top_curv_kislat_llh(local_r, local_delta_t, dummypar); // par is NOT used in this function btw
	  } else 
	    // This function chooses the right one based on fCurv:
	    llh += top_curv_gausspar_llh(local_r, local_delta_t, paramPtr); 
	  
	  log_trace("(%d,%d): After timing... total llh went up to %lf",ii->omkey.GetString(),ii->omkey.GetOM(),llh);
	}
      }
    }  // loop over the "normal hits"
    
    ////////////////////////////////////
    // likelihood of silent stations  //
    ////////////////////////////////////
    if (do_ldf) {
    for (std::vector<tankPulse>::iterator ie = inputEmptyData_.begin(); ie != inputEmptyData_.end(); ie++) {
      double local_r = GetDistToAxis(core,dir,ie);
      double local_ldf;
      if (fLDF == DLP_NAME)
	local_ldf = LateralFitFunctions::top_ldf_dlp(local_r, par); 
      else
	log_fatal("Sorry, no support yet for LDF function %s", fLDF.c_str());
      
      // Snow-attenuate the expected signal
      if (!snowservice_) log_error("There is no snowservice! Not attenuating!");
      else
	local_ldf = snowservice_->CalculateAttenuatedLogS(local_ldf, *ie, track, paramPtr);


      double local_sigma = LateralFitFunctions::top_ldf_sigma(local_r, std::max(local_ldf, logvem_threshold));
      // prob. to find that DOM without a pulse
      //double local_p_nohit = 0.5*(TMath::Erf((logvem_threshold-local_ldf)/sqrt(2.)/local_sigma)+1.);
      double local_p_nohit = 0.5*(boost::math::erf((logvem_threshold-local_ldf)/sqrt(2.)/local_sigma)+1.);
      // prob. to find that DOM with a pulse
      double local_p_hit = 1.-local_p_nohit;
      
      // prob. to find both tanks without a pulse 
      double local_station_nohit = (1.-pow(local_p_hit, 2.));
      
      //log_trace("No pulse at station %u. Expected pulse height of %f, probability of not triggering is %f",
      //	      inputShower->GetNoPulses().at(i).stationkey, local_ldf, local_station_nohit);
      if (local_station_nohit == 0.0){
      // use a small number (avoid infinities)
      // need to evaluate this a little more to get a useful number, or to correct the problem, where
      //   it really occurs
	llh += -log(DBL_MIN);  // The reason for weird bump in the Llh distribution/silent_llh distribution, but looks like a good fit, leave it like this
	log_trace("This one was a P=0 no-hit! %f whose log is %f", DBL_MIN, log(DBL_MIN));
	log_trace("(%d,%d): After P=0-no-hit... total llh went up to %lf",ie->omkey.GetString(),ie->omkey.GetOM(),llh);
      }
      else {
	log_trace("(%d,%d): Not-hit: llh = %.9lf",ie->omkey.GetString(),ie->omkey.GetOM(),
		  local_station_nohit);
	llh += -log(local_station_nohit);
	log_trace("(%d,%d): After no-hit... total llh went up to %lf",ie->omkey.GetString(),ie->omkey.GetOM(),llh);
      }
    }
    }
    if(fSaturation_){
      ////////////////////////////////////
      // likelihood of saturated tanks //
      ///////////////////////////////////
      
      for (std::vector<tankPulse>::iterator is = saturatedData_.begin(); is != saturatedData_.end(); is++) {
	double local_r = GetDistToAxis(core,dir,is); 

	//1) Saturated pulses have a good time, use the timellh
	if(combined_fit) {
	  double local_delta_t = GetDistToPlane(time,core,dir,is);
	  
	  log_trace("(%d,%d): This saturated hit: local_r = %f, local_delta_t = %f", is->omkey.GetString(),is->omkey.GetOM(), local_r, local_delta_t);
	  // Special case: the Kislat function
	  if (fCurv == KISLAT_NAME) {
	    double dummypar[1] = {0};
	    llh += LateralFitFunctions::top_curv_kislat_llh(local_r, local_delta_t, dummypar); // par is NOT used in this function btw
	  } else 
	    llh += top_curv_gausspar_llh(local_r, local_delta_t, paramPtr);

	  log_trace("After timing (saturated)... total llh went up to %lf",llh);
	}
	
	// 2) The probability pdf is gaussian distributed around the expected (which is never saturated)
	// because it's shower expectation. If the charge is saturated it could have any value between 
	// saturated value and infinity. Thus P(saturated) = integral_Qsat^+inf(Gaussian exp), or Erf
	// Similar as PnoHit Llh.
	if (do_ldf) {
	
	double local_ldf;
	if (fLDF == DLP_NAME)
	  local_ldf = LateralFitFunctions::top_ldf_dlp(local_r, par); 
	else
	  log_fatal("Sorry, no support yet for LDF function %s", fLDF.c_str());
	
	// No correction for snowdepth : signal could've been only higher, ie. more saturated, some Prob!
	
	double local_sigma = LateralFitFunctions::top_ldf_sigma(local_r, local_ldf);
	// prob. to find that a saturated DOM according to Gaussian around expected signal
	double logsat = is->logvem;
	// The opposite of the PnoHit term calculation. 
	//double local_p_sat = 0.5*(1.- TMath::Erf( (logsat-local_ldf)/sqrt(2.)/local_sigma) );
	double local_p_sat = 0.5*(1.- boost::math::erf( (logsat-local_ldf)/sqrt(2.)/local_sigma) );
		
	log_trace("Saturation Prob is %lf, for a signal of %lf VEM, when expecting %lf log10(VEM)",local_p_sat,is->logvem,local_ldf);
	llh += - local_p_sat;        // also need something like an upperbound, to avoid infinities???
	}
      }
    }
    //////
    log_debug("Total Llh: %.9f", llh);

    // All the documentation I read suggests that gulliver wants a positive
    // LLH from this function.  But for some reason, it doesn't work, and
    // DOES work if I return the negative instead.
    // Aha!  Figured it out... toplateral_charge_llh and its friends actually RETURN a negative log likelihood.
    // So by doing this we send gulliver the positive log likelihood.
    return -llh;
}


/********************************************************************/

/* Calculated at the end of the fitting for the quality of the final result */

// Very similar as GetLogL, but gets called only once !!
// DO both time (if fCurv) and charge chi2, as most code is identical anyway
void I3LaputopLikelihood::CalcChi2(const I3EventHypothesis &hypo, double &charge_chi2, double &time_chi2) {

  // Get all we need from the hypothesis
  I3ParticleConstPtr track = hypo.particle;
  I3LaputopParamsConstPtr paramPtr = boost::dynamic_pointer_cast<I3LaputopParams>(hypo.nonstd);
  if ( ! track ){
    log_fatal( "(%s) I failed to get the I3Particle hypothesis",
	       GetName().c_str() );
  }
  if ( ! paramPtr ){
    log_fatal( "(%s) wrong event hypothesis: missing I3LaputopParams",
	       GetName().c_str() );
  }
  const double s125 = paramPtr->Has(Laputop::Parameter::Log10_S125) ? pow(10,paramPtr->GetValue(Laputop::Parameter::Log10_S125)) : NAN;
  const double beta = paramPtr->Has(Laputop::Parameter::Beta) ? paramPtr->GetValue(Laputop::Parameter::Beta) : NAN;
  // const double snowfactor = paramPtr->SnowFactor;
  const I3Direction &dir = track->GetDir();
  const I3Position &core = track->GetPos();
  
  double coreX = core.GetX(); //old par[0]
  double coreY = core.GetY(); //old par[1]
  double logS125 = log10(s125);
  // double dirX = dir.GetX();
  // double dirY = dir.GetY();
  double time = track->GetTime();

  // This is the only line that's not accounted for from CutCorePulses
  // In the future, core_radius_ can be a dynamic variable
  core_radius_ = std::max(fCoreCut*I3Units::m, 
			  LateralFitFunctions::R0_PARAM * pow(10, -beta/(2*LateralFitFunctions::KAPPA)));

  // to use current LateralFitFunctions...
  double par[4] = {coreX,coreY,logS125,beta};    

  // double cosz = sqrt(1-dirX*dirX-dirY*dirY);

  int n_ldf = 0;
  int n_time = 0;
  for (std::vector<tankPulse>::iterator ii = inputData_.begin(); ii != inputData_.end(); ii++) {
      double local_r = GetDistToAxis(core,dir,ii);

      if (fLDF != "") {
    // Also for chi2????
    if ((fCoreCut <= 0.) || (local_r >= core_radius_)) {

      double local_ldf;
      if (fLDF == DLP_NAME)
        local_ldf = LateralFitFunctions::top_ldf_dlp(local_r, par);
      else
        log_fatal("Sorry, no support yet for LDF function %s", fLDF.c_str());

      // Snow-attenuate the expected signal
      local_ldf = snowservice_->CalculateAttenuatedLogS(local_ldf, *ii, track, paramPtr);

      double local_sigma_q = LateralFitFunctions::top_ldf_sigma(local_r, local_ldf);
      double local_delta_q = (ii->logvem - local_ldf)/local_sigma_q;
      log_debug("Chi2: r=%f, ldf=%f, sigma=%f, delta=%f", 
                local_r, local_ldf, local_sigma_q, local_delta_q);
      charge_chi2 += local_delta_q*local_delta_q;
      ++n_ldf;
    }
      }

    // time_chi2
    if(fCurv != ""){
      double local_delta_t = GetDistToPlane(time,core,dir,ii);

      double local_curv, local_sigma_t;
      if (fCurvType != Laputop::FrontDelay::None) {
	local_curv = paramPtr->ExpectedShowerFrontDelay(local_r, 0, fCurvType);
	local_sigma_t =  paramPtr->ExpectedShowerFrontDelayError(local_r, 0, fCurvType);
      }
      else if (fCurv == KISLAT_NAME) {
	log_debug("The KISLAT curvature doesn't really have a sigma, so this chi^2 will be garbage.");
	double dummypar[1] = {0};
	local_curv = LateralFitFunctions::top_curv_kislat(local_r, dummypar);
	local_sigma_t = paramPtr->ExpectedShowerFrontDelayError(local_r, 0, Laputop::FrontDelay::GaussParabola); 
      }
      else log_fatal("Sorry, the Curvature type is set to 'NONE'.  You shouldn't be here. fCurv=%s", fCurv.c_str());

      double local_delta_showerfront = (local_delta_t - local_curv)/local_sigma_t;
      time_chi2 += local_delta_showerfront*local_delta_showerfront;
      ++n_time;
    }

  }

  charge_chi2 /= n_ldf;
  time_chi2 /= n_time;

  log_trace("charge chi2/ndf of the pulses: %f", charge_chi2);
  log_trace("time chi2/ndf of the pulses: %f", time_chi2);

  return;
}

/********************************/
double I3LaputopLikelihood::GetDistToAxis(const I3Position &pos, const I3Direction & dir,std::vector<tankPulse>::iterator it){
  double x_c = pos.GetX();
  double y_c = pos.GetY();
  double z_c = pos.GetZ();
  double nx = dir.GetX();
  double ny = dir.GetY();

  double abs_x_sq = (it->x-x_c)*(it->x-x_c)
    + (it->y-y_c)*(it->y-y_c)
    + (it->z-z_c)*(it->z-z_c);
  double n_prod_x = nx*(it->x-x_c)
    + ny*(it->y-y_c)
    - sqrt(1. - nx*nx - ny*ny)*(it->z-z_c);
  return sqrt(abs_x_sq - n_prod_x * n_prod_x);
}

double I3LaputopLikelihood::GetDistToPlane(const double &time,const I3Position& pos, const I3Direction &dir,std::vector<tankPulse>::iterator it){
  double x_0 = pos.GetX();  // T0 is shifted to COG core in Seedservice=> X0,Y0 is core
  double y_0 = pos.GetY();
  double z_c = pos.GetZ();
  double nx = dir.GetX();
  double ny = dir.GetY();
  double t_0 = time;
  // in lateralfit : T0 = ShowerPlane->T0 + (nx * (x_c - inPlaneParams->X0) + ny * (y_c - inPlaneParams->Y0))/I3Constants::c;
  //                 X0, Y0 : both from ShowerPlane
  // here : use time at COG => Seed as reference, shouldn't matter (see l.134 in TTopRecoShower.cxx)
  // like in the 3rd iteration of lateralfit
  log_trace("Upstream-Downstream calc OK? t_0 : %lf, nx*(x_t-x_0) : %lf,ny*(y_t-y_0) : %lf, "
	    "nz*(z_t-z_0) : %lf",time,(nx*(it->x-x_0)),(ny*(it->y-y_0)),(sqrt(1. - nx*nx - ny*ny)*(it->z-z_c)));

  double t_plane = t_0 + (nx*(it->x-x_0) + ny*(it->y-y_0) - sqrt(1. - nx*nx - ny*ny)*(it->z-z_c))/I3Constants::c;
  return t_plane - it->t;
}  

// Generic likelihood for curvature functions.
// (Use to be hard-coded in LateralFitFunctions, but moved here  -KR)
double I3LaputopLikelihood::top_curv_gausspar_llh(double r, double deltaT, I3LaputopParamsConstPtr par) {
  // The old (hardcoded) way:
  //double local_curv = top_curv_gausspar(r, par);
  //double local_sigma_t =  2.92 + 3.77e-4*r*r; // from TW
  // The new way:
  double local_curv, local_sigma_t;
  if (fCurvType != Laputop::FrontDelay::None) {
    local_curv = par->ExpectedShowerFrontDelay(r, 0, fCurvType);
    local_sigma_t =  par->ExpectedShowerFrontDelayError(r, 0, fCurvType);
  }
  else log_fatal("Sorry, the Curvature type is set to 'NONE'.  You shouldn't be here. fCurv=%s", fCurv.c_str());

  double local_delta_t = (deltaT - local_curv)/local_sigma_t;

  return local_delta_t*local_delta_t/2.+log(local_sigma_t);
}


I3FrameObjectPtr I3LaputopLikelihood::GetDiagnostics(const I3EventHypothesis &hypo) {
  log_debug("I'm getting diagnostics today!");

  I3ParticlePtr track = hypo.particle;
  I3LaputopParamsPtr params = boost::dynamic_pointer_cast<I3LaputopParams>(hypo.nonstd);

  SnowCorrectionDiagnosticsPtr diagnost = boost::make_shared<SnowCorrectionDiagnostics>();

  snowservice_->FillSnowDiagnostics(diagnost, track, params);

  // Fill the key snow depths:
  diagnost->snowdepth_39B = snowdepth_39B_;
  diagnost->snowdepth_44A = snowdepth_44A_;
  diagnost->snowdepth_59A = snowdepth_59A_;
  diagnost->snowdepth_74A = snowdepth_74A_;

  return diagnost;
}

/*****************************************/
typedef I3SingleServiceFactory< I3LaputopLikelihood,I3EventLogLikelihoodBase >
I3LaputopLikelihoodServiceFactory;
I3_SERVICE_FACTORY( I3LaputopLikelihoodServiceFactory )

