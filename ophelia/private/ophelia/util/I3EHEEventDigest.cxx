
// This class
#include "ophelia/util/I3EHEEventDigest.h"

#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/physics/I3MCTreeUtils.h"
#include "dataclasses/I3Constants.h"

#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

// This value must be equal to IceCubeCoordinate.elevation
// in the Juliet. See $I3_WORK/juliet/java_lib/sources/iceCube/uhe/geometry/
// IceCubeCoordinate.java
const double I3EHEEventDigest::inIceDistance_ =  8.8392e4*I3Units::cm;


I3_MODULE(I3EHEEventDigest);

// constructor and destructor

//===================================================================
//* constructor -----------------------------------------------------
I3EHEEventDigest::I3EHEEventDigest(const I3Context& ctx) : 
  I3ConditionalModule(ctx),
  digestJulietParticle_(false),
  isNeutrino_(false),
  digestMMCTrack_(false),
  digestPortiaPulse_(true),
  baseTimeWindow_(true),
  digestEHEFirstGuess_(true),
  digestRecoTrack_(false),
  digestMillipede_(false),
  digestExecutiveSummary_(false),
  eventHeaderName_("I3EventHeader"),
  inputI3MCTreeName_("I3MCTree"),
  inputI3JulietParamsTreeName_("I3JulietParamsTree"),
  inputMMCTrackListName_("MMCTrackList"),
  inputPrimaryCRMCTreeName_("I3MCTree"),
  inputInIceDOMLaunchName_("CleanInIceRawData"),
  inputAtwdPulseName_("ATWDPortiaPulse"),
  inputFadcPulseName_("FADCPortiaPulse"),
  inputPortiaEventName_("PortiaEvent"),
  inputFirstguessName_("OpheliaFirstGuessBaseTimeWindow"),
  inputRecoTrackName_("SPEFit8"),
  inputMillipedeParticlesName_("MillipedeAmpsPortiaOphelia")
{

  eventNumber_ = 0;
  shiftX_ = 0.0;
  shiftY_ = 0.0;
  shiftZ_ = 0.0;

  AddParameter("GeometryShiftX","",shiftX_);
  AddParameter("GeometryShiftY","",shiftY_);
  AddParameter("GeometryShiftZ","",shiftZ_);

  AddParameter("digestJulietParticle","", 
	       digestJulietParticle_);  
  AddParameter("digestMMCTrack","", 
	       digestMMCTrack_);  
  AddParameter("inI3MCTreeName","", 
	       inputI3MCTreeName_);  
  AddParameter("inI3JulietParamsTreeName","", 
	       inputI3JulietParamsTreeName_);  
  AddParameter("inPrimaryCRTreeName", " ",
	       inputPrimaryCRMCTreeName_);
  AddParameter("inMMCTrackListName",  " ",
	       inputMMCTrackListName_);

  AddParameter("digestPortiaPulse","", 
	       digestPortiaPulse_);  
  AddParameter("baseTimeWindowCleaning","", 
	       baseTimeWindow_);  
  AddParameter("inInIceDOMLaunchName", "", inputInIceDOMLaunchName_);
  AddParameter("inAtwdPortiaName", "", inputAtwdPulseName_);
  AddParameter("inFadcPortiaName", "", inputFadcPulseName_);
  AddParameter("InputPortiaEventName",      "",  inputPortiaEventName_);

  AddParameter("digestEHEFirstGuess","", 
	       digestEHEFirstGuess_);  
  AddParameter("digestRecoTrack","", 
	       digestRecoTrack_);  
  AddParameter("digestMillipede","", 
	       digestMillipede_);  
  AddParameter("inFirstguessName"," ", inputFirstguessName_);
  AddParameter("inRecoTrackName"," ", inputRecoTrackName_);
  AddParameter("inMillipedeParticlesName"," ", inputMillipedeParticlesName_);

  AddParameter("digestExecutiveSummary","",  digestExecutiveSummary_);
  AddOutBox("OutBox");

}

