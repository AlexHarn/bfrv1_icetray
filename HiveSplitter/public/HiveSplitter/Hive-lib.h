/**
 * \file Hive-lib.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: Hive-lib.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 */

#ifndef HIVE_LIB_H
#define HIVE_LIB_H

#include <stdlib.h>
#include <vector>
#include <set>
#include <map>

#include "icetray/OMKey.h"

typedef unsigned int uint; //shorthand

namespace honey{
  /// a set of strings can define a ring (around a central string)
  typedef std::set<uint> Ring;
  
  /// A class that stores a central string and its surrounding rings; NOTE Ring 0 is identical to center
  class DOMHoneyComb {
    private:
    /// member: storing the index of the ring [0...n] and the contained strings;
    std::vector<std::set<unsigned int> > honey_;

    public:
    /// get the central string
    uint GetCenter() const;
    
    /// add the central string
    void AddCenter(const uint center);
    
    /// set the central string
    void SetCenter(const uint center);
    
    /// get the number of implemented Rings
    uint GetNRings() const;
    
    /** get the strings of the ring [ringnbr]
     * @param ringnbr number of the ring that should be returned
     * @return the strings contained on [ringnbr]
     */
    std::set<uint> GetRing(const uint ringnbr) const;

    /** Adds the strings of ring [ringnbr] to honey
     * @param ringnbr number of the ring to set
     * @param ring strings contained on that ring
     */
    void AddRing(const uint ringnbr, const std::set<uint> &ring);
    
    /** erases the previous ring [ringnbr], set the new strings of ring
     * @param ringnbr number of the ring to set
     * @param ring strings contained on that ring
     */
    void SetRing(const uint ringnbr, const std::set<uint> &ring);
    
    /** @brief Constructor with NO argument
     */
    DOMHoneyComb();
    
    /** @brief Constructor with 1 ring
     * @param center the center of the HoneyComb
     * @param ring1 strings that belong to the first ring
     */
    DOMHoneyComb(const uint center, const std::set<unsigned int>& ring1);
    
    /** @brief Constructor with 2 rings
     * @param center the center of the HoneyComb
     * @param ring1 strings that belong to the first ring
     * @param ring2 strings that belong to the second ring 
     */
    DOMHoneyComb(const uint center, const std::set<unsigned int>& ring1, const std::set<unsigned int>& ring2);
    
    /** @brief constructor with arbitrary number of rings
     * @param center the center of the HoneyComb
     * @param rings vector of rings and their contained strings
     */
    DOMHoneyComb(const uint center, std::vector< std::set<uint> >& rings);

    /** @brief Add the single string [string] to the honeycomb at central string [center] on ring [ringnbr]
     * @param center the central string
     * @param ringnbr the ring on which to add the string to
     * @param string that should be added
     */
    void AddStringToRing(const uint center, const uint ringnbr, const uint string);
  };

  ///Look-up table for the HoneyCombs, indexed by stringnumber
  typedef std::map<uint, DOMHoneyComb> DOMHoneyCombRegister;

  ///complex type that holds all magic information
  struct Hive{
    /// realtive scaling factor in comparison to regular IceCube string spacings
    uint scaleFactor_;
    /// the Register holding all information
    DOMHoneyCombRegister combs_;
    /// the maximum amount of registered rings
    uint max_rings_;
    ///constructor
    Hive(const uint scaleFactor, const DOMHoneyCombRegister& combs, uint max_rings):
      scaleFactor_(scaleFactor),
      combs_(combs),
      max_rings_(max_rings) {};
  };

  /** @brief Register this string [center_A] as on ring[ringnbr] on string [center_B] and vise vers
   * @param combs the combs these strings should be registered to
   * @param center_A the string to register
   * @param center_B the other string to register
   * @param ringnbr the ring that strings should be registered to; should not be further than number of already registered rings plus 1
   */
  void MutualAddStringToRing(DOMHoneyCombRegister &combs, const uint center_A, const uint center_B, const uint ringnbr);
  
  /** @brief is this [string] on ring[ringnbr] of [center]
   * @param combs the combs to look up this ring
   * @param center center string around to look
   * @param ringnbr ring number around center the string should be located on
   * @param string the string to be found
   * @return true, if string could be correctly located;
   * false, if not;
   */
  bool IsRingX (const DOMHoneyCombRegister &combs, const uint center, const uint ringnbr, const uint string);
  
  ///shorthand
  inline bool IsRing1(DOMHoneyCombRegister &combs, const uint center, const uint string);
  
