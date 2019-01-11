/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/algorithms/seededRT/I3SeededRTConfiguration.h
 * @date $Date$
 * @brief This file contains the definition of the I3SeededRTConfiguration
 *        class.
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
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTCONFIGURATION_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_I3SEEDEDRTCONFIGURATION_H_INCLUDED

#include <ostream>

#include "icetray/I3PointerTypedefs.h"
#include "icetray/I3Units.h"

#include "dataclasses/I3Constants.h"
#include "dataclasses/I3Vector.h"

#include "recclasses/I3STConfiguration.h"
#include "recclasses/OMKeyLinkSet.h"

static const unsigned i3seededrtconfiguration_version_ = 0;

//==============================================================================
class I3SeededRTConfiguration
  : public I3STConfiguration
{
  public:
    //__________________________________________________________________________
    /// Define the possible RT coordinate systems.
    enum SeededRTCoordSys {
        Sph = 1,
        Cyl = 2
    };

    //__________________________________________________________________________
    /** The default constructor, needed by icecube::serialization.
     */
    I3SeededRTConfiguration()
      : I3STConfiguration(),
        rtCoordSys_(Sph),
        rtTime_(NAN),
        rtRadius_(NAN),
        rtHeight_(NAN)
    {}

    //__________________________________________________________________________
    /** The normal constructor.
     *  It allows to set each configuration option.
     */
    I3SeededRTConfiguration(
        const std::string          &name,
        const I3VectorOMKeyLinkSet &omKeyLinkSets,
        SeededRTCoordSys           rtCoordSys,
        double                     rtTime,
        double                     rtRadius,
        double                     rtHeight
    )
      : I3STConfiguration(name, omKeyLinkSets),
        rtCoordSys_(rtCoordSys),
        rtTime_(rtTime),
        rtRadius_(rtRadius),
        rtHeight_(rtHeight)
    {}

    //__________________________________________________________________________
    I3SeededRTConfiguration(const I3SeededRTConfiguration &rhs)
      : I3STConfiguration(rhs),
        rtCoordSys_(rhs.GetRTCoordSys()),
        rtTime_(rhs.GetRTTime()),
        rtRadius_(rhs.GetRTRadius()),
        rtHeight_(rhs.GetRTHeight())
    {}

    //__________________________________________________________________________
    virtual
    ~I3SeededRTConfiguration();

    //////
    // Property getter functions.
    //__________________________________________________________________________
    inline SeededRTCoordSys GetRTCoordSys() const {
        return rtCoordSys_;
    }
    inline double GetRTTime() const {
        return rtTime_;
    }
    inline double GetRTRadius() const {
        return rtRadius_;
    }
    inline double GetRTHeight() const {
        return rtHeight_;
    }

    //////
    // Property setter functions.
    //__________________________________________________________________________
    inline void SetRTCoordSys(SeededRTCoordSys c) {
        rtCoordSys_ = c;
    }
    inline void SetRTTime(double t) {
        if(std::isnan(t)) {
            log_fatal("The RTTime property must not be NAN!");
        }
        rtTime_ = t;
    }
    inline void SetRTRadius(double r) {
        if(std::isnan(r)) {
            log_fatal("The RTRadius property must not be NAN!");
        }
        rtRadius_ = r;
    }
    inline void SetRTHeight(double h) {
        if(std::isnan(h) && rtCoordSys_ == Cyl) {
            log_fatal(
                "The RTHeight property must not be NAN, "
                "if the RTCoordSys property is set to Cyl!");
        }
        rtHeight_ = h;
    }

    //__________________________________________________________________________
    // Implement equality operator, so the object can be put into a std::vector.
    inline bool operator==(const I3SeededRTConfiguration &rhs) const {
        return (
            I3STConfiguration::operator==((const I3STConfiguration&)rhs) &&
            rtCoordSys_ == rhs.GetRTCoordSys() &&
            rtTime_     == rhs.GetRTTime()     &&
            rtRadius_   == rhs.GetRTRadius()   &&
            rtHeight_   == rhs.GetRTHeight()
        );
    }

    //__________________________________________________________________________
    std::string
    GetPrettySettingsStr(unsigned int nLeadingWS) const;

    std::string
    str() const
    {
        return GetPrettySettingsStr(0);
    }

    //__________________________________________________________________________
    /** The ST volume time (ST volume times ST time) that is
     *  spaned up by this ST configuration. This value can be used to compare
     *  two ST configurations in terms of their effectiveness.
     */
    inline
    double
    GetSTVolumetime() const
    {
        if(rtCoordSys_ == Sph) {
            return I3Constants::pi*4/3*rtRadius_*rtRadius_*rtRadius_ * rtTime_;
        }
        if(rtCoordSys_ == Cyl) {
            return I3Constants::pi*rtRadius_*rtRadius_*2*rtHeight_ * rtTime_;
        }

        log_fatal(
            "The RT coordinate system %d is not supported. "
            "This is a bug!",
            rtCoordSys_);
    }

  protected:
    //__________________________________________________________________________
    /** The type of the coordinate system to use for seededRT condition
     *  calculations.
     */
    SeededRTCoordSys rtCoordSys_;

    //__________________________________________________________________________
    /** The time interval to use for seededRT condition calculations.
     */
    double rtTime_;

    //__________________________________________________________________________
    /** The distance radius to use for seededRT condition calculations.
     */
    double rtRadius_;

    //__________________________________________________________________________
    /** If rtCoordSys_ is set to ``Cyl``, this specifies the height of
     *  the cylinder that should be used around an OM for calculating
     *  the seededRT condition. The radius of the cylinder is then defined
     *  through the value of rtRadius_.
     */
    double rtHeight_;

  private:
    friend class icecube::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, unsigned version);

    SET_LOGGER("I3SeededRTConfiguration");
};

//______________________________________________________________________________
std::ostream& operator<<(std::ostream& os, const I3SeededRTConfiguration &rhs);

//______________________________________________________________________________
I3_POINTER_TYPEDEFS(I3SeededRTConfiguration);
I3_CLASS_VERSION(I3SeededRTConfiguration, i3seededrtconfiguration_version_);

//______________________________________________________________________________
typedef I3Vector<I3SeededRTConfiguration> I3VectorSeededRTConfiguration;
I3_POINTER_TYPEDEFS(I3VectorSeededRTConfiguration);

#endif//STTOOLS_ALGORITHMS_CLASSICRT_I3CLASSICRTCONFIGURATION_H_INCLUDED
