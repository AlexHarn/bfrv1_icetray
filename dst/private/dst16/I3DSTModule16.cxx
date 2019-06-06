#include <icetray/I3Tray.h>
#include <icetray/I3TrayInfo.h>
#include <icetray/I3TrayInfoService.h>
#include <icetray/Utility.h>
#include "recclasses/I3DST16.h"
#include "dst/dst16/I3DSTModule16.h"
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
#include "CommonVariables/hit_statistics/calculator.h"
#include "recclasses/I3DirectHitsValues.h"
#include <ostream>
#include <fstream>


using namespace std;

I3_MODULE(I3DSTModule16);


I3DSTModule16::I3DSTModule16(const I3Context& ctx) : 
  I3ConditionalModule(ctx),
  init_(false),
  dstName_("I3DST16"),
  dstHeaderName_("I3DS16THeader"),
  eventIndex_(0),
  dstHeaderPrescale_(1000),
  mjd_(MJD16),  
  startTime_(0),
  trigger_name_("I3TriggerHierarchy"),
  utriggerIDs_(),
  triggerIDs_(),
  eventheader_name_("I3EventHeader"),
  i3DirectHitsName_("PoleMuonLlhFitDirectHitsBaseC"),
  ignoreDirectHits_(false),
  icrecoseries_name_("TWCMuonPulseSeriesReco"),
  inIceRaw_("CleanInIceRawData"),
  fitParamsName_("PoleMuonLlhFitFitParams"),
  energyEstimateName_("PoleMuonLlhFitMuE"),
  centerX_(0.),
  centerY_(0.),
  centerZ_(0.),
  hpix_nside_(1024),
  pickKey_("")
{
  i3recoList_.push_back("PoleMuonLinefit");
  i3recoList_.push_back("PoleMuonLlhFit");

  directHitsTimeRange_.push_back(-15*I3Units::ns); 
  directHitsTimeRange_.push_back(75*I3Units::ns); 

  utriggerIDs_.push_back(DST16Utils::IN_ICE_SMT8);
  utriggerIDs_.push_back(DST16Utils::IN_ICE_SMT3);
  utriggerIDs_.push_back(DST16Utils::IN_ICE_STRING_CLUSTER);
  utriggerIDs_.push_back(DST16Utils::IN_ICE_SLOW_MP);
  utriggerIDs_.push_back(DST16Utils::ICE_TOP_SMT);

  AddParameter("DSTName", "Name of I3DST16 object in frame",dstName_);
  AddParameter("DSTHeaderName", "Name of I3DSTHeader16 object in frame",dstHeaderName_);
  AddParameter("RecoList","Array of reconstructed I3Particle names to lookfor in decreasing order of importance (max 8 entries)", i3recoList_);
  AddParameter("TriggerName","The Trigger Name", trigger_name_);
  AddParameter("TriggerIDList","List of Trigger Config IDs to encode", utriggerIDs_);
  AddParameter("EventHeaderName","The Event Header Name", eventheader_name_);
  AddParameter("DetectorCenterX","cartesian x-component of center of detector", centerX_);
  AddParameter("DetectorCenterY","cartesian y-component of center of detector", centerY_);
  AddParameter("DetectorCenterZ","cartesian z-component of center of detector", centerZ_);
  AddParameter("I3DirectHitsName","Name of I3DirectHitValues object in frame to use", i3DirectHitsName_);
  AddParameter("IgnoreDirectHits","Omit DirectHits info", ignoreDirectHits_);
  AddParameter("RecoSeriesName","Name of recoseries map used to compute cut values", icrecoseries_name_);
  AddParameter("LaunchMapSource","Name of DOMLaunchSeriesMaps ",inIceRaw_);
  // it should be calculated from the CleanInIceRaw data

  AddParameter("LLHFitParamsName","I3LogLikelihoodFitParams", fitParamsName_);
  AddParameter("EnergyEstimate","Name of I3Particle where energy estimate is stored", energyEstimateName_);
  AddParameter("DirectHitsTimeRange","",directHitsTimeRange_);
  AddParameter("DSTHeaderPrescale","Number of events between DSTHeader16 in frame",dstHeaderPrescale_);
  AddParameter("HealPixNSide","HealPix paramter for determining the skymap binsize",hpix_nside_);
  AddParameter("TriggerTimePrescaleServiceKey",
	       "Key for an IcePick in the context that this module should check "
	       "before adding Trigger times to the DST object.", pickKey_);
  AddOutBox("OutBox");
}

