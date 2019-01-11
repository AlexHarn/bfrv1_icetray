/**
 * \file Hive-lib.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 */

#ifndef HIVE_LIB_H
#define HIVE_LIB_H

#include <cstdlib>
#include <vector>
#include <set>
#include <map>

#include "icetray/OMKey.h"

/** In this namespace build objects which have hexagonal structure, just like IceCube:
 * That is strings surrounded by rings of strings aka honey-Combs in different sizes.
 * Each 
 */
namespace honey{
  /// an alias for what it is
  typedef unsigned int StringNbr;
  /// a set of strings define a ring (around a central string)
  typedef std::set<StringNbr> Ring;
  
  /// A class that stores a central string and its surrounding rings; NOTE Ring 0 is the center-string
  class DOMHoneyComb {
    private:
    /// member: storing the index of the ring [0...n] and the contained strings;
    std::vector<Ring> honey_;

    public:
    /// get the central string
    StringNbr GetCenter() const;
    
    /// add the central string
    void AddCenter(const StringNbr center);
    
    /// set the central string
    void SetCenter(const StringNbr center);
    
    /// get the number of implemented Rings
    inline unsigned int GetNRings() const
      {return honey_.size()-1;};

    /** get the strings of the ring [ringnbr]
     * @param ringnbr number of the ring that should be returned
     * @return the strings contained on [ringnbr]
     */
    Ring GetRing(const unsigned int ringnbr) const;

    /** Adds the strings of ring [ringnbr] to honey
     * @param ringnbr number of the ring to set
     * @param ring strings contained on that ring
     */
    void AddRing(const unsigned int ringnbr, const Ring &ring);
    
    /** erases the previous ring [ringnbr], set the new strings of ring
     * @param ringnbr number of the ring to set
     * @param ring strings contained on that ring
     */
    void SetRing(const unsigned int ringnbr, const Ring &ring);
    
    /** @brief Constructor with NO argument
     */
    DOMHoneyComb();
    
    /** @brief Constructor with 1 ring
     * @param center the center of the HoneyComb
     * @param ring1 strings that belong to the first ring
     */
    DOMHoneyComb(const StringNbr center, const Ring& ring1);
    
    /** @brief Constructor with 2 rings
     * @param center the center of the HoneyComb
     * @param ring1 strings that belong to the first ring
     * @param ring2 strings that belong to the second ring 
     */
    DOMHoneyComb(const StringNbr center, const Ring& ring1, const Ring& ring2);
    
    /** @brief constructor with arbitrary number of rings
     * @param center the center of the HoneyComb
     * @param rings vector of rings and their contained strings
     */
    DOMHoneyComb(const StringNbr center, std::vector< Ring >& rings);

    /** @brief Add the single string [string] to the honeycomb at central string [center] on ring [ringnbr]
     * @param center the central string
     * @param ringnbr the ring on which to add the string to
     * @param string that should be added
     */
    void AddStringToRing(const StringNbr center, const unsigned int ringnbr, const StringNbr string);
  }; //class DOMHoneyComb

  ///Look-up table for the HoneyCombs, indexed by StringNbr
  typedef std::map<StringNbr, DOMHoneyComb> DOMHoneyCombRegister;

  /** @brief Register this string [center_A] as on ring[ringnbr] on string [center_B] and vise vers
   * @param combs the combs these strings should be registered to
   * @param center_A the string to register
   * @param center_B the other string to register
   * @param ringnbr the ring that strings should be registered to; should not be further than number of already registered rings plus 1
   */
  void MutualAddStringToRing(DOMHoneyCombRegister &combs,
                             const StringNbr center_A,
                             const StringNbr center_B,
                             const unsigned int ringnbr);
  
  /** @brief is this [string] on ring[ringnbr] of [center]
   * @param combs the combs to look up this ring
   * @param center center string around to look
   * @param ringnbr ring number around center the string should be located on
   * @param string the string to be found
   * @return true, if string could be correctly located;
   * false, if not;
   */
  bool IsRingX (const DOMHoneyCombRegister &combs,
                const StringNbr center,
                const unsigned int ringnbr,
                const StringNbr string);
  
  ///shorthand
  inline bool IsRing1(DOMHoneyCombRegister &combs,
                      const StringNbr center,
                      const StringNbr string);
  
  ///shorthand
  inline bool IsRing2(DOMHoneyCombRegister &combs,
                      const StringNbr center,
                      const StringNbr string);

