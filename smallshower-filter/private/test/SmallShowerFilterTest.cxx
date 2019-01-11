#include <I3Test.h>
#include <icetray/I3Tray.h>
#include <icetray/I3Bool.h>
#include <icetray/I3Int.h>
#include <icetray/test/ConstructorTest.h>
#include <interfaces/I3IcePickModule.h>
#include <interfaces/I3IceForkModule.h>
#include <dataclasses/physics/I3RecoPulse.h>

#include <smallshower-filter/I3SmallShowerFilter.h>

#include <boost/assign/list_inserter.hpp>
#include <boost/assign/list_of.hpp>

#include <string>


TEST_GROUP(CreateSmallShowerTest);

namespace create_small_shower_test {

  using namespace boost::assign;

  void fill2Station(I3RecoPulseSeriesMapPtr &pulses)
  {
    insert( *pulses )
      (OMKey(39, 61), I3RecoPulseSeries())
      (OMKey(39, 63), I3RecoPulseSeries())
      (OMKey(49, 61), I3RecoPulseSeries())
      (OMKey(49, 63), I3RecoPulseSeries());
  }

  void fill3Station(I3RecoPulseSeriesMapPtr &pulses)
  {
    insert( *pulses )
      (OMKey(39, 61), I3RecoPulseSeries())
      (OMKey(39, 63), I3RecoPulseSeries())
      (OMKey(49, 61), I3RecoPulseSeries())
      (OMKey(49, 63), I3RecoPulseSeries())
      (OMKey(48, 61), I3RecoPulseSeries())
      (OMKey(48, 63), I3RecoPulseSeries());
  }

  void fill3StationWrongGeo(I3RecoPulseSeriesMapPtr &pulses)
  {
    insert( *pulses )
      (OMKey(39, 61), I3RecoPulseSeries())
      (OMKey(39, 63), I3RecoPulseSeries())
      (OMKey(57, 61), I3RecoPulseSeries())
      (OMKey(57, 63), I3RecoPulseSeries())
      (OMKey(48, 61), I3RecoPulseSeries())
      (OMKey(48, 63), I3RecoPulseSeries());
  }

  void fill3StationMissingPulses(I3RecoPulseSeriesMapPtr &pulses)
  {
    insert( *pulses )
      (OMKey(39, 61), I3RecoPulseSeries())
      (OMKey(49, 61), I3RecoPulseSeries())
      (OMKey(48, 61), I3RecoPulseSeries());
  }

  void fill4Station(I3RecoPulseSeriesMapPtr &pulses)
  {
    insert( *pulses )
      (OMKey(58, 61), I3RecoPulseSeries())
      (OMKey(58, 63), I3RecoPulseSeries())
      (OMKey(39, 61), I3RecoPulseSeries())
      (OMKey(39, 63), I3RecoPulseSeries())
      (OMKey(49, 61), I3RecoPulseSeries())
      (OMKey(49, 63), I3RecoPulseSeries())
      (OMKey(48, 61), I3RecoPulseSeries())
      (OMKey(48, 63), I3RecoPulseSeries());
  }

  void fill4StationWrongGeo(I3RecoPulseSeriesMapPtr &pulses)
  {
    insert( *pulses )
      (OMKey(21, 61), I3RecoPulseSeries())
      (OMKey(21, 63), I3RecoPulseSeries())
      (OMKey(30, 61), I3RecoPulseSeries())
      (OMKey(30, 63), I3RecoPulseSeries())
      (OMKey(39, 61), I3RecoPulseSeries())
      (OMKey(39, 63), I3RecoPulseSeries())
      (OMKey(29, 61), I3RecoPulseSeries())
      (OMKey(29, 63), I3RecoPulseSeries());
  }

