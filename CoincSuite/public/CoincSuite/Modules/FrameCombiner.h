/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file FrameCombiner.h
 * @version $Revision$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 *
 * The main purpose of this class and collection of functions is to provide a unified interface and random access
 * to all frames that come along in a single FramePackage.
 * Each FramePackage is therefore buffered up in an ordered register 'FrameRegister',
 * that indicates the frame-type (Q-frame, SplitFrame, HypoFrame) and its later purpose.
 */

#ifndef FRAMECOMBINER_H
#define FRAMECOMBINER_H

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include "icetray/I3PacketModule.h"
#include "icetray/I3Frame.h"

#include "dataclasses/I3Double.h"
#include "icetray/I3Int.h"
#include "icetray/OMKey.h"

#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

#include "CoincSuite/Modules/FrameRegister.h"

/** @brief A class that provides basic low-level functions for event recombinations
 * Derive from this class all modules of CoincSuite as the this class will guide interactions to the
 * underlying frames of a whole Frame Packet. This class will explicitly help to figure out which frames
 * are to be treated as SplitFrames, HypoFrames, Q-frames.
 */
class FrameCombiner : public I3PacketModule {
  SET_LOGGER("FrameCombiner");
protected: //parameters
  /// PARAM: Name of the subevent-stream to try to recombine
  std::string splitName_;
  /// PARAM: Name of the hypothesis-splits
  std::string hypoName_;
protected: //properties
  /// create my most private instance of FrameRegister
  CoincSuite::FrameRegister FrameRegister_;
public: //icetray interaction
  /// std constructor
  FrameCombiner(const I3Context& context);
  /// std Configure function
  void Configure();
  /// std Finish function
  void Finish();
  
  /** @brief ACTION
   * do nothing: build and push frame register, INPUT == OUTPUT
   * @param packet the FramePacket
   */
  void FramePacket(std::vector<I3FramePtr> &packet);
  
  /** @brief Build the FrameRegister with the packet
   * Fills the object FrameRegister_.
   * takes silent parameters splitname_ and hypoName_.
   * @param packet the FramePacket to build from
   */
  void BuildFrameRegister(std::vector<I3FramePtr> &packet);

protected:
  /** @brief Push all frames in the packet back into the Outbox
   * @param packet the FramePacket to push
   */
  void PushPacket(std::vector<I3FramePtr> &packet);
	
  /** @brief Push all frames in the FrameRegister_ back into the Outbox
   * push every frame that is in the FrameRegister.AllFrames_ vector into the Outbox in the same order they appear
   */
  void PushFrameRegister();
	
  /** @brief helper: find on which place this splitframe is in the FrameRegister_.allFrames_ vector
   * Implements a first guess and a granular searching; unique identification is done by SubEventStream and SubEventID
   * @param subEventStream Name of the subEventStream that should be searched for
   * @param subEventID SubEventID of the event that should be searched for
   * @return a pair of position of that frame in the FramePackage and the Pointer to the frame itself
   */
  std::pair<int , I3FramePtr> FindCorrIndices(const std::string& subEventStream, const uint subEventID) const;
	
  /** @brief helper function to reduce the Effective SplitCount
   * @param reduce the effective SplitCount by that much, by incrementing the ReducedCount; defaults to 1
   */
  void ReduceEffSplitCount(const uint reduce=1);

  //convenience functions
  /// return length of vector AllFrames
  inline uint GetNumberAllFrames() {return FrameRegister_.GetNumberAllFrames();};
  /// return length of vector SplitFrames
  inline uint GetNumberQFrames() {return FrameRegister_.GetNumberQFrames();};
  /// return length of vector SplitFrames
  inline uint GetNumberSplitFrames() {return FrameRegister_.GetNumberSplitFrames();};
  /// return length of vector HypoFrames
  inline uint GetNumberHypoFrames() {return FrameRegister_.GetNumberHypoFrames();};
  /// return length of vector OtherFrames
  inline uint GetNumberOtherFrames() {return FrameRegister_.GetNumberOtherFrames();};
  /// return the value of 'SplitCount' in Qframe
  inline uint GetSplitCount() {return FrameRegister_.GetSplitCount();};
  /// return the value of 'ReducedCount' in Qframe
  inline uint GetReducedCount() {return FrameRegister_.GetReducedCount();};
  /// how many effective split remain?
  inline uint GetEffSplits() {return FrameRegister_.GetEffSplits();};
};

#endif //FRAMECOMBINER_H
