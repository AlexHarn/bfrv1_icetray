#include <I3Test.h>

#include "icetray/I3Tray.h"
#include "icetray/I3Units.h" 
//#include <TestWeights.h>
#include <test/PPTestSource.h>
#include "polyplopia/I3PolyplopiaExp.h"
#include "icetray/test/ConstructorTest.h" 
#include "polyplopia/I3PolyplopiaExp.h" 

#include <boost/filesystem.hpp>

class TestWeights;

TEST_GROUP(IceTrayTests);

using namespace std;

TEST(icetray_inspect){
  clean_constructor_test<I3PolyplopiaExp>();
} 


TEST(IceCubeOnlyTest)
{

  {
  I3Tray tray;
  
  int nFrames = 40;

  tray.AddService("I3GSLRandomServiceFactory", "random")
    ("Seed",1);

  string I3_BUILD(getenv("I3_BUILD"));
  string I3_TESTDATA(getenv("I3_TESTDATA"));
  string gcdfile(I3_TESTDATA+"/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz");
  

  I3Context &context = tray.GetContext();
  I3MapStringDoublePtr summary = I3MapStringDoublePtr(new I3MapStringDouble());
  context.Put(summary, "I3SummaryService");
  
  tray.AddModule("I3InfiniteSource", "source")
  ("Prefix", gcdfile);
  
  tray.AddModule("I3MCEventHeaderGenerator", "times");
  

  const int NPES(500);

  tray.AddModule("PPTestSource", "test_source")
    ("NPEs",NPES)
    ("MCTreeName","I3MCTree")
    ("MCPESeriesMapName","I3MCPESeriesMap");

  tray.AddModule("TestWeights", "weights");

  tray.AddModule("I3PolyplopiaExp", "merge")
	("MCTreeName","I3MCTree")
	("MMCTrackListName","MMCTrackList")
    ("TimeWindow", 50*I3Units::microsecond)
    ("Rate",10.5*I3Units::kilohertz);
  
  
  tray.AddModule("PPCheckWeights", "test")
	("MCTreeName","I3MCTree")
	("MMCTrackListName","MMCTrackList")
	("MCPESeriesMapName","I3MCPESeriesMap");

  

  tray.Execute(nFrames);
  
  }

  boost::filesystem::remove(".summary.xml");
}

