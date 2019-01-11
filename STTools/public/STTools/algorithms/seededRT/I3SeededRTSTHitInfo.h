/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * Olaf Schulz
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/algorithms/seededRT/I3SeededRTSTHitInfo.h
 * @date $Date$
 * @brief This file contains the definition of the I3SeededRTSTHitInfo template
 *        within the sttools::seededRT namespace.
 *
 *        ----------------------------------------------------------------------
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
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSTHITINFO_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSTHITINFO_H_INCLUDED

#include <stdint.h>

#include <set>
#include <vector>

#include <boost/make_shared.hpp>

#include "icetray/OMKey.h"

#include "dataclasses/I3Vector.h"

#include "STTools/OMKeyHasher.h"
#include "STTools/I3STHitInfo.h"

namespace sttools {
namespace seededRT {

//==============================================================================
/** The I3SeededRTSTHitInfo class provides a data container for storing ST
 *  related information about a particular hit used by the seededRT algorithm
 *  as it is known from the classic SeededRTCleaning project.
 */
template<class HitType>
class I3SeededRTSTHitInfo : public sttools::I3STHitInfo<HitType>
{
  public:
    //__________________________________________________________________________
    // Normal constructor.
    I3SeededRTSTHitInfo(
        const OMKey       omKey,
        const I3Position* omPosPtr,
        const HitType*    hitPtr,
        uint32_t          hitIdxWithinOMHitSeries,
        bool              isHitSelected=false,
        bool              isHitNewlySelected=false
    )
      : sttools::I3STHitInfo<HitType>(omKey, omPosPtr, hitPtr, hitIdxWithinOMHitSeries),
        isHitSelected_(isHitSelected),
        isHitNewlySelected_(isHitNewlySelected),
        stPartnerOMHashes_()
    {}

    //__________________________________________________________________________
    // Default constructor.
    I3SeededRTSTHitInfo()
      : sttools::I3STHitInfo<HitType>(),
        isHitSelected_(false),
        isHitNewlySelected_(false),
        stPartnerOMHashes_()
    {}

    //__________________________________________________________________________
    // Copy constructor.
    I3SeededRTSTHitInfo(const I3SeededRTSTHitInfo& rhs)
      : sttools::I3STHitInfo<HitType>(rhs),
        isHitSelected_(rhs.GetIsHitSelected()),
        isHitNewlySelected_(rhs.GetIsHitNewlySelected()),
        stPartnerOMHashes_(rhs.GetSTPartnerOMHashes())
    {}

    //////
    // Property getter methods.
    //__________________________________________________________________________
    inline bool GetIsHitSelected() const {
        return isHitSelected_;
    }

    //__________________________________________________________________________
    inline bool GetIsHitNewlySelected() const {
        return isHitNewlySelected_;
    }

    //__________________________________________________________________________
    inline std::set<uint32_t> GetSTPartnerOMHashes() const {
        return stPartnerOMHashes_;
    }
    //--------------------------------------------------------------------------
    inline std::set<uint32_t>& GetSTPartnerOMHashes() {
        return stPartnerOMHashes_;
    }

    //////
    // Property setter methods.
    //__________________________________________________________________________
    inline void SetIsHitSelected(bool b) {
        isHitSelected_ = b;
    }

    //__________________________________________________________________________
    inline void SetIsHitNewlySelected(bool b) {
        isHitNewlySelected_ = b;
    }

    //__________________________________________________________________________
    inline void SetSTPartnerOMHashes(const std::set<uint32_t> &hashes) {
        stPartnerOMHashes_ = hashes;
    }

    //__________________________________________________________________________
    /** Returns a std::vector< OMKey > holding the OMKey objects of the OMs,
     *  which fulfill the ST conditions for this hit.
     */
    std::vector<OMKey>
    GetSTPartnerOMKeys(const OMKeyHasher& hasher) const;

    //////
    // Alias methods.
    //__________________________________________________________________________
    inline
    bool IsHitSelected() const {
        return GetIsHitSelected();
    }

    //__________________________________________________________________________
    inline
    bool IsHitNewlySelected() const {
        return GetIsHitNewlySelected();
    }

  protected:
    //__________________________________________________________________________
    /// The flag if this hit is selected.
    bool isHitSelected_;

    //__________________________________________________________________________
    /// The temporary flag if this hit is selected newly by a particular
    /// algorithm.
    bool isHitNewlySelected_;

    //__________________________________________________________________________
    /// The set of partner OMs fulfilling the ST conditions for this hit.
    std::set<uint32_t> stPartnerOMHashes_;
};

//______________________________________________________________________________
template<class HitType>
std::vector<OMKey>
I3SeededRTSTHitInfo<HitType>::
GetSTPartnerOMKeys(const OMKeyHasher& hasher) const
{
    std::vector<OMKey> omkeys;
    std::set<uint32_t>::const_iterator citer;
    for(citer=stPartnerOMHashes_.begin(); citer!=stPartnerOMHashes_.end(); ++citer) {
        omkeys.push_back(hasher.DecodeHash(*citer));
    }
    return omkeys;
}

//==============================================================================

}// namespace seededRT
}// namespace sttools

#endif// ! STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTSTHITINFO_H_INCLUDED
