//IceTray
#include "icetray/I3Tray.h"
#include "icetray/I3Frame.h"
#include "icetray/I3TrayHeaders.h"
//basic frame info
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/I3DOMFunctions.h"
#include "icetray/OMKey.h"
#include "icetray/I3Units.h"

#include "dataclasses/I3Position.h"
//
#include "dataclasses/physics/I3Waveform.h"

//my class headers
#include "portia/I3Portia.h"
//my storage class
#include "recclasses/I3PortiaPulse.h"
#include "recclasses/I3PortiaEvent.h"

#include <numeric>
using namespace std;

/**
 * The Basetime window parameters
 */
double I3Portia::kStartTimeBtw_ = -4400.0*I3Units::ns; // 4.4 usec before the largest NPE hit time
double I3Portia::kEndTimeBtw_ = 6400.0*I3Units::ns; // 6.4 usec before the largest NPE hit time

/*
  A simple function for only debugging purpose
 */
void print(double n){ log_trace("%f [GV]", n); }
void print_in_mv(double n){ log_error("%f [mV]", n/I3Units::mV); }
double square_sum(double a, double b){return a + b*b;}

I3_MODULE(I3Portia);

////constructor and destructor////////////////////////////////////////////
I3Portia::I3Portia(const I3Context& ctx) : I3ConditionalModule(ctx),
					   //output
					   outATWDPulseSeriesName_("ATWDPulseSeries"),
					   outFADCPulseSeriesName_("FADCPulseSeris"),
					   outATWDPortiaPulseName_("ATWDPortiaPulse"),
					   outFADCPortiaPulseName_("FADCPortiaPulse"),
					   outPortiaEventName_("PortiaEvent"),
					   outBestPortiaPulseName_("BestPortiaPulse"),
					   //input
					   inDataReadoutName_("CleanInIceRawData"),
					   inTopDataReadoutName_("CleanIceTopRawData"),
					   inTopATWDWaveformName_("CalibratedIceTopATWD"),
					   inATWDWaveformName_("CalibratedATWD"),
					   inFADCWaveformName_("CalibratedFADC"),
					   inDOMMapName_("SplittedDOMMap"),
					   /* baseline
					      "first", "last", "lower", "first_or_last", 
					      "iteration", "eheoptimized"
					   */
					   optionATWDBaseLine_("eheoptimized"),
					   optionFADCBaseLine_("eheoptimized"),
					   //options
					   readExternalDOMMap_(false),
					   makeBestPulse_(false),
					   makeIceTopPulse_(false),
					   useFADC_(true),
					   //default number
					   pmtGain_(1.e7),
					   /* 
					      threshold charge, i.e., above this value,
					      a waveform is considered to be a portia pulse
					   */
					   atwdThresholdCharge_(0.1*I3Units::pC),
					   fadcThresholdCharge_(0.1*I3Units::pC),
					   /* 
					      leading edge time is obtained as the time
					      that waveforms become above this value
					   */
					   atwdThresholdLEAmp_(0.5*I3Units::mV),
					   fadcThresholdLEAmp_(0.5*I3Units::mV),
					   /* DOM with the largest NPE flag
					      for the basetime window calculation
					   */
					   foundLargestNPEDOM_(false){
  //Register Parameters

  AddParameter("DataReadoutName",          "input DOM launch name",     inDataReadoutName_);
  AddParameter("OutPortiaEventName",       "output Portia event name",  outPortiaEventName_);
  AddParameter("SplitDOMMapName",          "in DOM Map name",     inDOMMapName_);
  AddParameter("PMTGain",                  "default value of PMT Gain", pmtGain_);

  AddParameter("ReadExternalDOMMap",       "Bool to read splitted DOM map", readExternalDOMMap_);
  
  AddParameter("ATWDPulseSeriesName",      "output ATWD reco pulse series name", outATWDPulseSeriesName_);  
  AddParameter("ATWDPortiaPulseName",      "output ATWD portia pulse name",      outATWDPortiaPulseName_);  
  AddParameter("ATWDWaveformName",         "input ATWD waveform name",           inATWDWaveformName_);  
  AddParameter("ATWDBaseLineOption",       "ATWD baseline option", 		 optionATWDBaseLine_);
  AddParameter("UseFADC",                  "Bool to choose use fadc waveforms",  useFADC_);
  AddParameter("FADCPulseSeriesName",      "output FADC reco pulse series name", outFADCPulseSeriesName_);  
  AddParameter("FADCPortiaPulseName",      "output FADC portia pulse name",      outFADCPortiaPulseName_);  
  AddParameter("FADCWaveformName",         "input FADC waveform name",           inFADCWaveformName_);  
  AddParameter("FADCBaseLineOption",       "FADC baseline option", 		 optionFADCBaseLine_);
  AddParameter("MakeBestPulseSeries",      "Bool to choose to make best pulse",  makeBestPulse_);
  AddParameter("BestPortiaPulseName",      "output Best portia pulse ",          outBestPortiaPulseName_);  

  AddParameter("MakeIceTopPulse",          "Make IceTop pulse or not",           makeIceTopPulse_);
  AddParameter("TopDataReadoutName",       "IceTop Raw DOMLaunch Name",          inTopDataReadoutName_);
  AddParameter("inTopATWDWaveformName",    "IceTop calibrated waveform name",    inTopATWDWaveformName_);

  AddParameter("ATWDThresholdCharge",      "ATWD threshold charge", atwdThresholdCharge_);
  AddParameter("FADCThresholdCharge",      "FADC threshold charge", fadcThresholdCharge_);
  AddParameter("ATWDLEThresholdAmplitude", "ATWD LE threshold amp", atwdThresholdLEAmp_);
  AddParameter("FADCLEThresholdAmplitude", "FADC lE threshold amp", fadcThresholdLEAmp_);
  AddOutBox("OutBox");
}
/////////////////////////////////////////////////////////////////////////
I3Portia::~I3Portia(){
}
/////////////////////////////////////////////////////////////////////////
void I3Portia::Configure(){

  log_trace("Configuring the Portia");

  GetParameter("ReadExternalDOMMap", readExternalDOMMap_);

  if(readExternalDOMMap_){
    GetParameter("SplitDOMMapName", inDOMMapName_);
  }else{
    GetParameter("DataReadoutName", inDataReadoutName_);
  }
  GetParameter("PMTGain",         pmtGain_);
  GetParameter("ATWDWaveformName",inATWDWaveformName_);  

  log_info("==============================");
  if(!readExternalDOMMap_){
    log_info("DataReadoutName  is %s",  inDataReadoutName_.c_str());
  }else{
    log_info("SplitDOMMapName  is %s",  inDOMMapName_.c_str());
  }
  log_info("ATWDWaveformName is %s",  inATWDWaveformName_.c_str());
  log_info("PMTGain          is %f",  pmtGain_);
  log_info("==============================");

  GetParameter("ATWDPulseSeriesName",     outATWDPulseSeriesName_);
  GetParameter("ATWDPortiaPulseName",     outATWDPortiaPulseName_);
  GetParameter("ATWDBaseLineOption",      optionATWDBaseLine_);
  GetParameter("ATWDThresholdCharge",     atwdThresholdCharge_);
  GetParameter("ATWDLEThresholdAmplitude",atwdThresholdLEAmp_);
  GetParameter("OutPortiaEventName", outPortiaEventName_);

  log_trace("Get Portia Pulse!");
  log_info("ATWDPulseSeriesName is %s",          outATWDPulseSeriesName_.c_str());
  log_info("ATWDPortiaPulseName is %s",          outATWDPortiaPulseName_.c_str());
  log_info("ATWDBaseLineOption  is %s",          optionATWDBaseLine_.c_str());
  log_info("ATWDThresholdCharge is %f [pC]",     atwdThresholdCharge_/I3Units::pC);
  log_info("ATWDLEThresholdAmplitude is %f [mV]",atwdThresholdLEAmp_/I3Units::mV);


  GetParameter("UseFADC",  useFADC_);
  if(useFADC_)
    {
      log_trace("Get FADC Pulse!");
      GetParameter("FADCPulseSeriesName",     outFADCPulseSeriesName_);
      GetParameter("FADCPortiaPulseName",     outFADCPortiaPulseName_);
      GetParameter("FADCWaveformName",        inFADCWaveformName_);  
      GetParameter("FADCBaseLineOption",      optionFADCBaseLine_);
      GetParameter("FADCThresholdCharge",     fadcThresholdCharge_);
      GetParameter("FADCLEThresholdAmplitude",fadcThresholdLEAmp_);

      log_info("FADCPulseSeriesName is %s",          outFADCPulseSeriesName_.c_str());
      log_info("FADCBaseLineOption  is %s",          optionFADCBaseLine_.c_str());
      log_info("FADCThresholdCharge is %f [pC]",     fadcThresholdCharge_ /I3Units::pC);
      log_info("FADCLEThresholdAmplitude is %f [mV]",fadcThresholdLEAmp_ /I3Units::mV);

      GetParameter("MakeBestPulseSeries", makeBestPulse_);
      GetParameter("BestPortiaPulseName", outBestPortiaPulseName_);  
      GetParameter("MakeIceTopPulse",       makeIceTopPulse_);
      GetParameter("TopDataReadoutName",    inTopDataReadoutName_);
      GetParameter("inTopATWDWaveformName", inTopATWDWaveformName_);
    }
}
////////////////////////////////////////////////////////////////////////
void I3Portia::Physics(I3FramePtr frame){
  log_trace("Entering I3Portia::Physics()");
  int countATWDHits=0;
  int countFADCHits=0;
  //check if keynames for DOM launch and waveforms are set correctly
  if((!readExternalDOMMap_ && !frame->Has(inDataReadoutName_)) || 
     (readExternalDOMMap_ && !frame->Has(inDOMMapName_)) || 
     !frame->Has(inATWDWaveformName_) ||
     ( makeIceTopPulse_ && !frame->Has(inTopDataReadoutName_) ) ||
     ( makeIceTopPulse_ && !frame->Has(inTopATWDWaveformName_) )){
    
    if(!readExternalDOMMap_ && !frame->Has(inDataReadoutName_)){
      log_error("Keyname of DOM launch is not correct %s", inDataReadoutName_.c_str());
    }
    if(readExternalDOMMap_ && !frame->Has(inDOMMapName_)){
      log_error("Keyname of (splitted) DOM map is not correct %s", inDOMMapName_.c_str());
    }
    //Return empity
    //For ATWD
    I3PortiaPulseMapPtr atwdportiamap(new I3PortiaPulseMap);
    I3RecoPulseSeriesMapPtr atwdrecomap(new I3RecoPulseSeriesMap);
    //event-wise output
    I3PortiaEventPtr portia_event_ptr(new I3PortiaEvent);
    //For FADC
    I3PortiaPulseMapPtr fadcportiamap(new I3PortiaPulseMap);
    I3RecoPulseSeriesMapPtr fadcrecomap(new I3RecoPulseSeriesMap);
    
    frame->Put(outPortiaEventName_, portia_event_ptr);
    frame->Put(outFADCPortiaPulseName_, fadcportiamap);
    frame->Put(outFADCPulseSeriesName_, fadcrecomap);
    frame->Put(outATWDPortiaPulseName_, atwdportiamap);
    frame->Put(outATWDPulseSeriesName_, atwdrecomap);
    
    //if best pulse series want to be created
    if(makeBestPulse_){
      I3PortiaPulseMapPtr bestportiamap(new I3PortiaPulseMap);
      frame->Put(outBestPortiaPulseName_, bestportiamap);
    }
    
  
    if(!frame->Has(inATWDWaveformName_) )
      log_error("Keyname of ATWD waveform is not correct %s", inATWDWaveformName_.c_str());
    
    PushFrame(frame,"OutBox");
    return;
  }
  if( useFADC_ && !frame->Has(inFADCWaveformName_) ) {
    
    if(!frame->Has(inFADCWaveformName_) )
      log_error("FADC waveform %s doesn't exist in frame", inFADCWaveformName_.c_str());
    useFADC_ = false;
  }
  
  //getting out basic information
  const I3Geometry&       geo = frame->Get<I3Geometry>();
  const I3Calibration&    cal = frame->Get<I3Calibration>();
  const I3DetectorStatus& det = frame->Get<I3DetectorStatus>();

  //
  // Container to store launched OM keys and its launch time(s).
  // This map is built from I3DOMLaunchSeriesMap if(!readExternalDOMMap_)
  // or provided externally via frame presumably generated by
  // the I3PortiaSplitter module.
  //
  I3MapKeyVectorDouble launchMap;

  // getting out things we make pulses at first the in-ice dom launch
  //
  // Build the launch time container from the I3DOMLaunchSeriesMap
  if(!readExternalDOMMap_){
    //
    // First get the in-ice DOM launch series map
    //
    I3DOMLaunchSeriesMap& domLaunchSeriesMap = 
      const_cast<I3DOMLaunchSeriesMap&>(*(frame->Get<I3DOMLaunchSeriesMapConstPtr>(inDataReadoutName_)));
    log_debug("we have %zu in-ice dom lanuches", domLaunchSeriesMap.size());

    if(makeIceTopPulse_){
      /*
       * If we want to make IceTop Pulses, get the IceTop launch series map and 
       * combine them as the tail of in-ice dom launch series map
       */
      const I3DOMLaunchSeriesMap topLaunchMap = frame->Get<I3DOMLaunchSeriesMap>(inTopDataReadoutName_);
    
      log_debug("we have %zu ice-top dom lanuches", topLaunchMap.size());
      for(I3DOMLaunchSeriesMap::const_iterator topLaunchMap_iter = topLaunchMap.begin();
	  topLaunchMap_iter != topLaunchMap.end(); topLaunchMap_iter++){
	domLaunchSeriesMap.insert(*topLaunchMap_iter);	  
      }
    }//end makeIceTopPulse_

    if(!MakeSplittedDOMMap(domLaunchSeriesMap,launchMap,geo,makeIceTopPulse_)){
      log_error("no launched HLC DOMs is found. Something is wrong");
    }
    
  }else{ // The launch time map is obtained directly from the frame
    launchMap =  
      const_cast<I3MapKeyVectorDouble&>(*(frame->Get<I3MapKeyVectorDoubleConstPtr>(inDOMMapName_)));
  }

  //getting The InIce ATWD waveform series map
  const I3WaveformSeriesMap atwdwaveMap = frame->Get<I3WaveformSeriesMap>(inATWDWaveformName_);
  //Define the output Maps --- this is a map of I3Map<OMKey, I3PortiaPulse>
  //For ATWD
  I3PortiaPulseMapPtr atwdportiamap(new I3PortiaPulseMap);
  I3RecoPulseSeriesMapPtr atwdrecomap(new I3RecoPulseSeriesMap);
  //event-wise output
  I3PortiaEventPtr portia_event_ptr(new I3PortiaEvent);


  //getting The InIce FADC waveform series map
  //const I3WaveformSeriesMap fadcwaveMap = frame->Get<I3WaveformSeriesMap>(inFADCWaveformName_);
  I3WaveformSeriesMapConstPtr fadcwaveMap_ptr = frame->Get<I3WaveformSeriesMapConstPtr>(inFADCWaveformName_);
  
  //For FADC
  I3PortiaPulseMapPtr fadcportiamap(new I3PortiaPulseMap);
  I3RecoPulseSeriesMapPtr fadcrecomap(new I3RecoPulseSeriesMap);
  
  //if best pulse series want to be created
  I3PortiaPulseMapPtr bestportiamap(new I3PortiaPulseMap);
  I3RecoPulseSeriesMapPtr bestrecomap(new I3RecoPulseSeriesMap);
  
  ///////////////////////////////////
  //Loop over DOMs which has launched
  ///////////////////////////////////    ///////////////////////////////////

  ///////////////////////////
  /* Start DOM Launch Loop */
  ///////////////////////////
  //Note that DOMlaunches also include SLC launches
  ///////////////////////////
  for(I3MapKeyVectorDouble::const_iterator launchMap_iter = launchMap.begin();
      launchMap_iter!=launchMap.end(); launchMap_iter++){
    
    OMKey omkey = launchMap_iter->first;
    log_trace("string %d / OM %d", omkey.GetString(), omkey.GetOM());
    const vector<double>& launches = launchMap_iter->second;
    int HLC_count = launches.size(); // note the launch time map only contains HLC

    //////////////////////////////////////////
    /* Check if this om has proper Geometry */
    //////////////////////////////////////////
    I3OMGeoMap::const_iterator geo_iter = geo.omgeo.find(omkey);
    if(geo_iter==geo.omgeo.end()){
      log_error("Couldn't find OM geometry %d/%d: skip", omkey.GetString(), omkey.GetOM());
      continue;
    }
    const I3OMGeo& om_geo = geo_iter->second;
    I3OMGeo::OMType om_type = om_geo.omtype;    
    if(om_type != I3OMGeo::IceCube && om_type != I3OMGeo::IceTop){
      log_trace("non-IceCube/IceTop OMKey: (%d,%d): skip", omkey.GetString(), omkey.GetOM());
      continue;
    }
    const I3Position& om_pos = om_geo.position;    
    //////////////////////////////////////////
	  
    /////////////////////////////////////////////
    /* Check if this om has proper Calibration */
    /////////////////////////////////////////////
    map<OMKey, I3DOMCalibration>::const_iterator cal_iter = cal.domCal.find(omkey);
    if(cal_iter==cal.domCal.end()){
      log_error("Couldn't find calibration: skip");
      continue;
    }
    const I3DOMCalibration& om_cal = cal_iter->second;
    if( std::isnan(om_cal.GetATWDGain(0)) || std::isnan(om_cal.GetATWDGain(1)) || std::isnan(om_cal.GetATWDGain(2)) ){
      log_error("Check ATWDGatin (%f, %f, %f) for %d/%d: skip", 
		om_cal.GetATWDGain(0), om_cal.GetATWDGain(1), om_cal.GetATWDGain(2),
		omkey.GetString(),omkey.GetOM());
      continue;
    }
    else log_trace("Check ATWDGatin (%f, %f, %f) for %d/%d", 
		   om_cal.GetATWDGain(0), om_cal.GetATWDGain(1), om_cal.GetATWDGain(2),
		   omkey.GetString(),omkey.GetOM());
    /////////////////////////////////////////////
    
    ////////////////////////////////////////////////
    /* Check if this om has proper DetectorStatus */
    ////////////////////////////////////////////////
    map<OMKey, I3DOMStatus>::const_iterator det_iter = det.domStatus.find(omkey);
    
    if(det_iter==det.domStatus.end()){
      log_error("Couldn't find dom status: skip");
      continue;
    }
    const I3DOMStatus& om_det = det_iter->second;
    if(std::isnan(om_det.pmtHV) || om_det.pmtHV < 1.e-100){
      log_error("Check PMT HV Status (%f) for %d/%d: skip", om_det.pmtHV, omkey.GetString(), omkey.GetOM());
      continue;
    }
    else log_trace("Check PMT HV Status (%f) for %d/%d", om_det.pmtHV, omkey.GetString(), omkey.GetOM());
    ////////////////////////////////////////////////

    ///////////////////////////
    /* getting waveforms out */
    ///////////////////////////    
    bool atwd_w   = true;//if ATWD waveform exist
    bool fadc_w   = true; //if FADC waveform exist
    bool doneatwd = false;//if ATWD pulse created
    bool donefadc = false;//if FADC pulse created
    
    I3WaveformSeriesMap::const_iterator atwdMap_iter = atwdwaveMap.find(omkey);
    I3WaveformSeriesMap::const_iterator fadcMap_iter;
    
    /* Case no atwd waveform map entry found on the launched DOM */
    if( om_type == I3OMGeo::IceCube && atwdMap_iter == atwdwaveMap.end() ){
      log_debug("ATWD not found on (%d, %d)", omkey.GetString(), omkey.GetOM());      
      atwd_w = false; 
    }
    /* Case this is IceTop and makie icetop pulse */
    if( makeIceTopPulse_ && om_type == I3OMGeo::IceTop ){
      //ATWD IceTop
      I3WaveformSeriesMapConstPtr 
	topAtwdwaveMap_ptr = frame->Get<I3WaveformSeriesMapConstPtr>(inTopATWDWaveformName_);
      
      if(topAtwdwaveMap_ptr){
	atwdMap_iter = topAtwdwaveMap_ptr->find(omkey);
	
	if(atwdMap_iter == atwdwaveMap.end()){
	  log_debug("NO calibrated icetop atwd on string %d / OM %d", omkey.GetString(), omkey.GetOM());
	  atwd_w = false;
	}
      }
    }

    /* if atwd waveforms are zero */
    if(atwd_w && atwdMap_iter->second.size()==0) atwd_w = false;

    if(useFADC_){
      if(fadcwaveMap_ptr){
	fadcMap_iter = fadcwaveMap_ptr->find(omkey);
	/* Case no fadc waveform map entry found on the launched DOM */
	if(fadcMap_iter == fadcwaveMap_ptr->end()){
	  log_debug("NO calibrated fadc on string %d / OM %d", omkey.GetString(), omkey.GetOM());
	  fadc_w=false;
	}
      }
    } else fadc_w=false;//If FADC is not used

    /* Map entry found but zero HLC waveforms found */
    if(atwd_w && atwdMap_iter->second.size()==0) atwd_w = false;
    if(fadc_w && fadcMap_iter->second.size()==0) fadc_w = false;
    /* It shouldn't happen that the number of HLC waveforms different for ATWD/FADC */
    if( atwdMap_iter->second.size() != fadcMap_iter->second.size() )
      log_debug("HLC count %d, numbers of atwd and fadc HLC waveforms %d vs %d are different for string %d / OM %d", HLC_count, (int)atwdMap_iter->second.size(), (int)fadcMap_iter->second.size(), omkey.GetString(), omkey.GetOM());

    if(atwd_w == false && fadc_w == false){
      log_debug("no waveforms in this launch: skip this");
      continue;
    }

    ///////////////////////////////////
    /* Looping over all the launches */
    ///////////////////////////////////
    //the first entry in the launches assigned
    vector<double>::const_iterator launchSeries_iter = launches.begin();
    double currentTime = 0.0;
    //at first find the earliest DOMLaunch time among all HLC
    
    for(;launchSeries_iter != launches.end(); launchSeries_iter++){
      
      if(currentTime==0.0)currentTime = *launchSeries_iter;
      else if(currentTime > *launchSeries_iter){
	currentTime = *launchSeries_iter;
      }
      if(HLC_count>1)log_debug("HLC count %d, string %d / OM %d and %f, %f", HLC_count, omkey.GetString(), omkey.GetOM(), currentTime, *launchSeries_iter);
    }
    
    //Then set the earliest launch
    for(launchSeries_iter = launches.begin();launchSeries_iter != launches.end(); launchSeries_iter++){
      if(*launchSeries_iter <= currentTime)
	break; //this is it
    }
    
    double launchTimeOfThisDOM = *launchSeries_iter;
    if(HLC_count>1)log_debug("HLC count %d, string %d / OM %d, and obtained earliest time %f, %f", HLC_count, omkey.GetString(), omkey.GetOM(), currentTime, launchTimeOfThisDOM);
    
    if(atwd_w){
      I3WaveformSeries::const_iterator  atwdSeries_iter = atwdMap_iter->second.begin();
      /////////////////////////////////////////////////////////////////////////////////////////
      /* if more than one ATWD waveforms check if the waveform matchs to the dom launch time */
      /////////////////////////////////////////////////////////////////////////////////////////
      bool atwd_time_found = false;
      if(atwdMap_iter->second.size()>1){
	for(;atwdSeries_iter != atwdMap_iter->second.end();atwdSeries_iter++){
	  if(atwdSeries_iter->IsHLC() && (atwdSeries_iter->GetStartTime() <= launchTimeOfThisDOM) && (launchTimeOfThisDOM-atwdSeries_iter->GetStartTime())<1000.0 ) {
	    atwd_time_found = true;
	    break;//this is it
	  }
	}
	if(!atwd_time_found) {log_error("No corresponding ATWD Waveforms %f to the launch %f", atwdSeries_iter->GetStartTime(), launchTimeOfThisDOM);atwdSeries_iter = atwdMap_iter->second.begin();} 
	if(HLC_count>=1)log_debug("corresponding ATWD Waveforms %f to the launch %f", atwdSeries_iter->GetStartTime(), launchTimeOfThisDOM); 
      }
      log_trace("Atwd Start Time %f : Launch Time %f", atwdSeries_iter->GetStartTime(), launchTimeOfThisDOM);
      /////////////////////////////////////////////////////////////////////////////////////////

      const I3Waveform&  atwd = *atwdSeries_iter;
      I3PortiaPulse atwdportia;

      if(!atwd.GetWaveform().empty()&&atwd.GetWaveform().size()==128)
        doneatwd = MakeATWDPulses(launchTimeOfThisDOM, atwd, om_cal, om_det, om_pos, atwdportia);
      else{ log_error("ATWD is empity or not reasonable size %zu skip this DOM", atwd.GetWaveform().size()); continue; }
      
      if(doneatwd){
	++countATWDHits;
	I3RecoPulse&  atwdreco = atwdportia.GetRecoPulse();
	I3RecoPulseSeries atwdpulseseries;	  
	atwdpulseseries.push_back(atwdreco);
	log_trace("Created %d th ATWD pulse at string %d/OM %d", countATWDHits, omkey.GetString (), omkey.GetOM ());
	(*atwdportiamap)[omkey]=atwdportia;
	(*atwdrecomap)[omkey]=atwdpulseseries;
      }//if atwd pulse is created
    }//ATWD waveform exist

    //now for FADC
    if(useFADC_&&fadc_w)
      {
	I3WaveformSeries::const_iterator  fadcSeries_iter = fadcMap_iter->second.begin();
	////////////////////////////////////////////////////////////
	/* if more than one FADC waveforms take the earlier entry */
	////////////////////////////////////////////////////////////
	bool fadc_time_found = false;
	if(fadcMap_iter->second.size()>1){
	  for(;fadcSeries_iter != fadcMap_iter->second.end();fadcSeries_iter++){
	    if(fadcSeries_iter->IsHLC() && (fadcSeries_iter->GetStartTime() <=  launchTimeOfThisDOM ) && (launchTimeOfThisDOM - fadcSeries_iter->GetStartTime() ) < 1000.0){
	      fadc_time_found = true;
	      log_debug("corresponding FADC Waveforms time diff %f", fadcSeries_iter->GetStartTime() - launchTimeOfThisDOM);
	      break;//this is it
	    }
	  }
	  if(!fadc_time_found) {log_error("No corresponding FADC Waveforms %f to the launch %f", fadcSeries_iter->GetStartTime(), launchTimeOfThisDOM); 
	    fadcSeries_iter = fadcMap_iter->second.begin();}
	  if(HLC_count>1)log_debug("corresponding FADC Waveforms %f to the launch %f", fadcSeries_iter->GetStartTime(),launchTimeOfThisDOM); 

	}
	/////////////////////////////////////////////////////////////////////////////////////////
	const I3Waveform&  fadc = *fadcSeries_iter;
	I3PortiaPulse fadcportia;	
	const size_t nbins_wave_fadc = fadc.GetWaveform().size();
	
	if(!fadc.GetWaveform().empty()){
	  if(fadc.GetWaveform().size()==256){
	    donefadc = MakeFADCPulses(launchTimeOfThisDOM, fadc, om_cal, om_det, om_pos, fadcportia);
	  }else{log_error("bin numbers of calibrated waveform %zu", nbins_wave_fadc);}	  
	}

	if(donefadc){
	  ++countFADCHits;
	  log_trace("Created %d th FADC pulse at string %d/OM %d", countFADCHits, omkey.GetString (), omkey.GetOM ());
	  I3RecoPulse&  fadcreco = fadcportia.GetRecoPulse();
	  I3RecoPulseSeries fadcpulseseries;
	  fadcpulseseries.push_back(fadcreco);
	    (*fadcportiamap)[omkey]=fadcportia;
	    (*fadcrecomap)[omkey]=fadcpulseseries;
	}//if fadc pulse is created	
      }//Fadc waveform exist
    
    /////////////////////////
    /* Now Make Best Pulse */
    /////////////////////////
    bool donebest = false;

    if(makeBestPulse_&&donefadc&&doneatwd){

      /* Get FADC and ATWD pulses */
      I3PortiaPulseMap::const_iterator atwdPulseMap_iter = atwdportiamap->find(omkey);
      const I3PortiaPulse& atwd_p = atwdPulseMap_iter->second;
      I3PortiaPulseMap::const_iterator fadcPulseMap_iter = fadcportiamap->find(omkey);
      const I3PortiaPulse& fadc_p = fadcPulseMap_iter->second;

      /* Get FADC waveform */
      I3WaveformSeries::const_iterator  fadcSeries_iter = fadcMap_iter->second.begin();
      if(fadcMap_iter->second.size()>1){
	for(;fadcSeries_iter != fadcMap_iter->second.end();fadcSeries_iter++){
	  if(fadcSeries_iter->GetStartTime() <= launchTimeOfThisDOM)
	    break;//this is it
	}
      }      
      const I3Waveform&  fadc_waveform = *fadcSeries_iter;

      I3PortiaPulse bestportia;	
      donebest = MakeBestPulses(atwd_p, fadc_waveform, fadc_p, om_cal, bestportia);
      (void) donebest;  //use donebest, suppress compiler warning

      log_trace("Created Best pulse at string %d/OM %d", omkey.GetString (), omkey.GetOM ());
      I3RecoPulse&  bestreco = bestportia.GetRecoPulse();
      I3RecoPulseSeries bestpulseseries;
      bestpulseseries.push_back(bestreco);
      (*bestportiamap)[omkey]=bestportia;
      (*bestrecomap)[omkey]=bestpulseseries;

    }else if (makeBestPulse_ && donefadc){
      I3PortiaPulseMap::const_iterator fadcPulseMap_iter = fadcportiamap->find(omkey);
      const I3PortiaPulse& fadc_p = fadcPulseMap_iter->second;
      (*bestportiamap)[omkey]  = fadc_p;
      const I3RecoPulse&  fadc_p_rec = fadc_p.GetRecoPulse();
      I3RecoPulseSeries bestpulseseries;
      bestpulseseries.push_back(fadc_p_rec);
      (*bestrecomap)[omkey] = bestpulseseries;
      
    }else if (makeBestPulse_&&doneatwd){

      I3PortiaPulseMap::const_iterator atwdPulseMap_iter = atwdportiamap->find(omkey);
      const I3PortiaPulse& atwd_p = atwdPulseMap_iter->second;
      (*bestportiamap)[omkey]  = atwd_p;
      const I3RecoPulse&  atwd_p_rec = atwd_p.GetRecoPulse();
      I3RecoPulseSeries bestpulseseries;
      bestpulseseries.push_back(atwd_p_rec);
      (*bestrecomap)[omkey] = bestpulseseries;
    }

  }
  //////////////////////////////////////
  /* End Looping over all the launchs */
  //////////////////////////////////////


  //now make Event-wise quantity
  largestTime_ = 0.0;
  foundLargestNPEDOM_ = false;  // initialization
  if(makeBestPulse_)  MakePortiaEvent(launchMap, bestportiamap, fadcportiamap, portia_event_ptr);
  else MakePortiaEvent(launchMap, atwdportiamap, fadcportiamap, portia_event_ptr);
  
  log_info("total best NPE %e, atwd NPE %e, fadc NPE %e",
	    portia_event_ptr->GetTotalBestNPE(),
	    portia_event_ptr->GetTotalAtwdNPE(),
	    portia_event_ptr->GetTotalFadcNPE());
  if(foundLargestNPEDOM_){ // calculate NPEs and nch from waveforms within the base time window
    bool makeBtwEvent = true;
    if(makeBestPulse_)  MakePortiaEvent(launchMap, bestportiamap, fadcportiamap, portia_event_ptr,makeBtwEvent);
    else MakePortiaEvent(launchMap, atwdportiamap, fadcportiamap, portia_event_ptr,makeBtwEvent);
  }
  log_info("total best NPEbtw %e, atwd NPEbtw %e, fadc NPEbtw %e",
	    portia_event_ptr->GetTotalBestNPEbtw(),
	    portia_event_ptr->GetTotalAtwdNPEbtw(),
	    portia_event_ptr->GetTotalFadcNPEbtw());
  
  frame->Put(outPortiaEventName_, portia_event_ptr);
  frame->Put(outFADCPortiaPulseName_, fadcportiamap);
  frame->Put(outFADCPulseSeriesName_, fadcrecomap);
  frame->Put(outATWDPortiaPulseName_, atwdportiamap);
  frame->Put(outATWDPulseSeriesName_, atwdrecomap);
  
  if(makeBestPulse_)frame->Put(outBestPortiaPulseName_, bestportiamap);

  launchMap.clear();

  PushFrame(frame,"OutBox");
}//End physics()

