/**
 * \file HiveSplitterTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id:$
 * \version $Revision:$
 * \date $Date:$
 * \author mzoll <marcel.zoll@icecube.wisc.edu>
 *
 * Adapted code from Chris Weaver <chris.weaver@icecube.wisc.edu> of TopologicalSplitter
 * This is really not a unit test but a regression test, ensuring that this project's splitter continues
 * to produce exactly the same output as the original HiveSplitter. If any change is made to the
 * algorithm this test will fail, and if the change is intentional the test data will need to be regenerated
 * to put this test back on track.
 */

#include <I3Test.h>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <icetray/I3Tray.h>
#include <icetray/I3Units.h>
#include <icetray/I3PacketModule.h>
#include <dataclasses/physics/I3EventHeader.h>
#include <dataclasses/physics/I3RecoPulse.h>

#include <vector>

///An I3Module which compares that two different splitters produce exactly the same results
class SplitVerifier : public I3PacketModule{
private:
	unsigned int packetCount;
	std::string originalName;
	std::string currentName;
	std::string pulsesName;
public:
	SplitVerifier(const I3Context& context):
	I3PacketModule(context,I3Frame::DAQ),
		packetCount(0),
		originalName("OriginalHiveSplitter"),
		currentName("HiveSplitter"),
		pulsesName("MaskedOfflinePulses")
		{
		AddParameter("OriginalName", "Name of the original sub-eventstream", originalName);
		AddParameter("CurrentName", "Name of the reprocessed sub-eventstream", currentName);
		AddParameter("PulsesName", "Name of the Pulses", pulsesName);
		AddOutBox("OutBox");
	}
	virtual void Configure(){
		GetParameter("OriginalName", originalName);
		GetParameter("CurrentName", currentName);
		GetParameter("PulsesName", pulsesName);
	}

	virtual void FramePacket(std::vector<I3FramePtr> &packet){
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
	}
};

I3_MODULE(SplitVerifier);

TEST_GROUP(HiveSplitterRegression);

TEST(ChangeIsTheEnemy){
  I3Tray tray;

  boost::python::import("icecube.dataio");
  tray.AddModule("I3Reader","Reader")
          ("Filename",boost::lexical_cast<std::string>(getenv("I3_TESTDATA"))
          +"/HiveSplitter/HiveSplitter_testcase.i3.bz2");

  double _icringlimits_[6] = {300., 300., 272.7, 272.7, 165.8, 165.8};
  std::vector<double> icringlimits(_icringlimits_, _icringlimits_+6);
  double _dcringlimits_[6] = {150., 150., 131.5, 131.5, 40.8, 40.8};
  std::vector<double> dcringlimits(_dcringlimits_, _dcringlimits_+6);
  double _pinguringlimits_[8] = {150., 150.0, 144.1, 144.1, 124.7, 124.7, 82.8, 82.8};
  std::vector<double> pinguringlimits(_pinguringlimits_, _pinguringlimits_+8);
  tray.AddModule("I3HiveSplitter","HiveSplitter")
  ("InputName", "OfflinePulses")
  ("OutputName", "MaskedOfflinePulses")
  ("Multiplicity", 4)
  ("TimeWindow", 2000*I3Units::ns)
  ("TimeConeMinus", 1000*I3Units::ns)
  ("TimeConePlus", 1000*I3Units::ns)
  ("SingleDenseRingLimits", icringlimits)
  ("DoubleDenseRingLimits", dcringlimits)
  ("TrippleDenseRingLimits", pinguringlimits)
  ("SaveSplitCount", true);

  tray.AddModule("SplitVerifier","SplitVerifier");

  

  tray.Execute();
  
}
