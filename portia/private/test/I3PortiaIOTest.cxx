#include <I3Test.h>
#include <icetray/I3Tray.h>
#include "dataclasses/I3Constants.h"

#include <recclasses/I3PortiaEvent.h>
#include "portia/I3Portia.h"

TEST_GROUP(I3PortiaIOTest);

namespace portia_io_test
{

  class PortiaTest : public I3Module {
    
  public:
    PortiaTest(const I3Context& ctx) :  I3Module(ctx)
    {
      AddOutBox("OutBox");
    }
    
    void DAQ(I3FramePtr frame) {

      I3PortiaEventConstPtr portiaEvent_ptr = 
	frame->Get<I3PortiaEventConstPtr>("PortiaEvent");

      ENSURE((bool)portiaEvent_ptr);
      ENSURE_DISTANCE(portiaEvent_ptr->GetTotalAtwdNPE(), 50., 10000., "Total ATWD NPE is not reasonable value: calibration may be incorrect");
      ENSURE_DISTANCE(portiaEvent_ptr->GetTotalFadcNPE(), 50., 10000., "Total FADC NPE is not reasonable value: calibration may be incorrect");
      ENSURE_DISTANCE(portiaEvent_ptr->GetTotalBestNPE(), 50., 10000., "Total Best NPE is not reasonable value");
      ENSURE_DISTANCE(portiaEvent_ptr->GetTotalAtwdNPE(), 50., portiaEvent_ptr->GetTotalBestNPE(), "Total ATWD NPE must be equal or smaller than Best Value");
      ENSURE_DISTANCE(portiaEvent_ptr->GetTotalFadcNPE(), 50., portiaEvent_ptr->GetTotalBestNPE(), "Total FADC NPE must be equal or smaller than Best Value");

      PushFrame(frame,"OutBox");
    }
  };

}

I3_MODULE(portia_io_test::PortiaTest);

static void
feed(I3Tray &tray)
{
  boost::python::import("icecube.dataio");
  std::stringstream buf;
  buf << std::string(getenv("I3_TESTDATA"));
  buf << "/2007data/2007_I3Only_Run109732_Nch20.i3.gz";
  
  tray.AddModule("I3Reader", "reader")("FileName", buf.str());
  tray.AddModule("I3WaveCalibrator", "calibrator")("Launches", "InIceRawData")
    ("Waveforms","CalibratedWaveforms")
    ("CorrectDroop", 0);
  tray.AddModule("I3WaveformSplitter", "split")("Input", "CalibratedWaveforms")
    ("HLC_ATWD", "CalibratedATWD")
    ("HLC_FADC", "CalibratedFADC")
    ("Force", 1)
    ("PickUnsaturatedATWD", 1);
  tray.AddModule("I3Portia","pulse") ("DataReadoutName", "InIceRawData")
    ("ATWDWaveformName", "CalibratedATWD")
    ("FADCWaveformName", "CalibratedFADC");
  
}


TEST(test)
{
  
  I3Tray tray;

  feed(tray);

  tray.AddModule("portia_io_test::PortiaTest","pulsetest");

  

  tray.Execute(5);

  
}
