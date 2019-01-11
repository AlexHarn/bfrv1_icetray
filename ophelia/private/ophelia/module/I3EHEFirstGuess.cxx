/**
 *   copyright  (C) 2005
 *   the icecube collaboration
 *   $Id:  $
 *
 *   @version $Revision:  $
 *   @date $Date:  $
 *   @author Aya Ishihara <aya@icecube.wisc.edu>
 *
 *  Converted with dc2
 *  Input as best pulses added
 *  Local coincidence option added
 *  Firstguess quanlity parameter added
 */

//ophelia
#include "ophelia/module/I3EHEFirstGuess.h"
#include "recclasses/I3OpheliaFirstGuessTrack.h"
//portia
#include "recclasses/I3PortiaEvent.h"
#include "portia/I3Portia.h"
//IceTray
#include "icetray/I3Tray.h"
#include "icetray/I3Frame.h"
#include "icetray/I3TrayHeaders.h"
//basic frame info
#include "icetray/I3Units.h"
#include "icetray/OMKey.h"
#include "dataclasses/I3DOMFunctions.h"
#include "dataclasses/I3Position.h"

I3_MODULE(I3EHEFirstGuess);

////////////////////////////////////////////////////////////
I3EHEFirstGuess::~I3EHEFirstGuess(){}
////////////////////////////////////////////////////////////
I3EHEFirstGuess::I3EHEFirstGuess(const I3Context& ctx) : I3ConditionalModule(ctx),
							 eventCounter_(0),
							 inputPulseName1_("ATWDPortiaPulse"),
							 inputPulseName2_("FADCPortiaPulse"),
							 inputPortiaEventName_("PortiaEvent"),
							 inputLaunchName_("InIceRawData"),
							 inputDOMMapName_("SplittedDOMMap"),
							 outputTrackName_("OpheliaFirstGuess"),
							 outputTrackNameBtw_("OpheliaFirstGuessBaseTimeWindow"),
							 outputParticleName_(""),
							 outputParticleNameBtw_(""),
							 chargeOption_(0),
							 LCOption_(0),
							 recoNPEThres_(0.0),
							 minNumberDomWithPulse_(0){ 
  log_trace("In I3EHEFirstGuess constrcutor");

  AddParameter("MinimumNumberPulseDom","",  minNumberDomWithPulse_);
  AddParameter("OutputFirstguessName", "",  outputTrackName_);
  AddParameter("OutputFirstguessNameBtw", "",  outputTrackNameBtw_);
  AddParameter("OutputParticleName", "",    outputParticleName_);
  AddParameter("OutputParticleNameBtw", "",    outputParticleNameBtw_);
  AddParameter("InputPulseName1",      "",  inputPulseName1_);
  AddParameter("InputPulseName2",      "",  inputPulseName2_);
  AddParameter("InputPortiaEventName",      "",  inputPortiaEventName_);
  AddParameter("InputLaunchName",      "",  inputLaunchName_);
  AddParameter("inputSplitDOMMapName",   "", inputDOMMapName_);
  AddParameter("ChargeOption",         "[0 bigger(1 or 2) : use 1 : use 2 ]", chargeOption_);
  AddParameter("LCOption",             "if non-zero use only LC pulse",   LCOption_);
  AddParameter("NPEThreshold",         "Channel wise NPE threshold to be used",   recoNPEThres_);

  AddOutBox("OutBox");
}
////////////////////////////////////////////////////////////
void I3EHEFirstGuess::Configure(){  
  GetParameter("MinimumNumberPulseDom", minNumberDomWithPulse_);
  GetParameter("InputPulseName1",       inputPulseName1_);
  GetParameter("InputPulseName2",       inputPulseName2_);
  GetParameter("InputPortiaEventName",  inputPortiaEventName_);
  GetParameter("InputLaunchName",       inputLaunchName_);
  GetParameter("inputSplitDOMMapName",      inputDOMMapName_);
  GetParameter("OutputFirstguessName",  outputTrackName_);
  GetParameter("OutputFirstguessNameBtw",  outputTrackNameBtw_);
  GetParameter("OutputParticleName",    outputParticleName_);
  GetParameter("OutputParticleNameBtw",    outputParticleNameBtw_);
  GetParameter("ChargeOption",          chargeOption_);
  GetParameter("LCOption",          LCOption_);
  GetParameter("NPEThreshold",      recoNPEThres_);

  log_trace("===== Firstguess Configuration Information =====");
  log_trace("Minimum number of DOM with pulse: %d", minNumberDomWithPulse_);
  log_trace("Charge Option: %d", chargeOption_);
  log_trace("LC Option: %d",     LCOption_);

  if(!chargeOption_||chargeOption_==1)log_trace("Input Pulse Name 1: %s ",inputPulseName1_.c_str());	                 
  if(chargeOption_==2)log_trace("Input  Pulse Name 2: %s ",inputPulseName2_.c_str());	                 
  log_trace("Output Result Name: %s", outputTrackName_.c_str());
  log_trace("================================================");
}
////////////////////////////////////////////////////////////
void I3EHEFirstGuess::Physics(I3FramePtr frame){

  /**
   * DomLaunchMap is used only to loop over every dom launched 
   * in which may or maynot pulse exist
   */
  const I3Geometry& geo = frame->Get<I3Geometry>();
  I3MapKeyVectorDouble launchMap;
  if(frame->Has(inputDOMMapName_)){ // case when (splitted) DOM map is found
      launchMap = const_cast<I3MapKeyVectorDouble&>(*(frame->Get<I3MapKeyVectorDoubleConstPtr>(inputDOMMapName_)));
  }else{
    I3DOMLaunchSeriesMap& in_launch_series_map = 
      const_cast<I3DOMLaunchSeriesMap&>(*(frame->Get<I3DOMLaunchSeriesMapConstPtr>(inputLaunchName_)));

    // make the DOM launch time map from I3DOMLaunchSeriesMap
    if(!I3Portia::MakeSplittedDOMMap(in_launch_series_map,launchMap,geo)){
      log_error("no launched HLC DOMs is found. Something is wrong");
    }
  }

  I3PortiaPulseMapConstPtr     pulseMap1_ptr = frame->Get<I3PortiaPulseMapConstPtr>(inputPulseName1_);
  I3PortiaPulseMapConstPtr     pulseMap2_ptr = frame->Get<I3PortiaPulseMapConstPtr>(inputPulseName2_);
  I3PortiaEventConstPtr        portiaEvent_ptr = frame->Get<I3PortiaEventConstPtr>(inputPortiaEventName_);
  I3OpheliaFirstGuessTrackPtr result_ptr(new I3OpheliaFirstGuessTrack);
  // reco with DOMs within the basetime window
  I3OpheliaFirstGuessTrackPtr resultBtw_ptr(new I3OpheliaFirstGuessTrack); 

  /* Get LineFit results */
  switch (chargeOption_) {
    
  case 1:
    
    log_info("firstguess is reconstructed using %s only",inputPulseName1_.c_str());
    if(pulseMap1_ptr){
      GetLineFit(launchMap, pulseMap1_ptr, geo.omgeo, result_ptr);
      // Look up the DOM with the largest NPE
      OMKey largestOMKey = portiaEvent_ptr->GetLargestNPEOMKey();
      I3PortiaPulseMap::const_iterator atwdPulseMap_iter = pulseMap1_ptr->find(largestOMKey);
      const I3PortiaPulse& atwdPulse =  atwdPulseMap_iter->second;
      double largestNPETime = atwdPulse.GetRecoPulse().GetTime();
      bool baseTimeWindowEvent = true;
      GetLineFit(launchMap, pulseMap1_ptr, geo.omgeo, resultBtw_ptr, baseTimeWindowEvent, largestNPETime);
      // Position of the DOM with the largest NPE
      I3Position largestOMpos(atwdPulse.GetPositionX(), atwdPulse.GetPositionY(), atwdPulse.GetPositionZ());  
      resultBtw_ptr->SetLargestNPEDOMposition(largestOMpos);
      result_ptr->SetLargestNPEDOMposition(largestOMpos);
    }else{
      log_error("pulse name %s is not found, check configuration", inputPulseName1_.c_str());
    }

    break;
    
  case 2:
    
    log_info("firstguess is reconstructed using %s only",inputPulseName2_.c_str());
    if(pulseMap2_ptr){
      GetLineFit(launchMap, pulseMap2_ptr, geo.omgeo, result_ptr);
      // Look up the DOM with the largest NPE
      OMKey largestOMKey = portiaEvent_ptr->GetLargestNPEOMKey();
      I3PortiaPulseMap::const_iterator fadcPulseMap_iter = pulseMap2_ptr->find(largestOMKey);
      const I3PortiaPulse& fadcPulse =  fadcPulseMap_iter->second;
      double largestNPETime = fadcPulse.GetRecoPulse().GetTime();
      bool baseTimeWindowEvent = true;
      GetLineFit(launchMap, pulseMap2_ptr,geo.omgeo, resultBtw_ptr, baseTimeWindowEvent, largestNPETime);
      // Position of the DOM with the largest NPE
      I3Position largestOMpos(fadcPulse.GetPositionX(), fadcPulse.GetPositionY(), fadcPulse.GetPositionZ());  
      resultBtw_ptr->SetLargestNPEDOMposition(largestOMpos);
      result_ptr->SetLargestNPEDOMposition(largestOMpos);

    }else{
      log_error("pulse name %s is not found, check configuration", inputPulseName2_.c_str());
    }
    
    break;
    
  case 0:
  default:
    
    log_trace("firstguess is reconstructed using both %s and %s",
	     inputPulseName1_.c_str(), inputPulseName2_.c_str());
    if(pulseMap1_ptr && pulseMap2_ptr){

      I3PortiaPulseMapPtr bestPulseMap_ptr(new I3PortiaPulseMap);

      MakeBestPulseMap(launchMap, pulseMap1_ptr, pulseMap2_ptr, bestPulseMap_ptr);

      GetLineFit(launchMap, bestPulseMap_ptr, geo.omgeo,result_ptr);

      // Look up the DOM with the largest NPE
      OMKey largestOMKey = portiaEvent_ptr->GetLargestNPEOMKey();
      I3PortiaPulseMap::const_iterator atwdPulseMap_iter = pulseMap1_ptr->find(largestOMKey);
      const I3PortiaPulse& atwdPulse =  atwdPulseMap_iter->second;
      I3PortiaPulseMap::const_iterator fadcPulseMap_iter = pulseMap2_ptr->find(largestOMKey);
      const I3PortiaPulse& fadcPulse =  fadcPulseMap_iter->second;
      I3PortiaPulseMap::const_iterator bestPulseMap_iter = bestPulseMap_ptr->find(largestOMKey);
      const I3PortiaPulse& bestPulse =  bestPulseMap_iter->second;

      // chose ATWD or FADC depending on captured NPEs in each digitizers
      double largestNPETime = atwdPulse.GetRecoPulse().GetTime();
      if(fadcPulse.GetEstimatedNPE()>atwdPulse.GetEstimatedNPE()) 
	largestNPETime = fadcPulse.GetRecoPulse().GetTime();

      bool baseTimeWindowEvent = true;
      MakeBestPulseMap(launchMap, pulseMap1_ptr, pulseMap2_ptr, bestPulseMap_ptr, baseTimeWindowEvent);
      GetLineFit(launchMap, bestPulseMap_ptr, geo.omgeo,resultBtw_ptr, baseTimeWindowEvent, largestNPETime);

      // Position of the DOM with the largest NPE
      I3Position largestOMpos(bestPulse.GetPositionX(), bestPulse.GetPositionY(), bestPulse.GetPositionZ());  
      resultBtw_ptr->SetLargestNPEDOMposition(largestOMpos);
      result_ptr->SetLargestNPEDOMposition(largestOMpos);

    }else log_error("pulse is not found, check configuration");
    
  }

  /**
   * Put the linefit result if the fit was sucseeded. 
   */
  if(result_ptr->IsFitSuccessful()){
    log_trace("Fit Successful");
    frame->Put(outputTrackName_, result_ptr);
    frame->Put(outputTrackNameBtw_, resultBtw_ptr);

    /**
     * Put a pure I3Particle if the output particle name is not empty.
     */
    if(outputParticleName_ != ""){
      I3ParticlePtr p_ptr(new I3Particle()); 
      p_ptr->SetPos(result_ptr->GetConstI3Particle().GetPos()); 
      p_ptr->SetDir(result_ptr->GetConstI3Particle().GetDir()); 
      p_ptr->SetTime(result_ptr->GetConstI3Particle().GetTime()); 
      p_ptr->SetSpeed(result_ptr->GetConstI3Particle().GetSpeed()); 
      p_ptr->SetFitStatus(result_ptr->GetConstI3Particle().GetFitStatus()); 
      p_ptr->SetShape(result_ptr->GetConstI3Particle().GetShape()); 
      frame->Put(outputParticleName_, p_ptr); 
    } 
    if(outputParticleNameBtw_ != ""){
      I3ParticlePtr btw_ptr(new I3Particle()); 
      btw_ptr->SetPos(resultBtw_ptr->GetConstI3Particle().GetPos()); 
      btw_ptr->SetDir(resultBtw_ptr->GetConstI3Particle().GetDir()); 
      btw_ptr->SetTime(resultBtw_ptr->GetConstI3Particle().GetTime()); 
      btw_ptr->SetSpeed(resultBtw_ptr->GetConstI3Particle().GetSpeed()); 
      btw_ptr->SetFitStatus(resultBtw_ptr->GetConstI3Particle().GetFitStatus()); 
      btw_ptr->SetShape(resultBtw_ptr->GetConstI3Particle().GetShape()); 
      frame->Put(outputParticleNameBtw_, btw_ptr); 
    } 
  }else log_warn("Fit Failed");
  
  PushFrame(frame,"OutBox");
}
///////////////////////////////////////////////////////////
void I3EHEFirstGuess::GetLineFit(I3MapKeyVectorDouble& launchMap,
				 I3PortiaPulseMapConstPtr     pulseMap_ptr,
				 const I3OMGeoMap& omgeo,				 
				 I3OpheliaFirstGuessTrackPtr  result_ptr,
				 bool baseTimeWindowEvent,
				 double largestNPETime){

  /* Calculate integrated npe weighted averages over pulses */
  double averageTime = 0;
  double averageTimeSquare = 0;
  double averageX = 0;     
  double averageY = 0;     
  double averageZ = 0;
  double averageTimeX = 0; 
  double averageTimeY = 0; 
  double averageTimeZ = 0;

  int    totalNOM     = 0;
  int    totalNPulses = 0;
  double totalNPE = 0;

  log_trace("Launched DOM Number: %d", (int)launchMap.size());
  
  /**
   * 1 loop over all launch
   * 2 look for pulse
   * 3 calculate
   */
  I3MapKeyVectorDouble::const_iterator launchMap_iter = launchMap.begin();
  for(;launchMap_iter!=launchMap.end(); launchMap_iter++){

    const OMKey omkey = launchMap_iter->first;   
    log_trace("string %d / OM %d", omkey.GetString(), omkey.GetOM());

    /* At first make sure if this is not IceTop DOM */
    if(omgeo.find(omkey)->second.omtype==I3OMGeo::IceTop){
      log_debug("IceTop string %d / OM %d : skip this", omkey.GetString(), omkey.GetOM());
      continue;
    }
    
    ++totalNOM;    

    /* Check if pulse found on this dom launch */
    I3PortiaPulseMap::const_iterator pulseMap_iter = pulseMap_ptr->find(omkey);
    if(pulseMap_iter == pulseMap_ptr->end()){
      log_trace("NO Pulse found on string %d / OM %d, skip", omkey.GetString(), omkey.GetOM());
      continue;
    }

      const I3PortiaPulse& portia =  pulseMap_iter->second;
      const I3RecoPulse& recopulse = portia.GetRecoPulse();

      double time  = recopulse.GetTime();   //time10 in i3unit
      double npe   = recopulse.GetCharge(); //charge/singlePEQ in i3unit 
      double posX  = portia.GetPositionX();
      double posY  = portia.GetPositionY();
      double posZ  = portia.GetPositionZ();
      bool   lcBit = portia.GetLCBit();
      
      log_trace("Pulse Info: Time=%f, NPE=%f, position (%f, %f, %f)", time, npe, posX, posY, posZ);
      
      if(baseTimeWindowEvent){ // check if this DOM signal is within the basetime window
	double dt = time - largestNPETime;
	if(dt > I3Portia::kEndTimeBtw_) continue;
	if(dt < I3Portia::kStartTimeBtw_) continue;
      }

      if(!lcBit&&LCOption_){
	continue;
      }else if(npe<recoNPEThres_){
	continue;
	//cout <<"NPE/DOM is below 20.0, continue" <<endl; 
      }else{
	/* For average */
	++totalNPulses;
	totalNPE += npe;
	averageTime           += npe*time;
	averageTimeSquare     += npe*time*time;
	averageX              += npe*posX;
	averageY              += npe*posY;
	averageZ              += npe*posZ;
	averageTimeX          += npe*posX*time;
	averageTimeY          += npe*posY*time;
	averageTimeZ          += npe*posZ*time;
      }
  }//end of launched OM loop

  /* Calculate averages */
  if(totalNPulses>=0&&totalNPE>=0.){
    averageTime           /=totalNPE;
    averageTimeSquare     /=totalNPE;
    averageX              /=totalNPE;
    averageY              /=totalNPE;
    averageZ              /=totalNPE;
    averageTimeX          /=totalNPE;
    averageTimeY          /=totalNPE;
    averageTimeZ          /=totalNPE;
  }
  log_info("we have total number %d of pulses out of %d launches, total estimated npe %f", totalNPulses, totalNOM, totalNPE);

  /* Check if total number of DOMs with pulses is greater than a given threshold */
  if(totalNPulses <= minNumberDomWithPulse_){
    log_info("Request of the minimum number of pulse failed");
    I3Position null(0,0,0);
    result_ptr->SetVelocity(null);
    result_ptr->SetFitSuccessful(false);
    result_ptr->GetI3Particle().SetFitStatus(I3Particle::InsufficientHits);
    return; 
  }

  double varianceTime = averageTimeSquare - averageTime*averageTime;

  /* Calculation of velocity */
  I3Position Velocity;
  Velocity.SetX((averageTimeX - averageX*averageTime)/varianceTime);
  Velocity.SetY((averageTimeY - averageY*averageTime)/varianceTime);
  Velocity.SetZ((averageTimeZ - averageZ*averageTime)/varianceTime);

  log_trace("Line: averageTime %f",averageTime);
  log_trace("Line: time variance %f, averageTimeSquare %f, square of averageTime%f",
	    varianceTime, averageTimeSquare, averageTime*averageTime);
  log_trace("Line: position %f, %f, %f", averageX, averageY, averageZ);
  log_trace("Line: position*time %f, %f, %f", averageTimeX, averageTimeY, averageTimeZ);
  log_trace("Line: totalNPulses %d, minNumberPulse %d", totalNPulses, minNumberDomWithPulse_);
  log_trace("Line: speed %f", Velocity.Magnitude());
  log_trace("Line Velocity: %f, %f, %f",Velocity.GetX(),Velocity.GetY(),Velocity.GetZ());

  /* Set position of center of brightness */
  I3Position COB(averageX, averageY, averageZ);  
  log_trace("Line COB: %f, %f, %f",COB.GetX(), COB.GetY(), COB.GetZ());

  //////////////////////////////
  // Set-up First-guess Track //
  //////////////////////////////
  //we have 4 non-i3particle members
  result_ptr->SetCenterOfBrightness(COB);
  result_ptr->SetVelocity(Velocity);//m/ns
  result_ptr->SetFitSuccessful(true);

  /* Set I3Particle */  
  I3Particle& i3particle = result_ptr->GetI3Particle();
  i3particle.SetPos(averageX, averageY, averageZ);
  i3particle.SetDir(Velocity.GetX(), Velocity.GetY(), Velocity.GetZ());
  i3particle.SetTime(averageTime);
  i3particle.SetSpeed(Velocity.Magnitude());
  i3particle.SetLength(totalNPulses);
  i3particle.SetFitStatus(I3Particle::OK);
  i3particle.SetShape(I3Particle::InfiniteTrack);

  /* G/Set fit quality */
  double fitquality = GetFitQuality(launchMap, pulseMap_ptr, result_ptr, averageTime);
  result_ptr->SetFitQuality(fitquality);

  log_trace("center of brightness: %f, %f, %f",
	   result_ptr->GetCenterOfBrightness().GetX()/I3Units::m,
	   result_ptr->GetCenterOfBrightness().GetY()/I3Units::m,  
	   result_ptr->GetCenterOfBrightness().GetZ()/I3Units::m);

  log_trace("velocity: %f, %f, %f [m/ns]",
	   result_ptr->GetVelocity().GetX()/(I3Units::m/I3Units::ns),
	   result_ptr->GetVelocity().GetY()/(I3Units::m/I3Units::ns),  
	   result_ptr->GetVelocity().GetZ()/(I3Units::m/I3Units::ns));

  log_trace("direction: zenith %f [rad], azimuth %f [rad]",
	   result_ptr->GetConstI3Particle().GetDir().GetZenith()/I3Units::rad,
	   result_ptr->GetConstI3Particle().GetDir().GetAzimuth()/I3Units::rad);
	   
  log_trace("Fit quality %f ", result_ptr->GetFitQuality());

  return;
}