//===================================================================
//* destructor -----------------------------------------------------
I3EHEEventDigest::~I3EHEEventDigest()
{

}


//===================================================================
//* configure -----------------------------------------------------
void I3EHEEventDigest::Configure(){
  log_info("Configuring the EHE event selector");

  GetParameter("GeometryShiftX",shiftX_);
  GetParameter("GeometryShiftY",shiftY_);
  GetParameter("GeometryShiftZ",shiftZ_);

  GetParameter("digestJulietParticle",
	       digestJulietParticle_);  
  GetParameter("digestMMCTrack",
	       digestMMCTrack_);  
  GetParameter("inI3MCTreeName",
	       inputI3MCTreeName_);  
  GetParameter("inI3JulietParamsTreeName",
	       inputI3JulietParamsTreeName_);  
  GetParameter("inPrimaryCRTreeName",inputPrimaryCRMCTreeName_);
  GetParameter("inMMCTrackListName", inputMMCTrackListName_);

  GetParameter("digestPortiaPulse",
	       digestPortiaPulse_);  
  GetParameter("baseTimeWindowCleaning",baseTimeWindow_);  
  GetParameter("inInIceDOMLaunchName",inputInIceDOMLaunchName_);
  GetParameter("inAtwdPortiaName", inputAtwdPulseName_);
  GetParameter("inFadcPortiaName", inputFadcPulseName_);
  GetParameter("InputPortiaEventName",  inputPortiaEventName_);

  GetParameter("digestEHEFirstGuess",
	       digestEHEFirstGuess_);  
  GetParameter("digestRecoTrack",
	       digestRecoTrack_);  
  GetParameter("digestMillipede",
	       digestMillipede_);  
  GetParameter("inFirstguessName",inputFirstguessName_);
  GetParameter("inRecoTrackName",inputRecoTrackName_);
  GetParameter("inMillipedeParticlesName", inputMillipedeParticlesName_);

  GetParameter("digestExecutiveSummary",digestExecutiveSummary_);

  if(digestJulietParticle_){
    log_info("----You will digest Juliet Particles----");
    log_info("  Juliet I3MCTree Name is %s",  
             inputI3MCTreeName_.c_str()); 
    log_info("  I3JulietParamsTree Name is %s",  
             inputI3JulietParamsTreeName_.c_str()); 
  }

  if(digestMMCTrack_){
    log_info("----You will digest MMC tracks----");
    log_info("  Cosmic Ray I3MCTree Name is %s",  
             inputPrimaryCRMCTreeName_.c_str()); 
    log_info("  MMC track list Name is %s",  
             inputMMCTrackListName_.c_str()); 
  }

  if(baseTimeWindow_) 
    log_info("-- We digest reco/portia with the basetime window cleaing-----");

  if(digestPortiaPulse_){
    log_info("----You will digest Portia Pulse----");
    log_info("  Poirita Event pulse Name is %s", 
             inputPortiaEventName_.c_str()); 
  }

  if(digestEHEFirstGuess_){
    log_info("---You will digest EHE first guess track---");
    log_info("  Ophelia first guess track Name is %s", 
	     inputFirstguessName_.c_str());
  }
  if(digestRecoTrack_){
    log_info("---You will digest reconsruction track---");
    log_info("  reconstructed track I3Paticle Name is %s", 
	     inputRecoTrackName_.c_str());
  }

  if(digestMillipede_){
    log_info("---You will digest millipede particles---");
    log_info("  I3Paticles Name is %s", 
	     inputMillipedeParticlesName_.c_str());
  }


}


