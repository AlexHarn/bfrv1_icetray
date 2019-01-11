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
#include "ophelia/module/I3EHESpotVelocity.h"
//portia
#include "portia/I3Portia.h"
#include "recclasses/I3PortiaEvent.h"
//IceTray
#include "icetray/I3Tray.h"
#include "icetray/I3Frame.h"
#include "icetray/I3TrayHeaders.h"
//basic frame info
#include "icetray/I3Units.h"
#include "icetray/OMKey.h"
#include "dataclasses/I3DOMFunctions.h"
#include "dataclasses/physics/I3Particle.h"

using namespace std;

I3_MODULE(I3EHESpotVelocity);

////////////////////////////////////////////////////////////
I3EHESpotVelocity::~I3EHESpotVelocity(){}
////////////////////////////////////////////////////////////
I3EHESpotVelocity::I3EHESpotVelocity(const I3Context& ctx) : I3ConditionalModule(ctx),
							 inputPortiaEventName_("PortiaEvent"),
 							 inputFadcPulseName_("FADCPortiaPulse"),
							 inFADCWaveformName_("CalibratedFADC"),
							 outputSpotTrackName_("OpheliaSpotTrack"),
							 LCOption_(0),
							 minNumberDomWithPulse_(3){ 
  log_trace("In I3EHESpotVelocity constrcutor");

  AddParameter("MinimumNumberPulseDom","",  minNumberDomWithPulse_);
  AddParameter("InputPortiaEventName",      "",  inputPortiaEventName_);
  AddParameter("InputFadcPulseName",      "",  inputFadcPulseName_);
  AddParameter("FADCWaveformName",         "input FADC waveform name",           inFADCWaveformName_);
  AddParameter("OutputSpotTrackName", "",  outputSpotTrackName_);
  AddParameter("LCOption",             "if non-zero use only LC pulse",   LCOption_);

  AddOutBox("OutBox");
}
////////////////////////////////////////////////////////////
void I3EHESpotVelocity::Configure(){  
  GetParameter("MinimumNumberPulseDom", minNumberDomWithPulse_);
  GetParameter("InputPortiaEventName",  inputPortiaEventName_);
  GetParameter("InputFadcPulseName",  inputFadcPulseName_);
  GetParameter("FADCWaveformName",    inFADCWaveformName_);
  GetParameter("OutputSpotTrackName", outputSpotTrackName_);

  GetParameter("LCOption",          LCOption_);

  log_trace("LC Option: %d",     LCOption_);

  log_trace("================================================");
  log_trace("Minimum number of DOM with pulse: %d", minNumberDomWithPulse_);
  log_trace("Output Result Name: %s", outputSpotTrackName_.c_str());
}
////////////////////////////////////////////////////////////
void I3EHESpotVelocity::Physics(I3FramePtr frame){


  if(!frame->Has(inFADCWaveformName_) ||
     !frame->Has(inputPortiaEventName_) ||
     !frame->Has(inputFadcPulseName_) ){
    log_error("Keyname of DOM launch or waveform is not correct. Nor portia pulse exists.");

    PushFrame(frame,"OutBox");
    return;
  }


  const I3PortiaPulseMap&     fadcPulseMap = frame->Get<I3PortiaPulseMap>(inputFadcPulseName_);
  const I3PortiaEvent&        portiaEvent = frame->Get<I3PortiaEvent>(inputPortiaEventName_);
  const I3WaveformSeriesMap& fadcWaveMap = frame->Get<I3WaveformSeriesMap>(inFADCWaveformName_);


  /**
     Extract the time of the largest charge capture  within the base time window
   */
  OMKey largestOMKey = portiaEvent.GetLargestNPEOMKey();
  I3PortiaPulseMap::const_iterator fadcLargestPulseMap_iter = fadcPulseMap.find(largestOMKey);
  const I3PortiaPulse& largestFadcPulse = fadcLargestPulseMap_iter->second;
  double largestTime = largestFadcPulse.GetRecoPulse().GetTime(); // time of 10% charge capture

  /**
     Extract the earliest time hit within the base time window
   */
  OMKey earliestOMKey = portiaEvent.GetFirstPulseOMKeybtw();
  I3PortiaPulseMap::const_iterator fadcFirstPulseMap_iter = fadcPulseMap.find(earliestOMKey);
  const I3PortiaPulse& firstFadcPulse = fadcFirstPulseMap_iter->second;
  double earliestTime = firstFadcPulse.GetRecoPulse().GetTime(); // time of 10% charge capture
  if(fadcFirstPulseMap_iter == fadcPulseMap.end()){
    log_debug(" the DOM with the earliest hit does not have fADC launch");
    earliestTime = largestTime + I3Portia::kStartTimeBtw_;
  }
 
  /**
     Extract the last time hit within the base time window
   */
  OMKey lastOMKey = portiaEvent.GetLastPulseOMKeybtw();
  I3PortiaPulseMap::const_iterator fadcLastPulseMap_iter = fadcPulseMap.find(lastOMKey);
  const I3PortiaPulse& lastFadcPulse = fadcLastPulseMap_iter->second;
  double lastTime = lastFadcPulse.GetTime80();  // time of 80% charge capture
  if(fadcLastPulseMap_iter == fadcPulseMap.end()){
    log_debug(" the DOM with the last hit does not have fADC launch");
    lastTime = largestTime + I3Portia::kEndTimeBtw_;
  }
 
  const I3OMGeoMap& omgeo = frame->Get<I3Geometry>().omgeo;
  I3OpheliaFirstGuessTrackPtr spotTrackPtr = 
    GetSpotTrack(fadcPulseMap, fadcWaveMap, omgeo, earliestTime, lastTime);


  log_trace("Extraction of the spot velocity Successful");
  frame->Put(outputSpotTrackName_, spotTrackPtr);

  PushFrame(frame,"OutBox");
}
///////////////////////////////////////////////////////////
I3OpheliaFirstGuessTrackPtr I3EHESpotVelocity::GetSpotTrack(const I3PortiaPulseMap&     fadcPulseMap, 
				     const I3WaveformSeriesMap&  fadcWaveMap,
				     const I3OMGeoMap& omgeo,
				     double earliestTime,  double lastTime){

  log_trace("enter into spot track build");


  /**
   * 1 loop over time from earliestTime to lastTime
   * 2 look for fadc waveform
   * 3 calculate the spot location at the given time
   */

  I3WaveformSeriesMap::const_iterator fadcWaveMap_iter = fadcWaveMap.begin();
  I3WaveformSeries::const_iterator  fadcSeries_iter = fadcWaveMap_iter->second.begin();
  const I3Waveform& fadcWaveform = *fadcSeries_iter;
  double timeDuration = lastTime - earliestTime;
  double fadcTimeWidth = fadcWaveform.GetBinWidth();
  vector<double> spotX;
  vector<double> spotY;
  vector<double> spotZ;

  log_debug(" start time (%f nsec) end time (%f nsec) duration (%f nsec ) samling (%f nsec)", 
	    earliestTime/I3Units::ns, lastTime/I3Units::ns, timeDuration/I3Units::ns, fadcTimeWidth/I3Units::ns);

  //
  // time loop
  //
  double time = earliestTime+1.0e-2*I3Units::ns;
  while(time < lastTime){

    double averageX = 0.0;     
    double averageY = 0.0;     
    double averageZ = 0.0;
    double amplitude = 0.0;
    int totalNDOM = 0; bool spotPositionCalculated = false;

    //
    // fadc waveform loop
    //
    for(fadcWaveMap_iter = fadcWaveMap.begin();fadcWaveMap_iter != fadcWaveMap.end();
	fadcWaveMap_iter++){


      const OMKey omkey = fadcWaveMap_iter->first;
      /* At first make sure if this is not IceTop DOM */
      if(omgeo.find(omkey)->second.omtype == I3OMGeo::IceTop){
	log_trace("IceTop string %d / OM %d : skip this", omkey.GetString(), omkey.GetOM());
	continue;
      }
      /* Second, make sure if this is not Deep Core DOM */
      if(omkey.GetString()>80){
	log_trace("Deep Core string %d / OM %d : skip this", omkey.GetString(), omkey.GetOM());
	continue;
      }

      // get baseline estimated by portia and extract pmt gain, and om position stored in PortiaPulse
      I3PortiaPulseMap::const_iterator fadcPulseMap_iter = fadcPulseMap.find(omkey);
      if(fadcPulseMap_iter == fadcPulseMap.end()){
	log_debug("NO calibrated fadc on string %d / OM %d", omkey.GetString(), omkey.GetOM());
	continue;
      }
      const I3PortiaPulse& fadcPulse = fadcPulseMap_iter->second;
      double baseVoltage = fadcPulse.GetBaseLine();
      double pmtGain = fadcPulse.GetPMTGain();
      double posX = fadcPulse.GetPositionX();
      double posY = fadcPulse.GetPositionY();
      double posZ = fadcPulse.GetPositionZ();
      bool lcBit = fadcPulse.GetLCBit();

      if(!lcBit&&LCOption_) continue;

      // Get Fadc waveform and extract the amplitude at the given time
      I3WaveformSeries::const_iterator  fadcSeries_iter = fadcWaveMap_iter->second.begin();
      const I3Waveform& fadcWaveform = *fadcSeries_iter;
      double waveformStartTime = fadcWaveform.GetStartTime();
      double binWidth = fadcWaveform.GetBinWidth();
      const vector<double>& waveform = fadcWaveform.GetWaveform();
      int binNumber = waveform.size();

      int binIndex = (int )((time-waveformStartTime)/binWidth);
      if(0<=binIndex && binIndex < binNumber){ // waveform exists
	double voltage = waveform.at(binIndex)-baseVoltage;
	if(voltage >=0.0){ // no undershoot
	  averageX += posX*voltage/pmtGain;
	  averageY += posY*voltage/pmtGain;
	  averageZ += posZ*voltage/pmtGain;
	  amplitude += voltage/pmtGain;
	  ++totalNDOM;    
	}
      }

    }//end of fadc waveform map loop

    if(amplitude> 0.0 && totalNDOM >= minNumberDomWithPulse_){
      averageX              /=amplitude;
      averageY              /=amplitude;
      averageZ              /=amplitude;
      spotPositionCalculated = true;
    }

    if(spotPositionCalculated){
      spotX.push_back(averageX);
      spotY.push_back(averageY);
      spotZ.push_back(averageZ);
    }else{
      spotX.push_back(-1.0);
      spotY.push_back(-1.0);
      spotX.push_back(-1.0);
    }

    time += fadcTimeWidth;

    log_trace(" completed spot location calculation - time(%f nsec) amplitude(%e) NDOM(%d) z(%f)",
	      time/I3Units::ns, amplitude,totalNDOM,averageZ);
  }// end of time loop



  //
  // Now calculate velocity of the spot
  //
  log_debug(" now calculates series of the spot velocity");
  vector<double>::iterator iter_x = spotX.begin();
  vector<double>::iterator iter_y = spotY.begin();
  vector<double>::iterator iter_z = spotZ.begin();
  double d_time = 2*fadcTimeWidth; // 2 x fadc sampling interval = 50 nsec
  double aveVx = 0.0;  double aveVy = 0.0; double aveVz = 0.0; 
  double maxVz = 0.0; double aveZpos = 0.0;double aveZneg = 0.0;
  double neff = 0.0; double nEffNeg = 0.0; double nEffPos = 0.0;

  log_debug(" bin size x=%d y=%d z=%d", (int )spotX.size(),(int)spotY.size(),(int)spotZ.size());
  while(iter_x<spotX.end() && iter_y<spotY.end() && iter_z<spotZ.end()){
    double x = *iter_x; double y = *iter_y; double z = *iter_z;

    iter_x++; iter_y++; iter_z++;
    iter_x++; iter_y++; iter_z++;

    if(x == -1.0 && y == -1.0 && z == -1.0) continue; // the spot location not well defined
    if(iter_x<spotX.end() && iter_y<spotY.end() && iter_z<spotZ.end()){
      double nextX = *iter_x; double nextY = *iter_y; double nextZ = *iter_z;

      if(nextX == -1.0 && nextY == -1.0 && nextZ == -1.0){// the spot location not well defined
	iter_x++; iter_y++; iter_z++;
	iter_x++; iter_y++; iter_z++;
	continue; 
      }

      double dx = nextX - x; double dy = nextY - y; double dz = nextZ - z;
      if(dx != 0.0 && dy != 0.0 && dz != 0.0){ // OK, the spot is moving
	double velocityX = dx/d_time;
	double velocityY = dy/d_time;
	double velocityZ = dz/d_time;

	aveVx += velocityX; aveVy += velocityY; aveVz += velocityZ; neff+=1.0;
	if(fabs(velocityZ)> fabs(maxVz)) maxVz = velocityZ;
	if(velocityZ >= 0.0){
	  aveZpos += dz/d_time;
	  nEffPos += 1.0;
	}else{
	  aveZneg += dz/d_time;
	  nEffNeg += 1.0;
	}

      }
    }
  }

  if(neff>0.0){
    aveVx /= neff;aveVy /= neff;aveVz /= neff;
  }
  if(nEffPos>0.0) aveZpos /= nEffPos;
  if(nEffNeg>0.0) aveZneg /= nEffNeg;
 
  log_debug("  vz(%f) maxVz(%f) aveVzPositive(%f) aveVzNegative(%f) NDOM(%f)", 
	    aveVz,maxVz,aveZpos,aveZneg,neff);

  I3OpheliaFirstGuessTrackPtr spotTrackPtr(new I3OpheliaFirstGuessTrack);
  I3Position v(aveVx, aveVy, aveVz);
  spotTrackPtr->SetVelocity(v);
  spotTrackPtr->SetFitQuality(nEffNeg/neff);

  /* Set I3Particle */
  I3Particle& i3particle = spotTrackPtr->GetI3Particle();
  i3particle.SetPos(aveZpos,aveZneg,0.0);
  i3particle.SetSpeed(maxVz);
  i3particle.SetLength(neff);


  if(neff > 0.0){ // velocity caluclation well done
    spotTrackPtr->SetFitSuccessful(true);
    i3particle.SetFitStatus(I3Particle::OK);
    i3particle.SetShape(I3Particle::InfiniteTrack);
  }else{ // cannot extract velocity
    spotTrackPtr->SetFitSuccessful(false);
    i3particle.SetFitStatus(I3Particle::GeneralFailure);
    i3particle.SetShape(I3Particle::Null);
  }

  return spotTrackPtr;


}

/////////////////////////////////////////////////////////////////////////////////////
