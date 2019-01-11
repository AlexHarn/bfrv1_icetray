#include <I3Test.h>
#include <boost/make_shared.hpp>
#include <boost/math/constants/constants.hpp>
#include <icetray/I3Tray.h>
#include <icetray/I3ServiceFactory.h>
#include <dataclasses/I3Map.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/physics/I3Particle.h>
#include <shield/I3ShieldDataCollector.h>
#include <recclasses/I3ShieldHitRecord.h>
namespace constants = boost::math::constants;

//======================================
// Infrastructure for running a module without a tray

template<typename T>
class param_setter {
	T& owner;
	I3Configuration& config;
	bool final;
public:
	param_setter<T>(T& t, I3Configuration& c):owner(t),config(c),final(true){}
	param_setter<T>(param_setter<T>&& prev):owner(prev.owner),config(prev.config),final(true){
		prev.final=false;
	}
	~param_setter<T>(){
		if(final)
			owner.FinalizeConfiguration();
	}
	template <typename U>
	param_setter& operator()(const std::string& paramName, U value){
		config.Set(paramName, boost::python::object(value));
		return(*this);
	}
};

class ServiceFactoryWrapper{
private:
	std::string factoryClassName;
	I3Context& context;
	boost::shared_ptr<I3ServiceFactory> factory;
public:
	ServiceFactoryWrapper(I3Context& context, const std::string& factName):
	factoryClassName(factName),
	context(context),
	factory(I3::Singleton<I3ServiceFactoryFactory>::get_const_instance().Create(factoryClassName)(context))
	{}
	
	param_setter<ServiceFactoryWrapper> Configure(){
		//unfortunately this appears to be impossible without using a const_cast hack
		return(param_setter<ServiceFactoryWrapper>(*this,const_cast<I3Configuration&>(factory->GetConfiguration())));
	}
	void FinalizeConfiguration(){
		factory->Configure();
	}
	
	void installService(){
		factory->InitializeService(context);
	}
};

template<typename ModuleType>
class SingleModuleTestSetup{
private:
	boost::shared_ptr<std::deque<boost::shared_ptr<I3Frame>>> inbox;
	I3Context& context;
	boost::shared_ptr<ModuleType> module;
	struct CollectorModule : public I3Module{
		CollectorModule(I3Context& ctx):I3Module(ctx){}
		boost::shared_ptr<I3Frame> GetFrame(){ return(PopFrame()); }
	};
	boost::shared_ptr<CollectorModule> collector;
public:
	SingleModuleTestSetup(I3Context& context):
	inbox(new std::deque<boost::shared_ptr<I3Frame> >),
	context(context),
	module(new ModuleType(context)),
	collector(new CollectorModule(context))
	{
		module->configuration_.ClassName(I3::name_of(typeid(*module)));
		module->inbox_=inbox;
		module->ConnectOutBox("OutBox",collector);
	}
	
	param_setter<SingleModuleTestSetup<ModuleType>> Configure(){
		return(param_setter<SingleModuleTestSetup<ModuleType> >(*this,module->configuration_));
	}
	void FinalizeConfiguration(){
		module->Configure_();
	}
	boost::shared_ptr<I3Frame> processFrame(boost::shared_ptr<I3Frame> frame){
		if(!module)
			throw std::runtime_error("Test module not configured, or configuration not finalized");
		inbox->push_front(frame);
		module->Process_();
		return(collector->GetFrame());
	}
};

//======================================

//an arbitrary OM to use for testing, and basic calibration and status information for it
OMKey testOM(40,64);

//======================================
// Helper functions

//give the test OM an arbitrary position above the origin of the coordinate system
boost::shared_ptr<I3Geometry> getTestGeometry(){
	static boost::shared_ptr<I3Geometry> testGeometry;
	if(!testGeometry){
		testGeometry=boost::make_shared<I3Geometry>();
		testGeometry->omgeo[testOM].position=I3Position(300,400,2000);
		I3TankGeo tank;
		tank.position=testGeometry->omgeo[testOM].position;
		tank.omKeyList_.push_back(testOM);
		testGeometry->stationgeo[0].push_back(tank);
	}
	return(testGeometry);
}