//===================================================================
//* physics -----------------------------------------------------
void I3EHEEventDigest::Physics(I3FramePtr frame){
  log_info("Entering I3EHEEventDigest::Physics()");

  InitializeValuablesInDigest();

  //Take the event header if exists
  if(!digestJulietParticle_){
    I3EventHeaderConstPtr eventHeader = 
      frame->Get<I3EventHeaderConstPtr>(eventHeaderName_);
    if(eventHeader){
      //eventNumber_ = 100000*eventHeader->GetRunID() + eventHeader->GetEventID();
      eventNumber_ = (unsigned long )(eventHeader->GetRunID());
      runID_ = (unsigned long )(eventHeader->GetRunID());
      eventID_ = (unsigned long )(eventHeader->GetEventID());
      
    }
  }

  // Take the JulietParticle from the frame
  if(digestJulietParticle_){
    I3MCTreeConstPtr mc_tree_ptr = 
      frame->Get<I3MCTreeConstPtr>(inputI3MCTreeName_);
    I3JulietParamsTreeConstPtr params_tree_ptr = 
      frame->Get<I3JulietParamsTreeConstPtr>(inputI3JulietParamsTreeName_);
    DigestJulietParticle(*mc_tree_ptr, *params_tree_ptr);
  }else if(digestMMCTrack_){ // Take the Corsika event
    //MC Truth Tree with MMC Tracks
    I3MMCTrackListConstPtr muonBundle = 
      frame->Get<I3MMCTrackListConstPtr>(inputMMCTrackListName_);
    // Primary CR Tree
    I3MCTreeConstPtr crTree = 
      frame->Get<I3MCTreeConstPtr>(inputPrimaryCRMCTreeName_);
    DigestMMCTrack(*muonBundle,*crTree);
  }

  // Take the Portia Pulse from the frame
  if(digestPortiaPulse_){
    I3PortiaPulseMapConstPtr fadc_portia_map = 
      frame->Get<I3PortiaPulseMapConstPtr>(inputFadcPulseName_);
    I3PortiaPulseMapConstPtr atwd_portia_map = 
      frame->Get<I3PortiaPulseMapConstPtr>(inputAtwdPulseName_);
    I3PortiaEventConstPtr portia_event = 
      frame->Get<I3PortiaEventConstPtr>(inputPortiaEventName_);
    DigestPortiaPulse(portia_event,atwd_portia_map,fadc_portia_map,baseTimeWindow_);
  }


  // Take the OpheliaEHEFirstGuessTrack from the frame
  if(digestEHEFirstGuess_){
    I3OpheliaFirstGuessTrackConstPtr fgTrack = 
      frame->Get<I3OpheliaFirstGuessTrackConstPtr>(inputFirstguessName_);
    DigestFirstGuessTrack(fgTrack);
    digestRecoTrack_ = false;  // Use only the Ophelia track to get the track gemetry
  }

  // Take the Reco Track (I3Particle) from the frame
  if(digestRecoTrack_){
    I3ParticleConstPtr recoTrack = 
      frame->Get<I3ParticleConstPtr>(inputRecoTrackName_);
    DigestRecoTrack(recoTrack);
  }

  // Take the Millipede particles (I3VectorI3Particle) from the frame
  if(digestMillipede_){
    I3VectorI3ParticleConstPtr cascadeSeries = 
      frame->Get<I3VectorI3ParticleConstPtr>(inputMillipedeParticlesName_);
    DigestMillipede(cascadeSeries);
  }


  // Output this event in the gigested form
  if(!digestExecutiveSummary_) OutputEventDigest();
  else OutputEventExecutiveSummary();

  PushFrame(frame,"OutBox");

}//End Physics()


