/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: I3OpheliaRawDataEraser.cxx 4211 2005-11-10 12:23:15Z syoshida $

    @version $Revision: 1.2 $
    @date $Date: 2005-11-10 21:23:15 +0900
    @author Shigeru Yoshida
    @author Mio Ono <mio@hepburn.s.chiba-u.jp>

    I3OpheliaRawDataEraser

    This class erases rawATWD/rawFADC in I3DOMLaunch dataclass
    for compressing file size. The class is based on I3RawDataEraser 
    in brutus project which is originally developed for
    high energy MC data production where produced files are HUGE.
    
*/
//IceTray
#include "icetray/I3TrayHeaders.h"
// This class
#include "ophelia/util/I3OpheliaRawDataEraser.h"

//#include <iostream>
//#include <iomanip>
//#include <string>

using namespace std;

I3_MODULE(I3OpheliaRawDataEraser);

//===================================================================
//* constructor -----------------------------------------------------
I3OpheliaRawDataEraser::I3OpheliaRawDataEraser(const I3Context& ctx) : 
  I3ConditionalModule(ctx),
  inDataReadoutName_("InIceRawData"),
  atwdSaturationValue_(1022)
{
  
  AddParameter("DataReadoutName", "Input I3DOMLaunchSeriesMap name", inDataReadoutName_); 
  /* Thie value must be the same as what you set for I3DOMCalibrator */
  AddParameter("AtwdSaturationValue", "ATWD saturation value", atwdSaturationValue_); 
  AddOutBox("OutBox");

}

//===================================================================
//* destructor -----------------------------------------------------
I3OpheliaRawDataEraser::~I3OpheliaRawDataEraser()
{

}

//===================================================================
//* configure -----------------------------------------------------
void I3OpheliaRawDataEraser::Configure(){
  log_debug("Configuring the RawDataEraser");

  GetParameter("DataReadoutName", inDataReadoutName_); 
  GetParameter("AtwdSaturationValue", atwdSaturationValue_); 

  log_debug("DataReadoutName  is %s",  inDataReadoutName_.c_str()); 

}

//===================================================================
//* physics -----------------------------------------------------
void I3OpheliaRawDataEraser::Physics(I3FramePtr frame){

  log_info("Entering I3OpheliaRawDataEraser::Physics()");

  /**
   * Erase raw ATWD/FADC DOMLaunch data
   */
  I3DOMLaunchSeriesMapConstPtr 
    domLaunchPtr = frame->Get<I3DOMLaunchSeriesMapConstPtr>(inDataReadoutName_);
  if(domLaunchPtr){ // if exists

    I3DOMLaunchSeriesMap& domLaunchMap = 
      const_cast<I3DOMLaunchSeriesMap&>(*domLaunchPtr);

    I3DOMLaunchSeriesMap::iterator iter_domLaunchMap;
    log_debug("Going to loop through the I3DOMLaunchSeriesMap...");
    for(iter_domLaunchMap=domLaunchMap.begin(); 
	iter_domLaunchMap!=domLaunchMap.end(); iter_domLaunchMap++){
      
      I3DOMLaunchSeries& domLaunchSeries = iter_domLaunchMap->second;
      I3DOMLaunchSeries::iterator launchiter;
      for(launchiter = domLaunchSeries.begin();
	  launchiter!=domLaunchSeries.end();launchiter++){
	I3DOMLaunch& domLaunch = *launchiter;

	/* Erase rawATWD/FADC data */
	EraseRawDataInDOMLaunch(domLaunch);

      }// iteration over I3DOMLaunchSeries ends

    }// iteration over I3DOMLaunchSeriesMap ends
  }

  PushFrame(frame,"OutBox");

}//End Physics()
//===================================================================
//* EraseRawData ----------------------------------------------------
void I3OpheliaRawDataEraser::EraseRawDataInDOMLaunch(I3DOMLaunch& dom_launch){

  log_trace("Erasing DOMLaunch");

  /**
   * Find non saturated ATWD channel and fill those channel with dummy value,
   * in order to save which ATWD channel is used to make waveform 
   * by DOMcalibrator with calibration mode 1
   */
  unsigned int channel = FindNonSaturatedATWDChannel(dom_launch, atwdSaturationValue_);

  std::vector<int> &atwd0= dom_launch.GetRawATWD(0);
  atwd0.clear();
  //if(channel >= 0) // <- this will always be true (unsigned int)
  atwd0.push_back(dummyRawData_);

  std::vector<int> &atwd1= dom_launch.GetRawATWD(1);
  atwd1.clear();
  if(channel >= 1) atwd1.push_back(dummyRawData_);

  std::vector<int> &atwd2= dom_launch.GetRawATWD(2);
  atwd2.clear();
  if(channel >= 2) atwd2.push_back(dummyRawData_);

  std::vector<int> &atwd3= dom_launch.GetRawATWD(3);
  atwd3.clear();

  std::vector<int> &fadc= dom_launch.GetRawFADC();
  fadc.clear();

  log_trace("DOMLaunch size of ATWD channel 0, 1, 2, 3 and FADC is %d, %d, %d, %d and %d",
	    (int)atwd0.size(), (int)atwd1.size(), (int)atwd2.size(), 
	    (int)atwd3.size(), (int)fadc.size());
}
//===================================================================
//* FindNonSaturatedATWDChannel -------------------------------------
unsigned int
I3OpheliaRawDataEraser::FindNonSaturatedATWDChannel(I3DOMLaunch& domLaunch, int saturationVal)
{
  /**
   * Find non saturated ATWD channel.
   * This is a copy of I3DOMcalibrator::FindNonSaturatedATWDChannel.
   * Iterate through the ATWD channels and bins and find the highest gain channel
   * that is not saturated or if all else fails, the lowest gain channel.
   */

  log_trace("Finding non saturated ATWD channel");

  unsigned int retVal = 0;

  vector<int>::const_iterator iter;

  for ( unsigned int channel = 0; channel < 3; ++channel){
    retVal = channel;
    iter = find_if(domLaunch.GetRawATWD(channel).begin(),
		   domLaunch.GetRawATWD(channel).end(),
		   bind2nd(greater_equal<int>(), saturationVal));
    
    if ( ( ! domLaunch.GetRawATWD(channel).empty() )
	 && (iter == domLaunch.GetRawATWD(channel).end()) )
      break;
  }

  log_trace("Non saturated ATWD channel is %d", retVal);
  
  return retVal;
}
//===================================================================
