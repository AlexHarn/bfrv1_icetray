/**
 * \file OMKeyHash.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: OMKeyHash.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
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
  
  ///number of strings in the detector
  static const unsigned int MAX_STRINGS(86); //NOTE HARDCODING
  ///number of OMs on each string
  static const unsigned int MAX_OMS(64); //NOTE HARDCODING
  ///effective number of hashing range
  static const unsigned int MAX_SIMPLEINDEX = MAX_STRINGS*MAX_OMS-1;
  
  /// a hash type for OMKeys: indexing range [0, MAX_STRINGS*MAX_OMS-1]
  typedef unsigned int SimpleIndex;
  /// String numbers are unsigned integers in the range [1, MAX_STRINGS]
  typedef unsigned int StringNbr;
  /// OM numbers are unsigned integers in the range [1, MAX_OMS]
  typedef unsigned int OmNbr;

  /** @brief convert a SimpleIndex to an string-number
    * @param simpleIndex the plain index
    * @return the string number
    */
  inline StringNbr SimpleIndex2StringNbr (const SimpleIndex simpleIndex)
    {return StringNbr(simpleIndex/MAX_OMS)+1;};

  /** @brief convert a SimpleIndex to an OM-number
    * @param simpleIndex the plain index
    * @return the OM-number
    */
  inline OmNbr SimpleIndex2OmNbr (const SimpleIndex simpleIndex)
    {return OmNbr((simpleIndex)%MAX_OMS)+1;};

  /** @brief convert a combination string and OM-number to a SimpleIndex
    * @param string the string-number
    * @param om the OM-number
    * @return the plain index
    */
  inline SimpleIndex StringOmNbr2SimpleIndex (const StringNbr string, const OmNbr om)
    {return (string-1)*MAX_OMS+(om-1);};

  /** @brief convert an OMKey to a SimpleIndex
    * @param omkey the OMKey
    * @return the plain index
    */
  inline SimpleIndex OMKey2SimpleIndex (const OMKey &omkey)
    {return (omkey.GetString()-1)*MAX_OMS+(omkey.GetOM()-1);};
    
  /** @brief convert an ModuleKey to a SimpleIndex
    * @param mkey the ModuleKey
    * @return the plain index
    */
  inline SimpleIndex ModuleKey2SimpleIndex (const ModuleKey &mkey)
    {return (mkey.GetString()-1)*MAX_OMS+(mkey.GetOM()-1);};

  /** @brief convert a SimpleIndex to an OMKey
    * @param simpleIndex the plain index
    * @return the OMKey
    */
  inline OMKey SimpleIndex2OMKey (const SimpleIndex simpleIndex)
    {return OMKey(SimpleIndex2StringNbr(simpleIndex), SimpleIndex2OmNbr(simpleIndex));};
    
  /** @brief convert a SimpleIndex to an ModuleKey
    * @param simpleIndex the plain index
    * @return the ModuleKey
    */
  inline ModuleKey SimpleIndex2ModuleKey (const SimpleIndex simpleIndex)
    {return ModuleKey(SimpleIndex2StringNbr(simpleIndex), SimpleIndex2OmNbr(simpleIndex));};
};

#endif //OMKEYHASH_H
