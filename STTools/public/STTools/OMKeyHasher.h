/**
 * Copyright (C) 2011 - 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * Robert Franke
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/OMKeyHasher.h
 * @date $Date$
 * @brief This file contains the definition of the OMKeyHasher class within the
 *        sttools namespace.
 *
 *        This file is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation; either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef STTOOLS_OMKEYHASHER_H_INCLUDED
#define STTOOLS_OMKEYHASHER_H_INCLUDED

#include <stdint.h>
#include <limits>
#include <vector>

#include <boost/foreach.hpp>

#include "icetray/I3Logging.h"
#include "icetray/OMKey.h"
#include "icetray/I3PointerTypedefs.h"

#include "dataclasses/geometry/I3Geometry.h"

//##############################################################################
namespace sttools {

class OMKeyHasher
{
  public:
    //__________________________________________________________________________
    OMKeyHasher()
      : minString_(0)
      , minOM_(0)
      , stringRange_(0)
      , omRange_(0)
      , maxLinearHash_(0)
      , maxIndexHash_(0)
    {}

    //__________________________________________________________________________
    OMKeyHasher(const std::vector<OMKey> &keys)
    {
        std::vector<OMKey> omKeys;
        BOOST_FOREACH(const OMKey &item, keys)
        {
            if (item.GetPMT() == 0)
                omKeys.push_back(item);
        }

        init(omKeys);
    }

    //__________________________________________________________________________
    OMKeyHasher(const I3OMGeoMap &omGeoMap)
    {
        // Get the list of OMKeys from the I3OMGeoMap object.
        std::vector<OMKey> omKeys;
        BOOST_FOREACH(I3OMGeoMap::const_reference item, omGeoMap)
        {
            if (item.first.GetPMT() == 0)
                omKeys.push_back(item.first);
        }

        init(omKeys);
    }

    //__________________________________________________________________________
    /** Initializes the OMKeyHasher object based on a given OMKey list.
     */
    inline
    void
    init(const std::vector<OMKey> &omKeys)
    {
        minString_ = std::numeric_limits<int>::max();
        minOM_ = std::numeric_limits<unsigned int>::max();
        int maxString = std::numeric_limits<int>::min();
        unsigned int maxOM = std::numeric_limits<unsigned int>::min();

        // Find min and max values for each string and OM number.
        BOOST_FOREACH(std::vector<OMKey>::const_reference omKey, omKeys)
        {
            int string = omKey.GetString();
            unsigned int om = omKey.GetOM();

            if(string < minString_)
                minString_ = string;

            if(string > maxString)
                maxString = string;

            if(om < minOM_)
                minOM_ = om;

            if(om > maxOM)
                maxOM = om;
        }

        // Find range for each string and OM number.
        stringRange_ = maxString - minString_ + 1;
        omRange_ = maxOM - minOM_ + 1;

        // Calculate the maximal possible linear hash value.
        maxLinearHash_ = stringRange_ * omRange_;
        // Determine the maximal index hash.
        maxIndexHash_ = omKeys.size();

        // Create the linearHash2index and index2linearHash mappings.
        linearHash2index_.resize(maxLinearHash_);
        index2linearHash_.resize(maxIndexHash_);
        uint32_t index = 0;
        BOOST_FOREACH(std::vector<OMKey>::const_reference omKey, omKeys)
        {
            uint32_t const linearHash = CreateLinearHash(omKey);
            linearHash2index_[linearHash] = index;
            index2linearHash_[index] = linearHash;
            ++index;
        }

        log_debug(
            "Created OMKeyHasher object ("
            "MinString=%d, MaxString=%d, StringRange=%u, "
            "MinOM=%u, MaxOM=%u, OMRange=%u, maxLinearHash=%u, "
            "size=%u)",
            GetMinString(), GetMaxString(), GetStringRange(),
            GetMinOM(), GetMaxOM(), GetOMRange(), maxLinearHash_,
            GetSize());
    }

    //__________________________________________________________________________
    inline int GetMinString() const {
        return minString_;
    }

    //__________________________________________________________________________
    inline int GetMaxString() const {
        return minString_ + stringRange_ - 1;
    }

    //__________________________________________________________________________
    inline uint32_t GetStringRange() const {
        return stringRange_;
    }

    //__________________________________________________________________________
    inline unsigned int GetMinOM() const {
        return minOM_;
    }

    //__________________________________________________________________________
    inline unsigned int GetMaxOM() const
    {
        return minOM_ + omRange_ - 1;
    }

    //__________________________________________________________________________
    inline uint32_t GetOMRange() const
    {
        return omRange_;
    }

    //__________________________________________________________________________
    /** Returns the maximal possible index hash value.
     */
    inline uint32_t GetSize() const {
        return maxIndexHash_;
    }

    //__________________________________________________________________________
    /** Returns an unique hash for the given OMKey.
     *  This is just a shortway for the HashOMKey method.
     */
    inline
    uint32_t
    operator()(const OMKey& key) const
    {
        return HashOMKey(key);
    }

    //__________________________________________________________________________
    /** Returns an unique hash for the given OMKey.
     */
    inline
    uint32_t
    HashOMKey(const OMKey& key) const
    {
        return linearHash2index_[CreateLinearHash(key)];
    }

    //__________________________________________________________________________
    /** Decodes a given hash and returns the OMKey object, that corresponds
     *  to the given hash.
     */
    inline
    OMKey
    DecodeHash(uint32_t index) const
    {
        return DecodeLinearHash(index2linearHash_[index]);
    }

  protected:
    int          minString_;
    unsigned int minOM_;
    uint32_t     stringRange_;
    uint32_t     omRange_;
    uint32_t     maxLinearHash_;
    uint32_t     maxIndexHash_;

    /** The linearHash2index list provides the mapping from linear hashes to
     *  index values. Some of the list entries might be unused.
     */
    std::vector<uint32_t> linearHash2index_;

    /** The index2linearHash list provides the mapping from index values to
     *  linear hashes. All the entries are used.
     */
    std::vector<uint32_t> index2linearHash_;

    //__________________________________________________________________________
    /** Hashes the given OMKey linearly, i.e. encoding string and OM number.
     */
    inline
    uint32_t
    CreateLinearHash(const OMKey& key) const
    {
        uint32_t linearHash = (key.GetString() - minString_) * omRange_ + key.GetOM() - minOM_;
        if(linearHash >= maxLinearHash_)
        {
            log_fatal(
                "%s is out of range (max hash size: %u): Check Geometry! "
                "MinString=%d, MaxString=%d, MinOM=%u, MaxOM=%u",
                key.str().c_str(),
                maxLinearHash_,
                GetMinString(),
                GetMaxString(),
                GetMinOM(),
                GetMaxOM());
        }
        return linearHash;
    }

    //__________________________________________________________________________
    /** Decodes a given linear hash and returns the OMKey object, that
     *  corresponds to the given linear hash.
     */
    inline
    OMKey
    DecodeLinearHash(uint32_t hash) const
    {
        uint32_t x = hash % omRange_;
        unsigned int om = x + minOM_;
        int string = (hash - x) / omRange_ + minString_;
        return OMKey(string, om);
    }

  private:
    SET_LOGGER("OMKeyHasher");
};

I3_POINTER_TYPEDEFS(OMKeyHasher);

}/*sttools*/
//##############################################################################
#endif // STTOOLS_OMKEYHASHER_H_INCLUDED
