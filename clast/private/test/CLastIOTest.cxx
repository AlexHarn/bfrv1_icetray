#include <I3Test.h>
#include <icetray/I3Tray.h>
#include "dataclasses/I3Constants.h"
#include "dataclasses/physics/I3Particle.h"

#include "clast/I3CLastModule.h"
#include "recclasses/I3CLastFitParams.h"

TEST_GROUP(CLastIOTest);

namespace clast_io_test
{

class CLastTest : public I3Module {
 public:
  CLastTest(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
    const I3Particle&       track  = frame->Get<I3Particle>("CLast");
    const I3CLastFitParams& params = frame->Get<I3CLastFitParams>("CLastParams");
    ENSURE_DISTANCE( track.GetZenith()  , 0.0     , I3Constants::pi   , "Check: track.GetZenith() - outside of range [0,pi]"   );
    ENSURE_DISTANCE( track.GetAzimuth() , 0.0     , 2*I3Constants::pi , "Check: track.GetAzimuth() - outside of range [0,2pi]" );
    ENSURE_DISTANCE( params.mineval     , 1147.30 , 1.0e-2            , "Check: params.mineval - not near 1147.30+/-0.01 as expected for PulseTestEvent" );
    PushFrame(frame);
  }
};

class CLastTestMinHit : public I3Module {
 public:
  CLastTestMinHit(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
    const I3Particle& track = frame->Get<I3Particle>("CLast");
    ENSURE( std::isnan(track.GetZenith())  , "Check : track.GetZenith() - not isnan as expected for PulseTestEvent"  );
    ENSURE( std::isnan(track.GetAzimuth()) , "Check : track.GetAzimuth() - not isnan as expected for PulseTestEvent" );
    PushFrame(frame);
  }
};

}

I3_MODULE(clast_io_test::CLastTest);
I3_MODULE(clast_io_test::CLastTestMinHit);

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
    
  tray.AddModule("I3CLastModule","fit");
  tray.SetParameter("fit","Name","CLast");
  tray.SetParameter("fit","InputReadout","Pulses");
  tray.SetParameter("fit","MinHits",9999);

  tray.AddModule("clast_io_test::CLastTestMinHit","hits_test");
    
  
    
  tray.Execute();
  
}

TEST(angle_can)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("I3CLastModule","fit");
  tray.SetParameter("fit","Name","CLast");
  tray.SetParameter("fit","InputReadout","Pulses");

  tray.AddModule("clast_io_test::CLastTest","angle_test");
    
  
    
  tray.Execute();
  
}

