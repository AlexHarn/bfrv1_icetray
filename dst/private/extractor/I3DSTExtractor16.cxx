#include <icetray/I3Tray.h>
#include <icetray/I3TrayInfo.h>
#include <cmath>
#include <icetray/I3TrayInfoService.h>
#include <icetray/Utility.h>
#include "dst/extractor/I3DSTExtractor16.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3Trigger.h"
#include "dataclasses/TriggerKey.h"
#include "dataclasses/physics/I3TriggerHierarchy.h"
#include "dataclasses/I3Tree.h"
#include "dataclasses/I3Time.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/physics/I3TWRLaunch.h"
#include "phys-services/I3RandomService.h"
#include "phys-services/I3Calculator.h"
#include "phys-services/I3CutValues.h"
#include <ostream>
#include <fstream>

#include <astro/I3Astro.h>
#include <dataclasses/I3Time.h>

using namespace std;
namespace io = boost::iostreams;

I3_MODULE(I3DSTExtractor16);


I3DSTExtractor16::I3DSTExtractor16(const I3Context &ctx) :
    I3ConditionalModule(ctx), I3Splitter(configuration_),
    dstName_("I3DST16"),
    dstHeaderName_("I3DST16Header"),
    path_(""),
    weightMapName_(""),
    extractToFrame_(false),
    writeDSTFormat_(false),
    headerWritten_(false),
    ndom_name_("nDOM"),
    trigger_name_("I3InIceTrigger"),
    eventheader_name_("InIceEventHeader"),
    time_(0.0),
    offsetTime_(0),
    centerX_(0.),
    centerY_(0.),
    centerZ_(0.),
    zenithHi_(M_PI-0.02),
    zenithLo_(0.02),
    cut_data_(false)
{
    i3recoList_.push_back("NULL");
    i3recoList_.push_back("PoleMuonLinefit");
    i3recoList_.push_back("PoleMuonLlhfit");
    i3recoList_.push_back("ToI");

    keepTriggers_.push_back(1006);
    keepTriggers_.push_back(1007);
    keepTriggers_.push_back(1011);
    keepTriggers_.push_back(102);
    keepTriggers_.push_back(21001);
    keepTriggers_.push_back(23050);
    keepTriggers_.push_back(24002);

    AddOutBox("OutBox");
    AddParameter("SubEventStreamName", "The name of the SubEvent stream.",
		     configuration_.InstanceName());
    AddParameter("DSTName", "Name of I3DST16 object in frame", dstName_);
    AddParameter("DSTHeaderName", "Name of I3DSTHeader16 object in frame", dstHeaderName_);
    AddParameter("FileName", "Name of file to save DST to", path_);
    AddParameter("WeightMapName", "(optional) Name of weightmap for simulated events", weightMapName_);
    AddParameter("ExtractToFrame", "Should build I3 dataclasses in frame", extractToFrame_);
    AddParameter("EventHeaderName", "The Event Header Name (to add)", eventheader_name_);
    AddParameter("TriggerName", "The Name of the trigger H. (to add)", trigger_name_);
    AddParameter("DetectorCenterX", "cartesian x-component of center of detector", centerX_);
    AddParameter("DetectorCenterY", "cartesian y-component of center of detector", centerY_);
    AddParameter("DetectorCenterZ", "cartesian z-component of center of detector", centerZ_);
    AddParameter("ZenithHighCut", "Max zenith angle", zenithHi_);
    AddParameter("ZenithLowCut", "Min zenith angle", zenithLo_);
    AddParameter("KeepTriggers", "Keep events for which given triggers have fired", keepTriggers_);
    AddParameter("Cut", "Apply quality cuts", true);
    rand_ =  ctx.Get<I3RandomServicePtr>("I3RandomService");
}

