/**
 * @file PartialCOGTest.cxx
 * 
 * $Id: PartialCOGTest.cxx 104975 2013-06-03 18:46:34Z mzoll $
 * $Author: mzoll <marcel.zoll@fysik.su.se> $
 * $Date: 2013-06-03 20:46:34 +0200 (Mon, 03 Jun 2013) $
 * $Revision: 104975 $
 *
 * A Unit test which generates some artificial test cases and let the Splitter gnaw on it;
 */

//need to define operational parameters to define the correct tests
#include <I3Test.h>

#include "dataclasses/geometry/I3Geometry.h"

#include "CoincSuite/lib/PartialCOG.h"
#include "CoincSuite/lib/HitSorting.h"

#include <math.h>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

using namespace boost;
using namespace std;
using namespace OMKeyHash;
using namespace HitSorting;
using namespace PartialCOG;

I3Geometry CreateGeometry () {
  /* Make an icecube hexagon, inter-string spacing is 1., inter-dom spacing is 1;
   * positive x-axis to the right
   * positive y-axis upwards
   * positive z-axis perpendicular into the plane
   * 
   *         2      7
   * 
   *      3     1     6
   * 
   *         4      5
   */
  
  I3OMGeo omgeo;
  omgeo.omtype = I3OMGeo::IceCube;
  omgeo.orientation = I3Orientation(I3Direction(0, 0),I3Direction(M_PI, 0)); //straight_down
  omgeo.area = 0.0443999990821; //MAGIC hardcoding number
  
  I3Geometry i3geo;
  //depth
  omgeo.position = I3Position(0.,0.,0.);
  i3geo.omgeo[OMKey(1,1)] = omgeo;
  omgeo.position = I3Position(0.,0.,1.);
  i3geo.omgeo[OMKey(1,2)] = omgeo;
  omgeo.position = I3Position(0.,0.,2.);
  i3geo.omgeo[OMKey(1,3)] = omgeo;
  omgeo.position = I3Position(0.,0.,3.);
  i3geo.omgeo[OMKey(1,4)] = omgeo;
  omgeo.position = I3Position(0.,0.,4.);
  i3geo.omgeo[OMKey(1,5)] = omgeo;
  omgeo.position = I3Position(0.,0.,5.);
  i3geo.omgeo[OMKey(1,6)] = omgeo;
  omgeo.position = I3Position(0.,0.,6.);
  i3geo.omgeo[OMKey(1,7)] = omgeo;
  omgeo.position = I3Position(0.,0.,8.);
  i3geo.omgeo[OMKey(1,8)] = omgeo;
  //surrounding
  omgeo.position = I3Position(-1./2.,  sqrt(3./4.),0.); //north west
  i3geo.omgeo[OMKey(2,1)] = omgeo;
  omgeo.position = I3Position(    -1,           0.,0.); //west
  i3geo.omgeo[OMKey(3,1)] = omgeo;
  omgeo.position = I3Position(-1./2., -sqrt(3./4.),0.); //south west
  i3geo.omgeo[OMKey(4,1)] = omgeo;
  omgeo.position = I3Position( 1./2., -sqrt(3./4.),0.); //south east
  i3geo.omgeo[OMKey(5,1)] = omgeo;
  omgeo.position = I3Position(    1.,           0.,0.); //east
  i3geo.omgeo[OMKey(6,1)] = omgeo;
  omgeo.position = I3Position( 1./2.,  sqrt(3./4.),0.); //north east
  i3geo.omgeo[OMKey(7,1)] = omgeo;
  return i3geo;
};

// eight hits same DOM
HitSorting::TimeOrderedHitSet CreateHits_samedom (const OMKey &omkey= OMKey(1,1)) {
  HitSorting::TimeOrderedHitSet hits;  
  hits.insert(Hit(OMKey2SimpleIndex(omkey), 0, 0, 1.));
  hits.insert(Hit(OMKey2SimpleIndex(omkey), 1, 1, 2.));
  hits.insert(Hit(OMKey2SimpleIndex(omkey), 2, 2, 3.));
  hits.insert(Hit(OMKey2SimpleIndex(omkey), 3, 3, 4.));
  hits.insert(Hit(OMKey2SimpleIndex(omkey), 4, 4, 5.));
  hits.insert(Hit(OMKey2SimpleIndex(omkey), 5, 5, 6.));
  hits.insert(Hit(OMKey2SimpleIndex(omkey), 6, 6, 7.));
  hits.insert(Hit(OMKey2SimpleIndex(omkey), 7, 7, 8.));
  return hits;
};

