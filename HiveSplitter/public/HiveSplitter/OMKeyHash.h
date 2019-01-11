/**
 * \file OMKeyHash.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: PartialCOG.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 * 
 * Make a Hashing of OMkeys that is well ordered, complete, hierarchical and minimal injective into teh natural numbers (N) aka. unsigned integers
 * This makes it implicitly usable as an unique Hash and as a Index-range in regular array-apllications
 */

#ifndef OMKEYHASH_H
#define OMKEYHASH_H

#include "icetray/OMKey.h"

//number of strings in the detector
#define MAX_STRINGS 86 //DANGER HARDCODING
//number of OMs on each string
#define MAX_OMS 64 //DANGER HARDCODING
//effective number of Hashing range is MAX_STRINGS*MAX_OMS

namespace OMKeyHash {
  /// a hash type for OMKeys: indexing range [0, MAX_STRINGS*MAX_OMS-1]
  typedef unsigned int SimpleIndex;
  /// String numbers are unsigned integerts in the range [1, MAX_STRINGS]
  typedef unsigned int StringNbr;
  /// OM numbers are unsigned integerts in the range [1, MAX_OMS]
  typedef unsigned int OmNbr;

  /** @brief a simple transverse translator from a linear List index with offset 64 per string to StringNumber
    * @param simpleIndex the plain index
    * @return the string number
    */
  inline StringNbr SimpleIndex2StringNbr (const SimpleIndex simpleIndex)
    {return StringNbr(simpleIndex/MAX_OMS)+1;};

  /** @brief a simple transverse translator from a linear List index with offset 64 per string to OMNumber
    * @param simpleIndex the plain index
    * @return the om number
    */
  inline OmNbr SimpleIndex2OmNbr (const SimpleIndex simpleIndex)
    {return OmNbr((simpleIndex)%MAX_OMS)+1;};

  /** @brief a simple transverse translator from (string,OM) pair to a linear List index with offset 64 per string
    * @param string the string
    * @param om the om
    * @return the plain index
    */
  inline SimpleIndex String_OM_Nbr2SimpleIndex (const StringNbr string, const OmNbr om)
    {return (string-1)*MAX_OMS+(om-1);};

  /** @brief a simple transverse translator from OMKeys to a linear List index with offset 64 per string
    * @param omkey the omkey
    * @return the plain index
    */
  inline SimpleIndex OMKey2SimpleIndex (const OMKey omkey)
    {return (omkey.GetString()-1)*MAX_OMS+(omkey.GetOM()-1);};

  /** @brief a simple transverse translator from a linear List index with offset 64 per string to OMKeys
    * @param simpleIndex the plain index
    * @return the omkey
    */
  inline OMKey SimpleIndex2OMKey (const SimpleIndex simpleIndex)
    {return OMKey(SimpleIndex2StringNbr(simpleIndex), SimpleIndex2OmNbr(simpleIndex));};
};

#endif
