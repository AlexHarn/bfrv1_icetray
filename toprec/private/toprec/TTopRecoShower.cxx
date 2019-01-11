/**
 * Copyright (C) 2007
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file TTopRecoShower.cxx
 * @version $Rev: $
 * @date $Date$
 * @author $Author: $
 */


#include "toprec/TTopRecoShower.h"
#include <recclasses/I3TopLateralFitParams.h>
#include <recclasses/I3TopRecoPlaneFitParams.h>
#include "toprec/LateralFitFunctions.h"

#include "icetray/OMKey.h"
#include "dataclasses/TankKey.h"
#include "icetray/I3TrayHeaders.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/geometry/I3OMGeo.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/status/I3DOMStatus.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include "icetray/I3Units.h"
#include "dataclasses/I3Constants.h"

#include <algorithm>

using namespace std;


namespace {

  // To sort pulses
  bool ChargeOrdered(const showerPulse& lhs, const showerPulse& rhs)
  {
    return lhs.vem > rhs.vem;      //sort from big to small
  }

}


TTopRecoShower::TTopRecoShower()
  : dir(I3Direction(NAN, NAN)), nx(NAN), ny(NAN), t_0(NAN), x_0(NAN),
    x_c(NAN), y_c(NAN), z_c(NAN), height_corrected(false), 
    artificial_shift(-1), loudest_station(-1)
{

  ClearHits();

}


TTopRecoShower::~TTopRecoShower(){

  ClearHits();   

}

// **************************************************************** //

void TTopRecoShower::ClearHits(){

  pulseData.clear();
  noPulseData.clear();

}

// **************************************************************** //

