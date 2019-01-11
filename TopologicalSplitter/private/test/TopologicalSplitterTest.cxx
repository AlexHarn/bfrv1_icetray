/**
 * \file TopologicalSplitterTest.cxx
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id:$
 * \version $Revision:$
 * \date $Date:$
 * \author Chris Weaver <chris.weaver@icecube.wisc.edu>
 *
 * This is really not a unit test but a regression test, ensuring that this project's splitter continues
 * to produce exactly the same output as the original TTriggerSplitter. If any change is made to the 
 * algorithm this test will fail, and if the change is intentional the test data will need to be regenerated
 * to put this test back on track. 
 */

#include <I3Test.h>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <icetray/I3Tray.h>
#include <icetray/I3Units.h>
#include <icetray/I3PacketModule.h>
#include <icetray/I3Int.h>
#include <dataclasses/physics/I3EventHeader.h>
#include <dataclasses/physics/I3RecoPulse.h>

///An I3Module which compares that two different splitters produce exactly the same results
class SplitVerifier : public I3PacketModule{
private:
	unsigned int packetCount;
public:
	SplitVerifier(const I3Context& context):
	I3PacketModule(context,I3Frame::DAQ),packetCount(0){
		AddOutBox("OutBox");
	}
	virtual void Configure(){}
	
	virtual void FramePacket(std::vector<I3FramePtr> &packet){
		const std::string originalName = "OriginalTTrigger";
		const std::string currentName = "TopologicalSplitter";
		const std::string pulsesName = "TTPulses";
		std::vector<I3FramePtr> originalSplit;
		std::vector<I3FramePtr> currentSplit;
		packetCount++;
		
		//collect frames for comparison
		for(std::vector<I3FramePtr>::const_iterator frame=packet.begin(), end=packet.end(); frame!=end; frame++){
			PushFrame(*frame);
			if((*frame)->GetStop()==I3Frame::Physics){
				if((*frame)->Get<I3EventHeader>().GetSubEventStream() == originalName)
					originalSplit.push_back(*frame);
				else if((*frame)->Get<I3EventHeader>().GetSubEventStream() == currentName)
					currentSplit.push_back(*frame);
			}
		}
		
		//first, there need to be the same number of frames
		if(originalSplit.size()!=currentSplit.size())
			throw std::runtime_error("Regression: Different numbers of split frames in packet "
									 +boost::lexical_cast<std::string>(packetCount)+": original="
									 +boost::lexical_cast<std::string>(originalSplit.size())+", current="
									 +boost::lexical_cast<std::string>(currentSplit.size()));
		//next, step through the split frames in parallel and make sure that the the split pulse series are identical
		unsigned int frameCount=0;
		for(std::vector<I3FramePtr>::iterator old=originalSplit.begin(), cur=currentSplit.begin(), end=originalSplit.end();
		  old!=end; old++,cur++,frameCount++){
			I3RecoPulseSeriesMapConstPtr oldPulses=(**old).Get<I3RecoPulseSeriesMapConstPtr>(pulsesName);
			I3RecoPulseSeriesMapConstPtr curPulses=(**cur).Get<I3RecoPulseSeriesMapConstPtr>(pulsesName);
			
			//should be the same number of DOMs present in both maps
			if(oldPulses->size()!=curPulses->size())
				throw std::runtime_error("Regression: Different numbers of hit DOMs in packet "
										 +boost::lexical_cast<std::string>(packetCount)+", split frame "
										 +boost::lexical_cast<std::string>(frameCount)+": original="
										 +boost::lexical_cast<std::string>(oldPulses->size())+", current="
										 +boost::lexical_cast<std::string>(curPulses->size()));
			
			//iterate over all of the DOMs and ensure that the same DOMs are hit, and have the same pulses
			for(I3RecoPulseSeriesMap::const_iterator oldDOM=oldPulses->begin(), curDOM=curPulses->begin(), end=oldPulses->end();
			  oldDOM!=end; oldDOM++,curDOM++){
				//check that both pulse seires are for the same DOM
				if(oldDOM->first!=curDOM->first)
					throw std::runtime_error("Regression: Mismatched DOMs in packet "
											 +boost::lexical_cast<std::string>(packetCount)+", split frame "
											 +boost::lexical_cast<std::string>(frameCount)+": original="
											 +boost::lexical_cast<std::string>(oldDOM->first)+", current="
											 +boost::lexical_cast<std::string>(curDOM->first));
				
				//check that the DOM has the same number of pulses in both series
				if(oldDOM->second.size()!=curDOM->second.size())
					throw std::runtime_error("Regression: Different numbers of hits on DOM "
											 +boost::lexical_cast<std::string>(curDOM->first)+" in packet "
											 +boost::lexical_cast<std::string>(packetCount)+", split frame "
											 +boost::lexical_cast<std::string>(frameCount)+": original="
											 +boost::lexical_cast<std::string>(oldDOM->second.size())+", current="
											 +boost::lexical_cast<std::string>(curDOM->second.size()));
				
				//iterate over all of the pulses and check that they are the same
				unsigned int pulseCount=0;
				for(I3RecoPulseSeries::const_iterator oldPulse=oldDOM->second.begin(), curPulse=curDOM->second.begin(), end=oldDOM->second.end();
				  oldPulse!=end; oldPulse++,curPulse++,pulseCount++){
					if(!(*oldPulse==*curPulse))
						throw std::runtime_error("Regression: Pulse "+boost::lexical_cast<std::string>(pulseCount)+" on DOM "
												 +boost::lexical_cast<std::string>(curDOM->first)+" in packet "
												 +boost::lexical_cast<std::string>(packetCount)+", split frame "
												 +boost::lexical_cast<std::string>(frameCount)+" does not match");
				}
			}
		}
		
		//finally, if a split count was written, check that it's right
		if(packet.front()->Has("TopologicalSplitterSplitCount")){
			const I3Int& storedCount=packet.front()->Get<I3Int>("TopologicalSplitterSplitCount");
			ENSURE(storedCount.value>=0,"Stored frame count should not be negative");
			ENSURE(currentSplit.size()==(size_t)storedCount.value,"Stored frame count should match number of emitted frames");
		}
	}
};