boost::shared_ptr<I3Frame> frameWithData(float pulseTime, I3Position trackVertex, double trackTime, I3Direction trackDirection){
	boost::shared_ptr<I3Frame> frame(new I3Frame(I3Frame::Physics));
	frame->Put(getTestGeometry());
	
	I3RecoPulse pulse;
	pulse.SetTime(pulseTime);
	boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > pulses(new I3Map<OMKey,std::vector<I3RecoPulse> >);
	(*pulses)[testOM].push_back(pulse);
	frame->Put("TestPulses",pulses);
	
	boost::shared_ptr<I3Particle> track = boost::make_shared<I3Particle>(I3Particle::InfiniteTrack);
	track->SetPos(trackVertex);
	track->SetTime(trackTime);
	track->SetDir(trackDirection);
	frame->Put("TestTrack",track);
	
	return(frame);
}

//version without any pulse
boost::shared_ptr<I3Frame> frameWithData(I3Position trackVertex, double trackTime, I3Direction trackDirection){
	boost::shared_ptr<I3Frame> frame(new I3Frame(I3Frame::Physics));
	frame->Put(getTestGeometry());
	
	boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > pulses(new I3Map<OMKey,std::vector<I3RecoPulse> >);
	frame->Put("TestPulses",pulses);
	
	boost::shared_ptr<I3Particle> track = boost::make_shared<I3Particle>(I3Particle::InfiniteTrack);
	track->SetPos(trackVertex);
	track->SetTime(trackTime);
	track->SetDir(trackDirection);
	frame->Put("TestTrack",track);
	
	return(frame);
}

// version with pulse charge 
boost::shared_ptr<I3Frame> frameWithPulseCharge(float pulseCharge, float pulseTime, I3Position trackVertex, double trackTime, I3Direction trackDirection){
    boost::shared_ptr<I3Frame> frame(new I3Frame(I3Frame::Physics));
    frame->Put(getTestGeometry());
   
    I3RecoPulse pulse;
    pulse.SetTime(pulseTime);
    pulse.SetCharge(pulseCharge);
    boost::shared_ptr<I3Map<OMKey,std::vector<I3RecoPulse> > > pulses(new I3Map<OMKey,std::vector<I3RecoPulse> >);
    (*pulses)[testOM].push_back(pulse);
    frame->Put("TestPulses",pulses);
   
    boost::shared_ptr<I3Particle> track = boost::make_shared<I3Particle>(I3Particle::InfiniteTrack);
    track->SetPos(trackVertex);
    track->SetTime(trackTime);
    track->SetDir(trackDirection);
    frame->Put("TestTrack",track);
   
    return(frame);
} 

void markDOMBad(boost::shared_ptr<I3Frame> frame){
	boost::shared_ptr<I3Vector<OMKey> > badDOMs(new I3Vector<OMKey>);
	badDOMs->push_back(testOM);
	frame->Put("BadDomsList",badDOMs);
}

TEST_GROUP(Shield);