//===================================================================
// Digest  the Juliet Particle --------------------------------------
//===================================================================
void I3EHEEventDigest::DigestJulietParticle
(const I3MCTree& mc_tree, const I3JulietParamsTree& params_tree)
{
  log_info(" -- Entering DigestJulietParticle --");

  // Take the primary juliet particle info
  // assuming there is only one primary
  const vector<I3Particle>& i3juliet_track = I3MCTreeUtils::GetPrimaries(mc_tree);
  if(i3juliet_track.size()<1){
    log_warn("Couldn't find any primary!");
    return;
  }else if(i3juliet_track.size()>1){
    log_warn("Multiple primaries found!");
    return;
  }

  I3JulietParams i3juliet_track_params 
    = I3JulietUtils::GetJulietParams(params_tree, 
				     i3juliet_track[0].GetMajorID(), 
				     i3juliet_track[0].GetMinorID());

  I3JulietUtils::GetParticleFlavorAndDoublet(i3juliet_track[0].GetType(), 
			      flavor_, doublet_);


  // Check if the primary particle is neutrino
  isNeutrino_ = false; enhancementFactor_ = 1.0; trackLength_= 0.0;
  if(doublet_ == 0 && flavor_ <=2 ) isNeutrino_ = true;

  // geometry in the IceCube coordinates. 
  const I3Direction& i3_direction = i3juliet_track[0].GetDir();
  nxMC_ = i3_direction.GetX();
  nyMC_ = i3_direction.GetY();
  nzMC_ = i3_direction.GetZ();
  
  // energy
  energyMC_ = i3juliet_track[0].GetEnergy();

  // Neutrino Weight Option
  if(isNeutrino_){
    trackLength_ = i3juliet_track[0].GetLength();
    enhancementFactor_ = 
      I3JulietPrimaryParticleSource::GetNeutrinoEnhancementFactor(i3juliet_track[0].GetType(), energyMC_);
  }

  // propagation distance from the earth surface
  distanceFromEarthSurface_ = 
    i3juliet_track_params.GetDistanceFromEarthSurfaceToStartPoint();

  double cascadeEnergyMax = 0.;
  // find the most energetic cascade and store the info.
  I3MCTree::const_iterator iter;
  for (iter=mc_tree.begin(); iter!=mc_tree.end(); ++iter) {
    if (iter->GetEnergy() > cascadeEnergyMax && iter->IsCascade()) {
      cascadeEnergyMax = iter->GetEnergy();
      cascadeX_ = iter->GetX();
      cascadeY_ = iter->GetY();
      cascadeZ_ = iter->GetZ();
    }
  }

  cascadeX_ = (cascadeX_+shiftX_);
  cascadeY_ = (cascadeY_+shiftY_);
  cascadeZ_ = (cascadeZ_+shiftZ_);
}

