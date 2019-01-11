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
#include "ophelia/module/I3EHEStaticTWC.h"
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

I3_MODULE(I3EHEStaticTWC);

////////////////////////////////////////////////////////////
I3EHEStaticTWC::~I3EHEStaticTWC(){}
////////////////////////////////////////////////////////////
I3EHEStaticTWC::I3EHEStaticTWC(const I3Context& ctx) : I3ConditionalModule(ctx),
						       inputPulseName_("ATWDPortiaPulse"),
						       inputPortiaEventName_("PortiaEvent"),
						       outputPulseName_("ATWDPortiaPulseTWC"),
						       timeInterval_(10000.0*I3Units::ns),
						       timeWindowNegative_(-4400.0*I3Units::ns), 
						       timeWindowPositive_(6400.0*I3Units::ns){ 
  log_trace("In I3EHEStaticTWC constrcutor");
  AddParameter("InputPulseName",        "",  inputPulseName_);
  AddParameter("OutputPulseName",       "",  outputPulseName_);
  AddParameter("InputPortiaEventName",  "",  inputPortiaEventName_);
  AddParameter("TimeWidnowNegative",    "",  timeWindowNegative_);
  AddParameter("TimeWindowPositive",    "",  timeWindowPositive_);
  AddParameter("TimeInterval",    "",  timeInterval_);
  
  AddOutBox("OutBox");
}
////////////////////////////////////////////////////////////
void I3EHEStaticTWC::Configure(){  
  GetParameter("InputPulseName",        inputPulseName_);
  GetParameter("InputPortiaEventName",  inputPortiaEventName_);
  GetParameter("outputPulseName",       outputPulseName_);
  GetParameter("TimeWidnowNegative",    timeWindowNegative_);
  GetParameter("TimeWindowPositive",    timeWindowPositive_);
  GetParameter("TimeInterval",    timeInterval_);
}
////////////////////////////////////////////////////////////
//void I3EHEStaticTWC::DAQ(I3FramePtr frame){
void I3EHEStaticTWC::Physics(I3FramePtr frame){

  I3PortiaPulseMapConstPtr oldPulseMap_ptr = frame->Get<I3PortiaPulseMapConstPtr>(inputPulseName_);
  if(!oldPulseMap_ptr){
    log_warn("couldn't find the old pulse map %s", inputPulseName_.c_str());
    PushFrame(frame);
    return;
  }

  I3PortiaEventConstPtr    portiaEvent_ptr = frame->Get<I3PortiaEventConstPtr>(inputPortiaEventName_);
  if(!portiaEvent_ptr){
    log_warn("couldn't find the portia event %s", inputPortiaEventName_.c_str());
    PushFrame(frame);
    return;
  }

  OMKey largestOMKey = portiaEvent_ptr->GetLargestNPEOMKey();
  I3PortiaPulseMap::const_iterator PulseMap_iter = oldPulseMap_ptr->find(largestOMKey);
  if(PulseMap_iter == oldPulseMap_ptr->end()) {
    log_error("Couldn't find the largest pulse '%s', '%s' in the PortiaPulseMap!", 
	      inputPulseName_.c_str(), inputPortiaEventName_.c_str());
    PushFrame(frame);
    return;
  }
  
  const I3PortiaPulse& largestPulse =  PulseMap_iter->second;
  const double largestNPETime = largestPulse.GetRecoPulse().GetTime();
  
  if (!oldPulseMap_ptr) {
    log_error("Couldn't find pulses '%s' in the frame!", inputPulseName_.c_str());
    PushFrame(frame);
    return;
  }
  log_debug("size of initial portia pulse map %d", (int)oldPulseMap_ptr->size());
  
  //New data with cleaned hits
  I3PortiaPulseMapPtr newPulseMap_ptr(new I3PortiaPulseMap);
  
  
  for(PulseMap_iter = oldPulseMap_ptr->begin(); PulseMap_iter != oldPulseMap_ptr->end(); PulseMap_iter++){
    
    OMKey omkey = PulseMap_iter->first;

    const I3PortiaPulse& pulse = PulseMap_iter->second;
    const double thisTime10 = pulse.GetRecoPulse().GetTime(); 
    
    if(((thisTime10-largestNPETime)>=timeWindowNegative_) &&
       ((thisTime10-largestNPETime)<=timeWindowPositive_)){

    (*newPulseMap_ptr)[omkey]=pulse;

    }
  }
  log_debug("size of new portia pulse map %d", (int)newPulseMap_ptr->size());
  //Now we check the largest time interval in the series
  ////pulse is inserted into (time, omkey) map

  std::vector< double > orderedHits;
  orderedHits.reserve(200);   // Reserve some space
  //Fill all the first hits and ordered hits
  log_debug("extracting hits After Static Time Window Cleaning");  
  for(I3PortiaPulseMap::const_iterator PulseMap_iter = newPulseMap_ptr->begin(); PulseMap_iter != newPulseMap_ptr->end(); PulseMap_iter++){
    
    const I3PortiaPulse& pulse = PulseMap_iter->second;
    
    //We assume the pulses are in chronological order
    double temp_order = pulse.GetRecoPulse().GetTime();
    orderedHits.push_back( temp_order );
  }
  //sort
  std::sort(orderedHits.begin(),orderedHits.end());
  
  const double intervalThreshold = timeInterval_;
  double currentInterval = 0.0;
  std::vector< double >::iterator cIter = orderedHits.begin();
  double previousTime = *cIter;
  double thisTime = 0.0;
  for ( 	; cIter != orderedHits.end() ; ++cIter ){
    thisTime = *cIter;
    currentInterval = thisTime - previousTime;
    //std::cout << "currentInterval is " << currentInterval<<std::endl;
    //if current interval is above the threshold value, ask max npe time is before the after
    if(currentInterval > intervalThreshold){
      log_info( "ordered hit time=%f, previous time =%f", *cIter, previousTime);

      /*      
	      if( largestNPETime < thisTime){
	      
	      for(I3PortiaPulseMap::iterator  iter = newPulseMap_ptr -> begin(); iter !=
	      newPulseMap_ptr ->end(); )
	      {
	      //Is this hit within an acceptable time window of the earliest hit?
	      const I3PortiaPulse& pulse = iter->second;
	      
	      if(pulse.GetRecoPulse().GetTime() >= thisTime )
	      {
	      newPulseMap_ptr -> erase(iter++);
	      }
	      else iter++;
	      }
	      //this is done
	      break;
	      }
	      else*/ 
      if(largestNPETime > thisTime){
	
	//want to erase
	for(I3PortiaPulseMap::iterator  iter = newPulseMap_ptr -> begin(); iter !=
	      newPulseMap_ptr ->end(); )
	  {
	    //Is this hit within an acceptable time window of the earliest hit?
	    const I3PortiaPulse& pulse = iter->second;
	    if(pulse.GetRecoPulse().GetTime() < thisTime )
	      {
		newPulseMap_ptr -> erase(iter++);
	      }
	    else iter++;
	  }
	//then look for another interval
	previousTime = thisTime;
	continue;
      }
    }
    previousTime = thisTime;
  }
  ///////////////////////////////////////////////////////////////////////////////
  
  frame->Put(outputPulseName_, newPulseMap_ptr);
  PushFrame(frame,"OutBox");
}//End DAQ()
////////////////////////////////////////////////////////////
