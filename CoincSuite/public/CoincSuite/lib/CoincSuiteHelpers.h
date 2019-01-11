/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file CoincSuiteHelpers.h
 * @version $Revision$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 *
 * Provide some convenience functions
 */

#ifndef COINCSUITEHELPERS_H
#define COINCSUITEHELPERS_H

#include <vector>
#include <string>
#include <utility>

#include "icetray/I3Frame.h"

#include "icetray/I3Int.h"
#include "icetray/OMKey.h"

#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

#include "dataclasses/physics/I3TriggerHierarchy.h"
#include <sstream>

/// make uint accessible
typedef unsigned int uint;

///Helper functions and convenience
namespace CoincSuite{
  //general purpose helpers
  
  /** Get an unique identification string for frames with not so much information
   * @param frame 
   * @return a nicely formated string
   */
  std::string FrameIDstring (const I3FrameConstPtr frame);
  
  /**helper to return the SubEventID of a frame
   * @note if this is called on a frame not containing a I3EventHeader if will crash and burn, just don't do that
   * @param frame a Regular frame which has to contain a I3EventHeader
   */
  inline uint GetSubEventID (const I3FramePtr frame)
    {return frame->Get<I3EventHeaderConstPtr>("I3EventHeader")->GetSubEventID();};

  /** @brief Give the time-separation of two time windows; preferred either I3Time or double-expressions of whatever
  * @note the '<' operator has to be defined for this; aka the times have to obey an ordering principle
  * @param startA start of eventA
  * @param endA stop of eventA
  * @param startB start of eventB
  * @param endB stop of eventB
  * @return NAN: full inclusion,
  *         negative value: partial inclusion by so many ns,
  *         positive value: separated by that many ns
  */
  template <class time_var>
  double TimeSeparation(const time_var startA,
                        const time_var endA,
                        const time_var startB,
                        const time_var endB) {
    if (startA < startB) {//A before B
      if (startB < endA) {//B starts within A
        if (endA < endB) //B ends after A ends
          return startB-endA; //-o-o-[A-A-[AB-AB]-B-B]-o-o-
        else //A fully includes B
          return NAN; //-o-o-[A-A-[AB-AB-AB]-A-A]-o-o-
      }else //A and B are separated
        return startB-endA; //-o-o-[A-A-A]-o-[B-B-B]-o-o-
    }else{ //B before A
      if (startA < endB) {//A starts within B
        if (endB < endA) //A ends after B ends
          return startA-endB; //-o-o-[B-B-[AB-AB]-A-A]-o-o-
        else //B fully includes A
          return NAN; //-o-o-[B-B-[AB-AB-AB]-B-B]-o-o-
      }else //B and A are separated
        return startA-endB; //-o-o-[B-B-B]-o-[A-A-A]-o-o-
    }
  };
  
  /** Get a list of all deriations of this mask; children up front, grandparents in the back
   * @param frame The frame which is searched for the the mask and its ancestors
   * @param key The name of a I3RecoPulseSeriesMap or I3RecoPulseSeriesMapMask
   * @return the ancestry of this map/mask, parents in the back
   */
  std::vector<std::string> GetMaskAncestry (I3FrameConstPtr frame,
                                        const std::string &key);

  /**Get me the oldest common ancestors for masks
   * @param frame The frame which is searched for the the mask and its ancestors
   * @param key1 the one I3RecoPulseSeriesMapMask
   * @param key2 the other I3RecoPulseSeriesMapMask
   * @return key of the first common ancestor
   */
  std::vector<std::string> GetCommonMaskAncestry (I3FrameConstPtr frame,
                                 const std::string &key1,
                                 const std::string &key2);
  

  /// an convenience time order operator
  inline bool RecoPulse_timeorder (const I3RecoPulse& lhs, const I3RecoPulse& rhs)
    { return lhs.GetTime()<rhs.GetTime(); };

  /** @brief A convenience function to unite two RecoMaps into a single one
   * @param mapA the one map
   * @param mapB the other map
   * @return the united map
   */
  I3RecoPulseSeriesMap UniteRecoMaps (const I3RecoPulseSeriesMap& mapA,
                                      const I3RecoPulseSeriesMap& mapB);
  
  
  /** @brief A convenience function to unite multiple RecoMaps found in frames to one Mask pointing to a q-frame
   * @note it is assumed that all Masks/Maps ahve similar  derivition structure
   * @param key the key to the map or mask object in the frames
   * @param qframe the Q frame the frames derived from
   * @param frames a number of frames where maps/masks should be reunited
   * @return the united mask
   */
  I3RecoPulseSeriesMapMask UniteRecoMaps_To_Mask(const std::string &key,
                                                 const I3FramePtr qframe,
                                                 const std::vector<I3FramePtr> &frames);
  
  /** Unite two TriggererHierarchies into a single one
   * @param trigA the one TriggerHierarchy
   * @param trigB the other TriggerHierarchy
   * @return the united Hierarchy
   */
  I3TriggerHierarchy UniteTriggerHierarchies (const I3TriggerHierarchy &trigA,
                                              const I3TriggerHierarchy &trigB);
}
#endif //COINCSUITEHELPERS
