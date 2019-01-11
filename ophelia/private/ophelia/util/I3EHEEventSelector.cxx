// This class
#include "dataclasses/physics/I3MCTreeUtils.h"
#include "ophelia/util/I3EHEEventSelector.h"

#include <iostream>
#include <iomanip>

using namespace std;

I3_MODULE(I3EHEEventSelector);

// constructor and destructor

//===================================================================
//* constructor -----------------------------------------------------
I3EHEEventSelector::I3EHEEventSelector(const I3Context& ctx) : 
  I3ConditionalModule(ctx),
  setCriteriaOnEventHeader_(false),
  setCriteriaOnJulietParticle_(false),
  setCriteriaOnInIceDOMLaunch_(false),
  setCriteriaOnPortiaPulse_(false),
  baseTimeWindow_(true),
  setCriteriaOnEHEFirstGuess_(false),
  setEHECriteria_(false),
  subEventStreamName_("InIceSplit"),
  inputI3MCTreeName_("I3MCTree"),
  inputInIceDOMLaunchName_("CleanInIceRawData"),
  inputAtwdPulseName_("ATWDPortiaPulse"),
  inputFadcPulseName_("FADCPortiaPulse"),
  inputPortiaEventName_("PortiaEvent"),
  inputFirstguessName_("OpheliaFirstGuessBaseTimeWindow"),
  eheCriteriaFileName_("")
{
  
  energyMin_ = 1.0E5*I3Units::GeV;
  energyMax_ = 1.0E12*I3Units::GeV;
  
  particleType_ = I3Particle::MuMinus;

  cosZenithMin_ = -1.0;
  cosZenithMax_ = 1.0;
  azimuthMin_ = 0.0*I3Units::deg;
  azimuthMax_ = 360.0*I3Units::deg;

  distanceOftheHighestCascade_ = 10.0*I3Units::kilometer;

  nDOMLaunchMin_ = 50;

  lowestNPEs_ = 1.0E1, 
  highestNPEs_ = 1.0E8;
  nDOMmin_ = 50;

  fg_cosZenithMin_ = -1.0;
  fg_cosZenithMax_ = 1.0;
  fg_azimuthMin_ = 0.0*I3Units::deg;
  fg_azimuthMax_ = 360.0*I3Units::deg;

  fg_cogzMin_ = -500.*I3Units::m;
  fg_cogzMax_ = 500.*I3Units::m;
  fg_bdomzMin_ = -500.*I3Units::m;
  fg_bdomzMax_ = 500.*I3Units::m;

  distanceOfCOB_ = 10.0*I3Units::kilometer;
  velocityMin_ = 0.0*I3Units::meter/I3Units::second;



  AddParameter("setCriteriaOnEventHeader","", 
	       setCriteriaOnEventHeader_);  
  AddParameter("setCriteriaOnJulietParticle","", 
	       setCriteriaOnJulietParticle_);  
  AddParameter("energyMin","", energyMin_);
  AddParameter("energyMax","", energyMax_);
  AddParameter("particleType","", particleType_);
  AddParameter("cosZenithMin","", cosZenithMin_);
  AddParameter("cosZenithMax","", cosZenithMax_);
  AddParameter("AzimuthMin","", azimuthMin_);
  AddParameter("AzimuthMax","", azimuthMax_);
  AddParameter("firstGuessCoGZMin","", fg_cogzMin_);
  AddParameter("firstGuessCoGZMax","", fg_cogzMax_);
  AddParameter("firstGuessBDOMZMin","", fg_bdomzMin_);
  AddParameter("firstGuessBDOMZMax","", fg_bdomzMax_);
  AddParameter("distanceOfCascade","",
	       distanceOftheHighestCascade_);

  AddParameter("setCriteriaOnInIceDOMLaunch","", 
	       setCriteriaOnInIceDOMLaunch_);  
  AddParameter("baseTimeWindowCleaning","", 
	       baseTimeWindow_);  
  AddParameter("inInIceDOMLaunchName", "", inputInIceDOMLaunchName_);
  AddParameter("numberOfDOMsWithLaunch","", nDOMLaunchMin_);

  AddParameter("setCriteriaOnPortiaPulse","", 
	       setCriteriaOnPortiaPulse_);  
  AddParameter("inAtwdPortiaName", "", inputAtwdPulseName_);
  AddParameter("inFadcPortiaName", "", inputFadcPulseName_);
  AddParameter("InputPortiaEventName",      "",  inputPortiaEventName_);
  AddParameter("lowestNPEs","", lowestNPEs_);
  AddParameter("highestNPEs","", highestNPEs_);
  AddParameter("numberOfDOMs","", nDOMmin_);

  AddParameter("setCriteriaOnEHEFirstGuess","", 
	       setCriteriaOnEHEFirstGuess_);  
  AddParameter("inFirstguessName"," ", inputFirstguessName_);
  AddParameter("firstGuessCosZenithMin","", fg_cosZenithMin_);
  AddParameter("firstGuessCosZenithMax","", fg_cosZenithMax_);
  AddParameter("firstGuessAzimuthMin","", fg_azimuthMin_);
  AddParameter("firstGuessAzimuthMax","", fg_azimuthMax_);
  AddParameter("distanceOfCOB","", distanceOfCOB_);
  AddParameter("firstGuessVelocity","", velocityMin_);

  AddParameter("setEHECriteria","", setEHECriteria_);  
  AddParameter("EHECriteriaFileName","",eheCriteriaFileName_);


  AddOutBox("OutBox");

}