void I3DSTExtractor16::Configure()
{
    GetParameter("SubEventStreamName", sub_event_stream_name_);
    GetParameter("DSTName", dstName_);
    GetParameter("DSTHeaderName", dstHeaderName_);
    GetParameter("FileName", path_);
    GetParameter("WeightMapName", weightMapName_);
    GetParameter("ExtractToFrame", extractToFrame_);
    GetParameter("EventHeaderName", eventheader_name_);
    GetParameter("TriggerName", trigger_name_);
    GetParameter("DetectorCenterX", centerX_);
    GetParameter("DetectorCenterY", centerY_);
    GetParameter("DetectorCenterZ", centerZ_);
    GetParameter("ZenithHighCut", zenithHi_);
    GetParameter("ZenithLowCut", zenithLo_);
    GetParameter("KeepTriggers", keepTriggers_);
    GetParameter("Cut", cut_data_);

    eventCounter_ = 0;

    //compute and fill coordinate sky map
    dstcoord_ = HealPixCoordinate();
    tdst = TDSTPtr(new TDST());

    detectorCenter_ = I3PositionPtr(new I3Position(centerX_, centerY_, centerZ_));
    // Get the random service to smear the coordinates within bin
    dstcoord_.SetRNG(rand_);
}

void I3DSTExtractor16::DAQ(I3FramePtr frame)
{
    // If we have not written a DSTHeader we need to wait for the first
    // EventHeader in order to extract the necessary information needed to
    // fill the DSTHeader.

    tdst->isGoodLLH = false;
    if (!headerWritten_)
    {
         // Buffer all frames until we see an EventHeader
         log_info("header not writtten.. buffering");
         buffer_.push_front(frame);

         if (frame->Has(dstHeaderName_) && !headerWritten_)
         {
                I3DSTHeader16ConstPtr header = frame->Get<I3DSTHeader16ConstPtr>(dstHeaderName_);
                log_trace("found DST header");
                // Initialize Coordinates
                dstcoord_.ComputeBins( header->GetHealPixNSide() );

                // Check if we need to adjust mjd for first events
                I3DSTHeader16Ptr newheader = I3DSTHeader16Ptr(new I3DSTHeader16(*header));
                newheader->SetEventId(header->GetEventId() - buffer_.size() + 1); // subtract events that came before header

                // put the DSTHeader on the first frame
                if (!headerWritten_ && !buffer_.back()->Has(dstHeaderName_) )
                {
                    buffer_.back()->Put(dstHeaderName_, newheader);
                    headerWritten_ = true;
                }

                // offset event id by the number of preceding events to
                // event header
                while (buffer_.size())
                {
                    ProcessFrame(buffer_.back()); 
                    buffer_.pop_back();
                }

                headerWritten_ = true;
            }
    } 
    else 
    {
            log_trace("Found header. Processing frames");
            ProcessFrame(frame); 
    }
}