void I3DSTModule16::Configure()
{
  AddOutBox("OutBox");
  GetParameter("DSTName", dstName_);
  GetParameter("DSTHeaderName", dstHeaderName_);
  GetParameter("RecoList",i3recoList_);
  GetParameter("TriggerName", trigger_name_);
  GetParameter("TriggerIDList", utriggerIDs_);
  GetParameter("EventHeaderName", eventheader_name_);
  GetParameter("DetectorCenterX", centerX_);
  GetParameter("DetectorCenterY", centerY_);
  GetParameter("DetectorCenterZ", centerZ_);
  GetParameter("I3DirectHitsName", i3DirectHitsName_);
  GetParameter("IgnoreDirectHits",ignoreDirectHits_);
  GetParameter("RecoSeriesName",icrecoseries_name_);
  GetParameter("LaunchMapSource",inIceRaw_);
  GetParameter("LLHFitParamsName",fitParamsName_);
  GetParameter("EnergyEstimate", energyEstimateName_);
  GetParameter("DirectHitsTimeRange",directHitsTimeRange_);
  GetParameter("DSTHeaderPrescale",dstHeaderPrescale_);
  GetParameter("HealPixNSide",hpix_nside_);
  GetParameter("TriggerTimePrescaleServiceKey",pickKey_);

  if (ignoreDirectHits_) { 
        log_warn("I3DirectHitsValues '%s' will be ignored as per user request.",i3DirectHitsName_.c_str());
  } 

  //compute and fill coordinate sky map
  dstcoord_ = HealPixCoordinatePtr(new HealPixCoordinate());
  dstcoord_->ComputeBins(hpix_nside_);

  use_pick_ = false;
  dstHeaderWritten_ = false;
  if(pickKey_ != "") {
	use_pick_ = true;
    pick_ = GetContext().Get<I3IcePickPtr>(pickKey_);
  }

  // type conversion since there are no pybindings 
  for (vector<unsigned>::iterator it = utriggerIDs_.begin(); it != utriggerIDs_.end(); it++)
  {
       triggerIDs_.push_back(uint16_t(*it));
  }

  detectorCenter_ = I3PositionPtr(new I3Position(centerX_,centerY_,centerZ_));
  GetParameter("DirectHitsTimeRange",directHitsTimeRange_);
  if (directHitsTimeRange_.size() < 2) 
          log_fatal("you need two values for DirectHitsTimeRange");
}


void I3DSTModule16::DAQ(I3FramePtr frame)
{ 
     // get event header from frame
     I3EventHeaderConstPtr header = 
             frame->Get<I3EventHeaderConstPtr>(eventheader_name_); 
     if ( !header ) { 
             log_fatal("Could not find event header in frame \"%s\"!!!!", 
                          eventheader_name_.c_str()); 
     }

    // initialize dstheader 
    I3DSTHeader16Ptr dstheader(new I3DSTHeader16()); 

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


    // write dstheader to frame according to prescale
    if (!dstHeaderWritten_ 
         || !dstHeaderPrescale_ 
         || !(eventIndex_ % dstHeaderPrescale_ )
         || dstheader->GetModJulianDay() > mjd_ ) 
    { 
            frame->Put(dstHeaderName_,dstheader);
            mjd_ = dstheader->GetModJulianDay(); //update mjd
            dstHeaderWritten_ = true;
    }
    ++eventIndex_;
    PushFrame(frame,"OutBox");
}


