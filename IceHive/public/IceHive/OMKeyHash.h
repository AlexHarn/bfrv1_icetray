/**
 * \file OMKeyHash.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 *
 * Provide hashing of OMKeys and ModuleKeys that is well ordered, complete, hierarchical and minimal injective into the natural numbers (N) aka. unsigned integers
 * This makes it implicitly usable as an unique Hash (bidirectional) and as a Index-range in regular array-applications
 */

#ifndef OMKEYHASH_H
#define OMKEYHASH_H

#include "icetray/OMKey.h"
#include "dataclasses/ModuleKey.h"

///Defines hashing of OMKeys and ModuleKeys
namespace OMKeyHash {

  /// number of strings in the detector
  static const unsigned int MAX_STRINGS(86); // NOTE HARDCODING
  /// number of OMs on each string
  static const unsigned int MAX_OMS(64); // NOTE HARDCODING
  /// effective number of hashing range
  static const unsigned int MAX_SIMPLEINDEX = MAX_STRINGS*MAX_OMS-1;

  /// a hash type for OMKeys: indexing range [0, MAX_STRINGS*MAX_OMS-1]
  typedef unsigned int SimpleIndex;
  /// string numbers are unsigned integers in the range [1, MAX_STRINGS]
  typedef unsigned int StringNbr;
  /// OM numbers are unsigned integers in the range [1, MAX_OMS]
  typedef unsigned int OmNbr;

  /** @brief convert SimpleIndex to string number
    * @param simpleIndex plain index
    * @return string number
    */
  inline StringNbr SimpleIndex2StringNbr (const SimpleIndex simpleIndex)
  {
      if (simpleIndex <= MAX_SIMPLEINDEX)
      {
          return StringNbr(simpleIndex / MAX_OMS) + 1;
      }
      else
      {
          log_fatal("Index %d is not in range [0, %d].", simpleIndex, MAX_SIMPLEINDEX);
      }
  };

  /** @brief convert SimpleIndex to OM number
    * @param simpleIndex plain index
    * @return OM number
    */
  inline OmNbr SimpleIndex2OmNbr (const SimpleIndex simpleIndex)
  {
      if (simpleIndex <= MAX_SIMPLEINDEX)
      {
          return OmNbr(simpleIndex % MAX_OMS) + 1;
      }
      else
      {
          log_fatal("Index %d is not in range [0, %d].", simpleIndex, MAX_SIMPLEINDEX);
      }
  };

  /** @brief convert pair of string and OM numbers to SimpleIndex
    * @param string string number
    * @param om OM number
    * @return plain index
    */
  inline SimpleIndex StringOmNbr2SimpleIndex (const StringNbr string, const OmNbr om)
  {
      if (string >= 1 && string <= MAX_STRINGS)
      {
          if (om >= 1 && om <= MAX_OMS)
          {
              return (string - 1)*MAX_OMS + (om - 1);
          }
          else
          {
              log_fatal("OM number %d is not in range [1, %d].", om, MAX_OMS);
          }
      }
      else
      {
          log_fatal("String number %d is not in the range [1, %d].", string, MAX_STRINGS);
      }
  };

  /** @brief convert OMKey to SimpleIndex
    * @param omkey OMKey
    * @return plain index
    */
  inline SimpleIndex OMKey2SimpleIndex (const OMKey &omkey)
  {
      return StringOmNbr2SimpleIndex(omkey.GetString(), omkey.GetOM());
  };

  /** @brief convert ModuleKey to SimpleIndex
    * @param mkey ModuleKey
    * @return plain index
    */
  inline SimpleIndex ModuleKey2SimpleIndex (const ModuleKey &mkey)
  {
      return StringOmNbr2SimpleIndex(mkey.GetString(), mkey.GetOM());
  };

  /** @brief convert SimpleIndex to OMKey
    * @param simpleIndex plain index
    * @return OMKey
    */
  inline OMKey SimpleIndex2OMKey (const SimpleIndex simpleIndex)
  {
      return OMKey(SimpleIndex2StringNbr(simpleIndex), SimpleIndex2OmNbr(simpleIndex));
  };

  /** @brief convert SimpleIndex to ModuleKey
    * @param simpleIndex plain index
    * @return ModuleKey
    */
  inline ModuleKey SimpleIndex2ModuleKey (const SimpleIndex simpleIndex)
  {
      return ModuleKey(SimpleIndex2StringNbr(simpleIndex), SimpleIndex2OmNbr(simpleIndex));
  };

  /** @brief check if pair of string and OM numbers is hashable
    * @param string string number
    * @param om OM number
    * @return boolean that specifies if the pair is hashable
    */
  inline bool IsHashable (const StringNbr string, const OmNbr om)
  {
      return (string >= 1 && string <= MAX_STRINGS && om >= 1 && om <= MAX_OMS);
  };

  /** @brief check if OMKey is hashable
    * @param omkey OMKey
    * @return boolean that specifies if the OMKey is hashable
    */
  inline bool IsHashable (const OMKey &omkey)
  {
      return IsHashable(omkey.GetString(), omkey.GetOM());
  };
};

#endif //OMKEYHASH_H