bool TTopRecoShower::ReadData(I3FramePtr frame, string datalabel, string corelabel, string planelabel,
			      string badStationLabel, double software_threshold){

  // ----- check if everything's there
  if(!frame->Has("I3Geometry")) {log_fatal("No Geometry in Frame!"); return false;}
  if(!frame->Has("I3DetectorStatus")) {log_fatal("No DetectorStatus in Frame!"); return false;}
  if(!frame->Has(datalabel)) {log_debug("No Pulses in Frame!"); return false;}

  // ----- get what is there
  const I3DetectorStatus &status = frame->Get<I3DetectorStatus>();
  const I3Geometry &geometry = frame->Get<I3Geometry>();
  const I3OMGeoMap& om_map = geometry.omgeo;
  const map<OMKey, I3DOMStatus> &status_map = status.domStatus;
  I3RecoPulseSeriesMapConstPtr pulse_series_map =
    frame->Get<I3RecoPulseSeriesMapConstPtr> (datalabel);
  // no check for this since it's not guaranteed to be in the frame
  I3VectorIntConstPtr badStations = frame->Get<I3VectorIntConstPtr>(badStationLabel);
  log_trace ("pulse_series_map has %zu entries", pulse_series_map->size ());
  I3StationGeoMap smap = geometry.stationgeo;

  // ----- core and plane if ordered
  if(corelabel!=""){

    if(!frame->Has(corelabel)) {log_debug("No Core in Frame!"); return false;}
    const I3Particle &inCore = frame->Get<I3Particle>(corelabel);
    SetCore(inCore.GetPos());
    SetTime(inCore.GetTime());
    if(!(x_c*x_c>0)) return false;

  }

  if(planelabel!=""){

    I3ParticleConstPtr inPlane = frame->Get<I3ParticleConstPtr>(planelabel);
    if(!inPlane) {
      log_debug("No Plane in Frame!");
      return false;
    }

    I3TopRecoPlaneFitParamsConstPtr inPlaneParams =
      frame->Get<I3TopRecoPlaneFitParamsConstPtr>(planelabel + "Params");

    SetDirection(inPlane->GetDir());

    if (inPlaneParams) {
      //double T0 = inPlaneParams->T0 -
      //(nx * (x_c - inPlaneParams->X0) + ny * (y_c - inPlaneParams->Y0));
      // KR: BUG FIXED 6/17/2011
      double T0 = inPlaneParams->T0 +
	(nx * (x_c - inPlaneParams->X0) + ny * (y_c - inPlaneParams->Y0))/I3Constants::c;
      SetPlaneParams(T0, x_c, y_c);
      if (!(inPlaneParams->Chi2 > 0)) return false;
    } else {
      SetPlaneParams(inPlane->GetTime(), x_c, y_c);
    }

  }
 
  // ----- map to keep track which stations we have
  set<StationKey> foundPulseInStation;
  set<StationKey> foundLCInStation;

  // ----- loop over the DOMSs of the current detector setup
  double loudestPulse = 0.;
  log_debug ("Going to loop over DOMs");
  for (map<OMKey, I3DOMStatus>::const_iterator i_om = status_map.begin ();
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


    const I3RecoPulseSeries &pulse_series = iter->second;
    if (0 == pulse_series.size()){ // no pulse in the series?

      log_debug ("pulse_series found, but empty.");
      continue;

    }
    else log_trace("pulse_series succeeded to cast, has %zu entries", pulse_series.size ());

    // ----- Loop over the pulse series to extract the data
    for (I3RecoPulseSeries::const_iterator i_series = pulse_series.begin ();
         i_series != pulse_series.end (); ++i_series) {

      if((*i_series).GetCharge() > software_threshold) {
  
	if(foundPulseInStation.find(dom_key.GetString())!=foundPulseInStation.end()) {
	  foundLCInStation.insert(dom_key.GetString());
	}
	else {
	  foundPulseInStation.insert(dom_key.GetString());
	}
  
        AddPulse(dom_key, (om.position).GetX(), (om.position).GetY(), (om.position).GetZ(),
                 (*i_series).GetTime(), (*i_series).GetWidth(), (*i_series).GetCharge(),
		 true, true, thisdepth);
	log_trace("Adding a pulse.....d %f", thisdepth);
        log_trace("Got a pulse time of %f (%f, %f, %f)",
		  (*i_series).GetTime (),
		  (om.position).GetX (),
		  (om.position).GetY (),
		  (om.position).GetZ ());
        log_trace("    PE: %f, Width: %f)",
		  (*i_series).GetCharge (),
		  (*i_series).GetWidth ());
        if((*i_series).GetCharge()>loudestPulse) {
          loudestPulse = (*i_series).GetCharge();
          loudest_station = dom_key.GetString();
        }
      } // software threshold?

      break; // take only the first pulse (could be changed if wanted)

    } // loop over single pulses

  } // loop over oms

  //clean out LC
  if(software_threshold>0.) {
    vector<showerPulse> new_pulses;
    for(vector<showerPulse>::iterator it = pulseData.begin(); it!=pulseData.end(); it++){
      if(foundLCInStation.find((*it).omkey.GetString()) != foundLCInStation.end()) {
	new_pulses.push_back(*it);
      }
    }
    pulseData = new_pulses;
  }     
 
  // check which stations do not have a pulse
  for (map<OMKey, I3DOMStatus>::const_iterator i_om = status_map.begin ();
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
    avgdepth = (tankGeo0.snowheight+tankGeo1.snowheight)/2;
    log_trace("Station %d with no hits: depth: %f", dom_key.GetString(), avgdepth);


    AddNoPulse(dom_key.GetString(), (om.position).GetX(), (om.position).GetY(), (om.position).GetZ(), 
	       avgdepth);

  }

  return true;

}

