/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * Olaf Schulz
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/I3STHitInfo.h
 * @date $Date$
 * @brief This file contains the definition of the I3STHitInfo template
 *        class within the sttools namespace.
 *
 *        This class should be used as base class for all STHitInfo classes used
 *        by ST algorithms.
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
#ifndef STTOOLS_I3STHITINFO_H_INCLUDED
#define STTOOLS_I3STHITINFO_H_INCLUDED

#include "STTools/hit_info_getters.h"

//##############################################################################
namespace sttools {

//==============================================================================
template<class HitType>
class I3STHitInfo
{
  public:
    //__________________________________________________________________________
    /** Function to compare two I3STHitInfo objects by ascending hit time.
     */
    static bool compareHitTimeAsc(
        const I3STHitInfo<HitType> &lhs,
        const I3STHitInfo<HitType> &rhs
    ) {
        return (lhs.GetHitTime() < rhs.GetHitTime());
    }

    //__________________________________________________________________________
    // Normal constructor.
    I3STHitInfo(
        const OMKey       omKey,
        const I3Position* omPosPtr,
        const HitType*    hitPtr,
        uint32_t          hitIdxWithinOMHitSeries
    )
      : omKey_(omKey),
        omPosPtr_(omPosPtr),
        hitPtr_(hitPtr),
        hitIdxWithinOMHitSeries_(hitIdxWithinOMHitSeries)
    {}

    //__________________________________________________________________________
    // Default constructor.
    I3STHitInfo()
      : omKey_(),
        omPosPtr_(NULL),
        hitPtr_(NULL),
        hitIdxWithinOMHitSeries_(0)
    {}

    //__________________________________________________________________________
    // Copy constructor.
    I3STHitInfo(const I3STHitInfo &rhs)
      : omKey_(rhs.GetOMKey()),
        omPosPtr_(rhs.GetOMPositionPtr()),
        hitPtr_(rhs.GetHitPtr()),
        hitIdxWithinOMHitSeries_(rhs.GetHitIdxWithinOMHitSeries())
    {}

    //////
    // Property getter methods.
    //__________________________________________________________________________
    inline const OMKey& GetOMKey() const {
        return omKey_;
    }

    //__________________________________________________________________________
    /** @returns A raw pointer to the I3Position object for the hit OM.
     */
    inline const I3Position* GetOMPositionPtr() const {
        return omPosPtr_;
    }
    //--------------------------------------------------------------------------
    /** @returns A const reference to the I3Position object for the hit OM.
     */
    inline const I3Position& GetOMPosition() const {
        return *omPosPtr_;
    }

    //__________________________________________________________________________
    inline const HitType* GetHitPtr() const {
        return hitPtr_;
    }

    //__________________________________________________________________________
    inline uint32_t GetHitIdxWithinOMHitSeries() const {
        return hitIdxWithinOMHitSeries_;
    }

    //////
    // Property setter methods.
    //__________________________________________________________________________
    inline void SetOMKey(const OMKey& k) {
        omKey_ = k;
    }

    //__________________________________________________________________________
    inline void SetOMPositionPtr(const I3Position* ptr) {
        omPosPtr_ = ptr;
    }

    //__________________________________________________________________________
    inline void SetHitPtr(const HitType* ptr) {
        hitPtr_ = ptr;
    }

    //__________________________________________________________________________
    inline void SetHitIdxWithinOMHitSeries(uint32_t idx) {
        hitIdxWithinOMHitSeries_ = idx;
    }

    //////
    // Public interface methods.
    //__________________________________________________________________________
    /** @returns The time of the hit.
     */
    inline double GetHitTime() const {
        return getHitTime<HitType>(*hitPtr_);
    }

    //__________________________________________________________________________
    /** @returns The charge of the hit.
     */
    inline double GetHitCharge() const {
        return getHitCharge<HitType>(*hitPtr_);
    }

    //__________________________________________________________________________
    /** @returns ``true`` if the hit has set the LC bit, i.e. it is a HLC hit.
     */
    inline bool IsHLCHit() const {
        return getHitLCBit<HitType>(*hitPtr_);
    }

    //__________________________________________________________________________
    /** @returns ``true`` if the hit is a SLC hit, i.e. it is not a HLC hit.
     */
    inline bool IsSLCHit() const {
        return (! getHitLCBit<HitType>(*hitPtr_));
    }

    //__________________________________________________________________________
    /** @returns ``true`` if the hit is the first hit of its OM, i.e. it has
     *      the hitIdxWithinOMHitSeries_ member variable set to ``0``.
     */
    inline bool IsFirstOMHit() const {
        return (hitIdxWithinOMHitSeries_ == 0);
    }

  protected:
    //__________________________________________________________________________
    /// The OMKey to which this hit belongs to.
    OMKey omKey_;

    //__________________________________________________________________________
    /** The pointer to the I3Position object for the position of the OM of this
     *  hit.
     *
     *  @note Since we use a raw pointer here (instead of a copy of the
     *        I3Position object and only for performance reasons), the object
     *        this pointer points to MUST NOT change its location in memory!
     *        Usually the I3Position object was retrieved from a hit object
     *        stored in a std::vector object. In such cases
     *        it means, that the std::vector object MUST NOT change its
     *        capacity! Otherwise this pointer becomes invalid.
     */
    const I3Position* omPosPtr_;

    //__________________________________________________________________________
    /** The pointer to the hit object itself.
     *
     *  @note Since we use a raw pointer here (instead of a copy of the hit
     *        object and only for performance reasons), the object this pointer
     *        points to MUST NOT change its location in memory! Usually the
     *        hit object was retrieved from a std::vector object. In such cases
     *        it means, that the std::vector object MUST NOT change its
     *        capacity! Otherwise this pointer becomes invalid.
     */
    const HitType *hitPtr_;

    //__________________________________________________________________________
    /** The index of the hit within the OM's hit series. The first hit has
     *  the index 0.
     */
    uint32_t hitIdxWithinOMHitSeries_;
};

//==============================================================================

}/*sttools*/
//##############################################################################

#endif//STTOOLS_I3STHITINFO_H_INCLUDED
