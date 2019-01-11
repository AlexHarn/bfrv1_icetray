/**
 * \file OMKeyHashTest.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * Unit test to test the robustness of OMKey Hashing as it is implemented in IceHive/OMKeyHash.h
 */

#include "IceHive/OMKeyHash.h"

#include <I3Test.h>

TEST_GROUP(OMKeyHash);

/// sensible value test for hardcoded values
TEST(Definitions)
{
    ENSURE(OMKeyHash::MAX_STRINGS == 86);
    ENSURE(OMKeyHash::MAX_OMS == 64);
    ENSURE(OMKeyHash::MAX_SIMPLEINDEX == OMKeyHash::OMKey2SimpleIndex(OMKey(86, 64)));
}

/// make a full cycle of forth and back conversions of OMKeyHash objects
TEST(Cycling)
{
    std::vector<OMKey> doms;
    doms.push_back(OMKey(1, 1));
    doms.push_back(OMKey(1, OMKeyHash::MAX_OMS));
    doms.push_back(OMKey(OMKeyHash::MAX_STRINGS, 1));
    doms.push_back(OMKey(OMKeyHash::MAX_STRINGS, OMKeyHash::MAX_OMS));

    for (std::vector<OMKey>::const_iterator omkey=doms.begin(); omkey<doms.end(); omkey++)
    {
        ENSURE(OMKeyHash::IsHashable(*omkey));

        OMKeyHash::SimpleIndex index = OMKeyHash::OMKey2SimpleIndex(*omkey);

        // index has to be positive
        ENSURE(index >= 0);

        // functions are identical
        ENSURE_EQUAL(index, OMKeyHash::StringOmNbr2SimpleIndex(omkey->GetString(), omkey->GetOM()));

        // index is bejective for string numbers
        ENSURE_EQUAL(OMKeyHash::SimpleIndex2StringNbr(index), OMKeyHash::StringNbr(omkey->GetString()));

        // index is bejective for OM numbers
        ENSURE_EQUAL(OMKeyHash::SimpleIndex2OmNbr(index), omkey->GetOM());

        // index is bejective for all OM/string pairs
        ENSURE_EQUAL(OMKeyHash::SimpleIndex2OMKey(index), *omkey);
    }
}

TEST(Exceptions)
{
    std::vector<OMKey> doms;
    doms.push_back(OMKey(0, 1));
    doms.push_back(OMKey(OMKeyHash::MAX_STRINGS + 1, 1));
    doms.push_back(OMKey(1, 0));
    doms.push_back(OMKey(1, OMKeyHash::MAX_OMS + 1));

    for (std::vector<OMKey>::const_iterator omkey=doms.begin(); omkey<doms.end(); omkey++)
    {
        ENSURE(!OMKeyHash::IsHashable(omkey->GetString(), omkey->GetOM()));
        ENSURE(!OMKeyHash::IsHashable(*omkey));

        try
        {
            OMKeyHash::StringOmNbr2SimpleIndex(omkey->GetString(), omkey->GetOM());
            FAIL("Should throw if OMKey is not hashable.");
        }
        catch(std::runtime_error) {}

        try
        {
            OMKeyHash::OMKey2SimpleIndex(*omkey);
            FAIL("Should throw if OMKey is not hashable.");
        }
        catch(std::runtime_error) {}
    }
}