  /** @brief Which ring is this on?
   * @param combs the  Combs to look this strings up on
   * @param center the center from which the ring should be found
   * @param string the string to locate on any ring
   * @param search_depth the maximum ring that this string is searched for (does limit the amount of computation)
   * @return 0, if it is the center;
   * n, if it is on ring n;
   * -1, if string is not registered;
   */
  int WhichRing(const DOMHoneyCombRegister &combs,
                const StringNbr center,
                const StringNbr string,
                const unsigned int search_depth=1000); //DANGER hardcoding of a rediciulous huge number
  
  /** @brief Abstract a combs to next smaller RingSize definitions
   * The assignment is done by prescribed rings :
   * combs[ring0]->abstracted[ring0],
   * combs[ring1]->abstracted[ring2],
   * combs[ring2]->abstracted[ring4], ...
   * @param combs the filled Combs to abstract
   * @return the abstracted Hive
   */
  DOMHoneyCombRegister Abstract(const DOMHoneyCombRegister combs);
  
  /** @brief Expands combs and adds new rings to it
   * The combs must have at least 1 ring assigned and equal ring size on all strings,
   * it must also be a mutually complete (every string links against each other)
   * @param combs the filled combs to expand
   * @param scale_factor expand by using that many rings (this should be a scale to the next complete reverse abstraction)
   * @return n number of newly added rings
   *         -1 if not possible
   */
  //TODO this is not thoroughly tested; however it seems to work
  int ExpandRings(DOMHoneyCombRegister &combs,
                  const unsigned int scale_factor=1);
  
  /** @brief Unite two combs into one
   * @param combs1 the first combs to unite
   * @param combs2 the second combs to unite
   * @return the new united Hives
   */
  DOMHoneyCombRegister UniteHive (const DOMHoneyCombRegister& combs1,
                                  const DOMHoneyCombRegister& combs2);
  
  /** @brief Dump an entire entry of an combs
   * @param combs the Hive to dump the entry from
   * @param center the entry to dump
   * @return the dumped central string
   */
  std::string DumpCenter (const DOMHoneyCombRegister &combs,
                          const StringNbr center);

  ///complex type that holds all magic information associated DOMHoneyCombRegister
  ///use this type when working with different DOMHoneyCombRegisters, where the scaling of the rings are different
  struct Hive{
    /// relative scaling factor (in comparison to regular IceCube string spacings)
    unsigned int scaleFactor_;
    /// the maximum amount of registered rings
    unsigned int max_rings_;
    /// the Register holding all information
    DOMHoneyCombRegister combs_;
    
    ///constructor
    Hive(const unsigned int scaleFactor,
         const DOMHoneyCombRegister& combs,
         unsigned int max_rings);
  }; // struct Hive
  
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


///Namespace that provides information of Strings and DOMs to which detector topology they belong
///NOTE Definitions can be personal preference and are subject to change once the IC86 detector should be further expanded 
namespace builder {
  ///
  typedef unsigned int OmTopo;
  
  class StringTopo{
  private:
    ///store the oms: is a consecutive register, where 
    std::vector<OmTopo> omtopos_;
  public:
    inline unsigned int GetMaxOms() const
      {return (omtopos_.size()+1);};
    
    inline int GetOmTopo(unsigned int omNbr) const
      {return (omNbr>omtopos_.size() ? -1 : omtopos_.at(omNbr-1));};
      
    void AddOmTopo(OmTopo omtopo) {
      omtopos_.push_back(omtopo);
    };
  };
  
  /// in such an object the topology of a detector can be stored
  typedef std::map<unsigned int, StringTopo> TopologyRegister;
  
  /// describe the topologogy of OMKeys:
  /// where topo is: 0 for not on any topology
  ///                1,2,3... assorted to that topology by number N
  struct Topology {
    ///the maximum string index hold in this map
    unsigned int max_strings_;
    ///the register which holds all the information
    TopologyRegister topoReg_;
  };
    
  /// dump a topology to the a string
  std::string DumpTopology (const Topology &topo);
  
  /// read from a topology from a filedump
  Topology ReadTopologyFromFile (const std::string& fileName);
};

struct HiveGeometry {
  ///knows all the rings around each string
  std::vector<honey::Hive> hives_;
  ///knows the topology of each dom
  builder::Topology topo_;
};


#endif