#if FOR_GULLIVER
// this one also returns the multiplicity.
// (formerly, returned a boolean)
int TTopRecoShower::ReadDataFromSeed(const I3Frame &frame, 
				      I3ParticleConstPtr seed, 
				      std::string datalabel,
				      std::string badStationLabel, double software_threshold) {

  // ----- check if everything's there
  if(!frame.Has("I3Geometry")) {log_fatal("No Geometry in Frame!"); return false;}
  if(!frame.Has("I3DetectorStatus")) {log_fatal("No DetectorStatus in Frame!"); return false;}
  if(!frame.Has(datalabel)) {log_debug("No Pulses in Frame!"); return false;}

  // ----- get what is there
  const I3DetectorStatus &status = frame.Get<I3DetectorStatus>();
  const I3Geometry &geometry = frame.Get<I3Geometry>();
  const I3OMGeoMap& om_map = geometry.omgeo;
  const map<OMKey, I3DOMStatus> &status_map = status.domStatus;
  I3RecoPulseSeriesMapConstPtr pulse_series_map =
    frame.Get<I3RecoPulseSeriesMapConstPtr> (datalabel);
  // no check for this since it's not guaranteed to be in the frame
  I3VectorIntConstPtr badStations = frame.Get<I3VectorIntConstPtr>(badStationLabel);
  log_trace ("pulse_series_map has %zu entries", pulse_series_map->size ());
  I3StationGeoMap smap = geometry.stationgeo;

  // ----- core and plane if ordered
  SetCore(seed->GetPos());
  SetTime(seed->GetTime());
  // Don't check this yet, because the seed is a dummy the first time.
  //  if(!(x_c*x_c>0)) { log_warn("x_c is zero"); return false; }

  SetDirection(seed->GetDir());
  SetPlaneParams(seed->GetTime(),
		 seed->GetPos().GetX(),   // These last two lines set x0, y0
		 seed->GetPos().GetY());  // but these are the same as xc, yc?

  // ------------- the rest of this is the same as in ReadData() -------------

  // ----- map to keep track which stations we have
  set<StationKey> foundPulseInStation;
  set<StationKey> foundLCInStation;

  // ----- loop over the DOMSs of the current detector setup
  double loudestPulse = 0.;
  log_debug ("Going to loop over DOMs");
  for (map<OMKey, I3DOMStatus>::const_iterator i_om = status_map.begin ();
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


    const I3RecoPulseSeries pulse_series = iter->second;
    if (0 == pulse_series.size()){ // no pulse in the series?

      log_debug ("pulse_series found, but empty.");
      continue;

    }
    else log_trace("pulse_series succeeded to cast, has %zu entries", pulse_series.size ());

    // ----- Loop over the pulse series to extract the data
    for (I3RecoPulseSeries::const_iterator i_series = pulse_series.begin ();
         i_series != pulse_series.end (); ++i_series) {

      if((*i_series).GetCharge() > software_threshold) {
  
	if(foundPulseInStation.find(dom_key.GetString())!=foundPulseInStation.end()) {
	  foundLCInStation.insert(dom_key.GetString());
	}
	else {
	  foundPulseInStation.insert(dom_key.GetString());
	}
  
        AddPulse(dom_key, (om.position).GetX(), (om.position).GetY(), (om.position).GetZ(),
                 (*i_series).GetTime(), (*i_series).GetWidth(), (*i_series).GetCharge(),
		 true, true, thisdepth);
	log_trace("Adding a pulse.....d %f", thisdepth);
        log_trace("Got a pulse time of %f (%f, %f, %f)",
		  (*i_series).GetTime (),
		  (om.position).GetX (),
		  (om.position).GetY (),
		  (om.position).GetZ ());
        log_trace("    PE: %f, Width: %f)",
		  (*i_series).GetCharge (),
		  (*i_series).GetWidth ());
        if((*i_series).GetCharge()>loudestPulse) {
          loudestPulse = (*i_series).GetCharge();
          loudest_station = dom_key.GetString();
        }
      } // software threshold?

      break; // take only the first pulse (could be changed if wanted)

    } // loop over single pulses

  } // loop over oms

  //clean out LC
  if(software_threshold>0.) {
    vector<showerPulse> new_pulses;
    for(vector<showerPulse>::iterator it = pulseData.begin(); it!=pulseData.end(); it++){
      if(foundLCInStation.find((*it).omkey.GetString()) != foundLCInStation.end()) {
	new_pulses.push_back(*it);
      }
    }
    pulseData = new_pulses;
  }     
 
  // check which stations do not have a pulse
  for (map<OMKey, I3DOMStatus>::const_iterator i_om = status_map.begin ();
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
    avgdepth = (tankGeo0.snowheight+tankGeo1.snowheight)/2;
    log_trace("Station %d with no hits: depth: %f", dom_key.GetString(), avgdepth);


    AddNoPulse(dom_key.GetString(), (om.position).GetX(), (om.position).GetY(), (om.position).GetZ(), 
	       avgdepth);

  }
  
  return pulse_series_map->size();
  //return true;

}


