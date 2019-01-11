/**
 * Copyright (C) 2011 - 2013
 * Martin Wolf <martin.wolf@fysik.su.se>
 * Robert Franke
 * Keven J. Meagher
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file OMKeyPairMap.h
 * @date $Date$
 * @brief This file contains the definition of the OMKeyPairMap template class
 *        within the sttools namespace.
 *
 *        An OMKeyPairMap is an array that represents an OMKeyPair to
 *        <something> map. It can be used for example to signify if an OMKey
 *        pair fulfills some condition (for example a spatial condition).
 *
 *        Since it is implemented as a contiguous C-style array, reading its
 *        content is extemly fast, what is required when a map is created only
 *        once (based on an I3OMGeoMap from a Geometry frame) and then read
 *        several times by each event, as it is usually the case for ST
 *        algorithms.
 *
 *        The OMKey pair map has a symmetry property, that defines the symmetry
 *        between the two OMKeys of an OMKeyPair object. If the relationship
 *        between the two OMKeys is symmetric, the required memory to store the
 *        map is only approx. half of the required memory to store a map for
 *        an asymmetric relationship. However, the default setting for the
 *        symmetry of the OMKeyPairMap object is an asymmetric map.
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
#ifndef STTOOLS_OMKEYPAIRMAP_H_INCLUDED
#define STTOOLS_OMKEYPAIRMAP_H_INCLUDED

#include "dataclasses/I3Vector.h"
#include "dataclasses/geometry/I3OMGeo.h"

#include "STTools/OMKeyHasher.h"
#include "recclasses/OMKeyPair.h"

//##############################################################################
namespace sttools {

template<class T>
class OMKeyPairMap
{
  public:
    enum Symmetry {
        // The order of the OMKeys in an OMKeyPair matters and they
        // are allowed to be equal.
        AsymAndEqualOMKeysAreAllowed = 1,
        // The order of the OMKeys in an OMKeyPair does not matter and they
        // are allowed to be equal.
        SymAndEqualOMKeysAreAllowed = 2,
        // The order of the OMKeys in an OMKeyPair does not matter, but they
        // are not allowed to be equal.
        SymAndEqualOMKeysAreForbidden = 3
    };

    //__________________________________________________________________________
    // Mimic the STL a bit.
    typedef OMKeyPair key_type;
    typedef T mapped_type;

    //__________________________________________________________________________
    /**
     * Constructor, taking an I3OMGeoMap object as input to construct the
     * OMKeyHasher object.
     */
    OMKeyPairMap(
        const I3OMGeoMap &i3OMGeoMap,
        Symmetry symmetry=AsymAndEqualOMKeysAreAllowed
    )
      : omKeyHasher_(OMKeyHasher(i3OMGeoMap)),
        symmetry_(symmetry)
    {
        init();
    }

    //__________________________________________________________________________
    /**
     * Constructor, taking an I3VectorOMKey object as input to construct the
     * OMKeyHasher object.
     */
    OMKeyPairMap(
        const I3VectorOMKey &omKeys,
        Symmetry symmetry=AsymAndEqualOMKeysAreAllowed
    )
      : omKeyHasher_(OMKeyHasher(omKeys)),
        symmetry_(symmetry)
    {
        init();
    }

    //__________________________________________________________________________
    ~OMKeyPairMap()
    {
        log_debug("Destructing OMKeyPairMap object.");
        delete[] map_;
        map_ = NULL;
    }

    //__________________________________________________________________________
    inline
    const OMKeyHasher&
    GetOMKeyHasher() const {
        return omKeyHasher_;
    }

    //__________________________________________________________________________
    /** Gets a (const) reference to the mapped_type value for the given map
     *  array index.
     *
     *  @param idx The map array index. Must be in the range [0, size_).
     *  @return The reference to the mapped_type value for the given indexed
     *          element.
     */
    inline
    mapped_type&
    at(const size_t idx)
    {
        if(idx >= size_)
        {
            log_fatal("The map element index %zu is out of range, i.e. greater "
                      "than %u.", idx, size_-1);
        }

        return map_[idx];
    }
    //--------------------------------------------------------------------------
    inline
    const mapped_type&
    at(const size_t idx) const
    {
        if(idx >= size_)
        {
            log_fatal("The map element index %zu is out of range, i.e. greater "
                      "than %u.", idx, size_-1);
        }

        return map_[idx];
    }

    //__________________________________________________________________________
    /** Gets a (const) reference to the mapped_type value for the given two
     *  OMKey objects forming an OMKey pair.
     *
     *  @param omKey1 The OMKey object of the first DOM.
     *  @param omKey2 The OMKey object of the second DOM.
     *  @return The reference to the mapped_type value for the given OMKeyPair
     *      object.
     */
    inline
    mapped_type&
    at(const OMKey &omKey1, const OMKey &omKey2)
    {
        return map_[(*this.*GetIndex)(omKey1, omKey2)];
    }
    //--------------------------------------------------------------------------
    inline
    const mapped_type&
    at(const OMKey &omKey1, const OMKey &omKey2) const
    {
        return map_[(*this.*GetIndex)(omKey1, omKey2)];
    }

    //__________________________________________________________________________
    /** Gets a (constant) reference to the mapped_type value for the given
     *  OMKeyPair object.
     *
     *  @param pair The OMKeyPair object.
     *  @return The reference to the mapped_type value for the given OMKeyPair
     *      object.
     */
    inline
    mapped_type&
    at(const OMKeyPair &pair)
    {
        return map_[(*this.*GetIndex)(pair.GetKey1(), pair.GetKey2())];
    }
    //--------------------------------------------------------------------------
    inline
    const mapped_type&
    at(const OMKeyPair &pair) const
    {
        return map_[(*this.*GetIndex)(pair.GetKey1(), pair.GetKey2())];
    }

    //__________________________________________________________________________
    /** Gets a non-constant or constant reference to the mapped_type value for
     *  the given OMKeyPair object. This is a shortcut for the ``at`` method.
     */
    inline
    mapped_type&
    operator[](const OMKeyPair &pair)
    {
        return at(pair);
    }
    //--------------------------------------------------------------------------
    inline
    const mapped_type&
    operator[](const OMKeyPair &pair) const
    {
        return at(pair);
    }

    //__________________________________________________________________________
    /** Returns the size (number of elements) of the internal map array used to
     *  implement the map.
     */
    inline
    uint32_t
    GetSizeOfMapArray() const
    {
        return size_;
    }

    //__________________________________________________________________________
    /** Returns the memory size in bytes of the internal map array used to
     *  implement the map.
     */
    inline
    size_t
    GetMemSizeOfMapArray() const
    {
        if(map_) {
            return size_*sizeof(*map_);
        }
        return 0;
    }

  protected:
    //__________________________________________________________________________
    /**
     * Initializes this OMKeyPairMap based on the OMKeyHasher and Symmetry
     * objects.
     */
    void
    init()
    {
        const uint32_t n = omKeyHasher_.GetSize();
        if(symmetry_ == AsymAndEqualOMKeysAreAllowed)
        {
            size_ = n*n;
            GetIndex = &OMKeyPairMap::GetIndex_AsymAndEqualOMKeysAreAllowed;
        }
        else if(symmetry_ == SymAndEqualOMKeysAreAllowed)
        {
            size_ = (n*n - n)/2 + n;
            GetIndex = &OMKeyPairMap::GetIndex_SymAndEqualOMKeysAreAllowed;
        }
        else if(symmetry_ == SymAndEqualOMKeysAreForbidden)
        {
            size_ = (n*n - n)/2;
            GetIndex = &OMKeyPairMap::GetIndex_SymAndEqualOMKeysAreForbidden;
        }

        map_ = new T[size_];

        log_debug(
            "Created new OMKeyPairMap: "
            "Symmetry: %d, "
            "MinString: %d, MinOM: %u, "
            "StringRange: %u, OMRange: %u, "
            "Size: %u elements (%zu bytes (%.3f Mbytes))",
            symmetry_,
            omKeyHasher_.GetMinString(),
            omKeyHasher_.GetMinOM(),
            omKeyHasher_.GetStringRange(),
            omKeyHasher_.GetOMRange(),
            GetSizeOfMapArray(), GetMemSizeOfMapArray(),
            ((double)GetMemSizeOfMapArray())/(1024*1024));
    }

    //__________________________________________________________________________
    /// The OMKey hasher object for the particular I3OMGeoMap.
    OMKeyHasher omKeyHasher_;

    //__________________________________________________________________________
    /// The kind of symmetry that should be valid for this map of OMKey pairs.
    Symmetry symmetry_;

    //__________________________________________________________________________
    /// The size of the map, e.g. how many array elements there are.
    uint32_t size_;

    //__________________________________________________________________________
    /// T array implementing the internal map data.
    T *map_;

    //__________________________________________________________________________
    /// The member function pointer that points to the appropriate GetIndex_...
    /// method depending on the symmetry of this OMKey pair map.
    uint32_t (OMKeyPairMap::*GetIndex)(const OMKey &omKey1, const OMKey &omKey2) const;

    //__________________________________________________________________________
    /**
     * Gets the map array index for the given OMKeyPair object for an asymmetric
     * OMKey pair map where OMKey pairs with the same OMKeys are allowed.
     *
     * @param omKey1 The OMKey object of the first DOM.
     * @param omKey2 The OMKey object of the second DOM.
     * @returns The map index for the given OMKeyPair object.
     */
    uint32_t GetIndex_AsymAndEqualOMKeysAreAllowed(const OMKey &omKey1, const OMKey &omKey2) const;

    //__________________________________________________________________________
    /**
     * Gets the map array index for the given OMKeyPair object for a symmetric
     * OMKey pair map where OMKey pairs with the same OMKeys are allowed.
     *
     * @param omKey1 The OMKey object of the first DOM.
     * @param omKey2 The OMKey object of the second DOM.
     * @returns The map index for the given OMKeyPair object.
     */
    uint32_t GetIndex_SymAndEqualOMKeysAreAllowed(const OMKey &omKey1, const OMKey &omKey2) const;

    //__________________________________________________________________________
    /**
     * Gets the map array index for the given OMKeyPair object for a symmetric
     * OMKey pair map where OMKey pairs with the same OMKeys are forbidden.
     *
     * @param omKey1 The OMKey object of the first DOM.
     * @param omKey2 The OMKey object of the second DOM.
     * @returns The map index for the given OMKeyPair object.
     */
    uint32_t GetIndex_SymAndEqualOMKeysAreForbidden(const OMKey &omKey1, const OMKey &omKey2) const;

  private:
    SET_LOGGER("OMKeyPairMap");
};

