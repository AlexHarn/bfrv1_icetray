/**
 * \file OMKeyHashTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id:$
 * \version $Revision:$
 * \date $Date:$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * Unit test to test the robustness of OMKey Hashing as it is implemented in CoincSuite/OMKeyHash.h
 */

#include "CoincSuite/lib/OMKeyHash.h"

#include <I3Test.h>

TEST_GROUP(OMKeyHash);

using namespace OMKeyHash;

/// sensible value test for hardcoded values
TEST(Definitions){
  ENSURE(MAX_STRINGS == 86);
  ENSURE(MAX_OMS == 64);
  ENSURE(MAX_SIMPLEINDEX == OMKey2SimpleIndex(OMKey(86,64)));
}

/// Cycling test of conversions
/// Make a full cycle of forth and back conversions of OMKeyHash objects
TEST(Cycling){
  std::vector<OMKey> omkeys_to_test;
  omkeys_to_test.push_back(OMKey(1,1)); //First DOM
  omkeys_to_test.push_back(OMKey(1,MAX_OMS)); // IceTop
  omkeys_to_test.push_back(OMKey(MAX_STRINGS, 1));
  omkeys_to_test.push_back(OMKey(MAX_STRINGS, MAX_OMS));

  using namespace OMKeyHash;
  for (std::vector<OMKey>::const_iterator omkey_iter=omkeys_to_test.begin(); omkey_iter<omkeys_to_test.end(); omkey_iter++) {
    ENSURE( OMKey2SimpleIndex(*omkey_iter) >= 0 ); // has to be positive
    ENSURE( OMKey2SimpleIndex(*omkey_iter) == StringOmNbr2SimpleIndex(omkey_iter->GetString(), omkey_iter->GetOM()) ); //functions are identical
    ENSURE( SimpleIndex2StringNbr(OMKey2SimpleIndex(*omkey_iter)) == (uint)omkey_iter->GetString());// is bejective for string numbers
    ENSURE( SimpleIndex2OmNbr(OMKey2SimpleIndex(*omkey_iter)) == omkey_iter->GetOM());// is bejective for om numbers
    ENSURE(  SimpleIndex2OMKey(OMKey2SimpleIndex(*omkey_iter)) == *omkey_iter);// is bejective for all (om.str)-pairs
  }
}