/////////////////////////////////////////////////////////////////////////
bool I3Portia::MakeSplittedDOMMap(I3DOMLaunchSeriesMap& launchMap,
				  I3MapKeyVectorDouble&  launchTimeMap,
				  const I3Geometry& geo,
				  bool makeIceTopPulse){

  log_info("=== in MakeSplitterMap(): now loop over I3DOMLaunchSeriesMap");

  int numberOfDOMsInThisFrame = 0;
  for(I3DOMLaunchSeriesMap::const_iterator launchMap_iter = launchMap.begin();
      launchMap_iter!=launchMap.end(); launchMap_iter++){

    OMKey omkey = launchMap_iter->first;
    log_trace("=== in MakeSplitterMap(): string %d / OM %d", 
	      omkey.GetString(), omkey.GetOM());

    const I3DOMLaunchSeries &launches = launchMap_iter->second;

    /* if there are a zero number of launches somehow, skip this DOM */
    if(launches.size()==0){
      log_debug("=== in MakeSplitterMap(): No launches for string %d / OM %d : skip this", 
		omkey.GetString(), omkey.GetOM());
      continue;
    }

    /* skip DOMs with bad om numbers */
    if(omkey.GetOM()<=0||omkey.GetOM()>64){
      log_debug("=== in MakeSplitterMap(): BAD string %d / OM %d : skip this", 
		omkey.GetString(), omkey.GetOM());
      continue;
    }
    /* if this is IceTop and not making IceTop pulses */
    if(makeIceTopPulse && geo.omgeo.find(omkey)->second.omtype == I3OMGeo::IceTop){
      log_debug("=== in MakeSplitterMap(): IceTop string %d / OM %d : skip this", 
		omkey.GetString(), omkey.GetOM());
      continue;
    }

    // make LaunchTime series of this OM that fits to the p-frame time window
    vector<double> launchTimeSeries;
    if(MakeLaunchTimeSeries(launches,launchTimeSeries)){
      launchTimeMap[omkey]=launchTimeSeries;
      numberOfDOMsInThisFrame++;
      log_trace("=== in MakeSplitterMap(): this string has %zu splitted launches",
		launchTimeSeries.size());
      //if(launchTimeSeries.size()>1)
      //log_info("=== in MakeSplitterMap(): this string has %zu splitted launches",
      //	launchTimeSeries.size());
    }

  }// loop over OMs ends

  if(numberOfDOMsInThisFrame>=1) return true;
  else return false;
  

}
/////////////////////////////////////////////////////////////////////////
bool I3Portia::MakeLaunchTimeSeries(const I3DOMLaunchSeries& launches, vector<double>& launchTimeSeries){

  log_trace("     === in MakeLaunchTimeSeries(): now loop over I3DOMLaunchSeries");

  I3DOMLaunchSeries::const_iterator launchSeries_iter = launches.begin();
  for(;launchSeries_iter != launches.end(); launchSeries_iter++){
    // We require HLC launches.
    if(!launchSeries_iter->GetLCBit() || !launchSeries_iter->GetRawFADC().size()) continue;
    double launchTime = launchSeries_iter->GetStartTime();

    launchTimeSeries.push_back(launchTime);
  }

  if(launchTimeSeries.size()>=1) return true;
  else return false;

}


