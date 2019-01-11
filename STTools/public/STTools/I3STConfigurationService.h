/**
 * Copyright (C) 2013 - 2014
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/I3STConfigurationService.h
 * @date $Date$
 * @brief This file contains the definition of the I3STConfigurationService
 *        template. The I3STConfigurationService template is a base class for
 *        all ST algorithm configuration services.
 *
 *        It holds an boost::shared_ptr< I3Vector<STConfigurationType> > member
 *        variable named ``stConfigVecPtr_``. This is a list of ST configuration
 *        objects of type
 *        STConfigurationType. This type must be a class specific for a
 *        particular ST algorithm, and must be derived from the
 *        I3STConfiguration base class.
 *
 *        It also holds an object of SContextType for the spatial context
 *        valid for this ST configuration service. This spatial context is
 *        considered as constant as long as the ST configuration does not
 *        change, i.e. the ST configuration is frozen.
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
#ifndef STTOOLS_I3STCONFIGURATIONSERVICE_H_INCLUDED
#define STTOOLS_I3STCONFIGURATIONSERVICE_H_INCLUDED

#include <algorithm>
#include <set>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/utility.hpp>

#include "icetray/I3Logging.h"
#include "icetray/OMKey.h"

#include "dataclasses/I3Vector.h"
#include "dataclasses/geometry/I3OMGeo.h"

#include "STTools/OMKeyHasher.h"
#include "STTools/OMKeyPairMap.h"
#include "recclasses/OMKeyLink.h"
#include "recclasses/I3STConfiguration.h"

namespace sttools {

/**
 * The I3STConfigurationService is noncopyable because it stores an array of
 * raw pointers to STConfigurationType objects stored in itself. By copying the
 * service, the raw pointers would point to the wrong (i.e. the old)
 * STConfigurationType objects.
 */
template <
    class STConfigurationType,
    class SContextType
