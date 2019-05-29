#include <icetray/I3Tray.h>
#include <icetray/I3TrayInfo.h>
#include <cmath>
#include <icetray/I3TrayInfoService.h>
#include <icetray/Utility.h>
#include "dst/extractor/I3DSTExtractor13.h"
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

I3_MODULE(I3DSTExtractor13);


I3DSTExtractor13::I3DSTExtractor13(const I3Context &ctx) :
    I3ConditionalModule(ctx), I3Splitter(configuration_),
    dstName_("I3DST13"),
    dstHeaderName_("I3DSTHeader13"),
    path_(""),
    weightMapName_(""),
    extractToFrame_(false),
    makePframe_(false),
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
    zenithHi_(M_PI),
    zenithLo_(0.000)
{
    i3recoList_.push_back("NULL");
    i3recoList_.push_back("PoleMuonLinefit");
    i3recoList_.push_back("PoleMuonLlhfit");
    i3recoList_.push_back("ToI");

    keepTriggers_.push_back(1006);

    AddOutBox("OutBox");
    AddParameter("SubEventStreamName", "The name of the SubEvent stream.",
		     configuration_.InstanceName());
    AddParameter("DSTName", "Name of I3DST13 object in frame", dstName_);
    AddParameter("DSTHeaderName", "Name of I3DSTHeader13 object in frame", dstHeaderName_);
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
    rand_ =  ctx.Get<I3RandomServicePtr>("I3RandomService");
}

void I3DSTExtractor13::Configure()
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

    eventCounter_ = 0;

    //compute and fill coordinate sky map
    dstcoord_ = HealPixCoordinate();
    tdst = TDSTPtr(new TDST());

    detectorCenter_ = I3PositionPtr(new I3Position(centerX_, centerY_, centerZ_));
    // Get the random service to smear the coordinates within bin
    dstcoord_.SetRNG(rand_);
}

void I3DSTExtractor13::DAQ(I3FramePtr frame)
{
    // If we have not written a DSTHeader we need to wait for the first
    // EventHeader in order to extract the necessary information needed to
    // fill the DSTHeader.
    if (frame->Has(dstName_))
    {
        if (!startTimeSet_)
        {
            startTimeDST_ = frame->Get<I3DST13ConstPtr>(dstName_)->GetTime();
            startTimeSet_ = true;
        }

        if (!headerWritten_)
        {
            // Buffer all frames until we see an EventHeader
            log_trace("header not writtten.. buffering");
            buffer_.push_front(frame);

            I3DSTHeader13ConstPtr header = frame->Get<I3DSTHeader13ConstPtr>(dstHeaderName_);
            if ( header )
            {
                // Initialize Coordinates
                dstcoord_.ComputeBins( header->GetHealPixNSide() );

                // Check if we need to adjust mjd for first events
                I3DSTHeader13Ptr newheader = I3DSTHeader13Ptr(new I3DSTHeader13(*header));
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
                    ProcessDST(buffer_.back());
                    buffer_.pop_back();
                }

                headerWritten_ = true;
            }
        }
        else
        {
            log_trace("Found header. Processing frames");
            ProcessDST(frame);
        }
    }
    else
    {

        log_info("No DST found.");
        PushFrame(frame, "OutBox");
    }
}

