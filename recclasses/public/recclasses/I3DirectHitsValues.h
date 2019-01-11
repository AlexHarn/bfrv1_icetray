/**
 * $Id$
 *
 * Copyright (C) 2012
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Revision$
 * @date $Date$
 * @author Martin Wolf <martin.wolf@icecube.wisc.edu>
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef I3DIRECTHITSVALUES_H_INCLUDED
#define I3DIRECTHITSVALUES_H_INCLUDED

#include <iostream>

#include <serialization/version.hpp>

#include "icetray/I3FrameObject.h"
#include "icetray/I3Logging.h"
#include "icetray/I3PointerTypedefs.h"

#include "dataclasses/I3Map.h"

static const unsigned i3directhitsvalues_version_ = 0;

/**
 *  @brief Stores a particular class of direct hits generated by
 *         the I3DirectHitsCalculator module (see project CommonVariables).
 */
class I3DirectHitsValues : public I3FrameObject
{
  public:
    //__________________________________________________________________________
    I3DirectHitsValues()
      : nDirStrings_(0),
        nDirDoms_(0),
        nDirPulses_(0),
        qDirPulses_(NAN),
        nEarlyStrings_(0),
        nEarlyDoms_(0),
        nEarlyPulses_(0),
        qEarlyPulses_(NAN),
        nLateStrings_(0),
        nLateDoms_(0),
        nLatePulses_(0),
        qLatePulses_(NAN),
        dirTrackLength_(NAN),
        dirTrackHitDistributionSmoothness_(NAN)
    {}

    //__________________________________________________________________________
    I3DirectHitsValues(
        uint32_t nDirStrings,
        uint32_t nDirDoms,
        uint64_t nDirPulses,
        double   qDirPulses,
        uint32_t nEarlyStrings,
        uint32_t nEarlyDoms,
        uint64_t nEarlyPulses,
        double   qEarlyPulses,
        uint32_t nLateStrings,
        uint32_t nLateDoms,
        uint64_t nLatePulses,
        double   qLatePulses,
        double   dirTrackLength,
        double   dirTrackHitDistributionSmoothness
    )
      : nDirStrings_(nDirStrings),
        nDirDoms_(nDirDoms),
        nDirPulses_(nDirPulses),
        qDirPulses_(qDirPulses),
        nEarlyStrings_(nEarlyStrings),
        nEarlyDoms_(nEarlyDoms),
        nEarlyPulses_(nEarlyPulses),
        qEarlyPulses_(qEarlyPulses),
        nLateStrings_(nLateStrings),
        nLateDoms_(nLateDoms),
        nLatePulses_(nLatePulses),
        qLatePulses_(qLatePulses),
        dirTrackLength_(dirTrackLength),
        dirTrackHitDistributionSmoothness_(dirTrackHitDistributionSmoothness)
    {}

    //__________________________________________________________________________
    I3DirectHitsValues(const I3DirectHitsValues& rhs)
      : nDirStrings_(rhs.GetNDirStrings()),
        nDirDoms_(rhs.GetNDirDoms()),
        nDirPulses_(rhs.GetNDirPulses()),
        qDirPulses_(rhs.GetQDirPulses()),
        nEarlyStrings_(rhs.GetNEarlyStrings()),
        nEarlyDoms_(rhs.GetNEarlyDoms()),
        nEarlyPulses_(rhs.GetNEarlyPulses()),
        qEarlyPulses_(rhs.GetQEarlyPulses()),
        nLateStrings_(rhs.GetNLateStrings()),
        nLateDoms_(rhs.GetNLateDoms()),
        nLatePulses_(rhs.GetNLatePulses()),
        qLatePulses_(rhs.GetQLatePulses()),
        dirTrackLength_(rhs.GetDirTrackLength()),
        dirTrackHitDistributionSmoothness_(rhs.GetDirTrackHitDistributionSmoothness())
    {}

    //__________________________________________________________________________
    virtual
    ~I3DirectHitsValues()
    {}

    //__________________________________________________________________________
    std::ostream&
    Print(std::ostream&) const override;
  
    //__________________________________________________________________________
    inline
    double
    GetDirTrackHitDistributionSmoothness() const
    { return this->dirTrackHitDistributionSmoothness_; }

    //__________________________________________________________________________
    inline
    double
    GetDirTrackLength() const
    { return this->dirTrackLength_; }

    //__________________________________________________________________________
    inline
    uint32_t
    GetNDirDoms() const
    { return this->nDirDoms_; }

    //__________________________________________________________________________
    inline
    uint64_t
    GetNDirPulses() const
    { return this->nDirPulses_; }

    //__________________________________________________________________________
    inline
    uint32_t
    GetNDirStrings() const
    { return this->nDirStrings_; }

    //__________________________________________________________________________
    inline
    uint32_t
    GetNEarlyDoms() const
    { return this->nEarlyDoms_; }