  void fill5Station(I3RecoPulseSeriesMapPtr &pulses)
  {
    insert( *pulses )
      (OMKey(39, 61), I3RecoPulseSeries())
      (OMKey(39, 63), I3RecoPulseSeries())
      (OMKey(49, 61), I3RecoPulseSeries())
      (OMKey(49, 63), I3RecoPulseSeries())
      (OMKey(46, 61), I3RecoPulseSeries())
      (OMKey(46, 63), I3RecoPulseSeries())
      (OMKey(56, 61), I3RecoPulseSeries())
      (OMKey(56, 63), I3RecoPulseSeries())
      (OMKey(59, 61), I3RecoPulseSeries())
      (OMKey(59, 63), I3RecoPulseSeries());
  }


  class CreateSmallShowerIC59 : public I3Module {
  public:
    CreateSmallShowerIC59(const I3Context &ctx)
      : I3Module(ctx), selection_(""), generateSeries_(0),
	outputName_("TestSeries")
    {
      AddParameter("Selection",
		   "Select a pulse series: 2Station, 3Station, 3StationWrongGeo,"
		   "4Station, 4StationWrongGeo, 5Station",
		   selection_);
      AddParameter("OutputName",
		   "Name of the I3RecoPulseSeriesMap",
		   outputName_);
      AddOutBox("OutBox");
    }

    void Configure()
    {
      GetParameter("Selection", selection_);
      GetParameter("OutputName", outputName_);

      static const std::map<std::string, void (*)(I3RecoPulseSeriesMapPtr&)> m
	= map_list_of("2Station", &fill2Station)
                     ("3Station", &fill3Station)
	             ("3StationWrongGeo", &fill3StationWrongGeo)
                     ("3StationMissingPulses", &fill3StationMissingPulses)
                     ("4Station", &fill4Station)
	             ("4StationWrongGeo", &fill4StationWrongGeo)
                     ("5Station", &fill5Station);

      std::map<std::string, void (*)(I3RecoPulseSeriesMapPtr&)>::const_iterator it
	= m.find(selection_);
      if (it == m.end()) {
	log_fatal("Parameter Selection must be one of: 2Station, 3Station,"
		  "3StationWrongGeo, 4Station, 4StationWrongGeo, 5Station. Not \"%s\"",
		  selection_.c_str());
      }

      generateSeries_ = it->second;
    }

    void Physics(I3FramePtr frame)
    {
      I3RecoPulseSeriesMapPtr pulses(new I3RecoPulseSeriesMap);
      generateSeries_(pulses);
      frame->Put(outputName_, pulses);
      PushFrame (frame, "OutBox");
    }

  private:
    std::string selection_;
    void (*generateSeries_)(I3RecoPulseSeriesMapPtr&);
    std::string outputName_;
  };


  class SmallShowerIC59_Test : public I3Module
  {
  public:
    SmallShowerIC59_Test(const I3Context &ctx)
      : I3Module(ctx), filterResultName_(""), expectedResult_(false),
	nStationName_(""), expectedNStation_(0)
    {
      AddParameter("FilterResultName",
		   "Test this filter result",
		   filterResultName_);
      AddParameter("ExpectedResult",
		   "Expect this filter result",
		   expectedResult_);
      AddParameter("NStationName",
		   "Check this frame object for the number of stations",
		   nStationName_);
      AddParameter("ExpectedNStation",
		   "Expect this number of stations",
		   expectedNStation_);
      AddOutBox("OutBox");
    }

    void Configure()
    {
      GetParameter("FilterResultName", filterResultName_);
      GetParameter("ExpectedResult", expectedResult_);
      GetParameter("NStationName", nStationName_);
      GetParameter("ExpectedNStation", expectedNStation_);

      if (filterResultName_.size() == 0)
	log_fatal("Must configure FilterResultName");
    }

    void Physics(I3FramePtr frame)
    {
      I3BoolConstPtr decision = frame->Get<I3BoolConstPtr>(filterResultName_);
      ENSURE((bool)decision);
      ENSURE(decision->value == expectedResult_);

      if (nStationName_.size() > 0) {
	I3IntConstPtr nStation = frame->Get<I3IntConstPtr>(nStationName_);
	ENSURE((bool)nStation);
	ENSURE(nStation->value == expectedNStation_);
      }

      PushFrame(frame, "OutBox");
    }

  private:
    std::string filterResultName_;
    bool expectedResult_;
    std::string nStationName_;
    int expectedNStation_;
  };

}