/// eight hits going downwards on the same string
HitSorting::TimeOrderedHitSet CreateHits_stringCharge () {
  HitSorting::TimeOrderedHitSet hits;  
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,1), 0, 0, 1.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,2), 0, 1, 2.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,3), 0, 2, 3.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,4), 0, 3, 4.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,5), 0, 4, 5.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,6), 0, 5, 6.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,7), 0, 6, 7.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,8), 0, 7, 8.));
  return hits;
};

// eight hits on a hexagon, centerstring is hit twice
HitSorting::TimeOrderedHitSet CreateHits_hexagon () {
  HitSorting::TimeOrderedHitSet hits;
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,1), 0, 0, 1.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(2,1), 0, 1, 2.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(3,1), 0, 2, 3.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(4,1), 0, 3, 4.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(5,1), 0, 4, 5.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(6,1), 0, 5, 6.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(7,1), 0, 6, 7.));
  hits.insert(Hit(StringOmNbr2SimpleIndex(1,1), 1, 7, 7.));
  return hits;
};
 

//==========================================
TEST_GROUP(PartialCOG);

TEST (DomPos_cache) { //create an DOM-cache
  const DomPos_cache cache = BuildDomPos_cache(CreateGeometry());
  
  ENSURE_EQUAL(cache[OMKey2SimpleIndex(OMKey(1,1))], I3Position(0.,0.,0.));
  ENSURE_EQUAL(cache[OMKey2SimpleIndex(OMKey(1,2))], I3Position(0.,0.,1.));
  ENSURE_EQUAL(cache[OMKey2SimpleIndex(OMKey(1,3))], I3Position(0.,0.,2.));
  ENSURE_EQUAL(cache[OMKey2SimpleIndex(OMKey(2,1))], I3Position(-1./2.,  sqrt(3./4.),0.));
  ENSURE_EQUAL(cache[OMKey2SimpleIndex(OMKey(3,1))], I3Position(-1.,              0.,0.));
  
  //make shure that max index is accessable and returns empty values
  //ENSURE_EQUAL(cache[OMKey2SimpleIndex(OMKey(MAX_STRINGS,MAX_OMS))], I3Position(NAN, NAN, NAN));
  I3Position nanpos = cache[OMKey2SimpleIndex(OMKey(MAX_STRINGS,MAX_OMS))];
  //NOTE cannot do this because of (nan==nan)=false, so decompose
  ENSURE(std::isnan(nanpos.GetX()) && std::isnan(nanpos.GetY()) && std::isnan(nanpos.GetZ()));
}

//===============================================================
TEST (PartialCOGhit) {
  /* compute COG in pos and time of the fraction of DOMs using absolut position/numbers
   * @param hits map of OMKeys and time of their first hits
   * @param domPos_cache a ordered vector holding all I3Positions of the hashed OMKeys
   * @param nfrac number of fraction that should be used
   * @param useFrac use that fraction
   * @param heads_tails counting from front or back
   * @param min_inc include at least that many doms in the COG
   * @param max_inc include no more than that many doms in the COG
   * @param useCharge weight the COG by the charge of each hit
   * @return a pair of COG position and time (as average of the configured pulses)
   */
  
  HitSorting::TimeOrderedHitSet hits = CreateHits_samedom();
  DomPos_cache cache = BuildDomPos_cache(CreateGeometry());
  std::pair<I3Position, double> pos_time;
  
  //take all, by selecting nfrac==1
  pos_time = PartialCOGhit (hits, cache,1,1,true,1,8,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0+1+2+3+4+5+6+7)/8.);
  
  //take first six by max_inc==6
  pos_time = PartialCOGhit (hits, cache,1,1,true,1,6,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0+1+2+3+4+5)/6.);
  
  //take last six by min included max_inc==6 and heads_tails==false
  pos_time = PartialCOGhit (hits, cache,1,1,false,1,6,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2+3+4+5+6+7)/6.);
  
  //take the first half by nFrac==2 and useFrac==1
  pos_time = PartialCOGhit (hits, cache,2,1,true,1,8,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0+1+2+3)/4.);
  
  //take the last half  nFrac==2 and useFrac==2
  pos_time = PartialCOGhit (hits, cache,2,2,true,1,8,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(4+5+6+7)/4.);
  
  //take first six by min_inc==6
  pos_time = PartialCOGhit (hits, cache,2,1,true,6,6,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0+1+2+3+4+5)/6.);
  
  ///take middle 4 by including 4 and excluding 2 at front
  pos_time = PartialCOGhit (hits, cache,4,2,true,4,4,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2+3+4+5)/4.);
  
  //same but do it reverse
  pos_time = PartialCOGhit (hits, cache,4,3,false,4,4,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2+3+4+5)/4.);
};  

