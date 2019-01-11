#include "I3Test.h"

#include "finiteReco/I3FiniteCalc.h"
#include "icetray/open.h"
#include "icetray/I3Frame.h"
#include "phys-services/I3Calculator.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/make_shared.hpp>

TEST_GROUP(finiteCalsTest);

TEST(I3FiniteCalc)
{ 
  std::vector<std::string> skip;
  boost::iostreams::filtering_istream ifs;
  std::string i3testdata(getenv("I3_TESTDATA"));
  I3::dataio::open(ifs, i3testdata+"/sim/GeoCalibDetectorStatus_2012.56062_V0.i3.gz");
  
  I3Frame frame;
  I3Frame::Stream s;;
  do{
    frame.load(ifs, skip);
    s = frame.GetStop();
    ENSURE(s!=I3Frame::None);
  }while(s!=I3Frame::Geometry);
  
  const I3Geometry& geometry = frame.Get<I3Geometry>();
  
  int strings[9] = {21,29,30,38,39,40,49,50,59};
  I3RecoPulseSeriesMap pulsemap;
  for(int i=0;i<9;i++){
    I3RecoPulseSeries pulses;
    I3RecoPulse pulse;
    pulse.SetTime(1000 + i);
    pulses.push_back(pulse);
    OMKey key(strings[i],25+i);
    pulsemap.insert(make_pair(key,pulses));
  }

  I3Particle track;
  track.SetPos(0,0,0);
  track.SetShape(I3Particle::InfiniteTrack);
  track.SetDir(cos(30.*I3Units::degree),sin(30.*I3Units::degree),0);

  I3RecoPulseSeriesMapConstPtr pulsemapPtr(boost::make_shared<I3RecoPulseSeriesMap>(pulsemap));
  I3FiniteCalc finiteCalc(geometry,track,pulsemapPtr,1*I3Units::km);
  
  I3FiniteCalc::CylinderGeo myDetector = finiteCalc.FindDetectorEnds();
  ENSURE(std::isnan(myDetector.timeStart) && std::isnan(myDetector.timeStopp),"Detector properties depend on time. That should not.");
  ENSURE(myDetector.distStopp > myDetector.distStart,"The detector should first start and end afterwards");
  ENSURE(myDetector.distStopp - myDetector.distStart > 125,"The detector size is more than the string spacing");
  ENSURE(I3Calculator::DistanceAlongTrack(track,geometry.omgeo.find(myDetector.omStart)->second.position) == myDetector.distStart,"Bad distance calculation (start)");
  ENSURE(I3Calculator::DistanceAlongTrack(track,geometry.omgeo.find(myDetector.omStopp)->second.position) == myDetector.distStopp,"Bad distance calculation (stop)");

  I3FiniteCalc::CylinderGeo myEvent =  finiteCalc.FindEventEnds();
  ENSURE(!std::isnan(myEvent.timeStart),"Pulses have no time"); 
  ENSURE(!std::isnan(myEvent.timeStopp),"Pulses have no time");
  ENSURE(myEvent.distStopp > myEvent.distStart,"The event should first start and end afterwards");
  ENSURE(I3Calculator::DistanceAlongTrack(track,geometry.omgeo.find(myEvent.omStart)->second.position) == myEvent.distStart,"Bad distance calculation (start)");
  ENSURE(I3Calculator::DistanceAlongTrack(track,geometry.omgeo.find(myEvent.omStopp)->second.position) == myEvent.distStopp,"Bad distance calculation (stop)"); 
  
  I3Position trueStart = I3Calculator::ClosestApproachPosition(track,geometry.omgeo.find(myEvent.omStart)->second.position); 
  I3Position startPos = finiteCalc.GetEventStart();
  ENSURE(fabs(trueStart.GetX() - startPos.GetX()) < 1e-6,"Bad start position calculation");
  ENSURE(fabs(trueStart.GetY() - startPos.GetY()) < 1e-6,"Bad start position calculation");
  ENSURE(fabs(trueStart.GetZ() - startPos.GetZ()) < 1e-6,"Bad start position calculation");

  ENSURE(myDetector.distStopp >= myEvent.distStopp,"Max should be larger than normal");
  ENSURE(myDetector.distStart <= myEvent.distStart,"Max should be larger than normal");

  I3FiniteCalc::CylinderGeo myEvent2 =  finiteCalc.FindEventEndsMax();
  ENSURE(!std::isnan(myEvent2.timeStart),"Pulses have no time"); 
  ENSURE(!std::isnan(myEvent2.timeStopp),"Pulses have no time");
  ENSURE(myEvent2.distStopp > myEvent2.distStart,"The event should first start and end afterwards");
  ENSURE(I3Calculator::DistanceAlongTrack(track,geometry.omgeo.find(myEvent2.omStart)->second.position) == myEvent2.distStart,"Bad distance calculation (start)");
  ENSURE(I3Calculator::DistanceAlongTrack(track,geometry.omgeo.find(myEvent2.omStopp)->second.position) == myEvent2.distStopp,"Bad distance calculation (stop)");
  
  ENSURE(myEvent2.distStopp >= myEvent.distStopp,"Max should be larger than normal");
  ENSURE(myEvent2.distStart <= myEvent.distStart,"Max should be larger than normal");
}
