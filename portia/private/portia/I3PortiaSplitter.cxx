//IceTray
#include "icetray/I3Tray.h"
#include "icetray/I3Frame.h"
#include "icetray/I3TrayHeaders.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/I3Double.h"
//basic frame info
#include "icetray/OMKey.h"
#include "icetray/I3Units.h"

// My class
#include "portia/I3PortiaSplitter.h"

#include <numeric>
using namespace std;

/**
 * The Basetime window parameters
 */
double I3PortiaSplitter::kTimeMargin_ = 100.0*I3Units::ns; // allows 100 nsec margin to blacket [start-time end-time]
I3_MODULE(I3PortiaSplitter);

////constructor and destructor////////////////////////////////////////////
I3PortiaSplitter::I3PortiaSplitter(const I3Context& ctx) : 
  I3ConditionalModule(ctx),
  outDOMMapName_("SplittedDOMMap"), // splitted DOM map
  inDataReadoutName_("InIceRawData"), // I3DOMLaunchSeriesMap
  inEventHeaderName_("I3EventHeader"),// I3EventHeader
  subEventStreamName_("InIceSplit"), // split name in p-frame
  frameTimeWindowKeyName_("I3TimeWindow"),
  splitLaunchTime_(false)  // if false, select all launches
{
  // initialization
  frameStartTime_ = 0.0;
  frameEndTime_ = 0.0;

  // icetray parameters
  AddParameter("DataReadoutName",    "input DOM launch name",          inDataReadoutName_);
  AddParameter("EventHeaderName",    "input I3EventHeader name",       inEventHeaderName_);
  AddParameter("SplitDOMMapName",    "out DOM Map name",               outDOMMapName_);
  AddParameter("pframeName",         "input p-frame name",             subEventStreamName_);
  AddParameter("SplitLaunchTime",    "bool to split launch or not",    splitLaunchTime_);
  AddParameter("TimeWindowName",     "input time window name",         frameTimeWindowKeyName_);
}

/////////////////////////////////////////////////////////////////////////
I3PortiaSplitter::~I3PortiaSplitter(){
}
/////////////////////////////////////////////////////////////////////////
void I3PortiaSplitter::Configure(){

  log_trace("Configuring the Portia splitter");
  GetParameter("DataReadoutName",inDataReadoutName_);
  GetParameter("EventHeaderName", inEventHeaderName_);
  GetParameter("SplitDOMMapName", outDOMMapName_);
  GetParameter("pframeName",      subEventStreamName_);
  GetParameter("SplitLaunchTime", splitLaunchTime_);

  log_info("==============================");
  log_info("DataReadoutName  is %s",  inDataReadoutName_.c_str());
  log_info("Split DOM map name is %s",  outDOMMapName_.c_str());
  log_info("==============================");

  if(splitLaunchTime_){
    log_info("DOM launch times are splitted and bundled that fits in the p-frame time window");
    GetParameter("TimeWindowName", frameTimeWindowKeyName_);
  }else{
    log_info("evenry DOM launch times are bundled regardless of the p-frame time window");
  }
}
////////////////////////////////////////////////////////////////////////
void I3PortiaSplitter::Physics(I3FramePtr frame){
  log_trace("=== Entering I3PortiaSplitter::Physics()");

  I3DOMLaunchSeriesMapConstPtr LaunchMapPtr;
  I3EventHeaderConstPtr eventHeaderPtr;
  I3TimeWindowConstPtr twPtr;

  //check if keyname for DOM launch is set correctly
  if(!frame->Has(inDataReadoutName_)){
    log_error("No launch map %s is found in p-frame!",inDataReadoutName_.c_str());
    PushFrame(frame,"OutBox");
    return;
  }else{
    LaunchMapPtr = frame->Get<I3DOMLaunchSeriesMapConstPtr>(inDataReadoutName_);
  }

  if(splitLaunchTime_){ // Split launch time that fits in the p-frame time window
    // Then it must rely on [start-time, end-time] info in p-frame.
    // Otherwise this class never works.
    if(!frame->Has(inEventHeaderName_)){

      log_error("We miss I3EventHeader %s",inEventHeaderName_.c_str());
      PushFrame(frame,"OutBox");
      return;
    }else{
      eventHeaderPtr = frame->Get<I3EventHeaderConstPtr>(inEventHeaderName_);
      string p_frame_name = eventHeaderPtr->GetSubEventStream();
      twPtr = frame->Get<I3TimeWindowConstPtr>(frameTimeWindowKeyName_);


      if(p_frame_name != subEventStreamName_){ // this p-frame is not in-ice split. pass it over.
	log_debug(" this p-frame %s is not in-ice split. Go to next p-frame",p_frame_name.c_str());
	PushFrame(frame,"OutBox");
	return;
      }
      if(twPtr){
	frameStartTime_ = twPtr->GetStart();
	frameEndTime_ = twPtr->GetStop();
	log_debug("this p-frame %s is in-ice split",p_frame_name.c_str());
	log_debug("[start end]=[%f %f] (usec)",
		  frameStartTime_/I3Units::microsecond,frameEndTime_/I3Units::microsecond);
      }else{
	log_error("We miss time window info %s", frameTimeWindowKeyName_.c_str());
	PushFrame(frame,"OutBox");
	return;
      }
    }

  }


  // OK now we enter into the main process
  I3MapKeyVectorDoublePtr splittedDOMMapPtr(new I3MapKeyVectorDouble());
  const I3Geometry& geo = frame->Get<I3Geometry>();
  if(MakeSplittedDOMMap(*LaunchMapPtr, *splittedDOMMapPtr,geo)){
    log_info("Splitted DOM map is created. Push it to frame");
  }else{
    log_info("Cannot properly generate pplitted DOM map. Push emty map to frame");
  }
  frame->Put(outDOMMapName_, splittedDOMMapPtr);
  PushFrame(frame,"OutBox");


}