I3_MODULE(SplitVerifier);

TEST_GROUP(TopologicalSplitter);

TEST(ChangeIsTheEnemy){
	I3Tray tray;
	
	boost::python::import("icecube.dataio");
	tray.AddModule("I3Reader","Reader")
	("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
	            +"/Topological_Splitter_testcase.i3.bz2");
	
	tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
	("Multiplicity", 4)
	("TimeWindow", 4000*I3Units::ns)
	("XYDist", 300*I3Units::m)
	("ZDomDist", 20)
	("TimeCone", 800*I3Units::ns)
	("OutputName", "TTPulses")
	("InputName", "SRTOfflinePulses")
	("SaveSplitCount",true);
	
	tray.AddModule("SplitVerifier","Scrutineer");
	
	
    
	tray.Execute();
	
}

TEST(BadSettings){
	boost::python::import("icecube.dataio");
	
	//bad multiplicity
	try{
		I3Tray tray;
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 0)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses");
		
		tray.Execute(1);
		FAIL("Invalid multiplicities should be detected");
	} catch(std::runtime_error&){/*squash*/}
	
	//bad time window
	try{
		I3Tray tray;
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", -2*I3Units::day)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses");
		
		tray.Execute(1);
		FAIL("Invalid time windows should be detected");
	} catch(std::runtime_error&){/*squash*/}
	
	//bad horizontal distance
	try{
		I3Tray tray;
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", -68*I3Units::feet)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses");
		
		tray.Execute(1);
		FAIL("Invalid xy distances should be detected");
	} catch(std::runtime_error&){/*squash*/}
	
	//bad time tolerance
	try{
		I3Tray tray;
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", -6.2*I3Units::hour)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses");
		
		tray.Execute(1);
		FAIL("Invalid 'time cone' values should be detected");
	} catch(std::runtime_error&){/*squash*/}
}

//destroys all frames on one stream
class StreamEater : public I3Module{
public:
	StreamEater(const I3Context& context):I3Module(context){
		AddParameter("Stream","Stream to eliminate",I3Frame::Physics);
		AddOutBox("OutBox");
	}
	void Configure(){
		I3Frame::Stream stream;
		GetParameter("Stream",stream);
		Register(stream,&StreamEater::Eat);
	}
	void Eat(boost::shared_ptr<I3Frame>){
		//gulp
	}
};
I3_MODULE(StreamEater);

