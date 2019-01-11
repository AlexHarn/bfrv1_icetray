#include <I3Test.h>
#include <icetray/I3Tray.h>
#include "dataclasses/I3Constants.h"
#include "dataclasses/physics/I3Particle.h"

#include "linefit/I3LineFit.h"
#include "recclasses/I3LineFitParams.h"

TEST_GROUP(LineFitIOTest);

namespace linefit_io_test
{

class LFTest : public I3Module {
 public:
  LFTest(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
    const I3Particle& track = frame->Get<I3Particle>("LineFit");  
    const I3LineFitParams& params=frame->Get<I3LineFitParams>("LineFitParams");
    ENSURE_DISTANCE(track.GetZenith(), 0., I3Constants::pi);
    ENSURE_DISTANCE(track.GetAzimuth(), 0., 2*I3Constants::pi);
    ENSURE_DISTANCE(params.LFVelX*params.LFVelX + params.LFVelY*params.LFVelY
	    + params.LFVelZ*params.LFVelZ, params.LFVel*params.LFVel, 1.e-8);
    PushFrame(frame);
  }
};

class LFTestMinHit : public I3Module {
 public:
  LFTestMinHit(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
    const I3Particle& track = frame->Get<I3Particle>("LineFit");  
    ENSURE(std::isnan(track.GetZenith()));
    ENSURE(std::isnan(track.GetAzimuth()));
    PushFrame(frame);
  }
};

}

I3_MODULE(linefit_io_test::LFTest);
I3_MODULE(linefit_io_test::LFTestMinHit);

static void
feed(I3Tray &tray, int event)
{
	boost::python::import("icecube.dataio");
	std::stringstream buf;
	buf << std::string(getenv("I3_TESTDATA"));
	buf << "/reco-toolbox/I3TestEvent_Pulse_";
	buf << event;
	buf << ".i3.gz";
	
	tray.AddModule("I3Reader", "reader")("FileName", buf.str());
}

TEST(hits_can)
{
  I3Tray tray;

  feed(tray, 1);
    
  tray.AddModule("I3LineFit","fit");
  tray.SetParameter("fit","InputRecoPulses","Pulses");
  tray.SetParameter("fit","MinHits",9999);

  tray.AddModule("linefit_io_test::LFTestMinHit","hits_test");
    
  
    
  tray.Execute();
  
}

TEST(angle_can)
{
  I3Tray tray;

  feed(tray, 1);
    
  tray.AddModule("I3LineFit","fit");
  tray.SetParameter("fit","InputRecoPulses","Pulses");
    
  tray.AddModule("linefit_io_test::LFTest","angle_test");
    
  
    
  tray.Execute();
  
}