/////////////////////////////////////////////////////////////////////////
void I3Portia::MakePortiaEvent(I3MapKeyVectorDouble&  launchTimeMap,
			       I3PortiaPulseMapPtr     atwdPulseMap_ptr,
			       I3PortiaPulseMapPtr     fadcPulseMap_ptr,
			       I3PortiaEventPtr        portiaEvent_ptr,
			       bool baseTimeWindowEvent){

  // 1 loop over all launch
  // 2 look for atwd and fadc pulses associated with each omkey 
  // 3 calculate event-sum values
  // 4 find the first and the last OMKey
  // 5 SetTotalBestNPE(npe),SetTotalAtwdNPE(npe),SetTotalFadcNPE(npe)
  // 6 done
  
  double totalAtwdNPE=0.0;
  double totalFadcNPE=0.0;
  double totalBestNPE=0.0;
  
  int totalAtwdLCPulse = 0;
  int totalFadcLCPulse = 0;
  double minimumTime = 0.0;
  double maximumTime = 0.0;
  
  OMKey firstOMKey(0,0);
  OMKey lastOMKey(0,0);

  double largestNPE = 0.0;
  double largestNPEtime = 0.0;
  OMKey largestOMKey(0,0);

  int atwdNchWithinBaseTimeWindow = 0;
  int fadcNchWithinBaseTimeWindow = 0;
  int totalNchWithinBaseTimeWindow = 0;

  if(baseTimeWindowEvent) log_trace("   Entering MakePortiaEvent with the basetime window option");
  
  for(I3MapKeyVectorDouble::const_iterator launchMap_iter = launchTimeMap.begin();
      launchMap_iter!=launchTimeMap.end(); launchMap_iter++){
    
    OMKey omkey = launchMap_iter->first;   
    log_trace("string %d / OM %d", omkey.GetString(), omkey.GetOM());
    
    I3PortiaPulseMap::const_iterator atwdPulseMap_iter = atwdPulseMap_ptr->find(omkey);
    I3PortiaPulseMap::const_iterator fadcPulseMap_iter = fadcPulseMap_ptr->find(omkey);
    
    double thisAtwdNPE = 0.0;
    double thisFadcNPE = 0.0;
    double thisAtwdTime10 = 0.0;
    double thisFadcTime10 = 0.0;

    bool atwdWithinBaseTimeWindow = false;
    bool fadcWithinBaseTimeWindow = false;
    
    // ATWD
    if(atwdPulseMap_iter == atwdPulseMap_ptr->end()){
      log_debug("NO ATWD Pulse found on string %d / OM %d", omkey.GetString(), omkey.GetOM());
      thisAtwdNPE = 0.0;
      
    }else{  
      
      const I3PortiaPulse& atwdpulse = atwdPulseMap_iter->second;
      thisAtwdNPE = atwdpulse.GetEstimatedNPE();
      thisAtwdTime10 = atwdpulse.GetRecoPulse().GetTime(); 
      // time when 10% of charge is captured
      
      if(!baseTimeWindowEvent){
	totalAtwdNPE += thisAtwdNPE;
      }else{
	if(((thisAtwdTime10-largestTime_)>=kStartTimeBtw_) &&
	   ((thisAtwdTime10-largestTime_)<=kEndTimeBtw_)){
	  atwdWithinBaseTimeWindow = true;
	  atwdNchWithinBaseTimeWindow++;
	  totalAtwdNPE += thisAtwdNPE;
	}
      }
      
      if(!baseTimeWindowEvent){
	if(atwdpulse.GetLCBit())++totalAtwdLCPulse;
      }
    }
    
    // FADC
    if(fadcPulseMap_iter == fadcPulseMap_ptr->end()){
      log_debug("NO FADC Pulse found on string %d / OM %d", omkey.GetString(), omkey.GetOM());
      thisFadcNPE= 0.0;
      
    }else{
      const I3PortiaPulse& fadcpulse = fadcPulseMap_iter->second;
      thisFadcTime10 = fadcpulse.GetRecoPulse().GetTime(); 
      // time when 10% of charge is captured
      
      thisFadcNPE = fadcpulse.GetEstimatedNPE();
      if(!baseTimeWindowEvent){
	totalFadcNPE += thisFadcNPE;
      }else{
	if(((thisFadcTime10-largestTime_)>=kStartTimeBtw_) &&
	   ((thisFadcTime10-largestTime_)<=kEndTimeBtw_)){
	  fadcWithinBaseTimeWindow = true;
	  fadcNchWithinBaseTimeWindow++;
	  totalFadcNPE += thisFadcNPE;
	}
      }
      
      if(!baseTimeWindowEvent){
	if(fadcpulse.GetLCBit())++totalFadcLCPulse;
      }
    }
    
    /////////////////////////////////////
    // case when FADC is chosen for best NPE
    if( thisFadcNPE > thisAtwdNPE){
      if(!baseTimeWindowEvent){
	totalBestNPE += thisFadcNPE;
	// search for DOM with latgest NPE
	if(thisFadcNPE>largestNPE){
	  largestNPE = thisFadcNPE;
	  largestNPEtime = thisFadcTime10;
	  largestOMKey = omkey;
	  log_trace("  possibly lergest NPE %f from FADC in String %d / OM %d", 
		    thisFadcNPE, omkey.GetString(), omkey.GetOM());
	}
      }else{
	if(fadcWithinBaseTimeWindow) {
	  totalBestNPE += thisFadcNPE;
	  totalNchWithinBaseTimeWindow++;
	  log_trace("  OM( %d %d) is within the basetime window around the lnt %f", 
		    omkey.GetString(), omkey.GetOM(), largestTime_);
	}
      }
      // search for DOMs with the earliest and latest waveform capture
      if(!baseTimeWindowEvent||fadcWithinBaseTimeWindow){
	if(minimumTime<=0.0 || (thisFadcTime10 < minimumTime && thisFadcTime10>0)){
	  log_trace("  possibly earliest NPE hit from FADC in OM( %d %d) with t10 = %f", 
		    omkey.GetString(), omkey.GetOM(), thisFadcTime10);
	  minimumTime = thisFadcTime10;
	  firstOMKey = omkey;}
      
	if(thisFadcTime10 > maximumTime){maximumTime = thisFadcTime10;
	  lastOMKey = omkey;
	}
      }

    // case when ATWD is chosen for best NPE
    }else if( thisAtwdNPE >= thisFadcNPE){
      if(!baseTimeWindowEvent){
	totalBestNPE += thisAtwdNPE;
	// search for DOM with latgest NPE
	if(thisAtwdNPE>largestNPE){
	  largestNPE = thisAtwdNPE;
	  largestNPEtime = thisAtwdTime10;
	  largestOMKey = omkey;
	  log_trace("  possibly lergest NPE %f from ATWD in String %d / OM %d", 
		    thisAtwdNPE, omkey.GetString(), omkey.GetOM());
	}
      }else{
	if(atwdWithinBaseTimeWindow){
	  totalBestNPE += thisAtwdNPE;
	  totalNchWithinBaseTimeWindow++;
	  log_trace("  OM( %d %d) is within the basetime window around the lnt %f", 
		    omkey.GetString(), omkey.GetOM(), largestTime_);
	}
      }
      // search for DOMs with the earliest and latest waveform capture
      if(!baseTimeWindowEvent||atwdWithinBaseTimeWindow){
	if(minimumTime<=0.0 || (thisAtwdTime10 < minimumTime && thisAtwdTime10>0)){
	  log_trace("  possibly earliest NPE hit from ATWD in OM( %d %d) with t10 = %f", 
		    omkey.GetString(), omkey.GetOM(), thisAtwdTime10);
	  minimumTime = thisAtwdTime10;
	  firstOMKey = omkey;}
      
	if(thisAtwdTime10 > maximumTime){maximumTime = thisAtwdTime10;
	  lastOMKey = omkey;
	}
      }
      
    }
    /////////////////////////////////////
    
  }//dom launch loop
  

  if(!baseTimeWindowEvent){
    portiaEvent_ptr->SetFirstPulseOMKey(firstOMKey);
    portiaEvent_ptr->SetLastPulseOMKey(lastOMKey);
    portiaEvent_ptr->SetTotalBestNPE(totalBestNPE);
    portiaEvent_ptr->SetTotalAtwdNPE(totalAtwdNPE);
    portiaEvent_ptr->SetTotalFadcNPE(totalFadcNPE);
    portiaEvent_ptr->SetTotalNch(  launchTimeMap.size() );
    portiaEvent_ptr->SetAtwdNch(atwdPulseMap_ptr->size() );
    portiaEvent_ptr->SetFadcNch(fadcPulseMap_ptr->size() );
    portiaEvent_ptr->SetLargestNPEOMKey(largestOMKey);
    largestTime_ = largestNPEtime;
    //if(largestNPEtime>0.0 ) foundLargestNPEDOM_ = true;
    foundLargestNPEDOM_ = true;
  }else{
    portiaEvent_ptr->SetFirstPulseOMKeybtw(firstOMKey);
    portiaEvent_ptr->SetLastPulseOMKeybtw(lastOMKey);
    portiaEvent_ptr->SetTotalBestNPEbtw(totalBestNPE);
    portiaEvent_ptr->SetTotalAtwdNPEbtw(totalAtwdNPE);
    portiaEvent_ptr->SetTotalFadcNPEbtw(totalFadcNPE);
    portiaEvent_ptr->SetTotalNchbtw(totalNchWithinBaseTimeWindow);
    portiaEvent_ptr->SetAtwdNchbtw(atwdNchWithinBaseTimeWindow);
    portiaEvent_ptr->SetFadcNchbtw(fadcNchWithinBaseTimeWindow);
  }

}

