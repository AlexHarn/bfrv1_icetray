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


TEST_GROUP(OutOfMemoryModuleTest);

namespace DSTModuleTest
{
  class DSTTestModule : public I3Module
  {
	  private:
		  double dtheta0_;
		  double dphi0_;

		  std::string i3reco_1_;
		  std::string i3reco_2_;
		  std::string i3reco_2params_;
		  std::string inicerawdata_name_;
		  std::string icetoprawdata_name_;
		  std::string icrecoseries_name_;
		  std::string trigger_name_;

		  double centerX_;
		  double centerY_;
		  double centerZ_;

		  I3PositionPtr detectorCenter_;
		  I3RandomServicePtr rand_ ; 
		  uint64_t daqtime_;
		  uint32_t eventId_;

	  public:

		  DSTTestModule(const I3Context& context) : I3Module(context), 
		  dtheta0_(1.0*I3Units::deg),
		  dphi0_(1.0*I3Units::deg), 
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
		  eventId_ (1)
	{
	    AddParameter("ThetaBin","Size of zenith bin", dtheta0_);
	    AddParameter("PhiBin","Size of zenith bin", dphi0_);
	    AddParameter("I3Reco1","The First Reconstruction Info", i3reco_1_);
	    AddParameter("I3Reco2","The Second Reconstruction Info", i3reco_2_);
	    AddParameter("I3Reco2Params","The Second Reconstruction Reconstruction Parameters", i3reco_2params_);
	    AddParameter("InIceRawDataName","The InIce DOMLaunch Series Map Name", inicerawdata_name_);
	    AddParameter("RecoSeriesName","The Reco Hit Series Name", icrecoseries_name_);
	    AddParameter("TriggerName","The Trigger Name", trigger_name_);
	    AddParameter("DetectorCenterX","cartesian x-component of center of detector", centerX_);
	    AddParameter("DetectorCenterY","cartesian y-component of center of detector", centerY_);
	    AddParameter("DetectorCenterZ","cartesian z-component of center of detector", centerZ_);
	    AddOutBox("OutBox");
	  }
	    
    void Configure()
    { 
        GetParameter("ThetaBin", dtheta0_); 
        GetParameter("PhiBin", dphi0_); 
        GetParameter("I3Reco1", i3reco_1_); 
        GetParameter("I3Reco2", i3reco_2_); 
        GetParameter("I3Reco2Params", i3reco_2params_); 
        GetParameter("RecoSeriesName", icrecoseries_name_); 
        GetParameter("TriggerName", trigger_name_); 
        GetParameter("DetectorCenterX", centerX_); 
        GetParameter("DetectorCenterY", centerY_); 
        GetParameter("DetectorCenterZ", centerZ_);

        detectorCenter_ = I3PositionPtr(new I3Position(centerX_,centerY_,centerZ_));
        rand_  = context_.Get<I3RandomServicePtr>("I3RandomService");
    }
    
    void Physics(I3FramePtr frame)
    {
        
		I3TimePtr starttime(new I3Time(2009,daqtime_));

		// Trigger Info 
		uint16_t triggertag = DST_SIMPLE_MULTIPLICITY << DST_IN_ICE;
		I3TriggerHierarchyPtr mTriggers = I3TriggerHierarchyPtr(new I3TriggerHierarchy);
		SetTrigger(mTriggers, triggertag) ;
		frame->Put(trigger_name_,mTriggers);

		double theta_reco = rand_->Uniform(0.,0.5*I3Constants::pi);
		double phi_reco = rand_->Uniform(0.,2.0*I3Constants::pi);

		double theta_pos = rand_->Uniform(0.,1.0*I3Constants::pi);
		double phi_pos = rand_->Uniform(0.,2.0*I3Constants::pi);
		double distance = uint8_t(rand_->Gaus(0.,250.)/(10*I3Units::meter));

		//int nhit = rand_->Poisson(1.5); 

		I3ParticlePtr reco1(new I3Particle);
		reco1->SetDir(theta_reco*I3Units::rad,phi_reco*I3Units::rad);
		reco1->SetPos(theta_pos*I3Units::rad,phi_pos*I3Units::rad, 
                        distance*I3Units::meter*10.0, I3Position::sph);
		frame->Put(i3reco_1_,reco1);


		I3ParticlePtr reco2(new I3Particle);
		reco2->SetDir(theta_reco*I3Units::rad,phi_reco*I3Units::rad);
		reco2->SetPos(theta_pos*I3Units::rad,phi_pos*I3Units::rad, 
                        distance*I3Units::meter*10.0, I3Position::sph);
		frame->Put(i3reco_2_,reco2);
		I3LogLikelihoodFitParamsPtr llhparams(new I3LogLikelihoodFitParams(1.0,120.0));

		frame->Put(i3reco_2params_,llhparams);

		I3RecoHitSeriesMapPtr ichits(new I3RecoHitSeriesMap());
		frame->Put(icrecoseries_name_,ichits);

		daqtime_ += uint64_t(rand_->Exp(1.6e6));
		eventId_++;
		PushFrame(frame,"OutBox");
    }