//===================================================================
// Digest  the MMC tracks --------------------------------------
//===================================================================
void I3EHEEventDigest::DigestMMCTrack
(const I3MMCTrackList& muonBundle, const I3MCTree& cosmicRayTree)
{

  log_info(" -- Entering DigestMMCTrack --");

  if(muonBundle.size()<1){
    log_warn("Couldn't find any MMC tracks!");
    return;
  }

  flavor_ = 1; doublet_ = 1; 
  // muon by the Juliet particle definition

  I3MMCTrackList::const_iterator iterBundle = muonBundle.begin();

  // Loop over all MMC tracks (muons)
  // and average(or sum) their physical parameters.
  // It also calculate in-ice muon energies at inIceDistance_ from
  // the iceCube center by adding up secondary particles (e.x. e_pair) energies
  double x = 0.0; double y = 0.0; double z = 0.0;
  double azimuth = 0.0; double zenith = 0.0; double length = 0.0;
  double energySum = 0.0;

  muonEnergyAtSurface_ = 0.0;
  energyMC_ = 0.0;

  for(;iterBundle != muonBundle.end(); ++iterBundle){
    I3MMCTrack muon = *iterBundle;
    I3Particle& i3_muon = muon.GetParticle();
    double muonEnergyInIce = muon.GetEi(); // muon energy at entrance
    muonEnergyAtSurface_ += i3_muon.GetEnergy(); // muon energy at surface

    if(muonEnergyInIce>0.0){ // this muon indeed reached the IceCube volume

      azimuth += muonEnergyInIce*i3_muon.GetAzimuth();
      zenith += muonEnergyInIce*i3_muon.GetZenith();
      x += muonEnergyInIce*i3_muon.GetX();y += muonEnergyInIce*i3_muon.GetY();
      z += muonEnergyInIce*i3_muon.GetZ();
      length += muonEnergyInIce*i3_muon.GetLength();
      energySum += muonEnergyInIce;

      // Look into the composite particles (secondary particles)
      const vector<I3Particle>& secondaries =
	I3MCTreeUtils::GetDaughters(cosmicRayTree, i3_muon);

      log_debug("%zu secondaries found", secondaries.size());

      int n=1;
      vector<I3Particle>::const_iterator secondary;
      for (secondary = secondaries.begin();
	   secondary != secondaries.end(); secondary++) {

	log_trace("type: %zu", (*secondary).GetType());
	log_trace("Pos(%f, %f, %f)", (*secondary).GetX(),
		  (*secondary).GetY(), (*secondary).GetZ());
	log_trace("Dir(%f, %f)", (*secondary).GetZenith(),(*secondary).GetAzimuth());
	log_trace("Time: %f, Energy: %f", (*secondary).GetTime(), (*secondary).GetEnergy());

	muonEnergyInIce -= (*secondary).GetEnergy();

	const I3Position& daughter_vertex = (*secondary).GetPos();
	double distance = sqrt(
          daughter_vertex.GetX()*daughter_vertex.GetX() +
	  daughter_vertex.GetY()*daughter_vertex.GetY() +
	  daughter_vertex.GetZ()*daughter_vertex.GetZ());
	log_trace("   this muon energy = %f at distance %f [m] (%d th cascade)", 
		  muonEnergyInIce, distance/I3Units::m, n);
	if(distance< inIceDistance_) break; 
	n++;
      }

      energyMC_ += muonEnergyInIce;
    }
  }

  // Energy weighted average
  azimuth = azimuth/energySum; zenith = zenith/energySum;
  x = x/energySum; y = y/energySum; z = z/energySum;
  length = length/energySum;

  cascadeX_ = x;
  cascadeY_ = y;
  cascadeZ_ = z;
  distanceFromEarthSurface_ = length;

  double theta = pi-zenith;
  double phi = azimuth-pi;
  double rho = sin(theta);
  nxBundle_ = rho*cos(phi);
  nyBundle_ = rho*sin(phi);
  nzBundle_ = cos(theta);

  // look into primary cosmic ray particle
  const std::vector<I3Particle> primaries 
    = I3MCTreeUtils::GetPrimaries(cosmicRayTree);

  if(!primaries.size()) {
      log_error("Error no Primary Cosmic Ray Particles");
  } else {
    for(int i=0; i<(int)primaries.size(); i++){
      if(primaries[i].GetType()==I3Particle::PPlus||
	 primaries[i].GetType()==I3Particle::PMinus||
	 primaries[i].GetType()==I3Particle::He4Nucleus||
	 primaries[i].GetType()==I3Particle::O16Nucleus||
	 primaries[i].GetType()==I3Particle::Si28Nucleus||
	 primaries[i].GetType()==I3Particle::Fe56Nucleus){

	crEnergy_ = primaries[i].GetEnergy();
	const I3Direction& i3_direction = primaries[i].GetDir();
	nxMC_ = i3_direction.GetX();
	nyMC_ = i3_direction.GetY();
	nzMC_ = i3_direction.GetZ();

      }
    }
  }//end non-zero


}