//______________________________________________________________________________
template<class T>
uint32_t
OMKeyPairMap<T>::
GetIndex_AsymAndEqualOMKeysAreAllowed(const OMKey &omKey1, const OMKey &omKey2) const
{
    const uint32_t h1 = omKeyHasher_.HashOMKey(omKey1);
    const uint32_t h2 = omKeyHasher_.HashOMKey(omKey2);

    const uint32_t idx = h1 * omKeyHasher_.GetSize() + h2;

    if(idx >= size_) {
        log_fatal(
            "The map index %u is out of range! Map size: %u",
            idx, size_);
    }

    return idx;
}

//______________________________________________________________________________
template<class T>
uint32_t
OMKeyPairMap<T>::
GetIndex_SymAndEqualOMKeysAreAllowed(const OMKey &omKey1, const OMKey &omKey2) const
{
    uint32_t h1;
    uint32_t h2;
    if((omKey2 <  omKey1) ||
       (omKey1 == omKey2)
    ) {
        h1 = omKeyHasher_.HashOMKey(omKey1);
        h2 = omKeyHasher_.HashOMKey(omKey2);
    }
    else {
        // We need to switch the two OMKeys in order to be conform with the
        // internal map memory alignment.
        h1 = omKeyHasher_.HashOMKey(omKey2);
        h2 = omKeyHasher_.HashOMKey(omKey1);
    }

    const uint32_t idx = h1*(h1 + 1)/2 + h2;

    if(idx >= size_) {
        log_fatal_stream(
            "The map index " << idx << " is out of range! Map size: " << size_);
    }

    return idx;
}

