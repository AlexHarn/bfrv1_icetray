/**
 * \file HitSortingTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * Unit test to test the robustness of HitSorting as it is implemented in HiveSplitter/HitSorting.h
 */

#include "IceHive/HitSorting.h"
#include "TestHelpers.h"

#include <I3Test.h>

#include <boost/make_shared.hpp>

using namespace HitSorting;

typedef std::deque<HitObject<I3RecoPulse> > RecoHitObjDeque;


//Do testing on the example of an I3RecoPulse

TEST_GROUP(HitSorting);

///Forth and back conververt between a OMKeyMap an HitObjects
TEST(HitConvertions){
  I3RecoPulseSeriesMap recoMap;
  recoMap[OMKey(1,1)].push_back(MakeRecoPulse(0., 1.));
  recoMap[OMKey(1,1)].push_back(MakeRecoPulse(1., 2.));
  recoMap[OMKey(1,2)].push_back(MakeRecoPulse(0., 4.));

  RecoHitObjDeque hitObjs = OMKeyMap_To_HitObjects<I3RecoPulse, RecoHitObjDeque>(recoMap);

  ENSURE(hitObjs.size()==3, "Expected number of entries for extracted hits");
  
  I3RecoPulseSeriesMap recoMap_cycled = HitObjects_To_OMKeyMap<I3RecoPulse>(hitObjs);
  
  I3RecoPulseSeriesMap::const_iterator recoMap_cycled_iter = recoMap_cycled.begin();
  ENSURE(recoMap_cycled.size()==2 && recoMap_cycled_iter->second.size()==2, "Format of cycled Map is as expected");
  ENSURE(recoMap_cycled_iter->second[0] == recoMap_cycled_iter->second[0], "First Pulse from the cycled Map is identical to the original one and at the same position");
  ENSURE( recoMap_cycled_iter->second[1] == recoMap_cycled_iter->second[1], "Second Pulse from the cycled Map is identical to the original one and at the same position");
  recoMap_cycled_iter++; //next DOM
  ENSURE( recoMap_cycled_iter->second[0] == recoMap_cycled_iter->second[0], "Third Pulses from the cycled Map is identical to the original one and at the same position");
};


/// Extract and backconvert Hits by a HitFacility
TEST(OMKeyMap_HitFacility) {
  I3RecoPulseSeriesMap recoMap;
  recoMap[OMKey(1,1)].push_back(MakeRecoPulse(0., 1.));
  recoMap[OMKey(1,1)].push_back(MakeRecoPulse(1., 2.));
  recoMap[OMKey(1,2)].push_back(MakeRecoPulse(0., 4.));

  I3FramePtr frame = boost::make_shared<I3Frame>(I3Frame::Physics);

  frame->Put("KEY", boost::make_shared<I3RecoPulseSeriesMap>(recoMap));
  
  const OMKeyMap_HitFacility<I3RecoPulse> hf(frame, "KEY");
  
  HitDeque hits = hf.GetHits<HitDeque>();
  
  ENSURE(hits.size()==3, "Expected number of entries for extracted hits");
  
  const I3RecoPulseSeriesMap cycledMap = hf.MapFromHits(hits);
  ENSURE(cycledMap.size() == recoMap.size());
};


TEST(I3RecoPulseSeriesMap_HitFacility) {
  I3RecoPulseSeriesMap recoMap;
  recoMap[OMKey(1,1)].push_back(MakeRecoPulse(0., 1.));
  recoMap[OMKey(1,1)].push_back(MakeRecoPulse(1., 2.));
  recoMap[OMKey(1,2)].push_back(MakeRecoPulse(0., 4.));

  I3FramePtr frame = boost::make_shared<I3Frame>(I3Frame::Physics);

  frame->Put("KEY", boost::make_shared<I3RecoPulseSeriesMap>(recoMap));
  
  I3RecoPulseSeriesMap recoMap_sub;

  recoMap_sub.insert(*recoMap.begin());
  
  
  const I3RecoPulseSeriesMap_HitFacility hf(frame, "KEY");
  
  HitDeque hits = hf.GetHits<HitDeque>();
  
  ENSURE(hits.size()==3, "Expected number of entries for extracted hits");
  
  const I3RecoPulseSeriesMap cycledMap = hf.MapFromHits(hits);
  ENSURE(cycledMap.size() == recoMap.size());

  const I3RecoPulseSeriesMapMask cycledMask = hf.MaskFromHits(hits);
  const I3RecoPulseSeriesMap appliedMask = *(cycledMask.Apply(*frame));  
  ENSURE(appliedMask.size() == recoMap.size());
};