///////////////////////////////////////////////////////////

double I3EHEFirstGuess::GetFitQuality(I3MapKeyVectorDouble& launchMap,
				    I3PortiaPulseMapConstPtr       pulseMap_ptr, 
				    I3OpheliaFirstGuessTrackPtr    result_ptr,
				    double t0){

  //////////////////////////////
  // Set-up First-guess Track //
  //////////////////////////////
  if(!result_ptr->IsFitSuccessful())return 0.0;

  I3Position Velocity = result_ptr->GetVelocity();//m/ns
  I3Position COB = result_ptr->GetCenterOfBrightness();//m
  int    totalNPulses = 0;
  double totalNPE = 0;
  double averageDeltaTimeSquare = 0;

  I3MapKeyVectorDouble::const_iterator launchMap_iter = launchMap.begin();
  for(;launchMap_iter!=launchMap.end(); launchMap_iter++){

    const OMKey omkey = launchMap_iter->first;   
    //////// check if pulse found on this dom launch //////////    
    I3PortiaPulseMap::const_iterator pulseMap_iter = pulseMap_ptr->find(omkey);
    if(pulseMap_iter == pulseMap_ptr->end()) continue;
    
    const I3PortiaPulse& portia =  pulseMap_iter->second;
    const I3RecoPulse& recopulse = portia.GetRecoPulse();
    
    bool   lcBit = portia.GetLCBit();
    
    if(!lcBit&&LCOption_){
      continue;
    }else if(recopulse.GetCharge()<recoNPEThres_){
      continue;
      //cout <<"NPE/DOM is below 20.0, continue" <<endl; 
    }
    //for average//
      double npe   = recopulse.GetCharge();//charge/singlePEQ in i3unit 
      ++totalNPulses;
      totalNPE += npe;
      ///////////////

      I3Position VelocityTime = Velocity*(recopulse.GetTime());
      I3Position Position(portia.GetPositionX(), portia.GetPositionY(), portia.GetPositionZ());  
      I3Position VelocityLineTime = Velocity*t0 + (Position-COB);

      averageDeltaTimeSquare += npe*(VelocityTime - VelocityLineTime)*(VelocityTime - VelocityLineTime);
  
  }//end of OM loop

  if(totalNPulses&&totalNPE) {
    averageDeltaTimeSquare /= (totalNPE*totalNPulses);  // divided by degree of feedom
  }else{
    averageDeltaTimeSquare = -999.;
  }

  return averageDeltaTimeSquare;
}