//______________________________________________________________________________
template<class T>
uint32_t
OMKeyPairMap<T>::
GetIndex_SymAndEqualOMKeysAreForbidden(const OMKey &omKey1, const OMKey &omKey2) const
{
    if(omKey1 == omKey2) {
        log_fatal_stream(
            "This OMKeyPairMap object forbids OMKey pairs with equal "
            "OMKeys! Given OMKeyPair: " << OMKeyPair(omKey1, omKey2));
    }

    uint32_t h1;
    uint32_t h2;
    if(omKey2 < omKey1) {
        h1 = omKeyHasher_.HashOMKey(omKey1);
        h2 = omKeyHasher_.HashOMKey(omKey2);
    }
    else {
        // We need to switch the two OMKeys in order to be conform with the
        // internal map memory alignment.
        h1 = omKeyHasher_.HashOMKey(omKey2);
        h2 = omKeyHasher_.HashOMKey(omKey1);
    }

    const uint32_t idx = h1*(h1 - 1)/2 + h2;

    if(idx >= size_) {
        log_fatal(
            "The map index %u is out of range! Map size: %u",
            idx, size_);
    }

    return idx;
}

}/*sttools*/
//##############################################################################
#endif // STTOOLS_OMKEYPAIRMAP_H_INCLUDED