//===================================================================
//* destructor -----------------------------------------------------
I3EHEEventSelector::~I3EHEEventSelector()
{

}


//===================================================================
//* configure -----------------------------------------------------
void I3EHEEventSelector::Configure(){
  log_info("Configuring the EHE event selector");

  GetParameter("setCriteriaOnEventHeader",
	       setCriteriaOnEventHeader_);  
  GetParameter("setCriteriaOnJulietParticle",
	       setCriteriaOnJulietParticle_);  
  GetParameter("energyMin", energyMin_);
  GetParameter("energyMax", energyMax_);
  GetParameter("particleType", particleType_);
  GetParameter("cosZenithMin", cosZenithMin_);
  GetParameter("cosZenithMax", cosZenithMax_);
  GetParameter("AzimuthMin", azimuthMin_);
  GetParameter("AzimuthMax", azimuthMax_);
  GetParameter("distanceOfCascade",
	       distanceOftheHighestCascade_);
  GetParameter("setCriteriaOnInIceDOMLaunch",
	       setCriteriaOnInIceDOMLaunch_);  
  GetParameter("baseTimeWindowCleaning",baseTimeWindow_);  
  GetParameter("inInIceDOMLaunchName",inputInIceDOMLaunchName_);
  GetParameter("numberOfDOMsWithLaunch",nDOMLaunchMin_);
  GetParameter("setCriteriaOnPortiaPulse",
	       setCriteriaOnPortiaPulse_);  
  GetParameter("inAtwdPortiaName", inputAtwdPulseName_);
  GetParameter("inFadcPortiaName", inputFadcPulseName_);
  GetParameter("InputPortiaEventName",  inputPortiaEventName_);
  GetParameter("lowestNPEs", lowestNPEs_);
  GetParameter("highestNPEs", highestNPEs_);
  GetParameter("numberOfDOMs", nDOMmin_);
  GetParameter("setCriteriaOnEHEFirstGuess",
	       setCriteriaOnEHEFirstGuess_);  
  GetParameter("inFirstguessName",inputFirstguessName_);
  GetParameter("firstGuessCosZenithMin", fg_cosZenithMin_);
  GetParameter("firstGuessCosZenithMax", fg_cosZenithMax_);
  GetParameter("firstGuessAzimuthMin", fg_azimuthMin_);
  GetParameter("firstGuessAzimuthMax", fg_azimuthMax_);
  GetParameter("firstGuessCoGZMin", fg_cogzMin_);
  GetParameter("firstGuessCoGZMax", fg_cogzMax_);
  GetParameter("firstGuessBDOMZMin", fg_bdomzMin_);
  GetParameter("firstGuessBDOMZMax", fg_bdomzMax_);
  GetParameter("distanceOfCOB", distanceOfCOB_);
  GetParameter("firstGuessVelocity", velocityMin_);
  GetParameter("setEHECriteria", setEHECriteria_);  
  GetParameter("EHECriteriaFileName",eheCriteriaFileName_);

  if(setCriteriaOnEventHeader_){
    log_info("----You set criteria on Event Header----");
    log_info("  selecting in-ice split");
  }

  if(setCriteriaOnJulietParticle_){
    log_info("----You set criteria on Juliet Particle----");
    log_info("  I3MCTree Name is %s", 
	     inputI3MCTreeName_.c_str());
    log_info("  %e < energy [GeV] < %e",
	     energyMin_/I3Units::GeV,energyMax_/I3Units::GeV);
    log_info("  %f < cos(zenith) < %f",
	     cosZenithMin_,cosZenithMax_);
    log_info("  %f < azimuth [deg] < %f",
	     azimuthMin_/I3Units::deg,azimuthMax_/I3Units::deg);
    log_info("  %f < CoGZ < %f",
	      +     fg_cogzMin_/I3Units::deg,fg_cogzMax_/I3Units::deg);
    log_info("  %f > cascade distance [km]",
	     distanceOftheHighestCascade_/I3Units::kilometer);
  }

  if(setCriteriaOnInIceDOMLaunch_){
    log_info("----You set criteria on In0Ice DOM Launch Map----");
    log_info("   InIce DOM launch map Name is %s",
	     inputInIceDOMLaunchName_.c_str());
    log_info("  %d < Number Of DOMs with Launch", nDOMLaunchMin_);
  }

  if(setCriteriaOnPortiaPulse_){
    log_info("----You set criteria on Portia Pulse----");
    log_info("  Poirita ATWD pulse Name is %s",
	     inputAtwdPulseName_.c_str());
    log_info("  Poirita FADC pulse Name is %s",
	     inputFadcPulseName_.c_str());
    log_info("  %e < NPE < %e", lowestNPEs_, highestNPEs_);
    log_info("  %d < Number Of DOMs", nDOMmin_);
    if(baseTimeWindow_) 
      log_info("-- We digest reco/portia with the basetime window cleaing-----");
  }

  if(setCriteriaOnEHEFirstGuess_){
    log_info("---You set criteria on the Ophelia EHE first guess track---");
    log_info("  Ophelia first guess track Name is %s", 
	     inputFirstguessName_.c_str());
    log_info("  %f < cos(zenith) < %f",
	     fg_cosZenithMin_,fg_cosZenithMax_);
    log_info("  %f < azimuth [deg] < %f",
	     fg_azimuthMin_/I3Units::deg,fg_azimuthMax_/I3Units::deg);
    log_info("  %f < COB distance [km]",
	     distanceOfCOB_/I3Units::kilometer);
    log_info("  %f < veolocity [m\nsec]",
	     velocityMin_/(I3Units::meter/I3Units::ns));
  }

  if(setEHECriteria_){
    log_info("---You set EHE criteria by the Ophelia EHECriteria class---");
    if(eheCriteriaFileName_ == ""){
      log_info(" The default EHE criteria defined in ophelia/util/EHECriteria.cxx will be used");
    }else{
      log_info(" The EHE criteria in the file %s will be used",eheCriteriaFileName_.c_str());
    }

    eheCriteria_ = new EHECriteria(eheCriteriaFileName_);
  }
}