>
class I3STConfigurationService
  : public boost::noncopyable
{
  public:
    //__________________________________________________________________________
    I3STConfigurationService()
      : stConfigVecPtr_(boost::shared_ptr< I3Vector<STConfigurationType> >(new I3Vector<STConfigurationType>()) ),
        omGeoMapPtr_(NULL),
        isSTConfigVecFrozen_(false)
    {
        defaultSTConfig_ = CreateDefaultSTConfiguration<STConfigurationType>();
    }

    //__________________________________________________________________________
    virtual
    ~I3STConfigurationService()
    {
        log_trace("Destructing I3STConfigurationService object.");
    }

    //__________________________________________________________________________
    /** Creates the default ST configuration object. This method should be
     *  reimplemented by the derived class.
     */
    template<class STConfigurationType_>
    STConfigurationType_
    CreateDefaultSTConfiguration() const;

    //__________________________________________________________________________
    const typename SContextType::s_data_t &
    GetSDataForOMLink(const OMKey &omKey1, const OMKey &omKey2) const;

    //__________________________________________________________________________
    /** Returns the STConfigurationType object set for the given OM link. This
     *  function loops through the stConfigVecPtr_ member variable. Thus, it is
     *  slow and should be used only when initializing the
     *  algorithm. The algorithm itself should cache the ST configuration for a
     *  particular DOM link in any way for fast access during event processing.
     */
    const STConfigurationType &
    GetSTConfigurationForOMLink(const OMKey &omKey1, const OMKey &omKey2) const;

    //__________________________________________________________________________
    /** Returns an I3VectorOMKey that contains all OMs which are somehow
     *  configured by this ST configuration service.
     *
     *  @note The OMKeys in the returned list are unique.
     */
    I3VectorOMKey
    GetOMKeys() const;

    //__________________________________________________________________________
    /** Checks if the ST configuration provided by this ST configuration service
     *  is complete. This means, that all OMKeys present in the given geometry
     *  are also present in this ST configuration, i.e. each OM can be found
     *  in at least one OM link.
     */
    virtual
    bool
    IsSTConfigurationComplete(const I3OMGeoMap &omGeoMap) const;

    //__________________________________________________________________________
    /** Returns a boost::shared_ptr to the stConfigVecPtr_ member variable.
     */
    boost::shared_ptr< I3Vector<STConfigurationType> >
    GetSTConfigVecPtr()
    {
        return stConfigVecPtr_;
    }

    //__________________________________________________________________________
    /** Sets the stConfigVec_ member variable.
     */
    void
    SetSTConfigVecPtr(boost::shared_ptr< I3Vector<STConfigurationType> > &v)
    {
        if(isSTConfigVecFrozen_)
        {
            log_fatal(
                "The STConfigVec property cannot be set, if the ST "
                "configuration is frozen! Call the UnfreezeSTConfiguration() "
                "method first!");
        }
        stConfigVecPtr_ = v;
    }

    //__________________________________________________________________________
    /** Freezes the ST configuration (i.e. the stConfigVec_ member variable).
     */
    virtual
    void
    FreezeSTConfiguration()
    {
        isSTConfigVecFrozen_ = true;
    }

    //__________________________________________________________________________
    /** Unfreezes the ST configuration (i.e. the stConfigVec_ member variable).
     */
    virtual
    void
    UnfreezeSTConfiguration()
    {
        isSTConfigVecFrozen_ = false;
    }

    //__________________________________________________________________________
    /** Returns ``true`` if the ST configuration (i.e. the stConfigVecPtr_
     *  member variable) is frozen and ``false`` otherwise.
     *
     *  @note If the ST configuration is frozen, it must not be changed until
     *        the ``UnfreezeSTConfiguration`` method has been called!
     */
    virtual
    bool
    IsSTConfigurationFrozen() const {
        return isSTConfigVecFrozen_;
    }

    //__________________________________________________________________________
    /** Returns a const reference to the I3OMGeoMap object, that is used by
     *  this ST configuration service (to setup the spatial context).
     */
    const I3OMGeoMap&
    GetOMGeoMap() const {
        return *omGeoMapPtr_;
    }

    //__________________________________________________________________________
    /** Returns a (const) reference to the spatial context object.
     */
    const SContextType& GetSContext() const { return sContext_; }
    SContextType&       GetSContext()       { return sContext_; }

    //__________________________________________________________________________
    /** Returns a reference to the default ST configuration object.
     */
    const STConfigurationType&
    GetDefaultSTConfig() const
    {
        return defaultSTConfig_;
    }

    //__________________________________________________________________________
    /** The SetupSContext method should be re-implemented by the derived class,
     *  i.e the particular ST algorithm, and should setup the spatial ST context
     *  based on the given I3OMGeoMap object if, and only if, this method
     *  returns ``false``. Otherwise it should just do nothing.
     *  This method here checks if the ST configuration is frozen and
     *  throws an exception if not. Furthermore it checks if the ST
     *  configuration is complete, and if the given I3OMGeoMap object is seen
     *  by this ST configuration service by the first time.
     */
    virtual
    bool
    SetupSContext(const I3OMGeoMap &omGeoMap);

  protected:
    //__________________________________________________________________________
    /// The list of STConfigurationType objects. Because it should be possible
    /// to put the ST configuration into a frame, we need to use a boost::shared_ptr
    /// here.
    boost::shared_ptr< I3Vector<STConfigurationType> > stConfigVecPtr_;

    //__________________________________________________________________________
    /** The pointer to the I3OMGeoMap object for which the SContextType object
     *  is valid. This is needed in order to be able to setup the SContext only
     *  once for each geometry frame.
     */
    const I3OMGeoMap * omGeoMapPtr_;

    //__________________________________________________________________________
    /// The object for the spatial context valid for this ST configuration.
    SContextType sContext_;

  private:
    //__________________________________________________________________________
    /// The flag if the ST configuration (i.e. the stConfigVec_ member variable)
    /// is frozen.
    bool isSTConfigVecFrozen_;

    //__________________________________________________________________________
    /// The defaultSTConfig_ is a STConfiguationType object that will be
    /// returned by the GetSTConfigurationForOMLink method, when a specific OM
    /// link is not configured.
    STConfigurationType defaultSTConfig_;

    //__________________________________________________________________________
    SET_LOGGER("I3STConfigurationService");
};

//______________________________________________________________________________
template <
    class STConfigurationType,
    class SContextType
>
I3VectorOMKey
I3STConfigurationService<STConfigurationType, SContextType>::
GetOMKeys() const
{
    std::set<OMKey> omKeySet;
    BOOST_FOREACH(const STConfigurationType &stConfig, *stConfigVecPtr_)
    {
        const I3VectorOMKeyLinkSet &omKeyLinkSets = stConfig.GetOMKeyLinkSets();
        BOOST_FOREACH(const OMKeyLinkSet &omKeyLinkSet, omKeyLinkSets)
        {
            I3VectorOMKey omKeys = omKeyLinkSet.GetOMKeySet1().GetOMKeys();
            BOOST_FOREACH(const OMKey &omKey, omKeys)
            {
                omKeySet.insert(omKey);
            }
            omKeys = omKeyLinkSet.GetOMKeySet2().GetOMKeys();
            BOOST_FOREACH(const OMKey &omKey, omKeys)
            {
                omKeySet.insert(omKey);
            }
        }
    }

    return I3VectorOMKey(omKeySet.begin(), omKeySet.end());
}