//===================================================================
// Check if this event satisfies the criteria -----------------------
// on the Portia Pulse --------- ------------------------------------
//===================================================================
void I3EHEEventDigest::DigestPortiaPulse(I3PortiaEventConstPtr portia_event, 
					 I3PortiaPulseMapConstPtr atwd_pulse,
					 I3PortiaPulseMapConstPtr fadc_pulse,
					 bool baseTimeWindowEvent){

  log_info(" -- Entering DigestPortiaPulse --");

  if(!portia_event && !atwd_pulse && !fadc_pulse){
    log_error("  This event does not have a PortiaEvent dataclass");
    return;
  }

  if(!baseTimeWindowEvent){
    totalAtwdEstimatedNpe_ = portia_event->GetTotalAtwdNPE();
    totalFadcEstimatedNpe_ = portia_event->GetTotalFadcNPE();
    totalBestEstimatedNpe_ = portia_event->GetTotalBestNPE();
    nDOMsWithATWD_ = portia_event->GetAtwdNch();
    nDOMsWithFADC_ = portia_event->GetFadcNch();
    nDOMsWithLaunch_ = portia_event->GetTotalNch();
  }else{
    totalAtwdEstimatedNpe_ = portia_event->GetTotalAtwdNPEbtw();
    totalFadcEstimatedNpe_ = portia_event->GetTotalFadcNPEbtw();
    totalBestEstimatedNpe_ = portia_event->GetTotalBestNPEbtw();
    nDOMsWithATWD_ = portia_event->GetAtwdNchbtw();
    nDOMsWithFADC_ = portia_event->GetFadcNchbtw();
    nDOMsWithLaunch_ = portia_event->GetTotalNchbtw();
  }

  //
  // The largest NPE time
  //
  OMKey omkey_brightest =  portia_event->GetLargestNPEOMKey();
  I3PortiaPulseMap::const_iterator atwd_brightest_iter = atwd_pulse->find(omkey_brightest);
  const I3PortiaPulse& atwdPulse =  atwd_brightest_iter->second;
  I3PortiaPulseMap::const_iterator fadc_brightest_iter = fadc_pulse->find(omkey_brightest);
  const I3PortiaPulse& fadcPulse =  fadc_brightest_iter->second;

  // chose ATWD or FADC depending on captured NPEs in each digitizers
  largestNPETime_ = atwdPulse.GetRecoPulse().GetTime();
  if(fadcPulse.GetEstimatedNPE()>atwdPulse.GetEstimatedNPE()) 
    largestNPETime_ = fadcPulse.GetRecoPulse().GetTime();

  //
  // The earliest DOM time
  //
  OMKey omkey_first =  portia_event->GetFirstPulseOMKeybtw();
  if(!baseTimeWindowEvent) omkey_first = portia_event->GetFirstPulseOMKey();
  I3PortiaPulseMap::const_iterator atwd_first_iter = atwd_pulse->find(omkey_first);
  const I3PortiaPulse& atwdPulseFirst =  atwd_first_iter->second;
  I3PortiaPulseMap::const_iterator fadc_first_iter = fadc_pulse->find(omkey_first);
  const I3PortiaPulse& fadcPulseFirst =  fadc_first_iter->second;

  // chose ATWD or FADC depending on captured NPEs in each digitizers
  earliestTime_ = atwdPulseFirst.GetRecoPulse().GetTime();
  if(fadcPulseFirst.GetEstimatedNPE()>atwdPulseFirst.GetEstimatedNPE()) 
    earliestTime_ = fadcPulseFirst.GetRecoPulse().GetTime();

 
}


//===================================================================
// Check if this event satisfies the criteria -----------------------
// on the OpheliaFirstGuessTrack ------------------------------------
//===================================================================
void I3EHEEventDigest::DigestFirstGuessTrack(
		     I3OpheliaFirstGuessTrackConstPtr fgTrack){

  log_info(" -- Entering DigestFirstGuessTrack --");

  if(!fgTrack){
    log_error("  This event does not have a first guess track");
    return;
  }
  if(!(fgTrack->IsFitSuccessful())){
    log_error("  the first guess fit failed");
    return;
  }

  // take the OpheliaFirstGuessTrack info
  const I3Particle& i3track =  fgTrack->GetConstI3Particle();

  // geometry in the IceCube coordinates. 
  const I3Direction& i3_direction = i3track.GetDir();
  nxFG_ = i3_direction.GetX();
  nyFG_ = i3_direction.GetY();
  nzFG_ = i3_direction.GetZ();

  // Center of Brightness [cm]
  cobX_ = shiftX_+fgTrack->GetCenterOfBrightness().GetX();
  cobY_ = shiftY_+fgTrack->GetCenterOfBrightness().GetY();
  cobZ_ = shiftZ_+fgTrack->GetCenterOfBrightness().GetZ();
  //  double vX = fgTrack->GetVelocity().X();
  //  double vY = fgTrack->GetVelocity().Y();
  //  double vZ = fgTrack->GetVelocity().Z();

  //  velocity_ = sqrt(vX*vX+vY*vY+vZ*vZ);
  velocity_ = fgTrack->GetFitQuality();

  // Position of the largest DOM position [cm]
  lnpeDOMX_ = fgTrack->GetLargestNPEDOMposition().GetX();
  lnpeDOMY_ = fgTrack->GetLargestNPEDOMposition().GetY();
  lnpeDOMZ_ = fgTrack->GetLargestNPEDOMposition().GetZ();

}