void I3DSTExtractor16::ProcessFrame(I3FramePtr frame)
{
    if (extractToFrame_ && triggerkeys_.empty())
    {
        I3DetectorStatusConstPtr detstatus = frame->Get<I3DetectorStatusConstPtr>();
        if (!detstatus)
        {
            log_fatal("Missing I3DetectorStatus.");
        }
        std::map<TriggerKey, I3TriggerStatus>::const_iterator trig_status_iter;
        for (trig_status_iter = detstatus->triggerStatus.begin();
                trig_status_iter != detstatus->triggerStatus.end();
                trig_status_iter++)
        {
            if (trig_status_iter->first.CheckConfigID() )
            {
                triggerkeys_[ trig_status_iter->first.GetConfigID() ] =  trig_status_iter->first;
            }
        }
    }

    if (frame->Has(dstHeaderName_))
    {
        dstheader_  = I3DSTHeader16Ptr(new I3DSTHeader16( frame->Get<I3DSTHeader16>(dstHeaderName_) ));
        if (dstheader_->GetRecos().size())
        {
            i3recoList_ = dstheader_->GetRecos();
            i3recoList_.insert(i3recoList_.begin(), "NULL"); // reco 0 corresponds to a failed status
        }
        eventCounter_ = 0;   // reset the event ID offset
    }


    I3TriggerHierarchyPtr mTriggers = I3TriggerHierarchyPtr(new I3TriggerHierarchy);

    int reco_counter = 0;
    I3Time eventTime;
    uint32_t sec = uint32_t(time_ / I3Units::second); // number of seconds
    double ns = (time_ - sec * I3Units::second) / I3Units::nanosecond; // addtional ns
    eventTime.SetModJulianTime(dstheader_->GetModJulianDay(), sec, ns);


    // convert to I3Units
    I3TimePtr starttime(new I3Time(eventTime));
    I3EventHeaderPtr i3header(new I3EventHeader());
    i3header->SetStartTime(*starttime);
    i3header->SetRunID(dstheader_->GetRunId());
    i3header->SetEventID(dstheader_->GetEventId() + eventCounter_);
    i3header->SetSubEventStream(sub_event_stream_name_);
    if (!frame->Has(eventheader_name_))
         frame->Put(eventheader_name_, i3header);

    PushFrame(frame, "OutBox");

    std::deque<I3FramePtr> pbuffer;
    
    // Now we iterate through the frame searching for I3DSTRecos
    for ( I3Frame::typename_iterator frame_iter = frame->typename_begin(); frame_iter != frame->typename_end(); frame_iter++)
    {
        if (frame_iter->second == "I3DST16")
        {
            log_debug("found dst frame %s", frame_iter->first.c_str());
            I3DST16ConstPtr dstptr = frame->Get<I3DST16ConstPtr>(frame_iter->first);
            I3DST16Ptr dst(new I3DST16(*dstptr));

            if (!startTimeSet_)
            {
               startTimeDST_ = dst->GetTime();
               startTimeSet_ = true;
            }

            // Create P-frame
            I3FramePtr pframe = GetNextSubEvent(frame);
            if (!pframe->Has(eventheader_name_))
               pframe->Put(eventheader_name_, i3header);


            I3MapStringBool::iterator firedit;
            std::vector<unsigned int>::iterator trigit;

            bool keep = false;
            I3MapStringBoolPtr triggerMap = SetTrigger(mTriggers, dst, pframe);
            for (trigit = keepTriggers_.begin(); trigit != keepTriggers_.end(); trigit++)
            { 
                    stringstream trigss;
                    trigss << "TriggID_" << *trigit;
                    string trigstr = trigss.str();

                    firedit = triggerMap->find(trigstr);
                    if (firedit != triggerMap->end())
                    {
                            keep = true;
                            break;
                    }
            }
            if (!keep) {
                    for (firedit = triggerMap->begin(); firedit != triggerMap->end(); firedit++)
                            log_info("skipping trigger: %s, %u", firedit->first.c_str(), firedit->second);
                    continue;
            }


            if (ProcessDST(pframe, dst,reco_counter++)) {
                pbuffer.push_front(pframe); 
		log_trace("processed reco %s", frame_iter->first.c_str()); 
	    } else {
		log_info("did not processed reco %s", frame_iter->first.c_str()); 
	    }


            pframe->Put("TDSTTriggers", triggerMap);
            log_trace("added triggers from %s", frame_iter->first.c_str());

           // Write remnant keys, if they exist and it was requested, to a stub P
            if (!frame->Has(trigger_name_) && extractToFrame_ )
                frame->Put(trigger_name_, mTriggers);
         }
    }

    eventCounter_++;
    while (pbuffer.size())
    {
       PushFrame(pbuffer.back(), "OutBox");
       pbuffer.pop_back();
    }

}