//===================================================================
//* physics -----------------------------------------------------
void I3EHEEventSelector::Physics(I3FramePtr frame)
{
  log_info("Entering I3EHEEventSelector::Physics()");


  bool passHeader = true;
  // Take the event header from the frame
  if(setCriteriaOnEventHeader_){
      I3EventHeaderConstPtr eventHeaderPtr = frame->Get<I3EventHeaderConstPtr>();
      passHeader = PassesCriteriaOnEventHeader(eventHeaderPtr);
      if(passHeader) log_info("Passed criteria on event header");
  }

  bool passJuliet = true;
  // Take the JulietParticle from the frame
  if(setCriteriaOnJulietParticle_){
    I3MCTreeConstPtr mc_tree_ptr =
      frame->Get<I3MCTreeConstPtr>(inputI3MCTreeName_);
    passJuliet = PassesCriteriaOnJulietParticle(*mc_tree_ptr);
    if(passJuliet) log_info("Passed criteria on juliet particles");
  }

  bool passInIceDOMLaunch = true;
  // Take the InIceDOMLaunch map from the frame
  if(setCriteriaOnInIceDOMLaunch_){
    I3DOMLaunchSeriesMapConstPtr inIceLaunch = 
      frame->Get<I3DOMLaunchSeriesMapConstPtr>(inputInIceDOMLaunchName_);
    passInIceDOMLaunch = PassesCriteriaOnInIceDOMLaunch(inIceLaunch);
    if(passInIceDOMLaunch) log_info("Passed criteria on InIce DOM launch");
  }


  bool passPortia = true;
  // Take the Portia Pulse from the frame
  if(setCriteriaOnPortiaPulse_){
    I3PortiaEventConstPtr portia_event = 
      frame->Get<I3PortiaEventConstPtr>(inputPortiaEventName_);
    passPortia = PassesCriteriaOnPortiaPulse(portia_event,baseTimeWindow_);
    if(passPortia) log_info("Passed criteria on portia pulses");
  }


  bool passFG = true;
  // Take the OpheliaEHEFirstGuessTrack from the frame
  if(setCriteriaOnEHEFirstGuess_){
    I3OpheliaFirstGuessTrackConstPtr fgTrack = 
      frame->Get<I3OpheliaFirstGuessTrackConstPtr>(inputFirstguessName_);
    passFG = PassesCriteriaOnFirstGuessTrack(fgTrack);
    if(passFG) log_info("Passed criteria on EHE first guess");
  }

  bool passEHE = true;
  // Take the Portia Pulse from the frame
  if(setEHECriteria_){
    I3PortiaEventConstPtr portia_event = 
      frame->Get<I3PortiaEventConstPtr>(inputPortiaEventName_);
    I3PortiaPulseMapConstPtr fadc_portia_map = 
      frame->Get<I3PortiaPulseMapConstPtr>(inputFadcPulseName_);
    I3PortiaPulseMapConstPtr atwd_portia_map = 
      frame->Get<I3PortiaPulseMapConstPtr>(inputAtwdPulseName_);
    I3OpheliaFirstGuessTrackConstPtr fgTrack = 
      frame->Get<I3OpheliaFirstGuessTrackConstPtr>(inputFirstguessName_);
    passEHE = PassesEHECriteria(portia_event,atwd_portia_map,fadc_portia_map,fgTrack);
    if(passEHE) log_info("Passed EHE criteria");
  }

  // Push the frame if this event passes the criteria
  if(passHeader && passJuliet && passInIceDOMLaunch && passPortia && passFG && passEHE){
    log_info("This event has passed your criteria");
    PushFrame(frame,"OutBox");
  }

}//End Physics()


