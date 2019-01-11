#include <I3Test.h>
#include <icetray/I3Tray.h>
#include <dataclasses/I3Constants.h>
#include <dataclasses/physics/I3Particle.h>

#include "cramer-rao/CramerRao.h"
#include "recclasses/CramerRaoParams.h"

#include <cmath>

TEST_GROUP(cr-test);

namespace cr_test_tools
{
  class put_track : public I3Module {
  public:
    put_track(const I3Context& context);
    void Physics(I3FramePtr frame);
  };
  put_track::put_track(const I3Context& context): 
    I3Module(context) {
    AddOutBox("OutBox");
  }
  void put_track::Physics(I3FramePtr frame) {
    I3ParticlePtr part(new I3Particle());
    part->SetPos(1.0, 2.0, 3.0);
    part->SetDir(1.1,3.5);
    part->SetType(I3Particle::PMinus);
    part->SetEnergy(100*I3Units::GeV);
    part->SetSpeed(I3Constants::c);
    frame->Put("test-particle",part);
    log_info("put_track passed through");
    PushFrame(frame,"OutBox");
  }
  
  class result_check : public I3Module {
  public:
    result_check(const I3Context& ctx);
    void Configure();
    void Physics(I3FramePtr frame);
  private:
    bool should_not_work;
  };

  result_check::result_check(const I3Context& ctx)
    : I3Module(ctx), should_not_work(0)
  {
    AddParameter("should_not_work",
		 "should the cr calculation have not worked?",
    		 should_not_work);
    AddOutBox("OutBox");
  }

  void result_check::Configure() {
    GetParameter("should_not_work",
		 should_not_work);
  }

  void result_check::Physics(I3FramePtr frame) {
    const CramerRaoParams& cr_params=frame->Get<CramerRaoParams>("croutParams");
    if(!should_not_work){
    ENSURE(!std::isnan(cr_params.cramer_rao_theta));
    ENSURE(!std::isnan(cr_params.cramer_rao_phi));
    }
    PushFrame(frame);
    log_info("result_check passed through");
  }
}
I3_MODULE(cr_test_tools::put_track);
I3_MODULE(cr_test_tools::result_check);

static void
feed(I3Tray &tray)
{
  boost::python::import("icecube.dataio");
  std::string infile(getenv("I3_TESTDATA"));
  infile.append("/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3");
  tray.AddModule("I3Reader","reader")("FileName",infile);

}

TEST(regular){
  I3Tray tray;

  feed(tray);

  tray.AddModule("cr_test_tools::put_track","pt");

  tray.AddModule("CramerRao","resol");
  tray.SetParameter("resol","InputResponse","MaskedOfflinePulses");
  tray.SetParameter("resol","InputTrack","test-particle");
  tray.SetParameter("resol","OutputResult","crout");
  
  tray.AddModule("cr_test_tools::result_check","check");
    
  tray.Execute(4);
}

TEST(no_track){
  I3Tray tray;

  feed(tray);

  tray.AddModule("cr_test_tools::put_track","pt");

  tray.AddModule("CramerRao","resol");
  tray.SetParameter("resol","InputResponse","Pulses");
  tray.SetParameter("resol","InputTrack","non-existing-particle");
  tray.SetParameter("resol","OutputResult","crout");
  
  tray.AddModule("cr_test_tools::result_check","check");
  tray.SetParameter("check","should_not_work",1);

  tray.Execute(4);
}

TEST(no_pulses){
  I3Tray tray;

  feed(tray);

  tray.AddModule("cr_test_tools::put_track","pt");

  tray.AddModule("CramerRao","resol");
  tray.SetParameter("resol","InputResponse","non-existing-pulses");
  tray.SetParameter("resol","InputTrack","test-particle");
  tray.SetParameter("resol","OutputResult","crout");
  
  tray.AddModule("cr_test_tools::result_check","check");
  tray.SetParameter("check","should_not_work",1);
    
  tray.Execute(4);
}
