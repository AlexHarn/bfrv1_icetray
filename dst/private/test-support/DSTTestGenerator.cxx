#include <I3Test.h>
#include "icetray/I3TrayHeaders.h"
#include "icetray/I3Tray.h"
#include "dataclasses/I3Time.h"
#include "dataclasses/physics/I3Particle.h"
#include <dataclasses/I3Constants.h>
#include <dataclasses/I3Position.h>
#include <icetray/I3Units.h>
#include <icetray/I3Frame.h>
#include <phys-services/I3RandomService.h>
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/physics/I3TWRLaunch.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/physics/I3TriggerHierarchy.h"
#include "gulliver/I3LogLikelihoodFitParams.h"
#include <boost/filesystem.hpp>
#include <recclasses/I3DST16.h>
#include <dst/test-support/DSTTestGenerator.h>

I3_MODULE(DSTTestGenerator);

DSTTestGenerator::DSTTestGenerator(const I3Context& context) : 
        I3ConditionalModule(context), I3Splitter(configuration_),
        dtheta0_(1.0*I3Units::deg),
        dphi0_(1.0*I3Units::deg),
        currentTime_(0),
        timesum_(0),
        starttime_(0),
        i3reco_1_("PoleMuonLinefit"),
        i3reco_2_("PoleMuonLlhFit"),
        i3reco_2params_("PoleMuonLlhFitFitParams"),
        inicerawdata_name_("CleanInIceRawData"),
        icrecoseries_name_("TWCMuonPulseSeriesReco"),
        trigger_name_("I3TriggerHierarchy"),
        centerX_(0.),
        centerY_(0.),
        centerZ_(0.), 
        daqtime_ (156384000000000000ULL),
        eventId_ (1),
        nEvents_(10),
        nsubframes_ (1) 
        { 
                AddParameter("SubEventStreamName", 
                                "The name of the SubEvent stream.", 
                                configuration_.InstanceName());
                AddParameter("ThetaBin","Size of zenith bin", dtheta0_);
                AddParameter("NEvents","number of QFrames to generate", nEvents_);
                AddParameter("NSubframes","number of subframes to generate per QFrame", nsubframes_);
                AddParameter("PhiBin","Size of zenith bin", dphi0_);
                AddParameter("I3Reco1","The First Reconstruction Info", i3reco_1_);
                AddParameter("I3Reco2","The Second Reconstruction Info", i3reco_2_);
                AddParameter("I3Reco2Params","The Second Reconstruction Reconstruction Parameters", i3reco_2params_);
                AddParameter("InIceRawDataName","The InIce DOMLaunch Series Map Name", inicerawdata_name_);
                AddParameter("RecoSeriesName","The Reco Hit Series Name", icrecoseries_name_);
                AddParameter("TriggerName","The Trigger Name", trigger_name_);
                AddOutBox("OutBox"); 
        } 

  void DSTTestGenerator::Configure()
  { 
          GetParameter("SubEventStreamName", sub_event_stream_name_);
          GetParameter("NSubframes", nsubframes_); 
          GetParameter("ThetaBin", dtheta0_); 
          GetParameter("PhiBin", dphi0_); 
          GetParameter("I3Reco1", i3reco_1_); 
          GetParameter("I3Reco2", i3reco_2_); 
          GetParameter("I3Reco2Params", i3reco_2params_); 
          GetParameter("RecoSeriesName", icrecoseries_name_); 
          GetParameter("TriggerName", trigger_name_); 

          detectorCenter_ = I3PositionPtr(new I3Position(centerX_,centerY_,centerZ_));
          rand_  = context_.Get<I3RandomServicePtr>("I3RandomService");
    }
    
    void DSTTestGenerator::DAQ(I3FramePtr frame) 
    { 
            // I3EventHeader
            I3TimePtr starttime(new I3Time(2009,daqtime_));
            I3EventHeaderPtr header(new I3EventHeader());
            //header->SetStartTime(starttime);
            frame->Put("I3EventHeader",header);

            // Trigger Info 
            uint16_t triggertag = DST_SIMPLE_MULTIPLICITY << DST_IN_ICE; 
            I3TriggerHierarchyPtr mTriggers = I3TriggerHierarchyPtr(new I3TriggerHierarchy); 
            SetTrigger(mTriggers, triggertag) ; 
            frame->Put(trigger_name_,mTriggers);

            PushFrame(frame,"OutBox"); 

            for (unsigned pcount = 0;pcount < nsubframes_; pcount++) 
            { 
                    I3FramePtr pframe = GetNextSubEvent(frame); 
                    // Recontstructions 
                    double theta_reco = rand_->Uniform(0.,0.5*I3Constants::pi); 
                    double phi_reco = rand_->Uniform(0.,2.0*I3Constants::pi);

                    double theta_pos = rand_->Uniform(0.,1.0*I3Constants::pi); 
                    double phi_pos = rand_->Uniform(0.,2.0*I3Constants::pi); 
                    double distance = uint8_t(rand_->Gaus(0.,250.)/(10*I3Units::meter));

                    I3ParticlePtr reco1(new I3Particle); 
                    reco1->SetDir(theta_reco*I3Units::rad,phi_reco*I3Units::rad); 
                    reco1->SetPos(theta_pos*I3Units::rad,phi_pos*I3Units::rad, 
                        distance*I3Units::meter*10.0, I3Position::sph); 
                    reco1->SetFitStatus(I3Particle::OK);
                    pframe->Put(i3reco_1_,reco1);

                    I3ParticlePtr reco2(new I3Particle);
                    reco2->SetDir(theta_reco*I3Units::rad,phi_reco*I3Units::rad);
                    reco2->SetPos(theta_pos*I3Units::rad,phi_pos*I3Units::rad, 
                        distance*I3Units::meter*10.0, I3Position::sph); 
                    reco2->SetFitStatus(I3Particle::OK);
                    pframe->Put(i3reco_2_,reco2); 

                    I3LogLikelihoodFitParamsPtr llhparams(new I3LogLikelihoodFitParams(1.0,120.0));
                    pframe->Put(i3reco_2params_,llhparams);

                    I3RecoHitSeriesMapPtr ichits(new I3RecoHitSeriesMap()); 
                    I3RecoHitSeriesPtr ichitseries(new I3RecoHitSeries()); 
                    int nhit = rand_->Poisson(1.5); 

                    //OMKey ok(21,21);
                    for (int hit=0;hit< nhit;hit++) 
                    { 
                            I3RecoHitPtr recohit = I3RecoHitPtr(new I3RecoHit()); 
                            //ichitseries.append(hit); 
                    }
                    pframe->Put(icrecoseries_name_,ichits);
                    PushFrame(pframe,"OutBox"); 
            }
            daqtime_ += uint64_t(rand_->Exp(1.6e6)); 
            eventId_++; 
            if (eventId_ > nEvents_)
                RequestSuspension();
    }


    void DSTTestGenerator::SetTrigger(I3TriggerHierarchyPtr& triggers, uint16_t triggertag) 
    { 
            // Create a new trigger object to record the result, and set the variables 
            I3TriggerHierarchy::iterator mTriter; 
            I3Trigger Trigger; 
            I3Trigger inTrigger; 
            I3Trigger topTrigger; 

            // set trigger key of top-level trigger 
            Trigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::MERGED); 
            mTriter = triggers->insert(triggers->begin(), Trigger);

            uint16_t mask;
            if ( DSTUtils::isInIce(triggertag) ) { 
                    mask = triggertag >> DST_IN_ICE; 
                    inTrigger.SetTriggerFired(true); 

                    if ( mask & DST_SIMPLE_MULTIPLICITY ) { 
                            inTrigger.GetTriggerKey() = 
                                    TriggerKey(TriggerKey::IN_ICE,TriggerKey::SIMPLE_MULTIPLICITY);
                            inTrigger.GetTriggerKey().SetConfigID(DST16Utils::IN_ICE_SMT8); 
                            triggers->append_child(mTriter, inTrigger); 
                    } if ( mask & DST_MIN_BIAS ) { 
                            inTrigger.GetTriggerKey() = 
                                    TriggerKey(TriggerKey::IN_ICE,TriggerKey::MIN_BIAS); 
                            inTrigger.GetTriggerKey().SetConfigID(DST16Utils::IN_ICE_MINBIAS); 
                            triggers->append_child(mTriter, inTrigger); 
                    } 
            } 
            if ( DSTUtils::isIceTop(triggertag) ) { 
                    mask = triggertag >> DST_ICE_TOP; 
                    topTrigger.SetTriggerFired(true); 

                    if ( mask & DST_SIMPLE_MULTIPLICITY) { 
                            topTrigger.GetTriggerKey() = 
                                    TriggerKey(TriggerKey::ICE_TOP,TriggerKey::SIMPLE_MULTIPLICITY); 
                            inTrigger.GetTriggerKey().SetConfigID(DST16Utils::ICE_TOP_SMT); 
                            triggers->append_child(mTriter, topTrigger); 
                    } 
            } 
    }
  

