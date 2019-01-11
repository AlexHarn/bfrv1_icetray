/**
 * \file Hive-libTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id:$
 * \version $Revision:$
 * \date $Date:$
 * \author mzoll <marcel.zoll@icecube.wisc.edu>
 *
 * This Unit test is to check some interactions of the Hive-lib. but nothing fancy :/
 */

#include "HiveSplitter/Hive-lib.h"

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

TEST(Topology) {
  using namespace Topology;
  //test that all toplogies for some OMKeys are correctly made:
  // (1,1) (1,64) (36,1) (36, 24) (36,40) (79,1) (81, 1) (81,11) 
  OMKey omkey;

  omkey = OMKey(1,1);

  ENSURE(! IsDCDOM(omkey), "Is a DOM in DeepCore");
  ENSURE(! IsVetoCapDOM(omkey), "Is DOM in the DeepCore Veto layer");
  ENSURE(! IsDCFidDOM(omkey), "Is Dom in the denser populated DeepCore region");
  ENSURE(! IsVetoCapFidDOM(omkey), "Is  DOM in the denser populated  DeepCore Veto Region?");
  ENSURE(! IsDC_VetoCapFidDOM(omkey), "Is DOM in the undense populated regions");
  ENSURE(! IsIceTopDOM(omkey), "Is DOM in IceTop");
  //strings
  ENSURE(! IsDCString(omkey), "Is string in the DeepCore array");
  ENSURE(! IsPinguString(omkey), "Is string in the Pingu (DeepCore infill) array");
  ENSURE( IsICString(omkey), "is string a in the regular IceCube array");

  omkey = OMKey(1,64);
  
  ENSURE(! IsDCDOM(omkey), "Is a DOM in DeepCore");
  ENSURE(! IsVetoCapDOM(omkey), "Is DOM in the DeepCore Veto layer");
  ENSURE(! IsDCFidDOM(omkey), "Is Dom in the denser populated DeepCore region");
  ENSURE(! IsVetoCapFidDOM(omkey), "Is  DOM in the denser populated  DeepCore Veto Region?");
  ENSURE(! IsDC_VetoCapFidDOM(omkey), "Is DOM in the undense populated regions");
  ENSURE( IsIceTopDOM(omkey), "Is DOM in IceTop");
  //strings
  ENSURE(! IsDCString(omkey), "Is string in the DeepCore array");
  ENSURE(! IsPinguString(omkey), "Is string in the Pingu (DeepCore infill) array");
  ENSURE( IsICString(omkey), "is string a in the regular IceCube array");
  
  omkey = OMKey(36,1);

  ENSURE(! IsDCDOM(omkey), "Is a DOM in DeepCore");
  ENSURE(! IsVetoCapDOM(omkey), "Is DOM in the DeepCore Veto layer");
  ENSURE(! IsDCFidDOM(omkey), "Is Dom in the denser populated DeepCore region");
  ENSURE(! IsVetoCapFidDOM(omkey), "Is  DOM in the denser populated  DeepCore Veto Region?");
  ENSURE(! IsDC_VetoCapFidDOM(omkey), "Is DOM in the undense populated regions");
  ENSURE(! IsIceTopDOM(omkey), "Is DOM in IceTop");
  //strings
  ENSURE(! IsDCString(omkey), "Is string in the DeepCore array");
  ENSURE(! IsPinguString(omkey), "Is string in the Pingu (DeepCore infill) array");
  ENSURE( IsICString(omkey), "is string a in the regular IceCube array");

  omkey = OMKey(36,24);

  ENSURE(! IsDCDOM(omkey), "Is a DOM in DeepCore");
  ENSURE(! IsVetoCapDOM(omkey), "Is DOM in the DeepCore Veto layer");
  ENSURE(! IsDCFidDOM(omkey), "Is Dom in the denser populated DeepCore region");
  ENSURE( IsVetoCapFidDOM(omkey), "Is  DOM in the denser populated  DeepCore Veto Region?");
  ENSURE( IsDC_VetoCapFidDOM(omkey), "Is DOM in the undense populated regions");
  ENSURE(! IsIceTopDOM(omkey), "Is DOM in IceTop");
  //strings
  ENSURE(! IsDCString(omkey), "Is string in the DeepCore array");
  ENSURE(! IsPinguString(omkey), "Is string in the Pingu (DeepCore infill) array");
  ENSURE( IsICString(omkey), "is string a in the regular IceCube array");

  omkey = OMKey(36,40);

  ENSURE(! IsDCDOM(omkey), "Is a DOM in DeepCore");
  ENSURE(! IsVetoCapDOM(omkey), "Is DOM in the DeepCore Veto layer");
  ENSURE( IsDCFidDOM(omkey), "Is Dom in the denser populated DeepCore region");
  ENSURE(! IsVetoCapFidDOM(omkey), "Is  DOM in the denser populated  DeepCore Veto Region?");
  ENSURE( IsDC_VetoCapFidDOM(omkey), "Is DOM in the undense populated regions");
  ENSURE(! IsIceTopDOM(omkey), "Is DOM in IceTop");
  //strings
  ENSURE(! IsDCString(omkey), "Is string in the DeepCore array");
  ENSURE(! IsPinguString(omkey), "Is string in the Pingu (DeepCore infill) array");
  ENSURE( IsICString(omkey), "is string a in the regular IceCube array");

  omkey = OMKey(79,1);

  ENSURE(! IsDCDOM(omkey), "Is a DOM in DeepCore");
  ENSURE( IsVetoCapDOM(omkey), "Is DOM in the DeepCore Veto layer");
  ENSURE(! IsDCFidDOM(omkey), "Is Dom in the denser populated DeepCore region");
  ENSURE( IsVetoCapFidDOM(omkey), "Is  DOM in the denser populated  DeepCore Veto Region?");
  ENSURE( IsDC_VetoCapFidDOM(omkey), "Is DOM in the undense populated regions");
  ENSURE(! IsIceTopDOM(omkey), "Is DOM in IceTop");
  //strings
  ENSURE(! IsDCString(omkey), "Is string in the DeepCore array");
  ENSURE( IsPinguString(omkey), "Is string in the Pingu (DeepCore infill) array");
  ENSURE(! IsICString(omkey), "is string a in the regular IceCube array");

  omkey = OMKey(81,1);

  ENSURE(! IsDCDOM(omkey), "Is a DOM in DeepCore");
  ENSURE( IsVetoCapDOM(omkey), "Is DOM in the DeepCore Veto layer");
  ENSURE(! IsDCFidDOM(omkey), "Is Dom in the denser populated DeepCore region");
  ENSURE( IsVetoCapFidDOM(omkey), "Is  DOM in the denser populated  DeepCore Veto Region?");
  ENSURE( IsDC_VetoCapFidDOM(omkey), "Is DOM in the undense populated regions");
  ENSURE(! IsIceTopDOM(omkey), "Is DOM in IceTop");
  //strings
  ENSURE( IsDCString(omkey), "Is string in the DeepCore array");
  ENSURE(! IsPinguString(omkey), "Is string in the Pingu (DeepCore infill) array");
  ENSURE(! IsICString(omkey), "is string a in the regular IceCube array");

  omkey = OMKey(81,11);

  ENSURE( IsDCDOM(omkey), "Is a DOM in DeepCore");
  ENSURE(! IsVetoCapDOM(omkey), "Is DOM in the DeepCore Veto layer");
  ENSURE( IsDCFidDOM(omkey), "Is Dom in the denser populated DeepCore region");
  ENSURE(! IsVetoCapFidDOM(omkey), "Is  DOM in the denser populated  DeepCore Veto Region?");
  ENSURE( IsDC_VetoCapFidDOM(omkey), "Is DOM in the undense populated regions");
  ENSURE(! IsIceTopDOM(omkey), "Is DOM in IceTop");
  //strings
  ENSURE( IsDCString(omkey), "Is string in the DeepCore array");
  ENSURE(! IsPinguString(omkey), "Is string in the Pingu (DeepCore infill) array");
  ENSURE(! IsICString(omkey), "is string a in the regular IceCube array");
}
