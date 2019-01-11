#include <icetray/I3Tray.h>
#include <icetray/I3TrayInfo.h>
#include <icetray/I3TrayInfoService.h>
#include <icetray/Utility.h>
#include "dst/dst13/I3DSTDAQModule13.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/physics/I3Trigger.h"
#include "dataclasses/TriggerKey.h"
#include "dataclasses/physics/I3TriggerHierarchy.h"
#include "dataclasses/I3Tree.h"
#include "dataclasses/I3Time.h"
#include "icetray/I3Bool.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "gulliver/I3LogLikelihoodFitParams.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "phys-services/I3Cuts.h"
#include "phys-services/I3CutValues.h"
#include <ostream>
#include <fstream>


using namespace std;

I3_MODULE(I3DSTDAQModule13);


I3DSTDAQModule13::I3DSTDAQModule13(const I3Context& ctx) : 
  I3Module(ctx),
  init_(false),
  dstName_("I3DST13"),
  dstHeaderName_("I3DSTHeader13"),
  eventIndex_(0),
  dstHeaderPrescale_(1000),
  mjd_(MJD13),  
  startTime_(0),
  utriggerIDs_(),
  triggerIDs_(),
  hpix_nside_(1024),
  eventheader_name_("I3EventHeader"),
  dstHeaderWritten_(false)
{
  i3recoList_.push_back("PoleMuonLinefit");
  i3recoList_.push_back("PoleMuonLlhFit");

  utriggerIDs_.push_back(DST13Utils::IN_ICE_SMT8);
  utriggerIDs_.push_back(DST13Utils::IN_ICE_SMT3);
  utriggerIDs_.push_back(DST13Utils::IN_ICE_STRING_CLUSTER);
  utriggerIDs_.push_back(DST13Utils::IN_ICE_SLOW_MP);
  utriggerIDs_.push_back(DST13Utils::ICE_TOP_SMT);

  AddParameter("DSTName", "Name of I3DST13 object in frame",dstName_);
  AddParameter("DSTHeaderName", "Name of I3DSTHeader13 object in frame",dstHeaderName_);
  AddParameter("RecoList","Array of reconstructed I3Particle names to lookfor"
                  "in decreasing order of importance (max 8 entries)", i3recoList_);
  AddParameter("TriggerIDList","List of Trigger Config IDs to encode", utriggerIDs_);
  AddParameter("HealPixNSide","HealPix paramter for determining the skymap binsize",hpix_nside_);
  AddParameter("EventHeaderName","The Event Header Name", eventheader_name_);
  //AddParameter("RecoSeriesName","Name of recoseries map used to compute cut values", icrecoseries_name_);
  // it should be calculated from the CleanInIceRaw data

  AddParameter("DSTHeaderPrescale","Number of events between DSTHeader13 in frame",dstHeaderPrescale_);
  AddOutBox("OutBox");
}

void I3DSTDAQModule13::Configure()
{
  AddOutBox("OutBox");
  GetParameter("DSTName", dstName_);
  GetParameter("DSTHeaderName", dstHeaderName_);
  GetParameter("RecoList",i3recoList_);
  GetParameter("TriggerIDList", utriggerIDs_);
  GetParameter("EventHeaderName", eventheader_name_);
  GetParameter("DSTHeaderPrescale",dstHeaderPrescale_);
  GetParameter("HealPixNSide",hpix_nside_);


  // type conversion since there are no pybindings 
  for (vector<unsigned>::iterator it = utriggerIDs_.begin(); it != utriggerIDs_.end(); it++)
  {
       triggerIDs_.push_back(uint16_t(*it));
  }

}



void I3DSTDAQModule13::DAQ(I3FramePtr frame)
{ 

    I3DST13Ptr dst = I3DST13Ptr(new I3DST13());

    // get event header from frame
    I3EventHeaderConstPtr header = frame->Get<I3EventHeaderConstPtr>(eventheader_name_);
    if ( !header ) {
        log_fatal("Could not find event header in frame \"%s\"!!!!",eventheader_name_.c_str());
    }
    // initialize dstheader 
    I3DSTHeader13Ptr dstheader(new I3DSTHeader13());
    dstheader->SetTriggers(triggerIDs_);
    dstheader->SetRecos(i3recoList_);


    // extract time
    I3Time starttime = header->GetStartTime(); 
    I3Time endtime   = header->GetEndTime(); 

    // add header to frame
    dstheader->SetModJulianDay(uint16_t(starttime.GetModJulianDay())); 
    dstheader->SetEventId(header->GetEventID()); 
    dstheader->SetRunId(header->GetRunID());
    dstheader->SetHealPixNSide(hpix_nside_);

    // in nsec 
    double newTime = 
             starttime.GetModJulianSec()*I3Units::second 
            +starttime.GetModJulianNanoSec()*I3Units::nanosecond;

    // in 100 musec = 10^-4 sec
    dst->SetTime(uint64_t(newTime/(10*I3Units::microsecond)));  

    // Split triggers now have sub event IDs
    dst->SetEventId(header->GetEventID()); 

    // write dst to frame
    frame->Put(dstName_,dst);

    // write dstheader to frame according to prescale
    if ( !dstHeaderWritten_ 
         || !dstHeaderPrescale_ 
         || !(eventIndex_ % dstHeaderPrescale_ )
         || dstheader->GetModJulianDay() > mjd_ ) 
    { 
         frame->Put(dstHeaderName_,dstheader);
         mjd_ = dstheader->GetModJulianDay(); //update mjd
         dstHeaderWritten_ = true;
    }
    PushFrame(frame,"OutBox");
    ++eventIndex_;
}


void I3DSTDAQModule13::Finish()
{
}