TEST(PartialCOGhit_Charge) { //change the last argument option
  /* compute COG in pos and time of the fraction of DOMs using absolut position/numbers
   * @param hits map of OMKeys and time of their first hits
   * @param domPos_cache a ordered vector holding all I3Positions of the hashed OMKeys
   * @param nfrac number of fraction that should be used
   * @param useFrac use that fraction
   * @param heads_tails counting from front or back
   * @param min_inc include at least that many doms in the COG
   * @param max_inc include no more than that many doms in the COG
   * @param useCharge weight the COG by the charge of each hit
   * @return a pair of COG position and time (as average of the configured pulses)
   */
  
  HitSorting::TimeOrderedHitSet hits = CreateHits_samedom();
  DomPos_cache cache = BuildDomPos_cache(CreateGeometry());
  std::pair<I3Position, double> pos_time;

  //take all
  pos_time = PartialCOGhit (hits, cache,1,1,true,1,8,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6+6*7+7*8)/double(1+2+3+4+5+6+7+8));
  
  //take first six by min_included
  pos_time = PartialCOGhit (hits, cache,1,1,true,6,6,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6)/double(1+2+3+4+5+6));
  
  //take last six by min included
  pos_time = PartialCOGhit (hits, cache,1,1,false,6,6,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2*3+3*4+4*5+5*6+6*7+7*8)/double(3+4+5+6+7+8));
  
  //take the first half
  pos_time = PartialCOGhit (hits, cache,2,1,true,1,8,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4)/double(1+2+3+4));
  
  //take the last half
  pos_time = PartialCOGhit (hits, cache,2,2,true,1,8,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(4*5+5*6+6*7+7*8)/double(5+6+7+8));
  
  //take first half but require 6 included
  pos_time = PartialCOGhit (hits, cache,2,1,true,6,8,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6)/double(1+2+3+4+5+6));
  
  ///take middle 4 by including 4 and excluding 2 at front
  pos_time = PartialCOGhit (hits, cache,4,2,true,4,4,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2*3+3*4+4*5+5*6)/double(3+4+5+6));
  
  //same but do it reverse
  pos_time = PartialCOGhit (hits, cache,4,3,false,4,4,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2*3+3*4+4*5+5*6)/double(3+4+5+6));
}
  
  
//==========================================================
TEST (PartialCOGcharged) {  
  
  /* compute COG in pos and time of the fraction of DOMs using accumulated charge
   * @param hits map of OMKeys and time of their first hits
   * @param domPos_cache a ordered vector holding all I3Positions of the hashed OMKeys
   * @param nfrac number of fraction that should be used
   * @param useFrac use that fraction
   * @param heads_tails counting from front (true) or back (false)
   * @param min_inc include at least that many hits in the COG
   * @param max_inc include no more than that many hits in the COG
   * @return a pair of COG position and time (as average of the configured pulses)
   */    
  HitSorting::TimeOrderedHitSet hits = CreateHits_samedom();
  DomPos_cache cache = BuildDomPos_cache(CreateGeometry());
  std::pair<I3Position, double> pos_time;
  
  //===Enable Charge Counting===
  //take all, by selecting nfrac==1
  pos_time = PartialCOGcharged (hits, cache,1,1,true,1,8);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6+6*7+7*8)/double(1+2+3+4+5+6+7+8));
  
  //take first six by max_inc==6
  pos_time = PartialCOGcharged (hits, cache,1,1,true,6,6);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6)/double(1+2+3+4+5+6));
  
  //take last six by min included max_inc==6 and heads_tails==false
  pos_time = PartialCOGcharged (hits, cache,1,1,false,6,6);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2*3+3*4+4*5+5*6+6*7+7*8)/double(3+4+5+6+7+8));
  
  //take the first (charge) half by nFrac==2 and useFrac==1; first fraction is (h1,2,3,4,5,6; c=15) (h7,h8; c=21); hit6 is associated to first fractions
  pos_time = PartialCOGcharged (hits, cache,2,1,true,1,8);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6)/double(1+2+3+4+5+6));
  
  //take the last half nFrac==2 and useFrac==2
  pos_time = PartialCOGcharged (hits, cache,2,2,true,1,8);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(6*7+7*8)/double(7+8));
  
  //take first six by min_inc==6
  pos_time = PartialCOGcharged (hits, cache,2,1,true,6,8);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6)/double(1+2+3+4+5+6));
};

 
//===============================================================
TEST (PartialCOGexclude) {
  /* compute COG in pos and time of the fraction of DOMs using exclusions
   * @param hits map of OMKeys and time of their first hits
   * @param domPos_cache a ordered vector holding all I3Positions of the hashed OMKeys
   * @param heads_tails counting from front(true) or back (false)
   * @param n_many number of hits to include
   * @param n_exclude number of hits to exclude up front, before counting doms to include
   * @param useCharge weight the COG by the charge of each hit
   * @return a pair of COG position and time (as average of the configured pulses)
   */
  HitSorting::TimeOrderedHitSet hits = CreateHits_samedom();
  DomPos_cache cache = BuildDomPos_cache(CreateGeometry());
  std::pair<I3Position, double> pos_time;
  
  //===Use regular counting, each hit has weight 1===
   //take all, by selecting all, excluding none
  pos_time = PartialCOGexclude (hits, cache,true,8,0,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0+1+2+3+4+5+6+7)/8.);
  
  //take first six excluding none
  pos_time = PartialCOGexclude (hits, cache,true,6,0,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0+1+2+3+4+5)/6.);
  
  //take last six by min included max_inc==6 and excluding the first two n_excl=2
  pos_time = PartialCOGexclude (hits, cache,true,6,2,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2+3+4+5+6+7)/6.);
  
  //take the first half by counting from back and n_excl=4
  pos_time = PartialCOGexclude (hits, cache,false,4,4,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0+1+2+3)/4.);
  
  //take the last half counting from front
  pos_time = PartialCOGexclude (hits, cache,false,4,0,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(4+5+6+7)/4.);
  
  //take first last counting from back
  pos_time = PartialCOGexclude (hits, cache,false,4,0,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(7+6+5+4)/4.);
  
  ///take middle 4 by including 4 and excluding 2 at front
  pos_time = PartialCOGexclude (hits, cache,true,4,2,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2+3+4+5)/4.);
  //same but do it reverse
  pos_time = PartialCOGexclude (hits, cache,false,4,2,false);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2+3+4+5)/4.);
  
  //===Enable Charge Weighting===
  //take all
  pos_time = PartialCOGexclude (hits, cache,true,8,0,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6+6*7+7*8)/double(1+2+3+4+5+6+7+8));

  //take first six by min_included
  pos_time = PartialCOGexclude (hits, cache,true,6,0,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4+4*5+5*6)/double(1+2+3+4+5+6));
  
  //take last six by min included max_inc==6 and excluding the first two n_excl=2
  pos_time = PartialCOGexclude (hits, cache,true,6,2,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2*3+3*4+4*5+5*6+6*7+7*8)/double(3+4+5+6+7+8));
  
  //take the first half by counting from back and n_excl=4
  pos_time = PartialCOGexclude (hits, cache,false,4,4,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(0*1+1*2+2*3+3*4)/double(1+2+3+4));
  
  //take the last half counting from front
  pos_time = PartialCOGexclude (hits, cache,false,4,0,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(4*5+5*6+6*7+7*8)/double(5+6+7+8));
  
  //take last half counting from back
  pos_time = PartialCOGexclude (hits, cache,false,4,0,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(7*8+7*6+6*5+5*4)/double(8+7+6+5));
  
  //take middle 4 by including 4 and excluding 2 at front
  pos_time = PartialCOGexclude (hits, cache,true,4,2,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2*3+3*4+4*5+5*6)/double(3+4+5+6));
  //same but do it reverse
  pos_time = PartialCOGexclude (hits, cache,false,4,2,true);
  ENSURE_EQUAL(pos_time.first, I3Position(0,0,0));
  ENSURE_EQUAL(pos_time.second, double(2*3+3*4+4*5+5*6)/double(3+4+5+6));
};