I3_MODULE(create_small_shower_test::CreateSmallShowerIC59);
I3_MODULE(create_small_shower_test::SmallShowerIC59_Test);

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

/*
 * Test response for 2 station input. Outcome should be: filter not passed
 */
TEST(TwoStations)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("create_small_shower_test::CreateSmallShowerIC59",
		 "create");
  tray.SetParameter("create", "Selection", "2Station");

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "IC59");
  tray.SetParameter("filter", "TopPulseKey", "TestSeries");
  tray.SetParameter("filter", "DecisionName", "I3SmallShowerFilter");
  tray.SetParameter("filter", "NStationResultName", "NStation");

  tray.AddModule("create_small_shower_test::SmallShowerIC59_Test", "check");
  tray.SetParameter("check", "FilterResultName", "I3SmallShowerFilter");
  tray.SetParameter("check", "ExpectedResult", false);

  

  tray.Execute();
  
}


/*
 * Test response for 3 station input. Output should be: filter passed
 */
TEST(ThreeStations)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("create_small_shower_test::CreateSmallShowerIC59",
		 "create");
  tray.SetParameter("create", "Selection", "3Station");

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "IC59");
  tray.SetParameter("filter", "TopPulseKey", "TestSeries");
  tray.SetParameter("filter", "DecisionName", "I3SmallShowerFilter");
  tray.SetParameter("filter", "NStationResultName", "NStation");

  tray.AddModule("create_small_shower_test::SmallShowerIC59_Test", "check");
  tray.SetParameter("check", "FilterResultName", "I3SmallShowerFilter");
  tray.SetParameter("check", "ExpectedResult", true);
  tray.SetParameter("check", "NStationName", "NStation");
  tray.SetParameter("check", "ExpectedNStation", 3);

  

  tray.Execute();
  
}

/*
 * Test response for 3 station input. Pulses do not form a cluster.
 * Outcome should be: filter not passed
 */
TEST(ThreeStationsWrongGeo)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("create_small_shower_test::CreateSmallShowerIC59",
		 "create");
  tray.SetParameter("create", "Selection", "3StationWrongGeo");

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "IC59");
  tray.SetParameter("filter", "TopPulseKey", "TestSeries");
  tray.SetParameter("filter", "DecisionName", "I3SmallShowerFilter");
  tray.SetParameter("filter", "NStationResultName", "NStation");

  tray.AddModule("create_small_shower_test::SmallShowerIC59_Test", "check");
  tray.SetParameter("check", "FilterResultName", "I3SmallShowerFilter");
  tray.SetParameter("check", "ExpectedResult", false);

  

  tray.Execute();
  
}

/*
 * Test response for 3 station input. Each station has only one pulse.
 * Outcome should be: filter passed.
 */
TEST(ThreeStationsMissingPulses)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("create_small_shower_test::CreateSmallShowerIC59",
		 "create");
  tray.SetParameter("create", "Selection", "3StationMissingPulses");

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "IC59");
  tray.SetParameter("filter", "TopPulseKey", "TestSeries");
  tray.SetParameter("filter", "DecisionName", "I3SmallShowerFilter");
  tray.SetParameter("filter", "NStationResultName", "NStation");

  tray.AddModule("create_small_shower_test::SmallShowerIC59_Test", "check");
  tray.SetParameter("check", "FilterResultName", "I3SmallShowerFilter");
  tray.SetParameter("check", "ExpectedResult", true);
  tray.SetParameter("check", "NStationName", "NStation");
  tray.SetParameter("check", "ExpectedNStation", 3);

  

  tray.Execute();
  
}

/*
 * Test response for 4 station input. Outcome should be: filter passed
 */