class PulseTweak : public I3Module{
private:
	std::string pulsesName;
public:
	PulseTweak(const I3Context& context):I3Module(context){
		AddParameter("Pulses","Pulses to modify",pulsesName);
		AddOutBox("OutBox");
	}
	void Configure(){
		GetParameter("Pulses",pulsesName);
	}
	void DAQ(boost::shared_ptr<I3Frame> frame){
		boost::shared_ptr<const I3RecoPulseSeriesMap> oldPulses = frame->Get<boost::shared_ptr<const I3RecoPulseSeriesMap> >(pulsesName);
		boost::shared_ptr<I3RecoPulseSeriesMap> newPulses(new I3RecoPulseSeriesMap(*oldPulses));
		//Add a pulse on a DOM that definitely isn' in the geometry.
		//The other properties of the pulse don't really matter.
		newPulses->insert(std::make_pair(OMKey(46000,15),I3RecoPulseSeries(1,I3RecoPulse())));
		frame->Delete(pulsesName);
		frame->Put(pulsesName,newPulses);
		PushFrame(frame);
	}
};
I3_MODULE(PulseTweak);

TEST(BadInput){
	//no geometry
	try{
		I3Tray tray;
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("StreamEater","Ammit")("Stream",I3Frame::Geometry);
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses");
		
		tray.Execute(5);
		FAIL("Lack of geometry information should be detected");
	} catch(std::runtime_error&){/*squash*/}
	
	//no input pulses
	{
		I3Tray tray;
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		//eliminate existing P frames so it's easy to tell that no more are emitted
		tray.AddModule("StreamEater","Ammit")("Stream",I3Frame::Physics);
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "NonExistantPulses");
		
		//The only things which should appear here are the G frame and Q frames
		tray.AddModule("CountFrames","Counter")
		("DAQ",100)
		("Physics",0)
		("Geometry",1)
		("Calibration",0)
		("DetectorStatus",0);
		
		tray.Execute();
	}
	
	//pulse on unknown DOM
	try{
		I3Tray tray;
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("PulseTweak")("Pulses","SRTOfflinePulses");
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses");
		
		tray.Execute(5);
		FAIL("Pulses on DOMs not in the geometry should stop processing");
	} catch(std::runtime_error&){/*squash*/}
}

