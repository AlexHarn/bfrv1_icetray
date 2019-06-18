#include "I3Test.h"
#include "icetray/I3Tray.h"

#include "dataclasses/I3Constants.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "icetray/OMKey.h"
#include "linefit/I3LineFit.h"

#include "HuberFit/HuberFit.h"
#include "DelayCleaning/DelayCleaning.h"
#include "Debiasing/Debiasing.h"

#include <boost/python/import.hpp>

TEST_GROUP(ImprovedLinefitTest);

const std::string testDataName = "test_series";
const std::string delaySeriesName = "delay_series";
const std::string HuberFitName = "Huberfit";
const std::string debiasSeriesName = "debias_series";

OMKey om1(1,1), om2(2,2), om3(3,3), om4(4,4), om5(5,5),om6(10,5);
namespace improvedlinefit_test
{
//Generate some test data
class  createPulseSeries: public I3Module {
 public:
createPulseSeries(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
	I3RecoPulseSeriesMapPtr newMap (new I3RecoPulseSeriesMap);
	
	vector<I3RecoPulse> hits1, hits2, hits3, hits4, hits5, hits6;
	I3RecoPulse hit1, hit2, hit3, hit4, hit5, hit6; 
	
	hit1.SetCharge( 1);
	hit1.SetTime (100);
	hit2.SetCharge (1);
	hit2.SetTime (200);
	hit3.SetCharge (1);
	hit3.SetTime (300);	
	hit4.SetCharge (1);
	hit4.SetTime (400);	
	hit5.SetCharge (1);
	hit5.SetTime (500);	
	hit6.SetCharge (1);
	hit6.SetTime (1000);	
	
	hits1.push_back(hit1);
	hits2.push_back(hit2);
	hits3.push_back(hit3);
	hits4.push_back(hit4);
	hits5.push_back(hit5);
	hits6.push_back(hit6);
	
	(*newMap)[om1] = hits1;
	(*newMap)[om2] = hits2;
	(*newMap)[om3] = hits3;
	(*newMap)[om4] = hits4;
	(*newMap)[om5] = hits5;
	(*newMap)[om6] = hits6;
	
	frame->Put( testDataName,newMap);
	PushFrame(frame);
	}
};


//Test that the delay cleaning properly handles the case where the input series
//isn't in the frame
class ILFTestClean_noHits : public I3Module {
 public:
  ILFTestClean_noHits(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
    bool rejected= !(frame->Has(delaySeriesName));  
    ENSURE(rejected, "Delay cleaning should return nothing when given a missing hitseries.");
    
    PushFrame(frame);
  }
};
//Test that the delay cleaning properly handles the case of being too agressive
class ILFTestClean_fewHits : public I3Module {
 public:
  ILFTestClean_fewHits(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
  	I3RecoPulseSeriesMapConstPtr oldMap =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(testDataName);
	I3RecoPulseSeriesMapConstPtr newMap =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(delaySeriesName);
	ENSURE(oldMap != newMap, "DelayCleaning should have returned the seed hit series");
	PushFrame(frame);
  }
};

//Test that the delay cleaning removes the correct hits
class ILFTestClean_correctHits : public I3Module {
 public:
  ILFTestClean_correctHits(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {

	I3RecoPulseSeriesMapConstPtr launchMap =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(delaySeriesName);
	
	std::vector<OMKey> omnumbers;
	I3RecoPulseSeriesMap::const_iterator iter;
	
	for(iter = launchMap->begin() ; iter != launchMap->end() ; iter++)
    {
      omnumbers.push_back(iter->first);
    }
    
    ENSURE(omnumbers.size()==5,"DelayCleaning removed to many launches");
	ENSURE(omnumbers[0]==om1,"1st surviving launch after DelayCleaning should have been om1");
	ENSURE(omnumbers[1]==om2,"2nd surviving launch after DelayCleaning should have been om2");
	ENSURE(omnumbers[2]==om3,"3rd surviving launch after DelayCleaning should have been om3");
	ENSURE(omnumbers[3]==om4,"4th surviving launch after DelayCleaning should have been om4");
	ENSURE(omnumbers[4]==om5,"5th surviving launch after DelayCleaning should have been om5");
    PushFrame(frame);
  }
};

//Test that the Huber fit properly handles the case of there being no series in the frame
class ILFTestHuber_noSeries : public I3Module {
 public:
  ILFTestHuber_noSeries(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
    bool rejected= !(frame->Has(HuberFitName));  
    ENSURE(rejected);
    PushFrame(frame);
  }
};

//Test that the Huber fit properly fits the data 
class ILFTestHuber_correctFit : public I3Module {
 public:
  ILFTestHuber_correctFit(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {
  	//Check that the Huberfit is roughly where it should be
    const I3Particle& huberFit = frame->Get<I3Particle>(HuberFitName);   
    ENSURE(huberFit.GetZenith()<= 1.4 && huberFit.GetZenith() >= 1.3,"Huberfit is not fitting correctly.");
    ENSURE(huberFit.GetAzimuth()<= 3.8 && huberFit.GetAzimuth() >= 3.7,"Huberfit is not fitting correctly.");
    double x = huberFit.GetPos().GetX();
    double y = huberFit.GetPos().GetY();
    double z = huberFit.GetPos().GetZ();
    ENSURE(x <= -96 && x  >= -97,"Huberfit is not fitting correctly.");
    ENSURE(y <= -517 && y >= -518,"Huberfit is not fitting correctly.");
    ENSURE(z <= 492 && z <= 491,"Huberfit is not fitting correctly.");
    PushFrame(frame);
  }
};


//Test that the debiasing properly removes hits
class ILFTestDebias : public I3Module {
 public:
  ILFTestDebias(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }
  void Physics(I3FramePtr frame) {
	I3RecoPulseSeriesMapConstPtr launchMap =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(debiasSeriesName);
	
	std::vector<OMKey> omnumbers;
	I3RecoPulseSeriesMap::const_iterator iter;
	
	for(iter = launchMap->begin() ; iter != launchMap->end() ; iter++)
	{
	  omnumbers.push_back(iter->first);
	}
	ENSURE(omnumbers.size()==4,"Debiasing removed to many launches");
	ENSURE(omnumbers[0]==om2,"1st surviving launch after Debiasing should have been om2");
	ENSURE(omnumbers[1]==om3,"2nd surviving launch after Debiasing should have been om3");
	ENSURE(omnumbers[2]==om4,"3rd surviving launch after Debiasing should have been om4");
	ENSURE(omnumbers[3]==om6,"4th surviving launch after Debiasing should have been om6");
	PushFrame(frame);
  }
};

}

I3_MODULE(improvedlinefit_test::ILFTestClean_noHits);
I3_MODULE(improvedlinefit_test::ILFTestClean_fewHits);
I3_MODULE(improvedlinefit_test::createPulseSeries);
I3_MODULE(improvedlinefit_test::ILFTestClean_correctHits);
I3_MODULE(improvedlinefit_test::ILFTestHuber_noSeries);
I3_MODULE(improvedlinefit_test::ILFTestHuber_correctFit);
I3_MODULE(improvedlinefit_test::ILFTestDebias);

static void
feed(I3Tray &tray)
{
	std::vector<std::string> filenames;
	std::string i3testdata = getenv("I3_TESTDATA");
	
	std::string GCDfile = i3testdata + "/GCD/GeoCalibDetectorStatus_2012.56063_V0.i3.gz";
	std::string datafile = i3testdata + "/sim/nugen_numu_ic80_dc6.002488.000000.processed.i3.gz";
	filenames.push_back(GCDfile);
	filenames.push_back(datafile);
	
	
	tray.AddModule("I3Reader", "reader")("filenameList", filenames);
	
	
	tray.AddModule("I3EventCounter","nprocessed");
	tray.SetParameter("nprocessed", "NEvents",1);
    tray.SetParameter("nprocessed","CounterStep",1);

}

TEST(ILFTestClean_noSeries)
{
  boost::python::import("icecube.icetray");
  boost::python::import("icecube.dataio");

  I3Tray tray;

  feed(tray);
    
  tray.AddModule("DelayCleaning","clean");
  tray.SetParameter("clean","InputResponse","Incorrect_series");
  tray.SetParameter("clean","OutputResponse",delaySeriesName);
  tray.SetParameter("clean","TimeWindow",9999);
  tray.SetParameter("clean","Distance",9999);

  tray.AddModule("improvedlinefit_test::ILFTestClean_noHits","noNewSeries");
    
  
    
  tray.Execute();
  
}

TEST(ILFTestdelay_fewHits)
{
	boost::python::import("icecube.icetray");
	boost::python::import("icecube.dataio");
	
	I3Tray tray;
	
	feed(tray);
	
	tray.AddModule("improvedlinefit_test::createPulseSeries",testDataName);
	
	tray.AddModule("DelayCleaning","clean");
	tray.SetParameter("clean","InputResponse",testDataName);
	tray.SetParameter("clean","OutputResponse",delaySeriesName);
	tray.SetParameter("clean","TimeWindow",1);
	tray.SetParameter("clean","Distance",9999);
	
	tray.AddModule("improvedlinefit_test::ILFTestClean_fewHits","testSeries");
	
	
	
	tray.Execute();
	
}

TEST(delay_correctHits)
{
	boost::python::import("icecube.icetray");
	boost::python::import("icecube.dataio");
	
	I3Tray tray;
	
	feed(tray);
	tray.AddModule("improvedlinefit_test::createPulseSeries", testDataName);

	tray.AddModule("DelayCleaning","clean");
	tray.SetParameter("clean","InputResponse",testDataName);
	tray.SetParameter("clean","OutputResponse",delaySeriesName);
	tray.SetParameter("clean","TimeWindow",200);
	tray.SetParameter("clean","Distance",200);
		
	tray.AddModule("improvedlinefit_test::ILFTestClean_correctHits","correctSeries");
	
	
	
	tray.Execute();
	
}

TEST(huber_noSeries)
{
  boost::python::import("icecube.icetray");
  boost::python::import("icecube.dataio");

  I3Tray tray;

  feed(tray);
    
  tray.AddModule("HuberFit","huber");
  tray.SetParameter("huber","InputRecoPulses","Incorrect_series");
  tray.SetParameter("huber","name",HuberFitName);
  

  tray.AddModule("improvedlinefit_test::ILFTestHuber_noSeries","noSeries");
    
  
    
  tray.Execute();
  
}

TEST(huber_Correct)
{
	boost::python::import("icecube.icetray");
	boost::python::import("icecube.dataio");
	
	I3Tray tray;
	
	feed(tray);
	
	tray.AddModule("improvedlinefit_test::createPulseSeries", testDataName);
	
	tray.AddModule("HuberFit","huber");
	tray.SetParameter("huber","InputRecoPulses",testDataName);
	tray.SetParameter("huber","name",HuberFitName);
	tray.SetParameter("huber","Distance",100);
		
	
	tray.AddModule("improvedlinefit_test::ILFTestHuber_correctFit","fewHits");
	
	
	
	tray.Execute();
	
}

TEST(debias)
{
	boost::python::import("icecube.icetray");
	boost::python::import("icecube.dataio");
	
	I3Tray tray;
	
	feed(tray);
	tray.AddModule("improvedlinefit_test::createPulseSeries", "newData");

	tray.AddModule("I3LineFit","fit");
	tray.SetParameter("fit","InputRecoPulses",testDataName);
	tray.SetParameter("fit","Name","linefit");

	tray.AddModule("Debiasing","debias");
	tray.SetParameter("debias","InputResponse",testDataName);
	tray.SetParameter("debias","Seed","linefit");
	tray.SetParameter("debias","OutputResponse",debiasSeriesName);
	tray.SetParameter("debias","Distance", 150);
			
	tray.AddModule("improvedlinefit_test::ILFTestDebias","debias_check");
	
	
	
	tray.Execute();
	
}


