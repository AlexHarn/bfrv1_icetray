#include <I3Test.h>
#include "icetray/I3Tray.h"
#include "icetray/test/ConstructorTest.h"
#include "ophelia/module/I3EHEFirstGuess.h"
#include "I3OpheliaTestModule.h"

#include <boost/python/import.hpp>

TEST_GROUP(I3OpheliaTest);

TEST(inspect)
{
 clean_constructor_test<I3EHEFirstGuess>();
}
TEST(test)
{
  boost::python::import("icecube.icetray");
  boost::python::import("icecube.dataio");

  std::string i3testdata(getenv("I3_TESTDATA"));
  std::string infile = i3testdata + "/2007data/2007_I3Only_Run109732_Nch20.i3.gz";
  I3Tray tray;
  tray.AddModule("I3Reader","readerfactory")("Filename", infile);
  tray.AddModule("QConverter", "qify");
  // We have to drop all P frames coming out of QConverter, and this is C++,
  // so we're stuck with IcePick.
  tray.AddModule("I3IcePickModule<I3FrameObjectFilter>", "drop")
    ("FrameObjectKey", "zzzzz")("DiscardEvents", true);
  tray.AddModule("I3NullSplitter", "null");
  tray.AddModule("I3Portia","pulse") ("DataReadoutName", "InIceRawData")
		                     ("ATWDWaveformName", "CalibratedATWD")
                                     ("FADCWaveformName", "CalibratedFADC");
  tray.AddModule("I3EHEFirstGuess","reco")("MinimumNumberPulseDom", 2)
                                          ("OutputFirstguessName","OpheliaFirstGuess")
                                          ("InputPulseName1","ATWDPortiaPulse")
                                          ("InputPulseName2","FADCPortiaPulse")
                                          ("InputLaunchName","InIceRawData");
  tray.AddModule("I3OpheliaTestModule","recotest");
  
  tray.Execute(10);
  
}