void I3DSTExtractor13::ProcessDST(I3FramePtr frame)
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
        dstheader_  = I3DSTHeader13Ptr(new I3DSTHeader13( frame->Get<I3DSTHeader13>(dstHeaderName_) ));
        if (dstheader_->GetRecos().size())
        {
            i3recoList_ = dstheader_->GetRecos();
            i3recoList_.insert(i3recoList_.begin(), "NULL"); // reco 0 corresponds to a failed status
        }
        eventCounter_ = 0;   // reset the event ID offset
    }

    I3DST13ConstPtr dstptr = frame->Get<I3DST13ConstPtr>(dstName_);
    I3DST13Ptr dst(new I3DST13(*dstptr));

    I3TriggerHierarchyPtr mTriggers = I3TriggerHierarchyPtr(new I3TriggerHierarchy);

    int reco_counter = 0;
    //PushFrame(frame); // Push QFrame
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
        if (frame_iter->second == "I3DSTReco13")
        {
            I3DSTReco13ConstPtr dstrecoptr = frame->Get<I3DSTReco13ConstPtr>(frame_iter->first);
            I3DSTReco13Ptr dstreco(new I3DSTReco13(*dstrecoptr));

            // Create P-frame
            I3FramePtr pframe = GetNextSubEvent(frame);
            if (!pframe->Has(eventheader_name_))
               pframe->Put(eventheader_name_, i3header);


            I3MapStringBool::iterator firedit;
            std::vector<unsigned int>::iterator trigit;
            bool keep = false;

            I3MapStringBoolPtr triggerMap = SetTrigger(mTriggers, dstreco, pframe);
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

            if (!ProcessDSTReco(pframe, dst, dstreco,-1))
               continue;

            log_trace("processed reco %s", frame_iter->first.c_str()); 

            pframe->Put("TDSTTriggers", triggerMap);
            log_trace("added triggers from %s", frame_iter->first.c_str());

           // Write remnant keys, if they exist and it was requested, to a stub P
            if (!frame->Has(trigger_name_) && extractToFrame_ )
                frame->Put(trigger_name_, mTriggers);

            pbuffer.push_front(pframe);
         }
    }

    eventCounter_++;
    //PushFrame(frame, "OutBox");
    while (pbuffer.size())
    {
       PushFrame(pbuffer.back(), "OutBox");
       pbuffer.pop_back();
    }

}