//===================================================================
// Digest the reco results stored in I3Particle
//===================================================================
void I3EHEEventDigest::DigestRecoTrack(I3ParticleConstPtr i3Track){

  log_info(" -- Entering DigestRecoTrack --");

  if(!i3Track){
    log_error("  This event does not have an I3Particle track");
    return;
  }

  // geometry in the IceCube coordinates. 
  const I3Direction& i3_direction = i3Track->GetDir();
  nxFG_ = i3_direction.GetX();
  nyFG_ = i3_direction.GetY();
  nzFG_ = i3_direction.GetZ();

  // The track position
  cobX_ = i3Track->GetX();
  cobY_ = i3Track->GetY();
  cobZ_ = i3Track->GetZ();

}
//===================================================================
// Digest the millipede particles stored in I3VectorI3Particle
//===================================================================
void I3EHEEventDigest::DigestMillipede(I3VectorI3ParticleConstPtr cascadeSeries){

  log_info(" -- Entering DigestMillipede --");

  if(!cascadeSeries){
    log_error("  This event does not have an I3Particle Vector");
    return;
  }

  I3VectorI3Particle::const_iterator secondary = cascadeSeries->begin();
  startPosition_.SetX((*secondary).GetPos().GetX());
  startPosition_.SetY((*secondary).GetPos().GetY());
  startPosition_.SetZ((*secondary).GetPos().GetZ());
  for (secondary = cascadeSeries->begin(); secondary != cascadeSeries->end(); secondary++) {
    energyDeposit_ += (*secondary).GetEnergy();
  }
  secondary--;
  endPosition_.SetX((*secondary).GetPos().GetX());
  endPosition_.SetY((*secondary).GetPos().GetY());
  endPosition_.SetZ((*secondary).GetPos().GetZ());
}

//===================================================================
// Initialization
//===================================================================
void I3EHEEventDigest::InitializeValuablesInDigest(){


  eventNumber_++;
  runID_ = 0; eventID_ = 0;

  flavor_ = 1;
  doublet_ = 1;

  nxMC_ = 0.0;
  nyMC_ = 0.0;
  nzMC_ = 1.0;
  nxBundle_ = 0.0;
  nyBundle_ = 0.0;
  nzBundle_ = 1.0;

  energyMC_ = -1.0; 
  crEnergy_ = -1.0; muonEnergyAtSurface_ = -1.0;
  distanceFromEarthSurface_ = -1.0;

  cascadeX_ = -10.0*I3Units::kilometer;
  cascadeY_ = -10.0*I3Units::kilometer;
  cascadeZ_ = -10.0*I3Units::kilometer;


  totalAtwdEstimatedNpe_ = 0.0;
  totalFadcEstimatedNpe_ = 0.0;

  nDOMsWithATWD_ = 0;
  nDOMsWithFADC_ = 0;

  largestNPETime_ = -100.0*I3Units::ms;
  earliestTime_ = -100.0*I3Units::ms;

  nxFG_= 0.0;
  nyFG_ = 0.0;
  nzFG_ = 1.0;

  cobX_ = -10.0*I3Units::kilometer;
  cobY_ = -10.0*I3Units::kilometer;
  cobZ_ = -10.0*I3Units::kilometer;
  lnpeDOMX_ = -10.0*I3Units::kilometer;
  lnpeDOMY_ = -10.0*I3Units::kilometer;
  lnpeDOMZ_ = -10.0*I3Units::kilometer;

  velocity_ = -1.0;

  energyDeposit_ = 0.0;

}