bool TTopRecoShower::UpdateTrackOnly(I3ParticleConstPtr newtrack) { 

  // The newtrack also includes time, so no "adjusting" necessary
  SetPlaneParams(newtrack->GetTime(), newtrack->GetPos().GetX(), newtrack->GetPos().GetY());
  SetCore(newtrack->GetPos());
  SetTime(newtrack->GetTime());   // This one might be redundant.
  if(!(x_c*x_c>0)) { log_warn("Your x_c is zero"); return false; }

  SetDirection(newtrack->GetDir());
  return true;

}
#endif

// **************************************************************** //
void  TTopRecoShower::AddPulse(OMKey omKey, double x, double y, double z, double t,
			       double width, double vem, bool usepulsecharge, bool usepulsetime,
			       double snowdepth){

  showerPulse new_pulse;
  new_pulse.omkey=omKey;
  new_pulse.x=x;
  new_pulse.y=y;
  new_pulse.z=z;
  new_pulse.t=t;
  new_pulse.width=width;
  new_pulse.vem=vem;
  new_pulse.logvem=log10(vem);
  new_pulse.usepulsecharge=usepulsecharge;
  new_pulse.usepulsetime=usepulsetime;
  new_pulse.snowdepth = snowdepth;
  pulseData.push_back(new_pulse);

} 
 
// **************************************************************** //

void  TTopRecoShower::AddNoPulse(StationKey stationKey, double x, double y, double z, double avgdepth){

  noShowerPulse new_pulse;
  new_pulse.stationkey=stationKey;
  new_pulse.x=x;
  new_pulse.y=y;
  new_pulse.z=z;
  new_pulse.avgsnowdepth = avgdepth;
  noPulseData.push_back(new_pulse);

} 

// **************************************************************** //

unsigned int TTopRecoShower::SortOutBadCharges(){

  for (vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); it++)
    if(it->vem!=it->vem || it->vem<0) it->usepulsecharge=false;

  return GetNStationsUsedCharge();

}

// **************************************************************** //

unsigned int TTopRecoShower::SortOutBadTimes(){

  for (vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); it++)
    if(it->t!=it->t) it->usepulsetime=false;

  return GetNStationsUsedTime();

}

// **************************************************************** //

unsigned int TTopRecoShower::SortOutBadWidths(){

  for (vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); it++)
    if(it->width!=it->width) {it->usepulsecharge=false; it->usepulsetime=false;}

  return GetNStationsUsedCharge();

}

// **************************************************************** //

unsigned int  TTopRecoShower::GetNPulsesUsedCharge(){

  unsigned int n_used=0;
  for(vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); it++)
    if(it->usepulsecharge) n_used++;

  return n_used;

}

// **************************************************************** //

unsigned int  TTopRecoShower::GetNPulsesUsedTime(){

  unsigned int n_used=0;
  for(vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); it++)
    if(it->usepulsetime) n_used++;

  return n_used;

}

// **************************************************************** //

unsigned int  TTopRecoShower::GetNStationsUsedCharge(){

  set<int> stationmap;
  for(vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); it++)
    if(it->usepulsecharge) stationmap.insert(it->omkey.GetString());
  return stationmap.size();

}

// **************************************************************** //

unsigned int  TTopRecoShower::GetNStationsUsedTime(){

  set<int> stationmap;
  for(vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); it++)
    if(it->usepulsetime) stationmap.insert(it->omkey.GetString());
  return stationmap.size();

}

// **************************************************************** //

double TTopRecoShower::GetLogRadCOG(double (*sigma_func)(double r, double logq)){

  double sum=0;
  double weights=0;
  for (vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); it++){
    double radius=GetDistToAxisIt(it);
    // in principle, the radii should be weighted with 1/sigmasq, but it doesn't
    // lead to better results now, so I skip it
    //    double sigmach=sigma_func(radius, it->logvem);
    //    double sigmar=10*I3Units::m/radius*2.8/log(10);
    double sigmasq = 1.;//sigmach*sigmach + sigmar*sigmar;
    if(it->usepulsecharge) {
      sum+=log10(radius)/sigmasq;
      weights+=1./sigmasq;
    }
  }

  return pow(10., sum/weights);
}    

// **************************************************************** //