bool I3DSTExtractor13::ProcessDSTReco(I3FramePtr frame, I3DST13Ptr dst, I3DSTReco13Ptr dstreco, int reco_count)
{

    bool zenith_cut = false;
    char strbuff[200];
    // encode the index of the reconstruction used for reco1 and reco2
    vector<unsigned> reco_index;
    for (uint8_t index = 0; index < 8; index++)
    {
        if (dstreco->GetRecoLabel() & (1 << index))
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
    zenith_cut = (dstreco->GetReco2() != zero_bin);

    // Decode first reconstruction
    reco1->SetFitStatus(I3Particle::OK);

    floatpair coords1 = dstcoord_.GetCoords(dstreco->GetReco1());
    reco1->SetDir(coords1.first * I3Units::radian, coords1.second * I3Units::radian);
    reco1->SetPos(  dstreco->GetCOG().GetX() * 10.0 * I3Units::meter,
                        dstreco->GetCOG().GetY() * 10.0 * I3Units::meter,
                        dstreco->GetCOG().GetZ() * 10.0 * I3Units::meter);
    float distance1 = I3Calculator::ClosestApproachDistance(*reco1, *detectorCenter_);
    log_debug("distance reco 1 %g", distance1);

    // Decode second reconstruction
    floatpair coords2 = dstcoord_.GetCoords(dstreco->GetReco2());

    reco2->SetDir(coords2.first * I3Units::radian, coords2.second * I3Units::radian);
    reco2->SetPos(  dstreco->GetCOG().GetX() * 10.0 * I3Units::meter,
                        dstreco->GetCOG().GetY() * 10.0 * I3Units::meter,
                        dstreco->GetCOG().GetZ() * 10.0 * I3Units::meter);
    float distance2 = I3Calculator::ClosestApproachDistance(*reco2, *detectorCenter_);
    log_debug("distance reco 2 %g", distance2);

    // Decode energy reconstruction
    ereco->SetDir(coords2.first * I3Units::radian, coords2.second * I3Units::radian);
    ereco->SetPos(  dstreco->GetCOG().GetX() * 10.0 * I3Units::meter,
                        dstreco->GetCOG().GetY() * 10.0 * I3Units::meter,
                        dstreco->GetCOG().GetZ() * 10.0 * I3Units::meter);

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
            if (reco_index[1] == 2)
                reco2->SetFitStatus(I3Particle::OK);
            sprintf(strbuff, "DST_%s", i3recoList_[reco_index[1]].c_str());

            if (extractToFrame_ ) {
                frame->Put(string(strbuff), reco2);
            }

    }
    if (extractToFrame_ ) {
           frame->Put("DST_RLogL", I3DoublePtr(new I3Double(dstreco->GetRlogL())));
    }

    I3CutValuesPtr cuts(new I3CutValues());
    cuts->cog = I3Position(
                          dstreco->GetCOG().GetX()*10.0*I3Units::meter,
                          dstreco->GetCOG().GetY()*10.0*I3Units::meter,
                          dstreco->GetCOG().GetZ()*10.0*I3Units::meter
                         );
    cuts->Nstring = dstreco->GetNString(); 
    cuts->Nchan   = dstreco->GetNDOM(); 
    cuts->Ndir    = dstreco->GetNDir(); 
    cuts->Ldir    = dstreco->GetLDir()*10*I3Units::meter; 

    if (extractToFrame_ ) {
           frame->Put("DST_Cuts", cuts);
    }


    double mjd = dstheader_->GetModJulianDay() * I3Units::day;
    double mjdTime = mjd + newtime;
    double secsInDay = (mjdTime - mjd) / I3Units::second;
    double nsInDay = (secsInDay - floor(secsInDay)) / I3Units::nanosecond;


    // Place holders for when this is implemented in astro
    tdst->localMST = I3GetGMST(eventTime);
    tdst->mjdTime  = mjdTime/I3Units::day;
    tdst->llhAzimuth = reco2->GetDir().GetAzimuth() / I3Units::degree;
    tdst->llhZenith = reco2->GetDir().GetZenith() / I3Units::degree;
    tdst->linllhOpeningAngle = I3Calculator::Angle(*reco1, *reco2) / I3Units::degree;

    // Coordinate transformations
    I3Equatorial eq = I3GetEquatorialFromDirection(reco2->GetDir(), eventTime);
    tdst->RA = eq.ra / I3Units::degree;
    tdst->Dec = eq.dec / I3Units::degree;

    I3Equatorial moonEq = I3GetEquatorialFromDirection(I3GetMoonDirection(eventTime), eventTime);
    tdst->RAMoon = moonEq.ra / I3Units::degree;
    tdst->DecMoon = moonEq.dec / I3Units::degree;

    I3Equatorial sunEq = I3GetEquatorialFromDirection(I3GetSunDirection(eventTime), eventTime);
    tdst->RASun = sunEq.ra / I3Units::degree;
    tdst->DecSun = sunEq.dec / I3Units::degree;


    // Antisid frame
    double lst = I3GetGMST(eventTime);
    tdst->localAntiS = I3GetGMAST(eventTime);
    tdst->RAAntiS = (eq.ra - (lst + tdst->localAntiS)*I3Constants::pi/12)/ I3Units::degree;
    tdst->DecAntiS = eq.dec / I3Units::degree;

    // Solar frame
    double tod = tod = ( mjd - int(mjd)) * 24.;
    tdst->RASolar = (eq.ra - (lst + tod)*I3Constants::pi/12.)/ I3Units::degree;
    tdst->DecSolar = eq.dec / I3Units::degree;

    // Antisid frame
    tdst->localExtS = I3GetGMEST(eventTime);
    tdst->RAExtS = (eq.ra - (lst + tdst->localExtS)*I3Constants::pi/12)/ I3Units::degree;


    tdst->nchan      = dstreco->GetNDOM();
    tdst->triggertag = dstreco->GetTriggerTag();
    tdst->runId      = dstheader_->GetRunId();
    tdst->eventId    = dstheader_->GetEventId() + eventCounter_;
    tdst->subEventId = dstreco->GetSubEventId();
    tdst->time       = newtime / I3Units::microsecond;
    tdst->ndir       = dstreco->GetNDir();
    tdst->ldir       = dstreco->GetLDir() * 10;
    tdst->rlogl      = dstreco->GetRlogL();
    tdst->logMuE     = dstreco->GetLogE();
    tdst->nstring    = dstreco->GetNString();

    if (dstreco->GetCOG().IsDefined()) {
          tdst->cogx = dstreco->GetCOG().GetX() * 10.0;
          tdst->cogy = dstreco->GetCOG().GetY() * 10.0;
          tdst->cogz = dstreco->GetCOG().GetZ() * 10.0;
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
    tdst->isGoodLineFit = tdst->cut_nan() && zenith_cut;
    if (tdst->isGoodLineFit && zenith_cut)
       frame->Put("CutDST", tdst); 

    return tdst->isGoodLineFit && zenith_cut;
}



void I3DSTExtractor13::Finish()
{
    // push all remaining frames
    if (!headerWritten_)
    {
        log_error("No DSTheader was found in scanning file. Possibly a truncated file?");
        log_fatal("DSTheader missing. Aborting");
    }
    while (buffer_.size())
    {
        ProcessDST(buffer_.back());
        buffer_.pop_back();
    }
    Flush();
}


I3MapStringBoolPtr I3DSTExtractor13::SetTrigger(
                I3TriggerHierarchyPtr &triggers, 
                I3DSTReco13Ptr dst, 
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