//===================================================================
// Output an event digest to standard output
//===================================================================
void I3EHEEventDigest::OutputEventDigest(){

  printf(" EVENT %lu %d %d \n",eventNumber_,flavor_,doublet_);
  printf(" EVENT %e %e \n",energyMC_/I3Units::GeV, 
                           distanceFromEarthSurface_/I3Units::centimeter);

  if(digestJulietParticle_ || digestMMCTrack_){ // this is a MC event  
    printf(" EVENT %lf %lf %lf \n", nxMC_,  nyMC_,  nzMC_);
    printf(" EVENT %e %e %e \n", cascadeX_/I3Units::centimeter,
	   cascadeY_/I3Units::centimeter,
	   cascadeZ_/I3Units::centimeter);
    if(isNeutrino_ && !digestMillipede_) printf(" EVENT %lf %e \n", enhancementFactor_, 
			   trackLength_/I3Units::centimeter);
    if(digestMMCTrack_){ // This is a Corsila event
      printf(" EVENT %e %e \n",crEnergy_/I3Units::GeV, 
	     muonEnergyAtSurface_/I3Units::GeV);
      printf(" EVENT %lf %lf %lf \n", nxBundle_,  nyBundle_,  nzBundle_);
    }
  }

  if(digestEHEFirstGuess_||digestRecoTrack_){
    printf(" EVENT %lf %lf %lf %e \n", nxFG_,  nyFG_,  nzFG_,
 //                                       velocity_/(I3Units::centimeter/I3Units::s));
	   velocity_);
    printf(" EVENT %e %e %e \n", cobX_/I3Units::centimeter,
	                         cobY_/I3Units::centimeter,
                                 cobZ_/I3Units::centimeter);

  }

  if(digestMillipede_){
    printf(" EVENT %e \n",energyDeposit_/I3Units::GeV);
    printf(" EVENT %e %e %e \n", startPosition_.GetX()/I3Units::centimeter, startPosition_.GetY()/I3Units::centimeter,
	   startPosition_.GetZ()/I3Units::centimeter);
    printf(" EVENT %e %e %e \n", endPosition_.GetX()/I3Units::centimeter, endPosition_.GetY()/I3Units::centimeter,
	   endPosition_.GetZ()/I3Units::centimeter);
  }

  if(digestPortiaPulse_){
    printf(" EVENT %e %d %e %d %e %d \n",
	   totalAtwdEstimatedNpe_,nDOMsWithATWD_,
	   totalFadcEstimatedNpe_,nDOMsWithFADC_,
	   totalBestEstimatedNpe_,nDOMsWithLaunch_);
  }
}

//===================================================================
// Output an event executive summary to standard output
//===================================================================
void I3EHEEventDigest::OutputEventExecutiveSummary(){

  printf(" EVENT %lu %lu \n",runID_, eventID_);
  printf(" EVENT %lf %lf %lf \n", nxFG_,  nyFG_,  nzFG_); // reco after BTW cleaning
  printf(" EVENT %lf %lf \n", 
	 earliestTime_/I3Units::ns, largestNPETime_/I3Units::ns); // t1st, t-Largest-NPE-DOM
  printf(" EVENT %e %e %e \n", cobX_/I3Units::centimeter,
	 cobY_/I3Units::centimeter,
	 cobZ_/I3Units::centimeter); // Center of Brightness
  printf(" EVENT %e %e %e \n", lnpeDOMX_/I3Units::centimeter,
	 lnpeDOMY_/I3Units::centimeter,
	 lnpeDOMZ_/I3Units::centimeter);// Largest NPE DOM position
  printf(" EVENT %e %d \n",
	 totalBestEstimatedNpe_,nDOMsWithLaunch_); // Best NPE, NDOM
}