//______________________________________________________________________________
template <
    class STConfigurationType,
    class SContextType
>
const STConfigurationType &
I3STConfigurationService<STConfigurationType, SContextType>::
GetSTConfigurationForOMLink(const OMKey &omKey1, const OMKey &omKey2) const
{
    const OMKeyLink omKeyLink(omKey1, omKey2);
    BOOST_FOREACH(const STConfigurationType &stConfig, *stConfigVecPtr_)
    {
        const I3VectorOMKeyLinkSet &omKeyLinkSets = stConfig.GetOMKeyLinkSets();
        BOOST_FOREACH(const OMKeyLinkSet &omKeyLinkSet, omKeyLinkSets)
        {
            if(omKeyLinkSet.Contains(omKeyLink))
            {
                return stConfig;
            }
        }
    }

    // The OM link was not configured explicitly, so we return the default ST
    // configuration object.
    return GetDefaultSTConfig();
}

//______________________________________________________________________________
template <
    class STConfigurationType,
    class SContextType
>
const typename SContextType::s_data_t&
I3STConfigurationService<STConfigurationType, SContextType>::
GetSDataForOMLink(const OMKey &omKey1, const OMKey &omKey2) const
{
    if(! IsSTConfigurationFrozen())
    {
        log_fatal(
            "The ST configuration is not frozen! Call the "
            "FreezeSTConfiguration method of the ST configuration service "
            "before calling this function!");
    }

    const typename SContextType::s_data_t& sData = GetSContext().GetSDataMap()->at(omKey1, omKey2);

    return sData;
}

//______________________________________________________________________________
template <
    class STConfigurationType,
    class SContextType
>
bool
I3STConfigurationService<STConfigurationType, SContextType>::
IsSTConfigurationComplete(const I3OMGeoMap &omGeoMap) const
{
    I3VectorOMKey omKeys = GetOMKeys();

    BOOST_FOREACH(const I3OMGeoMap::value_type &om, omGeoMap)
    {
        const OMKey &omkey = om.first;
        if (omkey.GetPMT() != 0)
            continue;
        if(std::find(omKeys.begin(), omKeys.end(), omkey) == omKeys.end())
        {
            log_debug(
                "%s was not found within the ST configuration service which "
                "configures %zu OMs, i.e. the ST configuration is not "
                "complete. The I3OMGeoMap contains %zu OMs.",
                omkey.str().c_str(), omKeys.size(), omGeoMap.size());
            return false;
        }
    }

    return true;
}

//______________________________________________________________________________
template <
    class STConfigurationType,
    class SContextType
>
bool
I3STConfigurationService<STConfigurationType, SContextType>::
SetupSContext(const I3OMGeoMap &omGeoMap)
{
    if(! IsSTConfigurationFrozen()) {
        log_fatal(
            "The ST configuration is not frozen, so no spatial context "
            "can be setup! Call the FreezeSTConfiguration method first!");
    }

    // Check if the given I3OMGeoMap object is a new one for which a new
    // spatial context needed to be generated. If not return ``true`` (i.e. the
    // spatial context had been constructed by this method).
    if(&omGeoMap == omGeoMapPtr_) {
        return true;
    }

    //--------------------------------------------------------------------------
    // Check if all OMKeys are present somehow in the ST configuration, which
    // are also present in the current geometry.
    if(! IsSTConfigurationComplete(omGeoMap))
    {
        I3VectorOMKey omKeys = GetOMKeys();
        log_fatal(
            "The given ST configuration does not include all OMs (#%zu), "
            "which are also present in the given geometry (#%zu)!",
            omKeys.size(), omGeoMap.size());
    }

    // We got a new I3OMGeoMap object. Save its address and assume that the
    // derived class setups the spatial context, when this method returns
    // ``false``.
    omGeoMapPtr_ = &omGeoMap;
    return false;
}

}// namespace sttools

#endif// ! STTOOLS_I3STCONFIGURATIONSERVICE_H_INCLUDED
