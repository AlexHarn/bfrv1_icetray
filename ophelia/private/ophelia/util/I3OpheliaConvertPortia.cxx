/**
 * copyright  (C) 2008
 * the icecube collaboration
 * $Id$
 *
 * @file I3OpheliaConvertPortia.cxx
 * @version $Revision$
 * @date $Date$
 * @author mio
 *
 * This is an I3Module to convert dataclasses derived from portia project
 * to general dataclass.
 *    a) I3PortiaEvent to I3MapStringDouble
 *    b) I3PortiaPulseMap to I3RecoPulseSeriesMap
 */
#include "icetray/I3TrayHeaders.h"
#include "recclasses/I3PortiaPulse.h"
#include "recclasses/I3PortiaEvent.h"
#include "ophelia/util/I3OpheliaConvertPortia.h"

using namespace std;

I3_MODULE(I3OpheliaConvertPortia);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3OpheliaConvertPortia::I3OpheliaConvertPortia(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  AddOutBox("OutBox");

  /* Convert I3PortiaEvent to I3MapStringDouble */
  inputPortiaEvent_ = "PortiaEvent";
  AddParameter("InputPortiaEventName", 
	       "Input name of I3PortiaEvent", inputPortiaEvent_);

  outputPortiaEventMap_ = "";
  AddParameter("outputPortiaEventMapName", 
	       "Output name of I3MapStringDouble", outputPortiaEventMap_);

  /* Convert I3PortiaPulseMap to I3RecoPulseSeriesMap */
  inputPortiaPulse_ = "ATWDPortiaPulse";
  AddParameter("InputPortiaPulseName", 
	       "Input name of I3PortiaPulseMap", inputPortiaPulse_);

  outputRecoPulse_ = "";
  AddParameter("OutputRecoPulseName",
	       "Output name of I3RecoPulseSeriesMap", outputRecoPulse_);
}
/* ******************************************************************** */
/* Destructor                                                           */
/* ******************************************************************** */
I3OpheliaConvertPortia::~I3OpheliaConvertPortia(){
}
/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3OpheliaConvertPortia::Configure()
{
  GetParameter("InputPortiaEventName", inputPortiaEvent_);
  GetParameter("OutputPortiaEventMapName",  outputPortiaEventMap_);
  GetParameter("InputPortiaPulseName", inputPortiaPulse_);
  GetParameter("OutputRecoPulseName",  outputRecoPulse_);

  log_info ("Input:  InputPortiaEventName = %s",inputPortiaEvent_.c_str());
  log_info ("Output: OutputPortiaEventMapName = %s",outputPortiaEventMap_.c_str());
  log_info ("Input:  InputPortiaPulseName = %s",inputPortiaPulse_.c_str());
  log_info ("Output: OutputRecoPulseName = %s",outputRecoPulse_.c_str());

}
/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
//void I3OpheliaConvertPortia::DAQ(I3FramePtr frame)
void I3OpheliaConvertPortia::Physics(I3FramePtr frame)
{
  log_debug("Entering Physics...");

  //=====================================================================
  // Convert I3PortiaEvent to I3MapStringDouble
  //=====================================================================
  if(outputPortiaEventMap_ == ""){
    log_debug("Output name of I3MapStringDouble is not set.");
  }else if(!frame->Has(inputPortiaEvent_)){
    log_info("Couldn't find input I3PortiaEvent.");
  }else{
    const I3PortiaEvent& portiaEvent = frame->Get<I3PortiaEvent>(inputPortiaEvent_);
    //I3MapStringDouble portiaEventMap;
    I3MapStringDoublePtr portiaEventMap(new I3MapStringDouble);

    (*portiaEventMap)["PortiaBestNPE"]  = portiaEvent.GetTotalBestNPE();
    (*portiaEventMap)["PortiaAtwdNPE"]  = portiaEvent.GetTotalAtwdNPE();
    (*portiaEventMap)["PortiaFadcNPE"]  = portiaEvent.GetTotalFadcNPE();
    (*portiaEventMap)["PortiaTotalNch"] = portiaEvent.GetTotalNch();
    (*portiaEventMap)["PortiaAtwdNch"]  = portiaEvent.GetAtwdNch();
    (*portiaEventMap)["PortiaFadcNch"]  = portiaEvent.GetFadcNch();

    // Put it to the frame
    frame->Put(outputPortiaEventMap_, portiaEventMap);

  } // end if input portia event exists

  //=====================================================================
  // Convert I3PortiaPulseMap to I3RecoPulseSeriesMap
  //=====================================================================
  if(outputRecoPulse_ == ""){
    log_debug("Output name of I3RecoPulseSeriesMap is not set.");
  }else if(!frame->Has(inputPortiaPulse_)){
    log_info("Couldn't find input I3PortiaPulseMap.");
  }else{

    // Get I3PortiaPulseMap info from the frame.
    const I3PortiaPulseMap& portiaMap = frame->Get<I3PortiaPulseMap>(inputPortiaPulse_);
    
    // Create I3RecoPulseSeriesMap so we can fill it later and put it in frame.
    I3RecoPulseSeriesMapPtr recoPulseMapPtr(new I3RecoPulseSeriesMap );

    I3PortiaPulseMap::const_iterator portiaMap_iter = portiaMap.begin();
    for(;portiaMap_iter!=portiaMap.end(); portiaMap_iter++){

      const OMKey& omkey = portiaMap_iter->first;
      //const I3PortiaPulse& portiaPulse = portiaMap_iter->second;
      I3RecoPulseSeries recoPulseSeries;
      recoPulseSeries.push_back(portiaMap_iter->second.GetRecoPulse());
      recoPulseSeries[0].SetFlags(1);
      (*recoPulseMapPtr)[omkey]=recoPulseSeries;

    }
    
    log_trace("Input size is %d, out size is %d.", (int)portiaMap.size(), (int)recoPulseMapPtr->size());
    
    // Put it to the frame
    frame->Put(outputRecoPulse_, recoPulseMapPtr);

  } // end if input portia pulse exists

  PushFrame(frame,"OutBox");

  log_debug("Exiting Physics.");
}