/////////////////////////////////////////////////////////////////////////
bool I3PortiaSplitter::MakeSplittedDOMMap(const I3DOMLaunchSeriesMap& launchMap,
					  I3MapKeyVectorDouble&  splittedDOMMap,
					  const I3Geometry& geo
					  ){

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
    if(geo.omgeo.find(omkey)->second.omtype == I3OMGeo::IceTop){
      log_debug("=== in MakeSplitterMap(): IceTop string %d / OM %d : skip this", 
		omkey.GetString(), omkey.GetOM());
      continue;
      }

    // make LaunchTime series of this OM that fits to the p-frame time window
    vector<double> launchTimeSeries;
    if(MakeLaunchTimeSeries(launches,launchTimeSeries)){
      splittedDOMMap[omkey]=launchTimeSeries;
      numberOfDOMsInThisFrame++;
      log_trace("=== in MakeSplitterMap(): this string has %zu splitted launches",
		launchTimeSeries.size());
      //if(launchTimeSeries.size()>1)
      //log_info("=== in MakeSplitterMap(): this string has %zu splitted launches",
      //	launchTimeSeries.size());
    }else{
      //log_info("=== in MakeSplitterMap(): this string has null splitted launches",
      //		launchTimeSeries.size());
    }

  }// loop over OMs ends

  if(numberOfDOMsInThisFrame>=1) return true;
  else return false;
  

}
/////////////////////////////////////////////////////////////////////////
bool I3PortiaSplitter::MakeLaunchTimeSeries(const I3DOMLaunchSeries& launches, vector<double>& launchTimeSeries){

  log_trace("     === in MakeLaunchTimeSeries(): now loop over I3DOMLaunchSeries");

  I3DOMLaunchSeries::const_iterator launchSeries_iter = launches.begin();
  for(;launchSeries_iter != launches.end(); launchSeries_iter++){
    // We require HLC launches.
    if(!launchSeries_iter->GetLCBit() || !launchSeries_iter->GetRawFADC().size()) continue;
    double launchTime = launchSeries_iter->GetStartTime();

    if(!splitLaunchTime_){ // Pick up every launch no matter when
      launchTimeSeries.push_back(launchTime);
    }else { // Check if the launch time is within the time window
      double startTime = frameStartTime_ - kTimeMargin_;
      double endTime = frameEndTime_  + kTimeMargin_;
      if(startTime <= launchTime && launchTime <= endTime){ // it fits!
	launchTimeSeries.push_back(launchTime);
      }

    }
  }

  if(launchTimeSeries.size()>=1) return true;
  else return false;

}







 