bool I3DSTExtractor16::ProcessDST(I3FramePtr frame, I3DST16Ptr dst, int reco_count)
{

    bool zenith_cut = false;
    char strbuff[200];
    // encode the index of the reconstruction used for reco1 and reco2
    vector<unsigned> reco_index;
    for (uint8_t index = 0; index < 8; index++)
    {
        if (dst->GetRecoLabel() & (1 << index))
        {
            reco_index.push_back(index + 1);
            log_debug("reco%d %s", index, i3recoList_[index].c_str());
        }
    }

    double newtime = dst->GetTime() * 10 * I3Units::microsecond;
    double dt = double(newtime - time_) / I3Units::microsecond;

    if (dt < 0 || time_ < 10 * I3Units::microsecond )
        dt = 0.;

    time_ = newtime;
    I3Time eventTime;
    uint32_t sec = uint32_t(newtime / I3Units::second); // number of seconds
    double ns = (newtime - sec * I3Units::second) / I3Units::nanosecond; // addtional ns
    eventTime.SetModJulianTime(dstheader_->GetModJulianDay(), sec, ns);
    log_debug("sec %u , ns = %g ", sec, ns);

    I3ParticlePtr reco1(new I3Particle(I3Particle::InfiniteTrack, I3Particle::unknown));
    I3ParticlePtr reco2(new I3Particle(I3Particle::InfiniteTrack, I3Particle::unknown));
    I3ParticlePtr ereco(new I3Particle(I3Particle::InfiniteTrack, I3Particle::unknown));

    //Determine zero bin
    unsigned zero_bin = dstcoord_.GetIndex(0,0); 
    zenith_cut = (dst->GetReco2() != zero_bin);
    log_debug("reco in zero bin bin %u. Skipping", dst->GetReco2());

    // Decode first reconstruction
    reco1->SetFitStatus(I3Particle::OK);
    floatpair coords1 = dstcoord_.GetCoords(dst->GetReco1());
    reco1->SetDir(coords1.first * I3Units::radian, coords1.second * I3Units::radian);
    reco1->SetPos(  dst->GetCOG().GetX() * 10.0 * I3Units::meter,
                        dst->GetCOG().GetY() * 10.0 * I3Units::meter,
                        dst->GetCOG().GetZ() * 10.0 * I3Units::meter);
    float distance1 = I3Calculator::ClosestApproachDistance(*reco1, *detectorCenter_);
    log_debug("distance reco 1 %g", distance1);

    // Decode second reconstruction
    floatpair coords2 = dstcoord_.GetCoords(dst->GetReco2());
    reco2->SetDir(coords2.first * I3Units::radian, coords2.second * I3Units::radian);
    reco2->SetPos(  dst->GetCOG().GetX() * 10.0 * I3Units::meter,
                        dst->GetCOG().GetY() * 10.0 * I3Units::meter,
                        dst->GetCOG().GetZ() * 10.0 * I3Units::meter);
    float distance2 = I3Calculator::ClosestApproachDistance(*reco2, *detectorCenter_);
    log_debug("distance reco 2 %g", distance2);

    // Decode energy reconstruction
    ereco->SetDir(coords2.first * I3Units::radian, coords2.second * I3Units::radian);
    ereco->SetPos(  dst->GetCOG().GetX() * 10.0 * I3Units::meter,
                        dst->GetCOG().GetY() * 10.0 * I3Units::meter,
                        dst->GetCOG().GetZ() * 10.0 * I3Units::meter);

    if (reco_index.size() > 0)
    {
            if (reco_index[0] == 1)
                reco1->SetFitStatus(I3Particle::OK);
            sprintf(strbuff, "DST_%s", i3recoList_[reco_index[0]].c_str());

            if (extractToFrame_ ) {
                frame->Put(string(strbuff), reco1);
            }

    }
    if (reco_index.size() > 1)
    {
            if (reco_index[1] == 2) {
                tdst->isGoodLLH = true;
                reco2->SetFitStatus(I3Particle::OK);
            }
            sprintf(strbuff, "DST_%s", i3recoList_[reco_index[1]].c_str());

            if (extractToFrame_ ) {
                frame->Put(string(strbuff), reco2);
            }

    }
    if (extractToFrame_ ) {
           frame->Put("DST_RLogL", I3DoublePtr(new I3Double(dst->GetRlogL())));
    }

    I3CutValuesPtr cuts(new I3CutValues());
    cuts->cog = I3Position(
                          dst->GetCOG().GetX()*10.0*I3Units::meter,
                          dst->GetCOG().GetY()*10.0*I3Units::meter,
                          dst->GetCOG().GetZ()*10.0*I3Units::meter
                         );
    cuts->Nstring = dst->GetNString(); 
    cuts->Nchan   = dst->GetNDOM(); 
    cuts->Ndir    = dst->GetNDir(); 
    cuts->Ldir    = dst->GetLDir()*10*I3Units::meter; 

    if (extractToFrame_ ) {
           frame->Put("DST_Cuts", cuts);
    }


    double mjd = dstheader_->GetModJulianDay() * I3Units::day;
    double mjdTime = mjd + newtime;
    double secsInDay = (mjdTime - mjd) / I3Units::second;
    double nsInDay = (secsInDay - floor(secsInDay)) / I3Units::nanosecond;


    tdst->localMST = I3GetGMST(eventTime);
    tdst->mjdTime  = mjdTime/I3Units::day;

    tdst->lfAzimuth = reco1->GetDir().GetAzimuth() / I3Units::degree;
    tdst->lfZenith = reco1->GetDir().GetZenith() / I3Units::degree;

    tdst->llhAzimuth = reco2->GetDir().GetAzimuth() / I3Units::degree;
    tdst->llhZenith = reco2->GetDir().GetZenith() / I3Units::degree;

    tdst->linllhOpeningAngle = I3Calculator::Angle(*reco1, *reco2) / I3Units::degree;

    // Coordinate transformations
    I3Equatorial eq = I3GetEquatorialFromDirection(reco2->GetDir(), eventTime);
    tdst->RA = fmod(eq.ra / I3Units::degree,360);
    if (tdst->RA < 0) 
	    tdst->RA += 360;
    tdst->Dec = eq.dec / I3Units::degree;

    I3Equatorial moonEq = I3GetEquatorialFromDirection(I3GetMoonDirection(eventTime), eventTime);
    tdst->RAMoon = fmod(moonEq.ra / I3Units::degree,360);
    if (tdst->RAMoon < 0) 
	    tdst->RAMoon += 360;
    tdst->DecMoon = moonEq.dec / I3Units::degree;

    I3Equatorial sunEq = I3GetEquatorialFromDirection(I3GetSunDirection(eventTime), eventTime);
    tdst->RASun = fmod(sunEq.ra / I3Units::degree,360);
    if (tdst->RASun < 0) 
	    tdst->RASun += 360;
    tdst->DecSun = sunEq.dec / I3Units::degree;

    // Antisid frame
    double lst = I3GetGMST(eventTime);
    tdst->localAntiS = I3GetGMAST(eventTime);
    tdst->RAAntiS = fmod( (eq.ra - (lst + tdst->localAntiS)*I3Constants::pi/12)/ I3Units::degree,360);
    if (tdst->RAAntiS < 0) 
	    tdst->RAAntiS += 360;
    tdst->DecAntiS = eq.dec / I3Units::degree;

    // Solar frame
    double tod = ( tdst->mjdTime - int(tdst->mjdTime) )* 24.;
    tdst->RASolar = fmod((eq.ra - (lst + tod)*I3Constants::pi/12.)/ I3Units::degree,360);
    if (tdst->RASolar < 0) 
	    tdst->RASolar += 360;
    tdst->DecSolar = eq.dec / I3Units::degree;

    // Antisid frame
    tdst->localExtS = I3GetGMEST(eventTime);
    tdst->RAExtS = fmod( (eq.ra - (lst + tdst->localExtS)*I3Constants::pi/12)/ I3Units::degree,360);
    if (tdst->RAExtS < 0) 
	    tdst->RAExtS += 360;


    tdst->nchan      = dst->GetNDOM();
    tdst->triggertag = dst->GetTriggerTag();
    tdst->runId      = dstheader_->GetRunId();
    tdst->eventId    = dstheader_->GetEventId() + eventCounter_;
    tdst->subEventId = dst->GetSubEventId();
    tdst->time       = newtime / I3Units::microsecond;
    tdst->ndir       = dst->GetNDir();
    tdst->ldir       = dst->GetLDir() * 10;
    tdst->rlogl      = dst->GetRlogL();
    tdst->logMuE     = dst->GetLogE();
    tdst->nstring    = dst->GetNString();

    if (dst->GetCOG().IsDefined()) {
          tdst->cogx = dst->GetCOG().GetX() * 10.0;
          tdst->cogy = dst->GetCOG().GetY() * 10.0;
          tdst->cogz = dst->GetCOG().GetZ() * 10.0;
    } else {
          tdst->cogx = NAN;
          tdst->cogy = NAN;
          tdst->cogz = NAN;
    }

    // Weights
    // if weightMapName is not configured, all events have weight 1.
    if (!weightMapName_.empty())
    {
            if (frame->Has(weightMapName_))
            {
                I3MapStringDoubleConstPtr weightMapPtr =
                    frame->Get<I3MapStringDoubleConstPtr>(weightMapName_);
                tdst->weight         = weightMapPtr->at("Weight");
                tdst->diplopiaweight = weightMapPtr->at("DiplopiaWeight");
                tdst->TimeScale      = weightMapPtr->at("TimeScale");
            }
            else
            {
                tdst->weight         = 1.0;
                tdst->diplopiaweight = 1.0;
                tdst->TimeScale      = 1.0;
            }
    }

    zenith_cut = zenith_cut && reco2->GetZenith() > zenithLo_ && reco2->GetZenith() < zenithHi_ ;
    log_debug("zenith %f < %f < %f", zenithLo_,reco2->GetZenith(), zenithHi_);
    tdst->isGoodLineFit = tdst->cut_nan(); 
    if (!tdst->cut_nan())
        log_info("Event contains NaNs. Does not pass nan cut");

    if (!tdst->isGoodLineFit) 
        log_info("Event Does not have a good line fit");
    if (cut_data_ && !zenith_cut) 
        log_info("Event does not pass zenith cut.");

    tdst->isGoodLLH = tdst->isGoodLLH && zenith_cut;

    // keep if is good or cut_data disabled
    if ( tdst->isGoodLLH || !cut_data_)
          frame->Put("CutDST", tdst); 
    return ( tdst->isGoodLLH || !cut_data_);
}



