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

namespace DSTExtractorTest
{

  TEST(DSTExtractor)
  { 
    std::vector<std::string> recos;

    I3Tray tray;
    tray.AddService("I3GSLRandomServiceFactory", "rand");
    tray.SetParameter("rand","seed",1234);
    
    /* Pull in a random GCD file */
    ENSURE(getenv("I3_TESTDATA"));
    const std::string i3_ports(getenv("I3_TESTDATA"));
    boost::filesystem::path gcdfile(i3_ports + "/sim/GeoCalibDetectorStatus_IC80_DC6.54655.i3.gz");
    ENSURE(boost::filesystem::exists(gcdfile));

    tray.AddModule("I3InfiniteSource", "TMA-2")
        ("Stream", I3Frame::DAQ)
        ("Prefix", gcdfile.string())
        ;
    tray.AddModule("DSTTestGenerator", "generatedst");
    tray.SetParameter("generatedst","SubEventStreamName","dst_stream"); 
    tray.SetParameter("generatedst","NEvents",100); 


    tray.AddModule("I3DSTModule16", "dst");
    tray.SetParameter("dst","DSTName","I3DST");
    tray.SetParameter("dst","DSTHeaderName","I3DSTHeader");
    tray.SetParameter("dst","IgnoreDirectHits",true); 

    tray.AddModule("I3DSTExtractor16", "unpack_dst");
    tray.SetParameter("unpack_dst","DSTName","I3DST");
    tray.SetParameter("unpack_dst","DSTHeaderName","I3DSTHeader");
    tray.SetParameter("unpack_dst","SubEventStreamName", "TDST");
    tray.SetParameter("unpack_dst","ExtractToFrame", true);
    tray.SetParameter("unpack_dst","TriggerName", "I3TriggerHierarchy");
    tray.SetParameter("unpack_dst","Cut", true);
	

    try
      {
	    tray.Execute(1000);
      }
    catch(exception& e)
      {
	    FAIL(e.what());
      }
  }

  TEST(DSTExtractor13)
  { 
    std::vector<std::string> recos;

    I3Tray tray;
    tray.AddService("I3GSLRandomServiceFactory", "rand");
    tray.SetParameter("rand","seed",1234);
    
    // Pull in a random GCD file 
    ENSURE(getenv("I3_TESTDATA"));
    const std::string i3_ports(getenv("I3_TESTDATA"));
    boost::filesystem::path gcdfile(i3_ports + "/sim/GeoCalibDetectorStatus_IC80_DC6.54655.i3.gz");
    ENSURE(boost::filesystem::exists(gcdfile));

    tray.AddModule("I3InfiniteSource", "TMA-2")
        ("Stream", I3Frame::DAQ)
        ("Prefix", gcdfile.string())
        ;
    tray.AddModule("EventMaker", "streams");
    tray.SetParameter("streams","EventRunNumber",1); 
    tray.SetParameter("streams","EventTimeNnanosec",1); 
    tray.SetParameter("streams","EventTimeYear",2007); 

    tray.AddModule("DSTTestModule", "test");

    tray.AddModule("DSTTestGenerator", "generatedst");
    tray.SetParameter("generatedst","SubEventStreamName","dst_stream"); 
    tray.SetParameter("generatedst","NEvents",100); 


    tray.AddModule("I3DSTDAQModule13", "dstdaq_13");
    tray.SetParameter("dstdaq_13","DSTHeaderName","I3DST13Header");
    tray.SetParameter("dstdaq_13","DSTName","I3DST13");

    tray.AddModule("I3DSTModule13", "dst13");
    tray.SetParameter("dst13","IgnoreDirectHits",true); 

    tray.AddModule("I3DSTExtractor13", "unpack_dst");
    tray.SetParameter("unpack_dst","DSTName","I3DST13");
    tray.SetParameter("unpack_dst","DSTHeaderName","I3DST13Header");
    tray.SetParameter("unpack_dst","SubEventStreamName", "TDST13");
    tray.SetParameter("unpack_dst","ExtractToFrame", true);
    tray.SetParameter("unpack_dst","TriggerName", "I3TriggerHierarchy");
	

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