//===================================================================
// Check if this event satisfies the criteria -----------------------
// on the event header -------------------------------------------
// Selects p-frame of "in-ice split"
//===================================================================
bool I3EHEEventSelector::PassesCriteriaOnEventHeader(I3EventHeaderConstPtr eventHeader){

  log_info(" -- Entering PassesCriteriaOnEventHeader --");

  if(!eventHeader){
    log_error("  This event does not have an event header");
    return false;
  }

  string p_frame_name = eventHeader->GetSubEventStream();
  if(p_frame_name != subEventStreamName_){ // this p-frame is not in-ice split. pass it over.
    log_debug(" this p-frame %s is not in-ice split. Go to next p-frame",p_frame_name.c_str());
    return false;
  }
  return true;
}

//===================================================================
// Check if this event satisfies the criteria -----------------------
// on the Juliet Particle -------------------------------------------
//===================================================================
bool I3EHEEventSelector::PassesCriteriaOnJulietParticle
(const I3MCTree& mc_tree){

  log_info(" -- Entering PassesCriteriaOnJulietParticle --");

  // Take the primary juliet particle info
  // assuming there is only one primary
  const vector<I3Particle>& i3juliet_track 
    = I3MCTreeUtils::GetPrimaries(mc_tree);
  if(i3juliet_track.size()<1){
    log_warn("Couldn't find any primary!");
    return false;
  }else if(i3juliet_track.size()>1){
    log_warn("Multiple primaries found!");
    return false;
  }
  double energy = i3juliet_track[0].GetEnergy();
  double cosZenith = cos(i3juliet_track[0].GetZenith()/I3Units::radian);
  double azimuth = i3juliet_track[0].GetAzimuth();  


  double cascadeEnergyMax = 0.;
  double cascadeX = 0.;
  double cascadeY = 0.;
  double cascadeZ = 0.;

  // find the most energetic cascade and store the info.
  I3MCTree::const_iterator iter;
  for (iter=mc_tree.begin(); iter!=mc_tree.end(); ++iter) {
    if (iter->GetEnergy() > cascadeEnergyMax && iter->IsCascade()) {
      cascadeEnergyMax = iter->GetEnergy();
      cascadeX = iter->GetX();
      cascadeY = iter->GetY();
      cascadeZ = iter->GetZ();
    }
  }

  double distance =
    sqrt(cascadeX*cascadeX+cascadeY*cascadeY+cascadeZ*cascadeZ);

  log_debug("  Energy = %e [GeV]",energy/I3Units::GeV);
  log_debug("  cos(Zenith) = %f Azimuth=%f [deg]",
	    cosZenith,azimuth/I3Units::deg);
  log_debug("  Max Cacade Energy = %e [GeV]",
	    cascadeEnergyMax/I3Units::GeV);
  log_debug("  Cascade Distance = %lf [km]",distance/I3Units::kilometer);
  log_debug("  Cascade (X,Y,Z) = (%f %f %f) [m]",
	    cascadeX/I3Units::meter,cascadeY/I3Units::meter,cascadeZ/I3Units::meter);

  // Now check if this event passes the criteria
  bool pass = false;
  if((energyMin_ <= energy) && (energy <= energyMax_)){ // energy
    if((cosZenithMin_ <= cosZenith) && (cosZenith <= cosZenithMax_)){ // zenith
      if((azimuthMin_ <= azimuth) && (azimuth <= azimuthMax_)){ // azimuth
	if(distance < distanceOftheHighestCascade_ ){ // cascade distance
	  pass = true;
	}
      }
    }
  }

  return pass;
}


