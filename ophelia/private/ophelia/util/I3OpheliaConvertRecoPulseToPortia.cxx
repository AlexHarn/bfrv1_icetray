/**
 * copyright  (C) 2008
 * the icecube collaboration
 * $Id$
 *
 * @file I3OpheliaConvertRecoPulseToPortia.cxx
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
#include "ophelia/util/I3OpheliaConvertRecoPulseToPortia.h"
#include "dataclasses/geometry/I3Geometry.h"

using namespace std;

I3_MODULE(I3OpheliaConvertRecoPulseToPortia);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3OpheliaConvertRecoPulseToPortia::I3OpheliaConvertRecoPulseToPortia(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  AddOutBox("OutBox");

  /* Convert I3RecoPulseSeriesMap to I3PortiaPulseMap */
  outputPortiaPulse_ = "BestPortiaPulseCleaned";
  AddParameter("OutputPortiaPulseName", 
	       "Output name of I3PortiaPulseMap", outputPortiaPulse_);

  inputRecoPulse_ = "";
  AddParameter("InputRecoPulseName",
	       "Input name of I3RecoPulseSeriesMap", inputRecoPulse_);
}
/* ******************************************************************** */
/* Destructor                                                           */
/* ******************************************************************** */
I3OpheliaConvertRecoPulseToPortia::~I3OpheliaConvertRecoPulseToPortia(){
}
/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3OpheliaConvertRecoPulseToPortia::Configure()
{
  GetParameter("OutputPortiaPulseName", outputPortiaPulse_);
  GetParameter("InputRecoPulseName",  inputRecoPulse_);

  log_info ("Output:  InputPortiaPulseName = %s",outputPortiaPulse_.c_str());
  log_info ("Input: OutputRecoPulseName = %s",inputRecoPulse_.c_str());

}
/* ******************************************************************** */
/* Physics                                                             */
/* ******************************************************************** */
//void I3OpheliaConvertRecoPulseToPortia::DAQ(I3FramePtr frame)
void I3OpheliaConvertRecoPulseToPortia::Physics(I3FramePtr frame)
{
  log_debug("Entering Physics...");

  //getting out basic information
  const I3Geometry& geo = frame->Get<I3Geometry>();

  //=====================================================================
  // Convert I3PortiaPulseMap to I3RecoPulseSeriesMap
  //=====================================================================
  if(outputPortiaPulse_ == ""){
    log_debug("Output name of I3PortiaPulseMap is not set.");
  }else if(!frame->Has(inputRecoPulse_)){
    log_info("Couldn't find input I3RecoPulseSeriesMap.");
  }else{

    log_debug("getting %s", inputRecoPulse_.c_str());
    // Get I3RecoPulseSeriesMap info from the frame.
    I3RecoPulseSeriesMapConstPtr recoMap =
      frame->Get<I3RecoPulseSeriesMapConstPtr>(inputRecoPulse_);
    //const I3RecoPulseSeriesMap& recoMap = frame->Get<I3RecoPulseSeriesMap>(inputRecoPulse_);
    log_debug("got %s", inputRecoPulse_.c_str());

    // Create I3PortiaPulseMap so we can fill it later and put it in frame.
    I3PortiaPulseMapPtr portiaPulseMapPtr(new I3PortiaPulseMap );
    log_debug("new portia pulse created");

    I3RecoPulseSeriesMap::const_iterator recoMap_iter = recoMap->begin();
    for(;recoMap_iter!=recoMap->end(); recoMap_iter++){

      const OMKey& omkey = recoMap_iter->first;
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
      const I3RecoPulseSeries& recoPulseSeries = recoMap_iter->second;

      I3PortiaPulse portiaPulse;
      portiaPulse.SetPositionX(om_pos.GetX());
      portiaPulse.SetPositionY(om_pos.GetY());
      portiaPulse.SetPositionZ(om_pos.GetZ());
      portiaPulse.SetLCBit(1);

      I3RecoPulseSeries::const_iterator recoSeries_iter = recoPulseSeries.begin(); 
      portiaPulse.SetTOT(recoSeries_iter->GetWidth());

      I3RecoPulse& recopulse = portiaPulse.GetRecoPulse();
      recopulse.SetTime(recoSeries_iter->GetTime());//in i3unit
      recopulse.SetWidth(recoSeries_iter->GetWidth());//in i3unit

      double npe = 0.0;
      for(;
	  recoSeries_iter!=recoPulseSeries.end(); recoSeries_iter++){
	npe += recoSeries_iter->GetCharge();
      }
      
      portiaPulse.SetEstimatedNPE(npe);
      recopulse.SetCharge(npe);//in i3unit 

      /* push back portia pulse to PortiaPulseMap */
      (*portiaPulseMapPtr)[omkey]=portiaPulse;

    }
    
    log_info("Input size is %d, out size is %d.", (int)recoMap->size(), (int)portiaPulseMapPtr->size());
    
    // Put it to the frame
    frame->Put(outputPortiaPulse_, portiaPulseMapPtr);

  } // end if input portia pulse exists

  PushFrame(frame,"OutBox");

  log_debug("Exiting DAQ.");
}
