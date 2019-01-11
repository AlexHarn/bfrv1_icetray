/**
 * \file HitSortingTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id:$
 * \version $Revision:$
 * \date $Date:$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * Unit test to test the robustness of HitSorting as it is implemented in CoincSuite/HitSorting.h
 */

#include "CoincSuite/lib/HitSorting.h"

#include <I3Test.h>

using namespace HitSorting;

TEST_GROUP(HitSorting);

/// cheat function because I3RecoPulse does not have a constructor
/// @NOTE a similar constructor should be added to dataclasses/physics/I3RecoPulse.h
I3RecoPulse MakeRecoPulse (const double t, const double c, const double w=1., const uint8_t flags=0) {
  I3RecoPulse p;
  p.SetTime(t);
  p.SetCharge(c);
  p.SetWidth(w);
  p.SetFlags(flags);
  return p;
}

TEST(HitSorting_Convertions){
  //Make a reco Pulse series map
  /*
  Hit(1, 1, 0., 1.);
  Hit(1, 2, 1., 2.); //same DOM different time
  Hit(2, 1, 0., 4.); //different DOM same time
  */

  I3RecoPulseSeriesMap recoMap;
  recoMap[OMKey(1,1)].push_back(MakeRecoPulse(0., 1.));
  recoMap[OMKey(1,1)].push_back(MakeRecoPulse(1., 2.));
  recoMap[OMKey(1,2)].push_back(MakeRecoPulse(0., 4.));

  RetrievalOrderedHitSet hits_retrieval = ExtractHits<RetrievalOrderedHitSet, I3RecoPulse>(recoMap);

  ENSURE(hits_retrieval.size()==3, "Expected number of entries for extracted hits");
  RetrievalOrderedHitSet::const_iterator hits_retrieval_iter = hits_retrieval.begin(); hits_retrieval_iter++;
  ENSURE(hits_retrieval_iter->domIndex==0
    && hits_retrieval_iter->pulseIndex == 1,
    "Second Hit is following the ordering principle");

  RetrievalOrderedHitSet hits_first = ExtractHits<RetrievalOrderedHitSet, I3RecoPulse>(recoMap, Extract_FirstHitOnly);

  ENSURE(hits_first.size()==2, "Expected number of entries for extracted hits with firstHitsOnly option");
  RetrievalOrderedHitSet::const_iterator hits_first_iter = hits_first.begin();
  ENSURE( hits_first_iter->domIndex==0
    && hits_first_iter->pulseIndex == 0
    && hits_first_iter->charge == 1.
    && hits_first_iter->time==0., "The first hit was extracted as expected");

  RetrievalOrderedHitSet hits_csummed = ExtractHits<RetrievalOrderedHitSet, I3RecoPulse>(recoMap, Extract_TotalChargeToFirstHit);

  ENSURE(hits_csummed.size()==2, "Expected number of entries charge extracted hits with TotalChargeToFirstHit option");
  RetrievalOrderedHitSet::const_iterator hits_csummed_iter = hits_csummed.begin();
  ENSURE( hits_csummed_iter->domIndex==0 && hits_csummed_iter->pulseIndex == 0
    && hits_csummed_iter->charge == 3.
    && hits_csummed_iter->time==0., "The charge in the first DOM has been summed");

  TimeOrderedHitSet hits_timeordered = RetrievalOrderedToTimeOrdered(hits_retrieval);
  TimeOrderedHitSet::const_iterator hits_timeordered_iter = hits_timeordered.begin();
  ENSURE( hits_timeordered_iter->domIndex==0
    && hits_timeordered_iter->pulseIndex == 0
    && hits_timeordered_iter->charge == 1.
    && hits_timeordered_iter->time==0., "First Hit time-ordered as expected");
  hits_timeordered_iter++;
  ENSURE( hits_timeordered_iter->domIndex==1
    && hits_timeordered_iter->pulseIndex == 0
    && hits_timeordered_iter->charge == 4.
    && hits_timeordered_iter->time==0., "Second Hit time-ordered as expected");
  hits_timeordered_iter++;
  ENSURE( hits_timeordered_iter->domIndex==0
    && hits_timeordered_iter->pulseIndex == 1
    && hits_timeordered_iter->charge == 2.
    && hits_timeordered_iter->time==1., "Third Hit time-ordered as expected");

  RetrievalOrderedHitSet hits_reretrieval = TimeOrderedToRetrievalOrdered(hits_timeordered);
  hits_retrieval_iter =  hits_retrieval.begin();
  RetrievalOrderedHitSet::const_iterator hits_reretrieval_iter = hits_reretrieval.begin();
  ENSURE( hits_reretrieval_iter->domIndex==hits_retrieval_iter->domIndex
    && hits_reretrieval_iter->pulseIndex==hits_retrieval_iter->pulseIndex
    && hits_reretrieval_iter->charge==hits_retrieval_iter->charge
    && hits_reretrieval_iter->time==hits_retrieval_iter->time, "Identity of first hit cycled and uncyled");
  hits_retrieval_iter++;
  hits_reretrieval_iter++;
  ENSURE( hits_reretrieval_iter->domIndex==hits_retrieval_iter->domIndex
    && hits_reretrieval_iter->pulseIndex==hits_retrieval_iter->pulseIndex
    && hits_reretrieval_iter->charge==hits_retrieval_iter->charge
    && hits_reretrieval_iter->time==hits_retrieval_iter->time, "Identity of second hit cycled and uncyled");
  hits_retrieval_iter++;
  hits_reretrieval_iter++;
  ENSURE( hits_reretrieval_iter->domIndex==hits_retrieval_iter->domIndex
    && hits_reretrieval_iter->pulseIndex==hits_retrieval_iter->pulseIndex
    && hits_reretrieval_iter->charge==hits_retrieval_iter->charge
    && hits_reretrieval_iter->time==hits_retrieval_iter->time, "Identity of third hit cycled and uncyled");

  RetrievalOrderedHitSet hits_popped(hits_reretrieval);
  hits_popped.erase(hits_popped.begin());
  I3RecoPulseSeriesMap recoMap_popped = RevertHits(hits_popped, recoMap);

  ENSURE(recoMap_popped.size()==2, "Still two DOMs even when a secondary pulse got erased");
  ENSURE(recoMap_popped.begin()->second[0].GetCharge() == recoMap.begin()->second[1].GetCharge()
    && recoMap_popped.begin()->second[0].GetTime() == recoMap.begin()->second[1].GetTime(),
    "The only pulse the first DOM is indeed the second pulse of the original map");

  I3RecoPulseSeriesMap recoMap_recompleted = RevertHits(hits_popped, recoMap, true);

  ENSURE(recoMap_recompleted.begin()->second.size()==2, "All Pulses have been recovered");

  I3RecoPulseSeriesMap recoMap_cycled = RevertHits(hits_retrieval, recoMap);
  I3RecoPulseSeriesMap::const_iterator recoMap_cycled_iter = recoMap_cycled.begin();
  ENSURE(recoMap_cycled.size()==2 && recoMap_cycled_iter->second.size()==2, "Format of cycled Map is as expected");
  ENSURE(recoMap_cycled_iter->second[0] == recoMap_cycled_iter->second[0], "First Pulse from the cycled Map is identical to the original one and at the same position");
  ENSURE( recoMap_cycled_iter->second[1] == recoMap_cycled_iter->second[1], "Second Pulse from the cycled Map is identical to the original one and at the same position");
  recoMap_cycled_iter++; //next DOM
  ENSURE( recoMap_cycled_iter->second[0] == recoMap_cycled_iter->second[0], "Third Pulses from the cycled Map is identical to the original one and at the same position");

  I3RecoPulseSeriesMap recoMap_csummed = ConvertHits_To_ResponseSeriesMap<I3RecoPulse>( hits_csummed );

  ENSURE(recoMap_csummed.size()==2, "The reconverted Map with summed charge has the right size");
  ENSURE(recoMap_csummed.begin()->second[0].GetCharge()==3., "The charge has indeed be summed to the first hit");
};