TEST(1_EarlyHitBeforeVertex){
	I3Context context;
	SingleModuleTestSetup<I3ShieldDataCollector> mTest(context);
	mTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack");
	boost::shared_ptr<I3Frame> frame=frameWithData(-5000,I3Position(100,-100,0),1000,I3Direction(constants::pi<double>()/4,0));
	frame=mTest.processFrame(frame);
	ENSURE((bool)frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults"));
	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > results = frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults");
	ENSURE(results->size()==1);
	ENSURE_DISTANCE(results->front().GetTimeResidual(),-810.96,.01);
	ENSURE_DISTANCE(results->front().GetDistance(),1367.48,.01);
}

TEST(2_LateHitBeforeVertex){
	I3Context context;
	SingleModuleTestSetup<I3ShieldDataCollector> mTest(context);
	mTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack");
	boost::shared_ptr<I3Frame> frame=frameWithData(-5000,I3Position(100,-100,0),1000,I3Direction(constants::pi<double>()/6,0));
	frame=mTest.processFrame(frame);
	ENSURE((bool)frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults"));
	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > results = frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults");
	ENSURE(results->size()==1);
	ENSURE_DISTANCE(results->front().GetTimeResidual(),111.064,.01);
	ENSURE_DISTANCE(results->front().GetDistance(),966.225,.01);
}

TEST(3_EarlyHitAfterVertex){
	I3Context context;
	SingleModuleTestSetup<I3ShieldDataCollector> mTest(context);
	mTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack");
	boost::shared_ptr<I3Frame> frame=frameWithData(2000,I3Position(100,-100,0),1000,I3Direction(2*constants::pi<double>()/3,0));
	frame=mTest.processFrame(frame);
	ENSURE((bool)frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults"));
	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > results = frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults");
	ENSURE(results->size()==1);
	ENSURE_DISTANCE(results->front().GetTimeResidual(),-1757.89,.01);
	ENSURE_DISTANCE(results->front().GetDistance(),1899.06,.01);
}

TEST(4_LateHitAfterVertex){
	I3Context context;
	SingleModuleTestSetup<I3ShieldDataCollector> mTest(context);
	mTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack");
	boost::shared_ptr<I3Frame> frame=frameWithData(7000,I3Position(100,-100,0),1000,I3Direction(5*constants::pi<double>()/6,0));
	frame=mTest.processFrame(frame);
	ENSURE((bool)frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults"));
	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > results = frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults");
	ENSURE(results->size()==1);
	ENSURE_DISTANCE(results->front().GetTimeResidual(),556.064,.01);
	ENSURE_DISTANCE(results->front().GetDistance(),1275.31,.01);
}

TEST(5_UnhitDOM){
	I3Context context;
	SingleModuleTestSetup<I3ShieldDataCollector> mTest(context);
	mTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack")("ReportUnhitDOMs",true);
	boost::shared_ptr<I3Frame> frame=frameWithData(I3Position(100,-100,0),1000,I3Direction(0,0));
	frame=mTest.processFrame(frame);
	ENSURE((bool)frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults_UnHit"));
	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > results = frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults_UnHit");
	ENSURE(results->size()==1);
	ENSURE_DISTANCE(results->front().GetDistance(),538.52,.01);
	
	//try again with the DOM actually hit, so there should be no unhit records
	frame=frameWithData(1000,I3Position(100,-100,0),1000,I3Direction(0,0));
	frame=mTest.processFrame(frame);
	ENSURE((bool)frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults_UnHit"));
	results = frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults_UnHit");
	ENSURE(results->empty());
}

TEST(6_UnhitBadDOM){
	I3Context context;
	SingleModuleTestSetup<I3ShieldDataCollector> mTest(context);
	mTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack")("ReportUnhitDOMs",true);
	boost::shared_ptr<I3Frame> frame=frameWithData(I3Position(100,-100,0),1000,I3Direction(0,0));
	markDOMBad(frame);
	frame=mTest.processFrame(frame);
	ENSURE((bool)frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults_UnHit"));
	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > results = frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults_UnHit");
	ENSURE(results->empty());
}

TEST(7_CurvatureVsFlat){
	std::vector<double> coefficients;
	coefficients.push_back(1);
	coefficients.push_back(1);
	coefficients.push_back(1);
	I3Context context;
	SingleModuleTestSetup<I3ShieldDataCollector> mFlatTest(context);
	mFlatTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack")("useCurvatureApproximation", false);
	boost::shared_ptr<I3Frame> frameFlat=frameWithData(2000,I3Position(100,-100,0),1000,I3Direction(2*constants::pi<double>()/3,0));
	frameFlat=mFlatTest.processFrame(frameFlat);
	SingleModuleTestSetup<I3ShieldDataCollector> mCurvedTest(context);
	mCurvedTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack")("useCurvatureApproximation", true)
	("coefficients", coefficients);
	boost::shared_ptr<I3Frame> frameCurved=frameWithData(2000,I3Position(100,-100,0),1000,I3Direction(2*constants::pi<double>()/3,0));
	frameCurved=mCurvedTest.processFrame(frameCurved);

	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > resultsFlat = frameFlat->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults");
	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > resultsCurved = frameCurved->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults");
	ENSURE(resultsFlat->front().GetTimeResidual()>resultsCurved->front().GetTimeResidual());
	double lateralDistance = resultsFlat->front().GetDistance();
	double flatTime = resultsFlat->front().GetTimeResidual();
	double curvedTime = resultsCurved->front().GetTimeResidual();
	double deltaTime = lateralDistance*lateralDistance+lateralDistance+1;
	ENSURE_DISTANCE(flatTime-curvedTime,deltaTime,0.1);

}


TEST(8_PulseCharge){
    I3Context context;
    SingleModuleTestSetup<I3ShieldDataCollector> mTest(context);
    mTest.Configure()("InputRecoPulses","TestPulses")("InputTrack","TestTrack")("ReportCharge",true);
    boost::shared_ptr<I3Frame> frame=frameWithPulseCharge(3.0, 10, I3Position(100,-100,0),1000,I3Direction(0,0));
    frame=mTest.processFrame(frame);
	ENSURE((bool)frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults"));
	boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > results = frame->Get<boost::shared_ptr<const I3Vector<I3ShieldHitRecord> > >("ShieldResults");
    float charge = results->front().GetCharge();
    ENSURE(charge==3.0);
}