void I3DSTModule16::Physics(I3FramePtr frame)
{ 
    // See if we should keep trigger times
    bool keepTriggerTimes = true;
    if(use_pick_) {
      keepTriggerTimes = pick_->SelectFrameInterface(*frame);
    }

    I3DST16Ptr dst = I3DST16Ptr(new I3DST16());

    // get event header from frame
    I3EventHeaderConstPtr header = frame->Get<I3EventHeaderConstPtr>(eventheader_name_);
    if ( !header ) {
        log_fatal("Could not find event header in frame \"%s\"!!!!",eventheader_name_.c_str());
    }

    // extract time
    I3Time starttime = header->GetStartTime(); 
    I3Time endtime   = header->GetEndTime(); 

    // in nsec 
    double newTime = 
             starttime.GetModJulianSec()*I3Units::second 
            +starttime.GetModJulianNanoSec()*I3Units::nanosecond;

    // in 100 musec = 10^-4 sec
    dst->SetTime(uint64_t(newTime/(10*I3Units::microsecond)));  
    dst->SetEventId(header->GetEventID()); 

    I3DOMLaunchSeriesMapConstPtr domLaunches =
            frame->Get<I3DOMLaunchSeriesMapConstPtr>(inIceRaw_);

    // Split triggers now have sub event IDs
    dst->SetSubEventId(header->GetSubEventID()); 

    // Navigate TriggerHierarchy and compute 16bit trigger flag ad the bitwise
    // OR of all the triggers
    vector<unsigned> triggertimes(triggerIDs_.size(),0);
    dst->SetTriggerTag(0);
    I3TriggerHierarchyConstPtr triggers = frame->Get<I3TriggerHierarchyConstPtr>(trigger_name_);
    I3TriggerHierarchy::iterator trigger_it;

    int triggerIndex;
    if (triggers) {
        for (
            trigger_it = triggers->begin();
            trigger_it != triggers->end();
            trigger_it++)
        { 
            //bitwise OR
            triggerIndex = DST16Utils::TriggerIndex(trigger_it->GetTriggerKey(), triggerIDs_);
            if (! (triggerIndex < 0) ) {
                dst->AddTriggerTag(1<<triggerIndex);

                log_debug("triggertag = %d - trigger offset = %d\n",
                    dst->GetTriggerTag(), DST16Utils::TriggerOffset(triggerIndex));

               if (!triggertimes[triggerIndex]) // only record the first trigger of each kind
                  triggertimes[triggerIndex] = unsigned(trigger_it->GetTriggerTime()/(100*I3Units::ns));
            }
        }
        if (keepTriggerTimes) {
           for (vector<unsigned>::iterator trigtime_it = triggertimes.begin(); 
                                       trigtime_it != triggertimes.end(); 
                                       trigtime_it++ ) 
           {
               if (*trigtime_it) {
                  uint8_t t_blocks    =  (*trigtime_it) / 128 ; 
                  uint8_t t_remainder =  (*trigtime_it) % 128 ;
                  if ( t_blocks ) {
                     dst->PushTriggerTime( (t_blocks<<1)|1 );
                  }
                  dst->PushTriggerTime(t_remainder<<1);
               }
           }
        } // keep triggerTimes


        if (!dst->GetTriggerTag())  {
                log_info("!!!!triggertag = %d :%s\n",dst->GetTriggerTag(), 
                                DSTUtils::itoa(dst->GetTriggerTag()).c_str());
        }
        log_debug("triggertag = %d :%s\n",dst->GetTriggerTag(), 
                                DSTUtils::itoa(dst->GetTriggerTag()).c_str());
    } else {
        log_error("unable to find I3TriggerHierarchy %s \n", trigger_name_.c_str());
    }

    // Get the first reconstruction fit coordinates
    I3ParticleConstPtr reco; 
    uint8_t reco_index = 0;
    uint8_t reco_label_index = 0;
    dst->SetRecoLabel(0);

    while ( ( reco_label_index < i3recoList_.size() ) && ( reco_index < 2 ) ) 
    {
            reco = frame->Get<I3ParticleConstPtr>(i3recoList_[reco_label_index]);
            if (reco && reco->GetFitStatus() == I3Particle::OK) { 
                    // extract reco info
                    double theta_reco = reco->GetDir().GetZenith(); 
                    double phi_reco   = reco->GetDir().GetAzimuth(); 

                    // compute map index for sky coordinate
                    if (reco_index == 0) { 
                        dst->SetReco1Direction(dstcoord_->GetIndex(theta_reco,phi_reco)); 
                    } else if (reco_index == 1) {
                        dst->SetReco2Direction(dstcoord_->GetIndex(theta_reco,phi_reco)); 
                    } 
                    // encode the reconstructions used for reco1 and reco2
                    // into a single byte by flipping the corresponding bits 
                    if ( reco->GetFitStatus() == I3Particle::OK ) {
                        dst->AddRecoLabel(1<<reco_label_index); 
                    }

                    log_debug("reco%d %s",reco_index,i3recoList_[reco_label_index].c_str()); 
                    log_debug("reco%d %f,%f",reco_index,reco->GetDir().GetZenith(),reco->GetDir().GetAzimuth());
                    reco_index++;
            } ++reco_label_index;
            if (reco && reco->GetFitStatus() != I3Particle::OK) { 
               log_debug("reco status is bad");
            } else if (!reco) {
               log_debug("reco not found");
            }
    }

    int Nstring   = 0;
    int Nchan     = 0;
    int Nhit      = 0;
    int Ndir      = 0;
    int Ldir      = 0;
    double rlogl  = 0;
    double logE   = 0.;

    I3RecoPulseSeriesMapConstPtr inicepulses = 
                    frame->Get<I3RecoPulseSeriesMapConstPtr>(icrecoseries_name_);

     // Get geometry and calculate COG for incicepulses
    const I3Geometry& geometry = frame->Get<I3Geometry>();
    I3PositionPtr cog =  common_variables::hit_statistics::CalculateCOG( geometry, *inicepulses);
    dst->SetCOG( 
               cog->GetX()/(10*I3Units::meter), 
               cog->GetY()/(10*I3Units::meter), 
               cog->GetZ()/(10*I3Units::meter)); 


     // Record fit params from LLH fit
    I3LogLikelihoodFitParamsConstPtr fitparams = 
            frame->Get<I3LogLikelihoodFitParamsConstPtr>(fitParamsName_);
    if (fitparams) { 
        rlogl = fitparams->rlogl_;
    } else {
        log_debug("no fitparams '%s' in the frame",fitParamsName_.c_str());
    }

     // Record energy estimate log10(MuE)
    I3ParticleConstPtr energyEstimate = 
            frame->Get<I3ParticleConstPtr>(energyEstimateName_);
    if ( energyEstimate && energyEstimate->GetEnergy() > 0.0 ) { 
        logE = log10(energyEstimate->GetEnergy());
    } else {
        log_debug("no energy estimate '%s' in the frame",energyEstimateName_.c_str());
    }

    // Nchan calculation is pretty simple
    Nchan = inicepulses->size();


    // Attempt to get DirectHitsValues produced by CommonVariables
    if (ignoreDirectHits_) { 
            log_trace("I3DirectHitsValues '%s' will be ignored.",i3DirectHitsName_.c_str());
    } else if (frame->Has(i3DirectHitsName_)) { 
            const I3DirectHitsValues cuts = frame->Get<I3DirectHitsValues>(i3DirectHitsName_);
            Nhit    = cuts.GetNDirPulses(); 
	    (void) Nhit;  // "use" Nhit, suppress compiler warning
            Nstring = cuts.GetNDirStrings(); 
            Ndir    = cuts.GetNDirPulses();
            Ldir    = int16_t(cuts.GetDirTrackLength()/(10*I3Units::meter)); 
    } else { 
            log_error("no I3DirectHitsValues '%s' found.",i3DirectHitsName_.c_str());
    } 
    dst->SetNString(Nstring); 
    dst->SetNDOM(Nchan); 
    dst->SetNDir(min(Ndir,255));
    dst->SetLDir(min(Ldir,255));
    dst->SetRlogL(rlogl);
    dst->SetLogE(logE);


    log_trace("ndir %d", dst->GetNDir());
    log_debug("index(%05d)(%05d) -- ndom(%04d) -- trigger(%s : %d)",
            dst->GetReco1(), dst->GetReco2(), dst->GetNDOM(), 
            DSTUtils::itoa(dst->GetTriggerTag()).c_str(),
            dst->GetTriggerTag());

    // write dst to frame
    frame->Put(dstName_,dst);

    PushFrame(frame,"OutBox");
}


bool I3DSTModule16::ShouldDoProcess(I3FramePtr frame)
{
  if (frame->GetStop() == I3Frame::DAQ )
    return true;

  return I3ConditionalModule::ShouldDoProcess(frame);
}


void I3DSTModule16::Finish()
{
}


