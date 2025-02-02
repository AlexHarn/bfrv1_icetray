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
#ifndef I3TRACKCHARACTERISTICSVALUES_H_INCLUDED
#define I3TRACKCHARACTERISTICSVALUES_H_INCLUDED

#include <iostream>

#include <serialization/access.hpp>
#include <serialization/version.hpp>

#include "icetray/I3FrameObject.h"
#include "icetray/I3Logging.h"
#include "icetray/I3PointerTypedefs.h"

static const unsigned i3trackcharacteristicsvalues_version_ = 0;

/**
 *  @brief Stores a set of track characteristics (see project CommonVariables).
 *
 *  Output of the I3TrackCharacteristicsCalculator module.
 */
class I3TrackCharacteristicsValues : public I3FrameObject
{
  public:
    //__________________________________________________________________________
    I3TrackCharacteristicsValues()
      : avgDomDistQTotDom_(NAN),
        emptyHitsTrackLength_(NAN),
        trackHitsSeparationLength_(NAN),
        trackHitsDistributionSmoothness_(NAN)
    {}

    //__________________________________________________________________________
    I3TrackCharacteristicsValues(const I3TrackCharacteristicsValues& rhs)
      : avgDomDistQTotDom_(rhs.GetAvgDomDistQTotDom()),
        emptyHitsTrackLength_(rhs.GetEmptyHitsTrackLength()),
        trackHitsSeparationLength_(rhs.GetTrackHitsSeparationLength()),
        trackHitsDistributionSmoothness_(rhs.GetTrackHitsDistributionSmoothness())
    {}

    //__________________________________________________________________________
    I3TrackCharacteristicsValues(
        double avgDomDistQTotDom,
        double emptyHitsTrackLength,
        double trackHitsSeparationLength,
        double trackHitsDistributionSmoothness
    )
      : avgDomDistQTotDom_(avgDomDistQTotDom),
        emptyHitsTrackLength_(emptyHitsTrackLength),
        trackHitsSeparationLength_(trackHitsSeparationLength),
        trackHitsDistributionSmoothness_(trackHitsDistributionSmoothness)
    {}
  
    //__________________________________________________________________________
    std::ostream& Print(std::ostream&) const override;

    //__________________________________________________________________________
    inline
    double
    GetAvgDomDistQTotDom() const
    { return this->avgDomDistQTotDom_; }

    //__________________________________________________________________________
    inline
    double
    GetEmptyHitsTrackLength() const
    { return this->emptyHitsTrackLength_; }

    //__________________________________________________________________________
    inline
    double
    GetTrackHitsDistributionSmoothness() const
    { return this->trackHitsDistributionSmoothness_; }

    //__________________________________________________________________________
    inline
    double
    GetTrackHitsSeparationLength() const
    { return this->trackHitsSeparationLength_; }

    //__________________________________________________________________________
    inline
    void
    SetAvgDomDistQTotDom(double l)
    { this->avgDomDistQTotDom_ = l; }

    //__________________________________________________________________________
    inline
    void
    SetEmptyHitsTrackLength(double l)
    { this->emptyHitsTrackLength_ = l; }

    //__________________________________________________________________________
    inline
    void
    SetTrackHitsDistributionSmoothness(double v)
    { this->trackHitsDistributionSmoothness_ = v; }

    //__________________________________________________________________________
    inline
    void
    SetTrackHitsSeparationLength(double l)
    { this->trackHitsSeparationLength_ = l; }

  protected:
    /// The average DOM distance from the track weighted by the total charge of
    /// each DOM.
    double avgDomDistQTotDom_;
    /// The maximal track length of the track, which got no hits within the
    /// specified track cylinder radius.
    double emptyHitsTrackLength_;
    /// The length how far the COG positions of the first and the last quartile
    /// of the hits, within the specified track cylinder radius, are separated
    /// from each other.
    double trackHitsSeparationLength_;
    /// The track hits distribution smoothness value [-1.;+1.] how smooth
    /// the hits of the given I3RecoPulseSeriesMap, within the specified track
    /// cylinder radius, are distributed along the track.
    double trackHitsDistributionSmoothness_;

  private:
    friend class icecube::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned version);

    SET_LOGGER("I3TrackCharacteristicsValues")
};

std::ostream& operator<<(std::ostream& oss, const I3TrackCharacteristicsValues& rhs);

I3_POINTER_TYPEDEFS(I3TrackCharacteristicsValues);
I3_CLASS_VERSION(I3TrackCharacteristicsValues, i3trackcharacteristicsvalues_version_);

#endif // I3TRACKCHARACTERISTICSVALUES_H_INCLUDED