bool TTopRecoShower::CorrectForHeight(){

  if(height_corrected) {log_fatal("Height already corrected!"); return false;}
  if(!(dir.GetZenith()>=0.)) {log_error("No direction information!"); return false;}

  vector<showerPulse> new_data;
  showerPulse means = GetMeans();
  double zen = dir.GetZenith();
  for(vector<showerPulse>::iterator it = pulseData.begin(); it!=pulseData.end(); it++){

    showerPulse new_pulse = (*it);
    new_pulse.t+=(new_pulse.z-means.z)*cos(zen)/I3Constants::c;
    new_pulse.z=means.z;
    new_data.push_back(new_pulse);

  }
  
  pulseData = new_data;
  height_corrected = true; 
  return true;

}

// **************************************************************** //
// Shifting all VEM's by a constant amount
// This function is for TESTING things, NOT for reconstruction!
bool TTopRecoShower::ApplyArtificialVEMShift(double f) {
  log_debug("I am entering the ApplyArtificialVEMShift function");

  if(artificial_shift>0) {log_fatal("VEM's already artificially shifted!"); return false;}
  vector<showerPulse> new_data;

  for(vector<showerPulse>::iterator it = pulseData.begin(); it!=pulseData.end(); it++){
    showerPulse new_pulse = (*it);

    log_debug("Old VEM: %f", new_pulse.vem);
    new_pulse.vem *= f;
    log_debug("New VEM: %f", new_pulse.vem);

    // Also update the "logvem" variable
    new_pulse.logvem = log10(new_pulse.vem);

    new_data.push_back(new_pulse);
  }

  pulseData = new_data;
  artificial_shift = f;
  return true;

}

// **************************************************************** //


showerPulse TTopRecoShower::GetMeans(double power, short int nTanks) {

  showerPulse means;
  means.x=0;
  means.y=0;
  means.z=0;
  means.t=0;
  means.width=0;
  means.vem=0;
  means.logvem=0;
  double weightsum=0;

  // New Method : COG of nTanks highest pulses
  size_t useNTanks = pulseData.size();
  if (nTanks > 0) {
    sort(pulseData.begin(), pulseData.end(), ChargeOrdered);
    // Do calculation for lowest number of tanks, either NTanks present in pulseData, or nTanks requested.
    if(nTanks < (int)useNTanks)
      useNTanks = nTanks;
  }

  for (size_t i = 0; i < useNTanks ; ++i) {
    log_trace("OM %i in Station %i with charge %lf", pulseData[i].omkey.GetOM(),
	      pulseData[i].omkey.GetString(), pulseData[i].vem);
    showerPulse &pulse = pulseData[i];

    if (pulse.usepulsecharge && pulse.usepulsetime) {
      double weight = pow(pulse.vem, power);
      weightsum += weight;
      means.x += pulse.x*weight;
      means.y += pulse.y*weight;
      means.z += pulse.z*weight;
      means.t += pulse.t*weight;
      means.width += pulse.width*weight;
      means.vem += pulse.vem*weight;
      means.logvem += pulse.logvem*weight;
    }

  }

  means.x /= weightsum;
  means.y /= weightsum;
  means.z /= weightsum;
  means.t /= weightsum;
  means.width /= weightsum;
  means.vem /= weightsum;
  means.logvem /= weightsum;

  return means;

}

// **************************************************************** //

double TTopRecoShower::GetLogS125Start()
{
  const double beta = 3.0;  // assume average beta
  double sumLogS125 = 0;
  int nUsed = 0;
  for (vector<showerPulse>::iterator it = pulseData.begin(); it != pulseData.end(); ++it) {
    double logR = log10(GetDistToAxisIt(it)/LateralFitFunctions::R0_PARAM);
    if ((logR > log10(125./LateralFitFunctions::R0_PARAM)) &&
	(it->logvem == it->logvem)) {   // pulses too close to the core are unaccurate
      sumLogS125 += it->logvem + beta*logR + LateralFitFunctions::KAPPA*logR*logR;
      ++nUsed;
    }
  }
  if (nUsed < 5) return 0;

  if ((sumLogS125 != sumLogS125) || (pulseData.size() == 0)) {
    std::cout << sumLogS125 << std::endl;
    std::cout << pulseData.size() << "\n---------------------------------" << std::endl;
  }

  double meanLogS125 = sumLogS125/nUsed;
  if (meanLogS125 < -0.5) return 0;

  return meanLogS125;
}

// **************************************************************** //

void TTopRecoShower::Print(){
} 