TEST(DeprecatedInterface){
	boost::python::import("icecube.dataio");
	
	//Using the old name should work,
	//as long as all deprecated parameters are set to their defaults
	{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("TTriggerSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("Topo",1)
		("LCSpan",-1)
		("LCWindow",0)
		("CBWindow",0);
		
		tray.AddModule("SplitVerifier","Scrutineer");
		tray.Execute();
		
	}
	
	//Setting any of the deprecated parameters to a non-default value should
	//trigger an error message
	try{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("TTriggerSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("Topo",2)
		("LCSpan",-1)
		("LCWindow",0)
		("CBWindow",0);
		
		tray.AddModule("SplitVerifier","Scrutineer");
		tray.Execute(5);
		
		FAIL("Only Topo=1 should be allowed");
	} catch(std::runtime_error&){/*squash*/}
	
	try{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("TTriggerSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("Topo",1)
		("LCSpan",2)
		("LCWindow",0)
		("CBWindow",0);
		
		tray.AddModule("SplitVerifier","Scrutineer");
		tray.Execute(5);
		
		FAIL("Only LCSpan=-1 should be allowed");
	} catch(std::runtime_error&){/*squash*/}
	
	try{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("TTriggerSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("Topo",1)
		("LCSpan",-1)
		("LCWindow",38*I3Units::ns)
		("CBWindow",0);
		
		tray.AddModule("SplitVerifier","Scrutineer");
		tray.Execute(5);
		
		FAIL("Only LCWindow=0 should be allowed");
	} catch(std::runtime_error&){/*squash*/}
	
	try{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("TTriggerSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("Topo",1)
		("LCSpan",-1)
		("LCWindow",0)
		("CBWindow",2*I3Units::day);
		
		tray.AddModule("SplitVerifier","Scrutineer");
		tray.Execute(5);
		
		FAIL("Only CBWindow=0 should be allowed");
	} catch(std::runtime_error&){/*squash*/}
}

class PseudoSplitChecker : public I3Module{
private:
	std::string countName;
	std::string pulsesName;
public:
	PseudoSplitChecker(const I3Context& context):I3Module(context){
		AddParameter("Count","The name of the reference count");
		AddParameter("Pulses","The base pulse series name for which to look");
	}
	void Configure(){
		GetParameter("Count",countName);
		GetParameter("Pulses",pulsesName);
	}
	void Physics(boost::shared_ptr<I3Frame> frame){
		const I3Int& reference=frame->Get<I3Int>(countName);
		for(unsigned int i=0; true; i++){
			std::ostringstream ss;
			ss << pulsesName << i;
			boost::shared_ptr<const I3RecoPulseSeriesMap> pulses = frame->Get<boost::shared_ptr<const I3RecoPulseSeriesMap> >(ss.str());
			if(!pulses){
				ENSURE(reference.value>=0, "Reference split count should not be negative");
				ENSURE(i==(unsigned)reference.value, "Number of pulses series should match expected number of splits");
				break;
			}
			//We don't actually check that the splitting is correct,
			//but this should be fairly safe, since it is done by code covered
			//in multiple other tests
		}
	}
};
I3_MODULE(PseudoSplitChecker);

TEST(Deprecated_NonSplitter_Interface){
	boost::python::import("icecube.dataio");
	
	{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("SaveSplitCount",true);
		//eliminate existing P frames so the test output is easy to find
		tray.AddModule("StreamEater","Ammit")("Stream",I3Frame::Physics);
		tray.AddModule("I3NullSplitter","NullSplit");
		tray.AddModule("ttrigger<I3RecoPulse>","ttrigger")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("Topo",1)
		("LCSpan",-1)
		("LCWindow",0)
		("CBWindow",0);
		tray.AddModule("PseudoSplitChecker")
		("Count","TopologicalSplitterSplitCount")
		("Pulses","TTPulses");
		
		tray.Execute();
	}
	
	//using the old "InputNames" parameter should work for a single input name
	{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		tray.AddModule("I3TopologicalSplitter","TopologicalSplitter")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("SaveSplitCount",true);
		//eliminate existing P frames so the test output is easy to find
		tray.AddModule("StreamEater","Ammit")("Stream",I3Frame::Physics);
		tray.AddModule("I3NullSplitter","NullSplit");
		tray.AddModule("ttrigger<I3RecoPulse>","ttrigger")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputNames", std::vector<std::string>(1,"SRTOfflinePulses"))
		("Topo",1)
		("LCSpan",-1)
		("LCWindow",0)
		("CBWindow",0);
		tray.AddModule("PseudoSplitChecker")
		("Count","TopologicalSplitterSplitCount")
		("Pulses","TTPulses");
		
		tray.Execute();
	}
	
	//but it should not work for more than one
	try{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		//eliminate existing P frames so it's easy to tell what changes
		tray.AddModule("StreamEater","Ammit")("Stream",I3Frame::Physics);
		tray.AddModule("I3NullSplitter","NullSplit");
		tray.AddModule("ttrigger<I3RecoPulse>","ttrigger")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		//Note 2 copies of pulse name on next line
		("InputNames", std::vector<std::string>(2,"SRTOfflinePulses"))
		("Topo",1)
		("LCSpan",-1)
		("LCWindow",0)
		("CBWindow",0);
		
		tray.Execute();
	} catch(std::runtime_error&){/*squash*/}
	
	//Setting both InputName and InputNames should be rejected
	try{
		I3Tray tray;
		
		tray.AddModule("I3Reader","Reader")
		("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
		 +"/Topological_Splitter_testcase.i3.bz2");
		
		//eliminate existing P frames so it's easy to tell what changes
		tray.AddModule("StreamEater","Ammit")("Stream",I3Frame::Physics);
		tray.AddModule("I3NullSplitter","NullSplit");
		tray.AddModule("ttrigger<I3RecoPulse>","ttrigger")
		("Multiplicity", 4)
		("TimeWindow", 4000*I3Units::ns)
		("XYDist", 300*I3Units::m)
		("ZDomDist", 20)
		("TimeCone", 800*I3Units::ns)
		("OutputName", "TTPulses")
		("InputName", "SRTOfflinePulses")
		("InputNames", std::vector<std::string>(1,"SRTOfflinePulses"))
		("Topo",1)
		("LCSpan",-1)
		("LCWindow",0)
		("CBWindow",0);
		
		tray.Execute();
	} catch(std::runtime_error&){/*squash*/}
}