  ///shorthand
  inline bool IsRing2(DOMHoneyCombRegister &combs, const uint center, const uint string);

  /** @brief Which ring is this on?
   * @param combs the  Combs to look this strings up on
   * @param center the center from which the ring should be found
   * @param string the string to locate on any ring
   * @param search_depth the maximum ring that this string is searched for (does limit the amount of computation)
   * @return 0, if it is the center;
   * n, if it is on ring n;
   * -1, if string is not registered;
   */
  int WhichRing(const DOMHoneyCombRegister &combs, const uint center, const uint string, const uint search_depth=1000); //DANGER hardcoding of a rediciulous huge number

  /** @brief Build the HoneyCombs for IC86 all strings treated as Pingu
   * @return the HoneyRegister for IC86
   */
//  DOMHoneyCombRegister BuildCompleteTrippleDenseHoneyCombRegister (); //TODO DISABLED FOR NOW
  
  /** @brief Abstract a combs to next smaller RingSize definitions
   * The assignment is done by prescribed rings :
   * combs[ring0]->abstracted[ring0],
   * combs[ring1]->abstracted[ring2],
   * combs[ring2]->abstracted[ring4], ...
   * @param combs the filled Combs to abstract
   * @return the abstracted Hive
   */
  DOMHoneyCombRegister Abstract(const DOMHoneyCombRegister combs);
  
  /** @brief Expands a combs with the next ring
   * The combs must have at least 1 ring assigned and equal ring size on all strings,
   * it must also be a mutually complete (every string links against each other)
   * @param combs the filled Hive to expand
   * @return n number of newly added ring
   *         -1 if not possible
   */
  //TODO this function can substitute the direct initialisation of second ring.
  //NOTE DEPRICATED : use ExpandRings() instead
  int ExpandToNextRing(DOMHoneyCombRegister &combs);
  
  /** @brief Expands combs and adds new rings to it
   * The combs must have at least 1 ring assigned and equal ring size on all strings,
   * it must also be a mutually complete (every string links against each other)
   * @param combs the filled combs to expand
   * @param scale_factor expand by using that many rings (this should be a scale to the next complete reverse abstraction)
   * @return n number of newly added ring
   *         -1 if not possible
   */
  //TODO this is not thouroughly tested; however it seems to work
  int ExpandRings(DOMHoneyCombRegister &combs, const uint scale_factor=1);
  
  /** @brief Unite two combs into one
   * @param combs1 the first combs to unite
   * @param combs2 the second combs to unite
   * @return the new united Hives
   */
  DOMHoneyCombRegister UniteHive (const DOMHoneyCombRegister& combs1, const DOMHoneyCombRegister& combs2);
  
  /** @brief Dump an entire entry of an combs
   * @param combs the Hive to dump the entry from
   * @param center the entry to dump
   * @return the dumped central string
   */
  std::string DumpCenter (const DOMHoneyCombRegister &combs, const uint center);

  /** @brief Dump an entire hive
   * @param hive the Hive to dump
   * @return the dumped hive
   */
  std::string DumpHive (const Hive &hive);

  /** @brief Reads an entire Hive from a configuration file
   * @param fileName the path of the file to read from
   */
  Hive ReadHiveFromFile (const std::string& fileName);
};


///Namespace that provides information of Strings and DOMs to which Detector Topology they belong
///Definitions can be personal prference and are subject to change, once the IC86 detector should be further expanded 
namespace Topology{  
  /// Is this a DOM in DeepCore?
  bool IsDCDOM(const OMKey& omkey);
  
  /// Is this DOM in the DeepCore Veto layer?
  bool IsVetoCapDOM(const OMKey& omkey);
  
  /// Is this Dom in the denser populated DeepCore region?
  bool IsDCFidDOM(const OMKey& omkey);
  
  /// Is this DOM in the denser populated  DeepCore Veto Region?
  bool IsVetoCapFidDOM(const OMKey& omkey);
  
  /// Is this DOM in the undense populated regions?
  bool IsDC_VetoCapFidDOM(const OMKey& omkey);
  
  /// Is this DOM in IceTop?
  bool IsIceTopDOM(const OMKey& omkey);
  
  // just strings
  /// Is this string in the DeepCore array?
  bool IsDCString(const OMKey& omkey);
  
  /// Is this string in the Pingu (DeepCore infill) array
  bool IsPinguString(const OMKey& omkey) ;

  /// is this string a in the regular IceCube array?
  bool IsICString(const OMKey& omkey);
};

#endif