void I3DSTExtractor16::Finish()
{
    // push all remaining frames
    if (!headerWritten_)
    {
        log_error("No DSTheader was found in scanning file. Possibly a truncated file?");
        log_fatal("DSTheader '%s' missing. Aborting", dstHeaderName_.c_str() );
    }
    while (buffer_.size())
    {
        ProcessFrame(buffer_.back());
        buffer_.pop_back();
    }
    Flush();
}


I3MapStringBoolPtr I3DSTExtractor16::SetTrigger(
                I3TriggerHierarchyPtr &triggers, 
                I3DST16Ptr dst, 
                I3FramePtr frame)
{
    // Create a new trigger object to record the result, and set the variables
    I3TriggerHierarchy::iterator mTriter;
    I3Trigger Trigger;
    uint16_t triggertag = dst->GetTriggerTag();

    // set trigger key of top-level trigger
    Trigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::MERGED);
    Trigger.SetTriggerTime(NAN);
    Trigger.SetTriggerLength(NAN);
    Trigger.SetTriggerFired(true);

    mTriter = triggers->insert(triggers->begin(), Trigger);

    unsigned triggerindex = 0;

    vector<uint16_t> triggerIDs = dstheader_->GetTriggers();

    I3MapStringBoolPtr triggerMap(new I3MapStringBool());
    for (unsigned i = 0; i < triggerIDs.size(); i++)
    {

        trigg_c[triggerIDs[i]] = 0;
        if (triggertag & (1 << i))
        {
            trigg_c[triggerIDs[i]] = 1;
            I3Trigger trigger;
            // get trigger key from Detector Status
            trigger.GetTriggerKey() = triggerkeys_[triggerIDs[i]];
            trigger.SetTriggerFired(true);

            if (dst->HasTriggerTimes())
            {
                uint8_t t_blocks    = 0;
                uint8_t t_remainder = 0;
                uint8_t triggertime = dst->GetTriggerTime(triggerindex++);

                if (triggertime & 1)   // this is an exponent
                {
                    t_blocks  = ((triggertime & 254) >> 1);
                    triggertime = dst->GetTriggerTime(triggerindex++); // get mantisa
                }
                t_remainder  = ((triggertime & 254) >> 1);

                trigger.SetTriggerTime( (128 * unsigned(t_blocks) + unsigned(t_remainder)) * 100 * I3Units::ns);
            }
            else
            {
                trigger.SetTriggerTime( NAN );
            }
            trigger.SetTriggerLength(NAN);
            triggers->append_child(mTriter, trigger);
        }
        char strbuff0[20];
        sprintf(strbuff0, "TriggID_%d", triggerIDs[i]);
        (*triggerMap)[strbuff0] = trigg_c[triggerIDs[i]];
    }
    return triggerMap;
}

