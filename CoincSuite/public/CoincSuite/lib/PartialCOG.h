/**
 * \file PartialCOG.h
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 */

#ifndef PARTIALCOG_H
#define PARTIALCOG_H

#include <algorithm>
#include <cassert>
#include <limits>
#include <list>
#include <map>
#include <sstream>

#include "icetray/I3ConditionalModule.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

#include <boost/make_shared.hpp>

#include "OMKeyHash.h"
#include "HitSorting.h"

namespace PartialCOG{
  /// give me a 4-vector
  typedef std::pair<I3Position, double> PosTime;
  /// a cache for DOM-positions indexed by Hashed OMKeys
  typedef std::vector<I3Position> DomPos_cache;
  
  /**Build the DOMposition cache
  * @param geo the geometry
  * @return a ordered vector holding all I3Positions of the hashed OMKeys
  */
  std::vector<I3Position> BuildDomPos_cache(const I3Geometry &geo);

  /** compute COG in pos and time of the fraction of DOMs using absolut position/numbers
   * @param hits map of OMKeys and time of their first hits
   * @param domPos_cache a ordered vector holding all I3Positions of the hashed OMKeys
   * @param nFrac number of fractions that should be used
   * @param useFrac use that fraction (always absolute counting from the front)
   * @param heads_tails counting from front (true) or back (false) within the fraction
   * @param min_inc include at least that many doms in the COG
   * @param max_inc include no more than that many doms in the COG
   * @param useCharge weight the COG by the charge of each hit
   * @return a pair of COG position and time (as average of the configured pulses)
   */
  std::pair < I3Position, double > PartialCOGhit (const HitSorting::TimeOrderedHitSet& hits,
    const std::vector<I3Position>& domPos_cache,
    const uint nFrac,
    const uint useFrac,
    const bool heads_tails,
    const uint min_inc,
    const uint max_inc,
    const bool useCharge=true);

  /** compute COG in pos and time of the fraction of DOMs using accumulated charge
   * @param hits map of OMKeys and time of their first hits
   * @param domPos_cache a ordered vector holding all I3Positions of the hashed OMKeys
   * @param nFrac number of fractions that should be used
   * @param useFrac use that fraction (always absolute counting from the front)
   * @param heads_tails counting from front (true) or back (false) within the fraction
   * @param min_inc include at least that many hits in the COG
   * @param max_inc include no more than that many hits in the COG
   * @note the cog is always charge-weighted
   * @return a pair of COG position and time (as average of the configured pulses)
   */
  std::pair < I3Position, double > PartialCOGcharged (const HitSorting::TimeOrderedHitSet& hits,
    const std::vector<I3Position>& domPos_cache,
    const uint nFrac,
    const uint useFrac,
    const bool heads_tails,
    const uint min_inc,
    const uint max_inc);

  /** compute COG in pos and time of the fraction of DOMs using exclusions
   * @param hits map of OMKeys and time of their first hits
   * @param domPos_cache a ordered vector holding all I3Positions of the hashed OMKeys
   * @param heads_tails counting from front (true) or back (false) within the fraction
   * @param n_many number of hits to include 
   * @param n_exclude number of hits to exclude up front, before DOMs are being included
   * @param useCharge weight the COG by the charge of each hit
   * @return a pair of COG position and time (as average of the configured pulses)
   */
  std::pair < I3Position, double > PartialCOGexclude (const HitSorting::TimeOrderedHitSet& hits,
    const std::vector<I3Position>& domPos_cache,
    const bool heads_tails,
    const uint n_many,
    const uint n_exclude=0,
    const bool useCharge=true);
};

#endif