//===================================================================
// Check if this event satisfies the criteria -----------------------
// on the InIce DOM Launch  ---- ------------------------------------
//===================================================================
bool I3EHEEventSelector::PassesCriteriaOnInIceDOMLaunch(
		I3DOMLaunchSeriesMapConstPtr inIceLaunch){

  log_info(" -- Entering PassesCriteriaOnInIceDOMLaunch --");

  if(!inIceLaunch){
    log_error("  This event does not have a InIce DOM launch map");
    return false;
  }

  int nDOMsWithLaunch = inIceLaunch->size();
  log_debug("  NDOMs with Launches = %d", nDOMsWithLaunch);

  if(nDOMLaunchMin_ <= nDOMsWithLaunch) return true;
  else return false;


}

//===================================================================
// Check if this event satisfies the criteria -----------------------
// on the Portia Pulse --------- ------------------------------------
//===================================================================
bool I3EHEEventSelector::PassesCriteriaOnPortiaPulse(
                 I3PortiaEventConstPtr portia_event, 
	 bool baseTimeWindowEvent){

  log_info(" -- Entering PassesCriteriaOnPortiaPulse --");

  if(!portia_event){
    log_error("  This event does not have a Portia map");
    return false;
  }

  int AtwdPulseCounter = 0;
  int FadcPulseCounter = 0;
  double bestEstimatedNPE = 0.0;
  if(!baseTimeWindowEvent){
    bestEstimatedNPE = portia_event->GetTotalBestNPE();
    AtwdPulseCounter = (int )portia_event->GetAtwdNch();
    FadcPulseCounter = (int )portia_event->GetFadcNch();
  }else{
    bestEstimatedNPE = portia_event->GetTotalBestNPEbtw();
    AtwdPulseCounter = (int )portia_event->GetAtwdNchbtw();
    FadcPulseCounter = (int )portia_event->GetFadcNchbtw();
  }

  log_debug("  Best Npe = %e", bestEstimatedNPE);
  log_debug("  NDOMs with FADCs = %d NDOMs with ATWDs = %d",
	    AtwdPulseCounter,FadcPulseCounter);

  // Now check if this event passes the criteria
  bool pass = false;
  if(lowestNPEs_ <= bestEstimatedNPE && 
     bestEstimatedNPE <= highestNPEs_){ // Npe
    if(nDOMmin_ <= AtwdPulseCounter || nDOMmin_ <= FadcPulseCounter)
      pass = true;
  }

  return pass;

}


