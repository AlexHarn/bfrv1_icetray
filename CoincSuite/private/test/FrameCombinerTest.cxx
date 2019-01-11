/**
 * $Id: FrameCombinerTest.cxx 104975 2013-06-03 18:46:34Z mzoll $
 * $Author: mzoll <marcel.zoll@fysdik.su.se> $
 * $Date: 2013-06-03 20:46:34 +0200 (Mon, 03 Jun 2013) $
 * $Revision: 104975 $
 *
 * Test the functionality of the FrameRegister Class
 */

#include <I3Test.h>

#include "CoincSuite/Modules/FrameCombiner.h"

#include "icetray/I3Frame.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

using namespace boost;
using namespace std;

FramePack CreateScenario() {
  FramePack packet;
  
  I3FramePtr frame;
  I3EventHeader eh = I3EventHeader();
  
  eh.SetRunID(0);
  eh.SetEventID(0);
  
  frame = boost::make_shared<I3Frame>(I3Frame::DAQ);

  frame->Put("I3EventHeader", boost::make_shared<I3EventHeader>(eh));
  frame->Put("splitSplitCount", boost::make_shared<I3Int>(2));
  frame->Put("splitReducedCount", boost::make_shared<I3Int>(0));
  packet.push_back(frame);
  
  frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  eh.SetSubEventStream("split");
  eh.SetSubEventID(0);
  frame->Put("I3EventHeader", boost::make_shared<I3EventHeader>(eh));
  packet.push_back(frame);

  frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  eh.SetSubEventStream("split");
  eh.SetSubEventID(1);
  frame->Put("I3EventHeader", boost::make_shared<I3EventHeader>(eh));
  packet.push_back(frame);

  frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  eh.SetSubEventStream("hypo");
  eh.SetSubEventID(0);
  frame->Put("I3EventHeader", boost::make_shared<I3EventHeader>(eh));
  packet.push_back(frame);
  
  //insert another frame and see if its correctly added
  frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  eh.SetSubEventStream("somethingcompletelydifferent");
  eh.SetSubEventID(0);
  frame->Put("I3EventHeader", boost::make_shared<I3EventHeader>(eh));
  packet.push_back(frame);
  
  //insert another frame, which is not one of the special GCDQPI type, and see if its correctly added
  frame = boost::make_shared<I3Frame>('F');
  packet.push_back(frame);

  //FramePacket is: Q Ps Ps Ph Pd F
  log_info("Test scenario created");  
  return packet;
}

//==========================================
TEST_GROUP(FrameCombiner);

