/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id: FrameCombiner.h 122365 2014-08-11 13:21:53Z mzoll $
 *
 * @file FrameRegister.h
 * @version $Revision: 122365 $
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 *
 * The main purpose of this class and collection of functions is to provide a unified interface and random access
 * to all frames that come along in a single FramePackage.
 * Each FramePackage is therefore buffered up in an ordered register 'FrameRegister',
 * that indicates the frame-type (Q-frame, SplitFrame, HypoFrame) and its later purpose.
 */

#ifndef FRAMEREGISTER_H
#define FRAMEREGISTER_H

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

#include "CoincSuite/lib/CoincSuiteHelpers.h"


/// an object that buffers up all frames in a ordered structure, not different to FramePacket
typedef std::vector<I3FramePtr> FramePack; //NOTE the should be a global alias/typedef for this in I3Frame.h
/// an object that remembers the (original) position[0,...] of a given frame in the FramePack(et), and the pointer to it
typedef std::pair<uint, I3FramePtr> PosFrame;
/// a series of PosFrames of common traits
typedef std::vector<PosFrame> PosFrameSeries;

/// global namespace for all helpers
namespace CoincSuite {
  /** @brief a class which can buffer up and hold all frames in that package
   * consists of multiple vectors of PosFrames, which holds frames which fulfil certain criteria e.g. belong to a certain SubEventStream
   */
  class FrameRegister {
  public: // Properties // need to expose because sometimes its easier to access by index 
    /// holds all Frames there are
    std::vector<PosFrame> AllFrames_;
    /// holds only the (one?!) Q Frame
    std::vector<PosFrame> QFrames_;
    /// holds all frames of SubEventStream 'SplitName'
    std::vector<PosFrame> SplitFrames_;
    /// holds all frames of SubEventStream 'HypoName'
    std::vector<PosFrame> HypoFrames_;
    /// holds all frames of which are neither Q-frame, 'SplitName' nor 'HypoName'
    std::vector<PosFrame> OtherFrames_;
    /// stores the value of 'SplitCount' as read from the Q-frame: Number of splitframes as created by the splitter
    int SplitCount_;
    /// stores the value of 'ReducedCount' as read from the Q-frame: Number of splitframes which were removed by recombinations
    int ReducedCount_;
    /// remembers the name of SubEventStream for SplitFrames
    std::string SplitName_;
    /// remembers the name of SubEventStream for HypoFrames
    std::string HypoName_;
  public:
    /// constructor (setting everything void) 
    FrameRegister();
    
    /** @brief constructor (building the FrameRegister directly)
    * @param packet the FramePacket to fill the FrameRegister from
    * @param SplitName name of the split subevent stream
    * @param HypoName name of the HypoFrame subevent stream
    */
    FrameRegister(std::vector<I3FramePtr> &packet, const std::string& SplitName, const std::string& HypoName);
    
    /** @brief Build the frame register
    * @param packet the FramePacket to fill the FrameRegister from
    * @param SplitName name of the split subevent stream
    * @param HypoName name of the HypoFrame subevent stream
    */
    bool BuildFrameRegister(std::vector<I3FramePtr> &packet, const std::string& SplitName, const std::string& HypoName);
    
    /** @brief insert a singular frame
     * @param frame the frame to insert
     * @return true if success
     */
    bool InsertFrame(I3FramePtr frame);
    
    /// reset everything : purge all objects and all variables
    void Clear();
    
    ///retrieve the content of the frame register
    std::vector<I3FramePtr> RetrieveAllContent() const;

    /** retrieve the content from the specified special fields
     * @param qframes retrieve the (one!) Q-Frames
     * @param splitframes retrieve the AplitFrames
     * @param hypoframes retrieve the HypoFrmaes
     * @param otherframes retrieve the OtherFrames
     */
    std::vector<I3FramePtr> RetrieveFrameSequence(const bool qframes=true,
                                                  const bool splitframes=true,
                                                  const bool hypoframes=true,
                                                  const bool otherframes=true) const;

    /** Remove the frame at this position
     * find the position in the array of all frames, eraze it, and rebuild the FrameRegister from the remaining content
     * @param frameindex the position of the frame to remove
     */
    bool RemoveFrameAtPosition(const uint frameindex);

    /** @brief find the frame with the following subevent stream and ID
    * @param subEventStream the stream to search for; special for "Q" which will give the Q-frame
    * @param subEventID the ID to search for
    * @return the position and a pointer to that frame; default to (-1, *0) if not found
    */
    PosFrame FindCorrIndices(const std::string& subEventStream,
                             const uint subEventID) const;
    
    /// @brief Get me the entry at this position
    PosFrame GetFrameAtPosition(const uint position) const;

    /// return length of vector AllFrames
    inline uint GetNumberAllFrames() const {return AllFrames_.size();};
    /// return length of vector AllFrames
    inline uint GetNumberQFrames() const {return QFrames_.size();};
    /// return length of vector SplitFrames
    inline uint GetNumberSplitFrames() const {return SplitFrames_.size();};
    /// return length of vector HypoFrames
    inline uint GetNumberHypoFrames() const {return HypoFrames_.size();};
    /// return length of vector OtherFrames
    inline uint GetNumberOtherFrames() const {return OtherFrames_.size();};
    /// return the value of 'SplitCount' in Q-frame
    inline uint GetSplitCount() const {return SplitCount_;};
    /// return the value of 'ReducedCount' in Q-frame
    inline uint GetReducedCount() const {return ReducedCount_;};
    /// how many effective split remain
    inline uint GetEffSplits() const {return SplitCount_-ReducedCount_;};
  };
};//end namespace CoincSuite

#endif //FRAMEREGISTER_H
