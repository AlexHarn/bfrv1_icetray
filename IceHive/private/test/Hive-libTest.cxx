/**
 * \file Hive-libTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@icecube.wisc.edu>
 *
 * This Unit test is to check some interactions of the Hive-lib. but nothing fancy :/
 */

#include "IceHive/Hive-lib.h"

#include <I3Test.h>

TEST_GROUP(Hive-lib);

TEST(Build_a_hive) {
  using namespace honey;
  
  DOMHoneyComb comb;
  comb.AddCenter(36);
  ENSURE(comb.GetCenter()==36);
  ENSURE(comb.GetNRings()==0);
  uint first_ring[] = {26,27,35,37,45,46};
  Ring ring1(first_ring, first_ring+6);
  comb.AddRing(1, ring1);
  ENSURE(comb.GetNRings()==1);
  ENSURE(*comb.GetRing(0).begin()==36);
  ENSURE(*comb.GetRing(1).begin()==26);

  DOMHoneyCombRegister combs;
  combs[36]=comb;

  uint a_ring[] = {37};
  Ring another_ring(a_ring, a_ring+1);
  
  combs[38]=DOMHoneyComb(38, another_ring); // add just another string

  MutualAddStringToRing(combs, 36, 38, 2);

  ENSURE(WhichRing(combs, 36, 36)==0);
  ENSURE(WhichRing(combs, 36, 37)==1);
  ENSURE(WhichRing(combs, 36, 38)==2);
  ENSURE(WhichRing(combs, 36, 39)==-1);
  ENSURE(IsRingX(combs, 36, 2, 38));

  ENSURE(WhichRing(combs, 38, 36)==2);
  
  Hive hive(1, combs, 1);
}