TEST(FourStations)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("create_small_shower_test::CreateSmallShowerIC59",
		 "create");
  tray.SetParameter("create", "Selection", "4Station");

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "IC59");
  tray.SetParameter("filter", "TopPulseKey", "TestSeries");
  tray.SetParameter("filter", "DecisionName", "I3SmallShowerFilter");
  tray.SetParameter("filter", "NStationResultName", "NStation");

  tray.AddModule("create_small_shower_test::SmallShowerIC59_Test", "check");
  tray.SetParameter("check", "FilterResultName", "I3SmallShowerFilter");
  tray.SetParameter("check", "ExpectedResult", true);
  tray.SetParameter("check", "NStationName", "NStation");
  tray.SetParameter("check", "ExpectedNStation", 4);

  

  tray.Execute();
  
}

/*
 * Test 4 station input, stations form a cluster on the array boundary.
 * Expected outcome: filter not passed.
 */
TEST(FourStationsWrongGeo)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("create_small_shower_test::CreateSmallShowerIC59",
		 "create");
  tray.SetParameter("create", "Selection", "3StationWrongGeo");

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "IC59");
  tray.SetParameter("filter", "TopPulseKey", "TestSeries");
  tray.SetParameter("filter", "DecisionName", "I3SmallShowerFilter");
  tray.SetParameter("filter", "NStationResultName", "NStation");

  tray.AddModule("create_small_shower_test::SmallShowerIC59_Test", "check");
  tray.SetParameter("check", "FilterResultName", "I3SmallShowerFilter");
  tray.SetParameter("check", "ExpectedResult", false);

  

  tray.Execute();
  
}

/*
 * Test 5 station input. Expected outcome: filter not passed.
 */
TEST(FiveStations)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("create_small_shower_test::CreateSmallShowerIC59",
		 "create");
  tray.SetParameter("create", "Selection", "5Station");

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "IC59");
  tray.SetParameter("filter", "TopPulseKey", "TestSeries");
  tray.SetParameter("filter", "DecisionName", "I3SmallShowerFilter");
  tray.SetParameter("filter", "NStationResultName", "NStation");

  tray.AddModule("create_small_shower_test::SmallShowerIC59_Test", "check");
  tray.SetParameter("check", "FilterResultName", "I3SmallShowerFilter");
  tray.SetParameter("check", "ExpectedResult", false);

  

  tray.Execute();
  
}


/*
 * Test non-existent pulse series. Expected outcome: Module does not
 * crash. Filter not passed information written to frame.
 */
TEST(NoPulses)
{
  I3Tray tray;

  feed(tray, 1);

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "IC59");
  tray.SetParameter("filter", "TopPulseKey", "NoSeries");
  tray.SetParameter("filter", "DecisionName", "I3SmallShowerFilter");
  tray.SetParameter("filter", "NStationResultName", "NStation");

  tray.AddModule("create_small_shower_test::SmallShowerIC59_Test", "check");
  tray.SetParameter("check", "FilterResultName", "I3SmallShowerFilter");
  tray.SetParameter("check", "ExpectedResult", false);

  

  tray.Execute();
  
}


/*
 * Run smallshower-filter with an unknown input geometry.
 * Expected outcome: Module log_fatals
 */
TEST(UnknownInput)
{
  I3Tray tray;

  tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter");
  tray.SetParameter("filter", "FilterGeometry", "NoGeometry");
  EXPECT_THROW(tray.Execute(), "This should throw.");
}


// basically the same as the clean_constructor_test template in
// icetray/public/icetray/test/ConstructorTest.h, but with two OutBox connections
template <class T>
void icefork_constructor_test()
{
  typedef std::map<std::string, std::pair<FrameFifoPtr, I3ModulePtr> > outboxmap_t;
  I3Context context;
  {
    I3ConfigurationPtr config(new I3Configuration());
    context.Put(config);
    
    boost::shared_ptr<outboxmap_t> ob(new outboxmap_t);
    (*ob)["TrueBox"] = make_pair(FrameFifoPtr(), I3ModulePtr());
    (*ob)["FalseBox"] = make_pair(FrameFifoPtr(), I3ModulePtr());
    context.Put("OutBoxes",ob);

    I3IceForkModule<T> module(context);
  }  
}

TEST(CleanConstructor){
  clean_constructor_test<I3IcePickModule<I3SmallShowerFilter> >();
  icefork_constructor_test<I3SmallShowerFilter>();
}