//===================================================================
// Check if this event satisfies the criteria -----------------------
// on the OpheliaFirstGuessTrack ------------------------------------
//===================================================================
bool I3EHEEventSelector::PassesCriteriaOnFirstGuessTrack(
			 I3OpheliaFirstGuessTrackConstPtr fgTrack){

  log_info(" -- Entering PassesCriteriaOnFirstGuessTrack --");

  if(!fgTrack){
    log_error("  This event does not have a first guess track");
    return false;
  }
  if(!(fgTrack->IsFitSuccessful())){
    log_error("  the first guess fit failed");
    return false;
  }

  // take the OpheliaFirstGuessTrack info
  const I3Particle& i3track =  fgTrack->GetConstI3Particle();

  double cosZenith =  cos(i3track.GetZenith()/I3Units::radian);
  double azimuth =  i3track.GetAzimuth();
  double cobX = fgTrack->GetCenterOfBrightness().GetX();
  double cobY = fgTrack->GetCenterOfBrightness().GetY();
  double cobZ = fgTrack->GetCenterOfBrightness().GetZ();
  double vX = fgTrack->GetVelocity().GetX();
  double vY = fgTrack->GetVelocity().GetY();
  double vZ = fgTrack->GetVelocity().GetZ();
  double bdomZ = fgTrack->GetLargestNPEDOMposition().GetZ();

  double distance =
    sqrt(cobX*cobX+cobY*cobY+cobZ*cobZ);

  double velocity =
    sqrt(vX*vX+vY*vY+vZ*vZ);

  log_debug("  cos(Zenith) = %f Azimuth=%f [deg]",
	    cosZenith,azimuth/I3Units::deg);
  log_debug("  Cob Distance = %lf [km]",distance/I3Units::kilometer);
  log_debug("  Cob (X,Y,Z) = (%f %f %f) [m]",
	    cobX/I3Units::meter,cobY/I3Units::meter,cobZ/I3Units::meter);
  log_debug("  Velocity = %f [m/nsec]",
	    velocity/(I3Units::meter/I3Units::ns));

  // Now check if this event passes the criteria
  bool pass = false;
  if((fg_cosZenithMin_ <= cosZenith) && (cosZenith <= fg_cosZenithMax_)){ // zenith
    if((fg_azimuthMin_ <= azimuth) && (azimuth <= fg_azimuthMax_)){ // azimuth
      if((fg_cogzMin_ <= cobZ) && (cobZ <= fg_cogzMax_)){ // CoGZ
	if((fg_bdomzMin_ <= bdomZ) && (bdomZ <= fg_bdomzMax_)){ // brightestDOMZ
	  if(distance <= distanceOfCOB_ ){ // cob distance
	    if(velocity>= velocityMin_)  pass = true;
	  }
	}
      }
    }
  }

  return pass;
}
//===================================================================
// Check if this event satisfies the IceCube-EHE like criteria ------
// It is judged by Portia's output such as NPE and Ophelia's first guess
//===================================================================
bool I3EHEEventSelector::PassesEHECriteria(
                 I3PortiaEventConstPtr portia_event, 
                 I3PortiaPulseMapConstPtr atwd_pulse,
                 I3PortiaPulseMapConstPtr fadc_pulse,
	 I3OpheliaFirstGuessTrackConstPtr fgTrack){

  log_info(" -- Entering PassesEHECriteria --");

  if(!portia_event && !atwd_pulse && !fadc_pulse && !fgTrack){
    log_error("  This event does not have the dataclasses you need");
    return false;
  }

  //
  // The eventwise NPE and NDOM - after the basetime window cleaning
  double totalBestEstimatedNpe = portia_event->GetTotalBestNPEbtw();
  double nDOMsWithLaunch = portia_event->GetTotalNchbtw();

  //
  // The early check about NDOM - return here if this event fails to satisfy NDOM condition
  //
  if(nDOMsWithLaunch < eheCriteria_->GetNDOMThreshold()) return false;


  // Position of the largest DOM position [cm]
  double lnpeDOMZ = fgTrack->GetLargestNPEDOMposition().GetZ();
  // COB-z [cm]
  __attribute__((unused))
  double cob = fgTrack->GetCenterOfBrightness().GetZ();

  //
  // Categorize the shallow or deep
  //
  bool isThisEventShallow = eheCriteria_->IsThisEventShallow(lnpeDOMZ);

  //
  // Now check if this event satisfies the EHEcrietia
  //
  bool pass = false;
  //
  // The Shallow Event Case
  if(isThisEventShallow){
    log_debug("  this is a shallow event.");
    const I3Particle& i3track =  fgTrack->GetConstI3Particle();
    double cosZenith =  cos(i3track.GetZenith()/I3Units::radian);
    pass = eheCriteria_->PassEHECriteria(cosZenith,totalBestEstimatedNpe,(int )nDOMsWithLaunch,isThisEventShallow);
  //
  // The Deep Event Case
  }else{

    log_debug("  this is a deep event.");
    // The largest NPE time
    OMKey omkey_brightest =  portia_event->GetLargestNPEOMKey();
    I3PortiaPulseMap::const_iterator atwd_brightest_iter = atwd_pulse->find(omkey_brightest);
    const I3PortiaPulse& atwdPulse =  atwd_brightest_iter->second;
    I3PortiaPulseMap::const_iterator fadc_brightest_iter = fadc_pulse->find(omkey_brightest);
    const I3PortiaPulse& fadcPulse =  fadc_brightest_iter->second;

    double largestNPETime = atwdPulse.GetRecoPulse().GetTime();
    if(fadcPulse.GetEstimatedNPE()>atwdPulse.GetEstimatedNPE()) 
      largestNPETime = fadcPulse.GetRecoPulse().GetTime();

    // The earliest DOM time
    OMKey omkey_first =  portia_event->GetFirstPulseOMKeybtw();
    I3PortiaPulseMap::const_iterator atwd_first_iter = atwd_pulse->find(omkey_first);
    const I3PortiaPulse& atwdPulseFirst =  atwd_first_iter->second;
    I3PortiaPulseMap::const_iterator fadc_first_iter = fadc_pulse->find(omkey_first);
    const I3PortiaPulse& fadcPulseFirst =  fadc_first_iter->second;

    double earliestTime = atwdPulseFirst.GetRecoPulse().GetTime();
    if(fadcPulseFirst.GetEstimatedNPE()>atwdPulseFirst.GetEstimatedNPE()) 
      earliestTime = fadcPulseFirst.GetRecoPulse().GetTime();

    double dt = largestNPETime - earliestTime;
    pass = eheCriteria_->PassEHECriteria(dt,totalBestEstimatedNpe,(int )nDOMsWithLaunch,isThisEventShallow);
  }

  return pass;


}