///////////////////////////////////////////////////////////

void I3EHEFirstGuess::MakeBestPulseMap(I3MapKeyVectorDouble& launchMap,
			      I3PortiaPulseMapConstPtr     atwdPulseMap_ptr,
		              I3PortiaPulseMapConstPtr     fadcPulseMap_ptr,
                              I3PortiaPulseMapPtr          bestPulseMap_ptr,
			      bool baseTimeWindowEvent){

  // 1 loop over all launch
  // 2 look for atwd and fadc pulses associated with each omkey 
  // 3 compare
  // 4 push_back

  for(I3MapKeyVectorDouble::const_iterator launchMap_iter = launchMap.begin();
      launchMap_iter!=launchMap.end(); launchMap_iter++){
    
    OMKey omkey = launchMap_iter->first;   
    log_trace("string %d / OM %d", omkey.GetString(), omkey.GetOM());

    I3PortiaPulseMap::const_iterator atwdPulseMap_iter = atwdPulseMap_ptr->find(omkey);
    I3PortiaPulseMap::const_iterator fadcPulseMap_iter = fadcPulseMap_ptr->find(omkey);

    double atwd_charge=0;
    double fadc_charge=0;

    // ATWD
    if(atwdPulseMap_iter == atwdPulseMap_ptr->end()){
      log_debug("NO ATWD Pulse found on string %d / OM %d", omkey.GetString(), omkey.GetOM());
      atwd_charge = 0;
    }else{
      const I3PortiaPulse& atwdpulse = atwdPulseMap_iter->second;
      atwd_charge = atwdpulse.GetEstimatedNPE();
    }
    
    // FADC
    if(fadcPulseMap_iter == fadcPulseMap_ptr->end()){
      log_debug("NO FADC Pulse found on string %d / OM %d", omkey.GetString(), omkey.GetOM());
      fadc_charge = 0;
    }else{
      const I3PortiaPulse& fadcpulse = fadcPulseMap_iter->second;
      fadc_charge = fadcpulse.GetEstimatedNPE();
    }

    log_trace("atwd charge %f, fadc charge %f", atwd_charge, fadc_charge);
    if(fadc_charge==0&&atwd_charge==0)continue;
    if(fadc_charge>atwd_charge&&atwd_charge){
      
      //refill only time related
      const I3PortiaPulse& oldfadc =  fadcPulseMap_iter->second;
      const I3PortiaPulse& oldatwd =  atwdPulseMap_iter->second;
      I3PortiaPulse newportia;
      I3RecoPulse& newrecopulse = newportia.GetRecoPulse();

      newrecopulse.SetCharge(oldfadc.GetRecoPulse().GetCharge());//in i3unit 
      if(!baseTimeWindowEvent){ // take timing from ATWD
	newrecopulse.SetTime(oldatwd.GetRecoPulse().GetTime());//in i3unit
	newrecopulse.SetWidth(oldatwd.GetRecoPulse().GetWidth());//in i3unit
      }else{ // tale time from FADC
	newrecopulse.SetTime(oldfadc.GetRecoPulse().GetTime());//in i3unit
	newrecopulse.SetWidth(oldfadc.GetRecoPulse().GetWidth());//in i3unit
      } 
      newportia.SetIntegratedCharge(oldfadc.GetIntegratedCharge());//in i3units
      newportia.SetEstimatedNPE(oldfadc.GetEstimatedNPE());
      newportia.SetBaseLine(oldfadc.GetBaseLine());//in i3units
      newportia.SetAmplitude(oldfadc.GetAmplitude());//in i3units
      newportia.SetPositionX(oldfadc.GetPositionX());
      newportia.SetPositionY(oldfadc.GetPositionY());
      newportia.SetPositionZ(oldfadc.GetPositionZ());
      newportia.SetLCBit(oldfadc.GetLCBit());

      newportia.SetTime50(oldatwd.GetTime50());//in i3units
      newportia.SetTime80(oldatwd.GetTime80());//in i3units
      newportia.SetLETime(oldatwd.GetLETime());//in i3units
      newportia.SetLaunchTime(oldatwd.GetLaunchTime());//in i3units
      newportia.SetPeakBinTime(oldatwd.GetPeakBinTime());
      (*bestPulseMap_ptr)[omkey] = newportia;
      log_trace("New Pulse Info: Time=%f, NPE=%f, position (%f, %f, %f)", 
		newrecopulse.GetTime(),
		newrecopulse.GetCharge(), 
		newportia.GetPositionX(),
		newportia.GetPositionY(),
		newportia.GetPositionZ());
    }else if(atwd_charge==0) {
      (*bestPulseMap_ptr)[omkey] = fadcPulseMap_iter->second;
      const I3PortiaPulse& oldfadc =  fadcPulseMap_iter->second;
      log_trace("FADC Pulse Info: Time=%f, NPE=%f, position (%f, %f, %f)", 
		oldfadc.GetRecoPulse().GetTime(),
		oldfadc.GetRecoPulse().GetCharge(), 
		oldfadc.GetPositionX(),
		oldfadc.GetPositionY(),
		oldfadc.GetPositionZ());
    }else{
      (*bestPulseMap_ptr)[omkey] = atwdPulseMap_iter->second;
      const I3PortiaPulse& oldatwd =  atwdPulseMap_iter->second;
     log_trace("ATWD Pulse Info: Time=%f, NPE=%f, position (%f, %f, %f)", 
		oldatwd.GetRecoPulse().GetTime(),
		oldatwd.GetRecoPulse().GetCharge(), 
		oldatwd.GetPositionX(),
		oldatwd.GetPositionY(),
		oldatwd.GetPositionZ());
    }
    

  }
}
/////////////////////////////////////////////////////////////////////////////////////