TEST (TestCase1) { //create some artificial frames and push them into the frame register
  FramePack packet = CreateScenario(); //create test scenario

  CoincSuite::FrameRegister frameRegister_(packet, "split", "hypo"); //use the direct constructor
  
  ENSURE_EQUAL(frameRegister_.SplitName_, "split");
  ENSURE_EQUAL(frameRegister_.HypoName_, "hypo");
  
  ENSURE_EQUAL(frameRegister_.GetSplitCount(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetReducedCount(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetEffSplits(), (size_t)2);

  ENSURE_EQUAL(frameRegister_.GetNumberAllFrames(), (size_t)6);
  ENSURE_EQUAL(frameRegister_.GetNumberQFrames(), (size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberSplitFrames(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetNumberHypoFrames(), (size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberOtherFrames(), (size_t)2);

  
  //check if FrameRegister can be refilled and puts the frames where they belong
  frameRegister_.BuildFrameRegister(packet, "split", "hypo");

  ENSURE_EQUAL(frameRegister_.SplitName_, "split");
  ENSURE_EQUAL(frameRegister_.HypoName_, "hypo");
  
  ENSURE_EQUAL(frameRegister_.GetSplitCount(),(size_t)2);
  ENSURE_EQUAL(frameRegister_.GetReducedCount(),(size_t)0);
  ENSURE_EQUAL(frameRegister_.GetEffSplits(),(size_t)2);
  
  ENSURE_EQUAL(frameRegister_.GetNumberAllFrames(),(size_t)6);
  ENSURE_EQUAL(frameRegister_.GetNumberQFrames(),(size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberSplitFrames(),(size_t)2);
  ENSURE_EQUAL(frameRegister_.GetNumberHypoFrames(),(size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberOtherFrames(),(size_t)2);
}
  
TEST (TestCase2) { //see that frames can be arbitraily added to the stream
  FramePack packet = CreateScenario(); //create test scenario

  CoincSuite::FrameRegister frameRegister_(packet, "split", "hypo");
  
  I3FramePtr frame;
  
  //insert another splitframe
  frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  I3EventHeaderPtr eh = boost::make_shared<I3EventHeader>();
  eh->SetSubEventStream("split");
  eh->SetSubEventID(2);
  frame->Put("I3EventHeader", eh);
  ENSURE(frameRegister_.InsertFrame(frame), "Another SplitFrame correct added");
  
  ENSURE_EQUAL(frameRegister_.GetSplitCount(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetReducedCount(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetEffSplits(), (size_t)2);
  
  ENSURE_EQUAL(frameRegister_.GetNumberAllFrames(), (size_t)7);
  ENSURE_EQUAL(frameRegister_.GetNumberQFrames(), (size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberSplitFrames(), (size_t)3);
  ENSURE_EQUAL(frameRegister_.GetNumberHypoFrames(), (size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberOtherFrames(), (size_t)2);
  
  //insert another hypoframe
  frame = boost::make_shared<I3Frame>(I3Frame::Physics);
  eh->SetSubEventStream("hypo");
  eh->SetSubEventID(1);
  frame->Put("I3EventHeader", eh);
  ENSURE(frameRegister_.InsertFrame(frame), "Another HypoFrame correct added");
  
  ENSURE_EQUAL(frameRegister_.GetSplitCount(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetReducedCount(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetEffSplits(), (size_t)2);
  
  ENSURE_EQUAL(frameRegister_.GetNumberAllFrames(), (size_t)8);
  ENSURE_EQUAL(frameRegister_.GetNumberQFrames(), (size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberSplitFrames(), (size_t)3);
  ENSURE_EQUAL(frameRegister_.GetNumberHypoFrames(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetNumberOtherFrames(), (size_t)2);
};
  
TEST (TestCase3) { ///test if we can retrieve correctly frames
  FramePack packet = CreateScenario(); //create test scenario
  
  CoincSuite::FrameRegister frameRegister_(packet, "split", "hypo"); //use the direct constructor
  
  ENSURE_EQUAL(frameRegister_.RetrieveAllContent().size(),(size_t)6, "The expected number of Frames have been retieved back from the FrameRegister");
  
  //try to retrieve all frames in squence
  ENSURE_EQUAL(frameRegister_.RetrieveFrameSequence(true,true,true,true).size(),(size_t)6);
  //try to retrieve all frames execpt 'other' in squence
  ENSURE_EQUAL(frameRegister_.RetrieveFrameSequence(true,true,true,false).size(),(size_t)4);
  //try to retrieve all frames execpt 'hypo' in squence
  ENSURE_EQUAL(frameRegister_.RetrieveFrameSequence(true,true,false,true).size(),(size_t)5);
  //try to retrieve all frames execpt 'split' in squence
  ENSURE_EQUAL(frameRegister_.RetrieveFrameSequence(true,false,true,true).size(),(size_t)4);
  //try to retrieve all frames execpt 'Q' in squence
  ENSURE_EQUAL(frameRegister_.RetrieveFrameSequence(false,true,true,true).size(),(size_t)5);
};


TEST (TestCase4) { ///test if we can erase frames correctly; remebmber the frame-index is [0...n]
  FramePack packet = CreateScenario();
  
  CoincSuite::FrameRegister frameRegister_(packet, "split", "hypo"); //use the direct constructor
  ENSURE_EQUAL(frameRegister_.GetNumberAllFrames(), (size_t)6);
  
  ENSURE(! frameRegister_.RemoveFrameAtPosition(6)); // there is nothing at this position, because [0...5] are the 6 elements

  ENSURE(frameRegister_.RemoveFrameAtPosition(5)); // this removes the 'F' frame

  ENSURE_EQUAL(frameRegister_.GetSplitCount(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetReducedCount(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetEffSplits(), (size_t)2);
  
  ENSURE_EQUAL(frameRegister_.GetNumberAllFrames(), (size_t)5);
  ENSURE_EQUAL(frameRegister_.GetNumberQFrames(), (size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberSplitFrames(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetNumberHypoFrames(), (size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberOtherFrames(), (size_t)1);
  
  ENSURE(frameRegister_.RemoveFrameAtPosition(3)); // this removes the 'P'_hypo frame

  ENSURE_EQUAL(frameRegister_.GetSplitCount(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetReducedCount(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetEffSplits(), (size_t)2);
  
  ENSURE_EQUAL(frameRegister_.GetNumberAllFrames(), (size_t)4);
  ENSURE_EQUAL(frameRegister_.GetNumberQFrames(), (size_t)1);
  ENSURE_EQUAL(frameRegister_.GetNumberSplitFrames(), (size_t)2);
  ENSURE_EQUAL(frameRegister_.GetNumberHypoFrames(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetNumberOtherFrames(), (size_t)1);
}

TEST (TestCase5) { /// test if all objects are purged for real
  FramePack packet = CreateScenario(); //create test scenario
  
  CoincSuite::FrameRegister frameRegister_(packet, "split", "hypo"); //use the direct constructor
  
  frameRegister_.Clear(); // reset everything : purge all objects and all variables
  
  ENSURE_EQUAL(frameRegister_.SplitName_ , "");
  ENSURE_EQUAL(frameRegister_.HypoName_ , "");
  
  ENSURE_EQUAL(frameRegister_.GetSplitCount(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetReducedCount(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetEffSplits(), (size_t)0);
  
  ENSURE_EQUAL(frameRegister_.GetNumberAllFrames(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetNumberQFrames(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetNumberSplitFrames(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetNumberHypoFrames(), (size_t)0);
  ENSURE_EQUAL(frameRegister_.GetNumberOtherFrames(), (size_t)0);
}


TEST (TestCase6) { ///try some direct and indirect access
  FramePack packet = CreateScenario(); //create test scenario
  
  CoincSuite::FrameRegister frameRegister_(packet, "split", "hypo"); //use the direct constructor
  
  PosFrame pf;
  
  pf = frameRegister_.FindCorrIndices("Q", 0);
  ENSURE_EQUAL(pf.first, (size_t)0);
  
  pf = frameRegister_.FindCorrIndices("split", 0);
  ENSURE_EQUAL(pf.first, (size_t)1);
  
  pf = frameRegister_.FindCorrIndices("split", 1);
  ENSURE_EQUAL(pf.first, (size_t)2);
  
  pf = frameRegister_.FindCorrIndices("hypo", 0);
  ENSURE_EQUAL(pf.first, (size_t)3);
  
  pf = frameRegister_.FindCorrIndices("somethingcompletelydifferent", 0);
  ENSURE_EQUAL(pf.first, (size_t)4);
  
  //remeber, we can not retireve the because there is no I3EventHeader, but that is OK
  
  //go the reverse way and get the frame at position
  pf = frameRegister_.GetFrameAtPosition(0);
  ENSURE_EQUAL(pf.second->GetStop(), I3Frame::DAQ);
  
  pf = frameRegister_.GetFrameAtPosition(1);
  ENSURE(pf.second->GetStop() == I3Frame::Physics);
  I3EventHeaderConstPtr eh = pf.second->Get<I3EventHeaderConstPtr>("I3EventHeader");
  ENSURE_EQUAL(eh->GetSubEventStream(), "split");
  ENSURE_EQUAL(eh->GetSubEventID(), (size_t)0);
  
  pf = frameRegister_.GetFrameAtPosition(2);
  ENSURE(pf.second->GetStop() == I3Frame::Physics);
  eh = pf.second->Get<I3EventHeaderConstPtr>("I3EventHeader");
  ENSURE_EQUAL(eh->GetSubEventStream(), "split");
  ENSURE_EQUAL(eh->GetSubEventID(), (size_t)1);
  
  pf = frameRegister_.GetFrameAtPosition(3);
  ENSURE(pf.second->GetStop() == I3Frame::Physics);
  eh = pf.second->Get<I3EventHeaderConstPtr>("I3EventHeader");
  ENSURE_EQUAL(eh->GetSubEventStream(), "hypo");
  ENSURE_EQUAL(eh->GetSubEventID(), (size_t)0);
  
  pf = frameRegister_.GetFrameAtPosition(4);
  ENSURE(pf.second->GetStop() == I3Frame::Physics);
  eh = pf.second->Get<I3EventHeaderConstPtr>("I3EventHeader");
  ENSURE_EQUAL(eh->GetSubEventStream(), "somethingcompletelydifferent");
  ENSURE_EQUAL(eh->GetSubEventID(), (size_t)0);
  
  pf = frameRegister_.GetFrameAtPosition(5);
  ENSURE_EQUAL(pf.second->GetStop(), 'F');
}