/////////////////////////////////////////////////////////////////////////
bool I3Portia::MakeBestPulses(
			      const I3PortiaPulse&    atwd_p,
			      const I3Waveform&       fadc_w, 
			      const I3PortiaPulse&    fadc_p,
			      const I3DOMCalibration& calib,
			      I3PortiaPulse&          best_p){    

  /**
    Now I use the fact that the saturation point of ATWD is much higher than FADC
    and upto the end of ATWD capture, ATWD estimated NPE is more reliable.
  */
  const vector<double>& calibratedFADC  = fadc_w.GetWaveform();
  
  /**
     Get NPE and the end time of the ATWD waveforms
  */
  double atwd_based_NPE = atwd_p.GetEstimatedNPE();
  double atwd_end_time = atwd_p.GetStartTime() + atwd_p.GetBinSize()*atwd_p.GetBinNumber();  

  int fadc_bin_number = fadc_p.GetBinNumber();
  double fadc_bin_size = fadc_p.GetBinSize();//in i3unist
  double fadc_starttime = fadc_p.GetStartTime();
  
  vector<double> pedestralFADC(fadc_bin_number); double fadc_baseline = fadc_p.GetBaseLine();
  transform( calibratedFADC.begin(), calibratedFADC.end(), pedestralFADC.begin(), bind2nd(minus<double>(), fadc_baseline) );  
  
  double fadc_charge = 0.0; int thres_bin_number = 0; double thres_time = 0.0;

  for(int i=0;  i< fadc_bin_number;  ++i){
    /**
       at first find the time new fadc starts after atwd waveforms end
       then add FADC charge after words
    */
    double currentTime = fadc_starttime + i*fadc_bin_size;
    if(currentTime >= atwd_end_time){
      if(thres_bin_number==0){ thres_bin_number = i; thres_time = currentTime; }
      if(pedestralFADC.at(i)>0.0){ fadc_charge += pedestralFADC.at(i)*fadc_bin_size; }//timecharge has unit [ns*V] at this point
    }
  }
  
  double lefttime_bin_size = (thres_time - atwd_end_time);
  if(lefttime_bin_size > fadc_bin_size)
    log_error("lefttime %f should be smaller than binsize %f", lefttime_bin_size, fadc_bin_size);

  /* Then add i-1th bin */
  if(pedestralFADC.at(thres_bin_number - 1)>0.0){ fadc_charge += pedestralFADC.at(thres_bin_number - 1 )*lefttime_bin_size; }

  if(fadc_p.GetPMTGain() <=1.0  || std::isnan(fadc_p.GetPMTGain()) || fadc_p.GetPMTGain() == 0.0)
    {
      log_error("PMT Gain from DOM Cal file is not correctly filled %f", fadc_p.GetPMTGain());
      log_error("Set PMT Gain to the default value of %f", pmtGain_);
    }

  //in ohm
  const double          impedance          = calib.GetFrontEndImpedance();
  fadc_charge /= impedance;
  double fadc_npe  = fadc_charge / pmtGain_;

  I3RecoPulse& bestrecopulse = best_p.GetRecoPulse();

  //In the case that atwd NPE is more than 10% of fadc after atwd NPE
  if(atwd_based_NPE > fadc_npe/10.0){
    
    bestrecopulse.SetTime(atwd_p.GetRecoPulse().GetTime());//in i3unit
    bestrecopulse.SetWidth(atwd_p.GetRecoPulse().GetWidth());//in i3unit 	  
    
  }else{
    
    bestrecopulse.SetTime(fadc_p.GetRecoPulse().GetTime());//in i3unit
    bestrecopulse.SetWidth(fadc_p.GetRecoPulse().GetWidth());//in i3unit 	  
    
  }

  bestrecopulse.SetCharge(atwd_p.GetRecoPulse().GetCharge() + fadc_npe);//in npe
 
  best_p.SetIntegratedCharge(atwd_p.GetIntegratedCharge() + fadc_charge);//in i3units
  best_p.SetEstimatedNPE(fadc_npe + atwd_based_NPE);
  best_p.SetBaseLine( atwd_p.GetBaseLine() + fadc_p.GetBaseLine() );//in i3units

  if(atwd_p.GetAmplitude()>fadc_p.GetAmplitude()){
    best_p.SetAmplitude(atwd_p.GetAmplitude());//in i3units
    best_p.SetPeakBinTime(atwd_p.GetPeakBinTime());
  } else {
    best_p.SetAmplitude(fadc_p.GetAmplitude());//in i3units
    best_p.SetPeakBinTime(fadc_p.GetPeakBinTime());
  }
  
  best_p.SetPositionX(atwd_p.GetPositionX());
  best_p.SetPositionY(atwd_p.GetPositionY());
  best_p.SetPositionZ(atwd_p.GetPositionZ());
  best_p.SetLCBit(atwd_p.GetLCBit());
  
  best_p.SetTime50(atwd_p.GetTime50());//in i3units
  best_p.SetTime80(atwd_p.GetTime80());//in i3units
  best_p.SetTOT(atwd_p.GetTOT());//in i3units
  best_p.SetLETime(atwd_p.GetLETime());//in i3units
  best_p.SetLaunchTime(atwd_p.GetLaunchTime());//in i3units
  best_p.SetStartTime(atwd_p.GetStartTime());//in i3units  best_p.SetBinNumber(fadc_p.GetBinNumber());
  best_p.SetBinSize(lefttime_bin_size);//in i3unist
  
  return true;
}
/////////////////////////////////////////////////////////////////////////
bool I3Portia::MakeFADCPulses(double launchTimeOfThisDOM,
			      const I3Waveform&       wave, //calibrated
			      const I3DOMCalibration& calib,
			      const I3DOMStatus&      status,
			      const I3Position&       pos,
			      I3PortiaPulse&          portia){    

  log_trace("Let's Find FADC Pulses!");

  // 1) Get waveform 
  //calibrated FADC returns values with Giga Volt (I3Units internal default)
  const std::vector<double> &calibratedFADC = wave.GetWaveform();
  const double          startTime          = wave.GetStartTime();
  const double          launchTime         = launchTimeOfThisDOM;
  const bool            lcbit              = true;
  const double          fadcBinSize        = wave.GetBinWidth();
  const int             fadcBinNumber      = calibratedFADC.size();
  const double          pmtHV              = status.pmtHV/I3Units::V;
  const double          HVGainFitSlope     = calib.GetHVGainFit().slope;
  const double          HVGainFitIntercept = calib.GetHVGainFit().intercept;
  //in ohm
  const double          impedance          = calib.GetFrontEndImpedance();
  /* 
    Definition of pmt gain
    Get DOMCAL measured PMT gain/HV relation-> log(10)Gain = slope*log(10)V + intercept;
  */
  double domcalPmtGain = pow(10, log10(pmtHV) * HVGainFitSlope + HVGainFitIntercept);
  /*
    Sometime DB is not correctly filled, e.g., default value of 10^7 is somthing like 1 or NAN, often the case of DB fail
  */
  if(domcalPmtGain <=1.0  || std::isnan(domcalPmtGain) || domcalPmtGain == 0.0)
    {
      log_error("PMT Gain from DOM Cal file is not correctly filled %f", domcalPmtGain);
      log_error("Set PMT Gain to the default value of %f", pmtGain_);
      domcalPmtGain=pmtGain_;
    }

  //for the case we are getting raw fadc - comment out below
  /*
  const vector<int>& rawFADC  = launch.GetRawFADCWaveform();
  vector<double> calibratedFADC(fadcBinNumber);
  const double   conversion =  0.0021;
  double fadcGain      = calib.GetFADCGain();
  if(!fadcGain) fadcGain = 22.5;
  transform( rawFADC.begin(), rawFADC.end(), calibratedFADC.begin(), bind2nd(multiplies<double>(), conversion*I3Units::V/fadcGain) );  
  for_each( rawFADC.begin(), rawFADC.end(), print );
  for_each( calibratedFADC.begin(), calibratedFADC.end(), print );
  */
 
  if(calibratedFADC.size()<256)for_each( calibratedFADC.begin(), calibratedFADC.end(), print_in_mv);
  /* 2) Base line*/
  double base=0.0;
  string decidedOption;

  if (optionFADCBaseLine_=="eheoptimized")
    { 
      int nbin=3; double avebin=0.0;
      for(int i=0; i<nbin; ++i)
	{
	  avebin += calibratedFADC.at(i);
	}
      avebin /= nbin;
      base=avebin;
      log_trace("Average over first %d bins gives: baseline %f [mV]", nbin, base/I3Units::mV);
      if(fabs(base)>0.5*I3Units::mV) decidedOption="zerobaseline";
    } 

 
    if(optionFADCBaseLine_=="zerobaseline"||decidedOption=="zerobaseline")
      {
	base=0.0*I3Units::mV;
	log_trace("zero baseline is used for FADC: baseline %f [mV]", base/I3Units::mV);
      }
    else if(optionFADCBaseLine_=="first")
      {
	int nbin=3; double avebin=0.0;
	for(int i=0; i<nbin; ++i)
	  {
	    avebin += calibratedFADC.at(i);
	  }
	avebin /= nbin;base=avebin;
	log_trace("Average over first %d bins gives: baseline %f [mV]", nbin, base/I3Units::mV);
      }
    else if(optionFADCBaseLine_=="last")
      {
	int nbin=3; double avebin=0.0;
	vector<double>::const_reverse_iterator rItr = calibratedFADC.rbegin();
	for( int i=0; i <nbin ; ++i)
	  {
	    avebin += (*rItr);
	    ++rItr;
	  }
	avebin /= nbin; base=avebin;
	log_trace("Average over last %d bins gives: baseline %f [mV]", nbin, base/I3Units::mV);
      }
    else if(optionFADCBaseLine_=="lower")
      {
	vector<double> sortedFADC(calibratedFADC.begin(),calibratedFADC.end());  
	int nbin=0;double avebin=0.0;
	vector<double>::iterator	sitr1    = sortedFADC.begin();
	vector<double>::iterator	sitr2    = sortedFADC.end();
	
	sort( sitr1, sitr2 );
	//for_each( sitr1, sitr2, print );
	
	for(int i=0; i<fadcBinNumber/16; ++i)//average of lowest 8 bins
	  { 
	    avebin += calibratedFADC.at(i); 
	    ++nbin; 
	  }
	avebin/=nbin;base=avebin;
	log_trace("Average over lower %d bins gives: baseline %f [mV]", nbin, base/I3Units::mV);
      }
    else if (optionFADCBaseLine_=="first_or_last")
      {
	int nbin_first=3; double avebin_first=0.0;
	for(int i=0; i<nbin_first; ++i)
	  {
	    avebin_first += calibratedFADC.at(i);
	  }
	avebin_first /= nbin_first; 
	

	int nbin_last=3; double avebin_last=0.0;
	vector<double>::const_reverse_iterator rItr = calibratedFADC.rbegin();
	for( int i=0; i <nbin_last ; ++i)
	  {
	    avebin_last += (*rItr);
	    ++rItr;
	  }
	avebin_last /= nbin_last; 
	

	if(avebin_first>avebin_last){
	  base=avebin_last;
	  log_trace("Average over last %d bins gives: baseline %f [mV]", nbin_last, base/I3Units::mV);
	} else {
	  base=avebin_first;      
	  log_trace("Average over first %d bins gives: baseline %f [mV]", nbin_first, base/I3Units::mV);
	}
      }
    else if (optionFADCBaseLine_=="iteration")
      {
	//step1:take average about all samples
	double avebin = accumulate(calibratedFADC.begin(), calibratedFADC.end(), 0.0);
	avebin /= (fadcBinNumber);
	log_trace("Exclude bins if the bin enrtry is larger than: %f [micro volt]", avebin/I3Units::microvolt);
	
	//copy calibrated FADC
	vector<double> reducedFADC(calibratedFADC.begin(), calibratedFADC.end());
	//remove if entry is greater than the value
	reducedFADC.erase(remove_if(reducedFADC.begin(), reducedFADC.end(), bind2nd(greater<double>(), avebin)), reducedFADC.end());

	log_trace("reduced waveform %zu counts out of initial %zu count ", 
		  reducedFADC.size(), calibratedFADC.size());
	
	//sum filled reduced FADC entries
	double avebin_reduced = accumulate(reducedFADC.begin(), reducedFADC.end(), 0.0);
	//sum filled reduced FADC entry SQUARE
	double avebin_square_reduced = accumulate(reducedFADC.begin(), reducedFADC.end(), 0.0, square_sum);
	//or double avebin_square_reduced = inner_product(reducedFADC.begin(), reducedFADC.end(), reducedFADC.begin(), 0.0);
	avebin_reduced /= reducedFADC.size();
	avebin_square_reduced /= reducedFADC.size();
	//obtain RMS
	double baselineRMS= sqrt(avebin_square_reduced - avebin_reduced*avebin_reduced);
	log_trace("reduce if |bin| larger than: %f [micro volts]", baselineRMS*1.5/I3Units::microvolt);
	
	//try 10 times
	int number_of_trial =10;
	for(int k=0; k < number_of_trial; k++){				
	  double baselineSum=0.0;
	  double baselineSumSquare=0.0;
	  int    count=0;
	  
	  vector<double>::const_iterator reduce_iter = reducedFADC.begin();
	  for(; reduce_iter!=reducedFADC.end(); reduce_iter++){
	    
	    if(fabs((*reduce_iter)-avebin_reduced)<=baselineRMS*1.5){
	      baselineSum+=(*reduce_iter);
	      baselineSumSquare+=(*reduce_iter)*(*reduce_iter);
	      count++;
	    }
	  }
	  
	  avebin_reduced = baselineSum/count;
	  double baselineSquareAverage=baselineSumSquare/count;
	  
	  baselineRMS=sqrt(baselineSquareAverage-avebin_reduced*avebin_reduced);
	  log_trace("new average:%f over count %d obtained new RMS: %f", avebin_reduced, count, baselineRMS);
	  //if entry is smaller than 30 take the value
	  if(count<=30)break;
	}
	
	base = avebin_reduced;
	//////////////////////////////////////
  	log_trace("all bin initial average:%f [micro volt]", avebin/I3Units::microvolt);
	log_trace("baseline obtained :%f [micro volt]", base/I3Units::microvolt);
      }

        else if(optionFADCBaseLine_=="eheoptimized"&&decidedOption!="iteration"){
	}
	else {
	  
	  log_error("No such base line option %s for FADC implemented", optionFADCBaseLine_.c_str());
	  base=0.0;
	}
  
  log_trace("Base Line %f is obtained", base/I3Units::mV);
  
  /* 3) */

  vector<double> pedestralFADC(fadcBinNumber);  
  transform( calibratedFADC.begin(), calibratedFADC.end(), pedestralFADC.begin(), bind2nd(minus<double>(), base) );  
  
  /* 4) sum charge up*/
  bool aboveThreshold=false;
  double charge=0.0;
  for(int i=0; i<fadcBinNumber; ++i){
    if(pedestralFADC.at(i)>0.0) charge += pedestralFADC.at(i)*fadcBinSize;
  }

  /* 5) find time*/
  double timecharge=0.0;
  double time10 = 0.0;
  double time50 = 0.0;
  double time80 = 0.0;
  double LEtime = 0.0;

  for(int i=0; i<fadcBinNumber; ++i) //loop over fadc bins
    {  

      if((LEtime==0.0)&&(pedestralFADC.at(i) >= fadcThresholdLEAmp_))
	{ //LE time
	  LEtime=startTime+(i*fadcBinSize);
	}

      if(pedestralFADC.at(i)>0.0){
	//timecharge has unit [ns*V] at this point
	timecharge += pedestralFADC.at(i)*fadcBinSize;
	
	if(time10==0.0&&timecharge>=charge*0.1)
	  { //10%
	    time10=startTime+(i*fadcBinSize);
	  }
	if(time50==0.0&&timecharge>=charge*0.5)
	  { //10%
	    time50=startTime+(i*fadcBinSize);
	  }
	if(time80==0.0&&timecharge>=charge*0.8)
	  { //10%
	    time80=startTime+(i*fadcBinSize);
	  }
      }
    }

  double EndEdgetime = 0.0;
   for(int i=fadcBinNumber-1; i>0; --i) //loop over atwd bins
    {  
      if((EndEdgetime==0.0)&&(pedestralFADC.at(i) >= fadcThresholdLEAmp_))
	{ //EndEdge time
	  EndEdgetime=startTime+(i*fadcBinSize);
	}
    }
  //charge to GV*ns = C
  charge /= impedance; //now in [1/C]

  //Charge has unit [pC] at this point
  //threshold charge also in [pC]
  if(charge >= fadcThresholdCharge_)aboveThreshold=true;
  if(!aboveThreshold)return false;

  log_trace("FADC IntegratedCharge is %f [pC], estimaed NPE %f", charge/I3Units::pC, charge/domcalPmtGain);
  log_trace("Start Time is %f [ns], Time10 is %f [ns] at when sum charge exceeds %f [pC]", 
	   startTime/I3Units::ns, 
	   time10/I3Units::ns, 
	   charge*0.1/I3Units::pC);

  log_trace("LE Time is %f [ns] with the threshold of %f [mV]", 
	    LEtime/I3Units::ns, 
	    fadcThresholdLEAmp_/I3Units::mV);

  /* 6) find highest bin position */
  vector<double>::iterator highestEntryItr = max_element(pedestralFADC.begin(), pedestralFADC.end());
  int binPos=distance(pedestralFADC.begin(), highestEntryItr);

  /* 7) fill results back to dataclasses */
  I3RecoPulse& recopulse = portia.GetRecoPulse();
  
  recopulse.SetTime(time10);//in i3unit
  recopulse.SetCharge(charge/domcalPmtGain);//in i3unit 
  recopulse.SetWidth(time80-time10);//in i3unit 
  portia.SetTime50(time50);//in i3units
  portia.SetTime80(time80);//in i3units
  portia.SetTOT(EndEdgetime-LEtime);//in i3units
  portia.SetBinNumber(fadcBinNumber);//in i3units
  portia.SetBinSize(fadcBinSize);//in i3unist
  portia.SetLETime(LEtime);//in i3units
  portia.SetLaunchTime(launchTime);//in i3units
  portia.SetStartTime(startTime);//in i3units
  portia.SetPeakBinTime(startTime+binPos*fadcBinSize);
  portia.SetIntegratedCharge(charge);//in i3units
  portia.SetEstimatedNPE(charge/domcalPmtGain);
  portia.SetBaseLine(base);//in i3units
  portia.SetAmplitude(*highestEntryItr);//in i3units
  portia.SetPositionX(pos.GetX());
  portia.SetPositionY(pos.GetY());
  portia.SetPositionZ(pos.GetZ());
  portia.SetLCBit(lcbit);
  portia.SetPMTGain(domcalPmtGain);
  log_trace("fadc t10 %f [ns],  startTime %f [ns]", 
	    portia.GetRecoPulse().GetTime()/I3Units::ns, portia.GetStartTime()/I3Units::ns );
 
  return true;
}