    //__________________________________________________________________________
    inline
    uint64_t
    GetNEarlyPulses() const
    { return this->nEarlyPulses_; }

    //__________________________________________________________________________
    inline
    uint32_t
    GetNEarlyStrings() const
    { return this->nEarlyStrings_; }

    //__________________________________________________________________________
    inline
    uint32_t
    GetNLateDoms() const
    { return this->nLateDoms_; }

    //__________________________________________________________________________
    inline
    uint64_t
    GetNLatePulses() const
    { return this->nLatePulses_; }

    //__________________________________________________________________________
    inline
    uint32_t
    GetNLateStrings() const
    { return this->nLateStrings_; }

    //__________________________________________________________________________
    inline
    double
    GetQDirPulses() const
    { return this->qDirPulses_; }

    //__________________________________________________________________________
    inline
    double
    GetQEarlyPulses() const
    { return this->qEarlyPulses_; }

    //__________________________________________________________________________
    inline
    double
    GetQLatePulses() const
    { return this->qLatePulses_; }

    //__________________________________________________________________________
    inline
    void
    SetDirTrackHitDistributionSmoothness(double v)
    { this->dirTrackHitDistributionSmoothness_ = v; }

    //__________________________________________________________________________
    inline
    void
    SetDirTrackLength(double l)
    { this->dirTrackLength_ = l; }

    //__________________________________________________________________________
    inline
    void
    SetNDirDoms(uint32_t n)
    { this->nDirDoms_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetNDirPulses(uint64_t n)
    { this->nDirPulses_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetNDirStrings(uint32_t n)
    { this->nDirStrings_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetNEarlyDoms(uint32_t n)
    { this->nEarlyDoms_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetNEarlyPulses(uint64_t n)
    { this->nEarlyPulses_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetNEarlyStrings(uint32_t n)
    { this->nEarlyStrings_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetNLateDoms(uint32_t n)
    { this->nLateDoms_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetNLatePulses(uint64_t n)
    { this->nLatePulses_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetNLateStrings(uint32_t n)
    { this->nLateStrings_ = n; }

    //__________________________________________________________________________
    inline
    void
    SetQDirPulses(double q)
    { this->qDirPulses_ = q; }

    //__________________________________________________________________________
    inline
    void
    SetQEarlyPulses(double q)
    { this->qEarlyPulses_ = q; }

    //__________________________________________________________________________
    inline
    void
    SetQLatePulses(double q)
    { this->qLatePulses_ = q; }

  protected:
    /// The total number of strings, which have at least one direct pulse. A
    /// direct pulse has a time residual, that is inside the given direct hits
    /// time window.
    uint32_t nDirStrings_;
    /// The total number of DOMs, which have at least one direct pulse for the
    /// given time window.
    uint32_t nDirDoms_;
    /// The total number of direct pulses for the given time window.
    uint64_t nDirPulses_;
    /// The total charge of all direct pulses for the given time window.
    double qDirPulses_;
    /// The total number of strings, which have at least one early pulse. An
    /// early pulse has a time residual, that is before the direct hits time
    /// window.
    uint32_t nEarlyStrings_;
    /// The total number of DOMs, which have at least one early pulse. An early
    /// pulse has a time residual, that is before the direct hits time window.
    uint32_t nEarlyDoms_;
    /// The total number of pulses before the given time window.
    uint64_t nEarlyPulses_;
    /// The total charge of all early pulses.
    double qEarlyPulses_;
    /// The total number of strings, which have at least one late pulse. A
    /// late pulse has a time residual, that is after the direct hits time
    /// window.
    uint32_t nLateStrings_;
    /// The total number of DOMs, which have at least one late pulse. A late
    /// pulse has a time residual, that is after the direct hits time window.
    uint32_t nLateDoms_;
    /// The total number of pulses after the given time window
    uint64_t nLatePulses_;
    /// The total charge of all late pulses.
    double qLatePulses_;
    /// The length of the track, which is defined as the distance along the
    /// track from the first hit DOM to the last hit DOM perpendicular to the
    /// track direction.
    double dirTrackLength_;
    /// The smoothness value, based on the direct hit DOMs: How uniformly
    /// distributed are the direct hit DOM projections onto the track.
    double dirTrackHitDistributionSmoothness_;

  private:
    friend class icecube::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned version);

    SET_LOGGER("I3DirectHitsValues")
};

//______________________________________________________________________________
std::ostream& operator<<(std::ostream& oss, const I3DirectHitsValues& rhs);

//______________________________________________________________________________
I3_POINTER_TYPEDEFS(I3DirectHitsValues);
I3_CLASS_VERSION(I3DirectHitsValues, i3directhitsvalues_version_);

//==============================================================================

//______________________________________________________________________________
typedef I3Map<std::string, I3DirectHitsValues> I3DirectHitsValuesMap;
I3_POINTER_TYPEDEFS(I3DirectHitsValuesMap);

#endif // I3DIRECTHITSVALUES_H_INCLUDED
