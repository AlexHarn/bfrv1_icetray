/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/I3SContext.h
 * @date $Date$
 * @brief This file contains the definition of the I3SContext class inside the
 *        sttools namespace. The I3SContext class is used to store internal
 *        API state variables like a spatial data map.
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
#ifndef STTOOLS_I3SCONTEXT_H_INCLUDED
#define STTOOLS_I3SCONTEXT_H_INCLUDED

#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <dataclasses/geometry/I3OMGeo.h>

#include <STTools/OMKeyPairMap.h>

//##############################################################################
namespace sttools {

//==============================================================================
template <class SDataType>
class I3SContext
{
  public:
    typedef SDataType s_data_t;

    //__________________________________________________________________________
    /** The default constructor.
     */
    I3SContext()
    {}

    //__________________________________________________________________________
    I3SContext(const I3OMGeoMap &omGeoMap, typename OMKeyPairMap<SDataType>::Symmetry sDataMapSym
    )
      : sDataMap_(boost::make_shared< OMKeyPairMap<SDataType> >(omGeoMap, sDataMapSym))
    {}

    //////
    // Public methods.
    //__________________________________________________________________________
    /** Returns a shared pointer to the spatial data map.
     */
    boost::shared_ptr< OMKeyPairMap<SDataType> >
    GetSDataMap() {
        return sDataMap_;
    }
    //--------------------------------------------------------------------------
    boost::shared_ptr< const OMKeyPairMap<SDataType> >
    GetSDataMap() const {
        return sDataMap_;
    }

    //__________________________________________________________________________
    /** Constructs a new spatial data map and sets it to the context.
     *
     *  @param omGeoMap The I3OMGeoMap object for which the spatial data
     *      map will be valid.
     *  @param sDataMapSym The OMKey pair symmetry of the spatial data map.
     *
     *  @note An already constructed spatial data map will get destroyed if no
     *        shared pointer to the old spatial data map was retrieved via
     *        the ``GetSDataMap`` method before.
     */
    void
    ConstructSDataMap(
        const I3OMGeoMap &omGeoMap,
        typename OMKeyPairMap<SDataType>::Symmetry sDataMapSym
    ) {
        sDataMap_ = boost::make_shared< OMKeyPairMap<SDataType> >(omGeoMap, sDataMapSym);
    }

  protected:
    //__________________________________________________________________________
    /** The OMKeyPairMap, that stores the spatial data for each possible
     *  OMKey pair of the detector.
     */
    boost::shared_ptr< OMKeyPairMap<SDataType> > sDataMap_;

  private:
    SET_LOGGER("I3SContext")
};

//==============================================================================

}/*sttools*/
//##############################################################################

#endif//STTOOLS_I3SCONTEXT_H_INCLUDED