/////////////////////////////////////////////////////////////////////////
bool I3Portia::MakeATWDPulses(double launchTimeOfThisDOM,
			      const I3Waveform&       wave, 
			      const I3DOMCalibration& calib,
			      const I3DOMStatus&      status,
			      const I3Position&       pos,
			      I3PortiaPulse&          portia){
  log_trace("Let's Find ATWD Portia Pulse!");

  /**
   * In this function we will 1) get  basic waveform and atwd numbers  
   *                          2) find baseline
   *                          3) get pedestral subtracted waveform
   *                          4) sum up the waveform bins to get integrated charge
   *                          5) re-sum to find out 10% charge point
   *                          6) create EHEHits and fill these info
   *                          7) fill back the EHEHits into dataclasses PulseSeriesDict
   */  

  /* 1) Get waveform */
  //calibrated ATWD returns values with Giga Volt
  const vector<double>& calibratedATWD  = wave.GetWaveform();
  const double   launchTime    = launchTimeOfThisDOM;
  const double   startTime     = wave.GetStartTime();
  const bool     lcbit         = true;
  const double   atwdBinSize   = wave.GetBinWidth();
  const int      atwdBinNumber = calibratedATWD.size();
  const double pmtHV = status.pmtHV/I3Units::V;
  const double HVGainFitSlope = calib.GetHVGainFit().slope;
  const double HVGainFitIntercept = calib.GetHVGainFit().intercept;
  //in ohm
  const double          impedance          = calib.GetFrontEndImpedance();
  
  /*
    Get DOMCAL measured PMT gain/HV relation-> log(10)Gain = slope*log(10)V + intercept;
  */
  double domcalPmtGain = pow(10, log10(pmtHV) * HVGainFitSlope + HVGainFitIntercept);

  /*
    Sometime DB is not correctly filled, e.g., default value of 10^7 is somthing like 1 or NAN, often the case of DB fail
  */
  if(domcalPmtGain <=1.0  || std::isnan(domcalPmtGain) || domcalPmtGain == 0.0)
    {
      log_error("PMT Gain from DOM Cal file is not correctly filled %f", domcalPmtGain);
      log_error("Set PMT Gain to the default value of %f", pmtGain_);
      domcalPmtGain=pmtGain_;
    }

  /* 2) Base line*/
  double base=0.0*I3Units::mV;
  string decidedOption;

  if(optionATWDBaseLine_=="eheoptimized")
    {
      int nbin_first=2; double avebin_first=0.0;
      for(int i=0; i<nbin_first; ++i) avebin_first += calibratedATWD.at(i);

      avebin_first /= nbin_first; 
      base=avebin_first;
	
      if(base<=-1.0*I3Units::mV || base>=2.0*I3Units::mV) base=0.0*I3Units::mV;

      vector<double> testpedestralATWD(atwdBinNumber);  
      transform( calibratedATWD.begin(), calibratedATWD.end(), testpedestralATWD.begin(), bind2nd(minus<double>(), base) );  

      double testNPE=0.0;
      for(int i=0; i<atwdBinNumber; ++i) testNPE += testpedestralATWD.at(i)*atwdBinSize;
      //testNPE has internal unit [ns*GV] at this point
      //charge to GV*ns = C
      testNPE /= impedance;
      testNPE /= domcalPmtGain;
      if((testNPE<=20.0)&&(testNPE>=-10.0)&&(avebin_first>=-1.0*I3Units::mV&&avebin_first<=2.0*I3Units::mV)) decidedOption="iteration";
    }


  /////////////////////////////////////   /////////////////////////////////////
  if(optionATWDBaseLine_=="zerobaseline" || decidedOption=="zerobaseline")
    {
      base=0.0*I3Units::mV;
      log_trace("zero baseline is used for ATWD: baseline %f [mV]", base/I3Units::mV);
    }
  else if(optionATWDBaseLine_=="first")
    {
      int nbin=2; double avebin=0.0;
      for(int i=0; i<nbin; ++i)
	{
	  avebin += calibratedATWD.at(i);
	}
      avebin /= nbin;base=avebin;
      log_trace("Average over first %d bins gives: baseline %f [mV]", nbin, base/I3Units::mV);
    }
  else if(optionATWDBaseLine_=="last")
    {
      int nbin=3; double avebin=0.0;
      vector<double>::const_reverse_iterator rItr = calibratedATWD.rbegin();
      for( int i=0; i <nbin ; ++i)
	{
	  avebin += (*rItr);
	  ++rItr;
	}
      avebin /= nbin; base=avebin;
      log_trace("Average over last %d bins gives: baseline %f [mV]", nbin, base/I3Units::mV);
    }
  else if(optionATWDBaseLine_=="lower")
    {
      vector<double> sortedATWD(calibratedATWD.begin(),calibratedATWD.end());  
      int nbin=0;double avebin=0.0;
      vector<double>::iterator	sitr1    = sortedATWD.begin();
      vector<double>::iterator	sitr2    = sortedATWD.end();
      
      sort( sitr1, sitr2 );

      for(int i=0; i<atwdBinNumber/16; ++i)//average of lowest 8 bins
	{ 
	  avebin += calibratedATWD.at(i); 
	  ++nbin; 
	}
      avebin/=nbin;base=avebin;
      log_trace("Average over lower %d bins gives: baseline %f [mV]", nbin, base/I3Units::mV);
    }
  else if (optionATWDBaseLine_=="first_or_last")
    {
      int nbin_first=3; double avebin_first=0.0;
      for(int i=0; i<nbin_first; ++i)
	{
	  avebin_first += calibratedATWD.at(i);
	}
      avebin_first /= nbin_first; 
      //--
      int nbin_last=3; double avebin_last=0.0;
      vector<double>::const_reverse_iterator rItr = calibratedATWD.rbegin();
      for( int i=0; i <nbin_last ; ++i)
	{
	  avebin_last += (*rItr);
	  ++rItr;
	}
      avebin_last /= nbin_last; 
      ////////
      if(avebin_first>avebin_last){
	base=avebin_last;
	log_trace("Average over last %d bins gives: baseline %f [mV]", nbin_last, base/I3Units::mV);
      } else {
	base=avebin_first;      
	log_trace("Average over first %d bins gives: baseline %f [mV]", nbin_first, base/I3Units::mV);
      }
      ////////
    } 
  else if (optionATWDBaseLine_=="iteration"||decidedOption=="iteration")
    {
      //step1:take average about all samples except the first bin
      double avebin = accumulate(++calibratedATWD.begin(), calibratedATWD.end(), 0.0);
      avebin /= (atwdBinNumber-1);
      log_trace("Exclude bins if the bin enrtry is larger than: %f [micro volt]", avebin/I3Units::microvolt);

      //copy calibrated ATWD except the first bin
      vector<double> reducedATWD(++calibratedATWD.begin(), calibratedATWD.end());
      //remove if entry is greater than the value
      reducedATWD.erase(remove_if(reducedATWD.begin(), reducedATWD.end(), bind2nd(greater<double>(), avebin)), reducedATWD.end());
      //for_each( reducedATWD.begin(), reducedATWD.end(), print_in_mv );
      log_trace("reduced waveform %zu counts out of initial %zu count ", 
		reducedATWD.size(), calibratedATWD.size());

      //sum filled reduced ATWD entries
      double avebin_reduced = accumulate(reducedATWD.begin(), reducedATWD.end(), 0.0);
      //sum filled reduced ATWD entry SQUARE
      double avebin_square_reduced = accumulate(reducedATWD.begin(), reducedATWD.end(), 0.0, square_sum);
      //or double avebin_square_reduced = inner_product(reducedATWD.begin(), reducedATWD.end(), reducedATWD.begin(), 0.0);
      avebin_reduced /= reducedATWD.size();
      avebin_square_reduced /= reducedATWD.size();
      //obtain RMS
      double baselineRMS= sqrt(avebin_square_reduced - avebin_reduced*avebin_reduced);
      log_trace("reduce if |bin| larger than: %f [micro volts]", baselineRMS*1.5/I3Units::microvolt);

      //try 10 times
      int number_of_trial =10;
      for(int k=0; k < number_of_trial; k++){				
	double baselineSum=0.0;
	double baselineSumSquare=0.0;
	int    count=0;
	
	vector<double>::const_iterator reduce_iter = reducedATWD.begin();
	for(; reduce_iter!=reducedATWD.end(); reduce_iter++){
	  
	  if(fabs((*reduce_iter)-avebin_reduced)<=baselineRMS*1.5){
	    baselineSum+=(*reduce_iter);
	    baselineSumSquare+=(*reduce_iter)*(*reduce_iter);
	    count++;
	  }
	}

	avebin_reduced = baselineSum/count;
	double baselineSquareAverage=baselineSumSquare/count;
	
	baselineRMS=sqrt(baselineSquareAverage-avebin_reduced*avebin_reduced);
	log_trace("new average:%f over count %d obtained new RMS: %f", avebin_reduced, count, baselineRMS);
	//if entry is smaller than 30 take the value
	if(count<=30)break;
      }
      
      base = avebin_reduced;
      //////////////////////////////////////
      log_trace("all bin initial average:%f [micro volt]", avebin/I3Units::microvolt);
      log_trace("baseline obtained :%f [micro volt]", base/I3Units::microvolt);
    }/////////////////////////////////////
      else if(optionATWDBaseLine_=="eheoptimized"&&decidedOption!="iteration"){
      }
  else {
    log_trace("No such base line option %s for ATWD implemented", optionATWDBaseLine_.c_str());
      base=0.0;
    }


  log_trace("Base Line %f is obtained", base/I3Units::mV);

  /* 3) */
  vector<double> pedestralATWD(atwdBinNumber);  
  transform( calibratedATWD.begin(), calibratedATWD.end(), pedestralATWD.begin(), bind2nd(minus<double>(), base) );  
  
  /* 4) sum charge up*/
  bool aboveThreshold=false;
  double charge=0.0;
  for(int i=0; i<atwdBinNumber; ++i) charge += pedestralATWD.at(i)*atwdBinSize;

  /* 5) find time*/
  double timecharge=0.0;
  double time10 = 0.0;
  double time50 = 0.0;
  double time80 = 0.0;
  double LEtime = 0.0;

  for(int i=0; i<atwdBinNumber; ++i) //loop over atwd bins
    {  
      if((LEtime==0.0)&&(pedestralATWD.at(i) >= atwdThresholdLEAmp_))
	{ //LE time
	  LEtime=startTime+(i*atwdBinSize);
	}

      if(pedestralATWD.at(i)>0.0){
	//timecharge has unit [ns*V] at this point
	timecharge += pedestralATWD.at(i)*atwdBinSize;
	
	if(time10==0.0&&timecharge>=charge*0.1)
	  { //10%
	    time10=startTime+(i*atwdBinSize);
	  }
	if(time50==0.0&&timecharge>=charge*0.5)
	  { //10%
	    time50=startTime+(i*atwdBinSize);
	  }
	if(time80==0.0&&timecharge>=charge*0.8)
	  { //10%
	    time80=startTime+(i*atwdBinSize);
	  }
      }
    }

  double EndEdgetime = 0.0;
   for(int i=atwdBinNumber-1; i>0; --i) //loop over atwd bins
    {  
      if((EndEdgetime==0.0)&&(pedestralATWD.at(i) >= atwdThresholdLEAmp_))
	{ //EndEdge time
	  EndEdgetime=startTime+(i*atwdBinSize);
	}
    } 

  //charge to GV*ns = C
  charge /= impedance; //now in [1/C]
  
  //Charge has unit [pC] at this point
  //threshold charge also in [pC]
  if(charge >= atwdThresholdCharge_)aboveThreshold=true;
  
  if(!aboveThreshold)return false;

  log_trace("ATWD IntegratedCharge is %f [pC], estimaed NPE %f", charge/I3Units::pC, charge/domcalPmtGain);
  log_trace("Launch Time is %f [ns], Time10 is %f [ns] at when sum charge exceeds %f [pC]", 
	   startTime/I3Units::ns, 
	   time10/I3Units::ns, 
	   charge*0.1/I3Units::pC);

  log_trace("LE Time is %f [ns] with the threshold of %f [mV]", 
	    LEtime/I3Units::ns, 
	    atwdThresholdLEAmp_/I3Units::mV);

  /* 6) find highest bin position */
  vector<double>::iterator highestEntryItr = max_element(pedestralATWD.begin(), pedestralATWD.end());
  int binPos=distance(pedestralATWD.begin(), highestEntryItr);
  //log_info("atwd peak position %d th bin found", binPos);

  /* 7) fill results back to dataclasses */
  I3RecoPulse& recopulse = portia.GetRecoPulse();

  recopulse.SetTime(time10);//in i3unit
  recopulse.SetCharge(charge/domcalPmtGain);//in i3unit 
  recopulse.SetWidth(time80-time10);//in i3unit

  portia.SetTime50(time50);//in i3units
  portia.SetTime80(time80);//in i3units
  portia.SetLETime(LEtime);//in i3units
  portia.SetTOT(EndEdgetime - LEtime);//in i3units
  portia.SetBinNumber(atwdBinNumber);//in i3units
  portia.SetBinSize(atwdBinSize);//in i3unist
  portia.SetLaunchTime(launchTime);//in i3units
  portia.SetStartTime(startTime);//in i3units
  portia.SetPeakBinTime(startTime+binPos*atwdBinSize);
  portia.SetIntegratedCharge(charge);//in i3units
  portia.SetEstimatedNPE(charge/domcalPmtGain);
  portia.SetBaseLine(base);//in i3units
  portia.SetAmplitude(*highestEntryItr);//in i3units
  portia.SetPositionX(pos.GetX());
  portia.SetPositionY(pos.GetY());
  portia.SetPositionZ(pos.GetZ());
  portia.SetLCBit(lcbit);

  portia.SetPMTGain(domcalPmtGain);

  log_trace("atwd t10 %f [ns], start time %f [ns]", 
	    portia.GetRecoPulse().GetTime()/I3Units::ns, portia.GetStartTime()/I3Units::ns );
 
  return true;
}
/////////////////////////////////////////////////