    void SetTrigger(I3TriggerHierarchyPtr& triggers, uint16_t triggertag) 
    { 
        // Create a new trigger object to record the result, and set the variables 
        I3TriggerHierarchy::iterator mTriter; 
        I3Trigger Trigger; 
        I3Trigger inTrigger; 
        I3Trigger topTrigger; 
        I3Trigger twrTrigger; 
        I3Trigger spaseTrigger;

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
        } if ( DSTUtils::isIceTop(triggertag) ) {
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
  };
  
  I3_MODULE(DSTTestModule);

  TEST(DSTModule)
  { 
    std::vector<std::string> recos;

    I3Tray tray;
    tray.AddService("I3GSLRandomServiceFactory", "rand");
    tray.SetParameter("rand","seed",1234);
    
    /* Pull in a random GCD file */
    ENSURE(getenv("I3_TESTDATA"));
    const std::string i3_ports(getenv("I3_TESTDATA"));
    boost::filesystem::path gcdfile(i3_ports + "/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz");
    ENSURE(boost::filesystem::exists(gcdfile));

    tray.AddModule("I3InfiniteSource", "TMA-2")
        ("Stream", I3Frame::Physics)
        ("Prefix", gcdfile.string())
        ;
    tray.AddModule("EventMaker", "streams");
    tray.SetParameter("streams","EventRunNumber",1); 
    tray.SetParameter("streams","EventTimeNnanosec",1); 
    tray.SetParameter("streams","EventTimeYear",2007); 

    tray.AddModule("DSTTestModule", "test");
    tray.AddModule("I3DSTModule16", "dst");
    tray.SetParameter("dst","IgnoreDirectHits",true); 
    

    try
      {
	    tray.Execute(1000);
      }
    catch(exception& e)
      {
	    FAIL(e.what());
      }
  }

  TEST(DSTModule13)
  { 
    std::vector<std::string> recos;

    I3Tray tray;
    tray.AddService("I3GSLRandomServiceFactory", "rand");
    tray.SetParameter("rand","seed",1234);
    
    // Pull in a random GCD file 
    ENSURE(getenv("I3_TESTDATA"));
    const std::string i3_ports(getenv("I3_TESTDATA"));
    boost::filesystem::path gcdfile(i3_ports + "/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz");
    ENSURE(boost::filesystem::exists(gcdfile));

    tray.AddModule("I3InfiniteSource", "TMA-2")
        ("Stream", I3Frame::Physics)
        ("Prefix", gcdfile.string())
        ;
    tray.AddModule("EventMaker", "streams");
    tray.SetParameter("streams","EventRunNumber",1); 
    tray.SetParameter("streams","EventTimeNnanosec",1); 
    tray.SetParameter("streams","EventTimeYear",2007); 

    tray.AddModule("DSTTestModule", "test");
    tray.AddModule("I3DSTDAQModule13", "dstdaq_13");
    tray.AddModule("I3DSTModule13", "dst13");

    tray.SetParameter("dst13","IgnoreDirectHits",true); 
    

    try
      {
	    tray.Execute(1000);
      }
    catch(exception& e)
      {
	    FAIL(e.what());
      }
  }


}
