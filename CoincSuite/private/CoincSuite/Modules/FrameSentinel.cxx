/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file FrameSentinel.cxx
 * @version $Revision$
 * @author mzoll marcel.zoll@fysik.su.se
 * @date $Date: Sept 24 2013
 */

#include "CoincSuite/Modules/FrameCombiner.h"

#include <vector>
#include <string>
#include <limits>

/// A Module that kills frames that contain a certain frame-key signaling that this frame should be discared
// @NOTE deprecated code; however might still be needed for unit tests; TODO find out and delete if so
class FrameSentinel : public FrameCombiner {
private:
  SET_LOGGER("FrameSentinel");
  /// PARAM: the flag which should be discarded
  std::string discardFlag_;
public:
  ///constructor
  FrameSentinel(const I3Context& context):
    FrameCombiner(context),
    discardFlag_()
  {
    AddParameter("DiscardFlag","The flag that signals this frame should be discared", discardFlag_);
  };

  void Configure() {
    FrameCombiner::Configure();
    GetParameter("DiscardFlag", discardFlag_);
    
    if (discardFlag_.empty())
      log_fatal("Configure the parameter 'DiscardFlag'");
  };

  /// Clean-out and push
  void FramePacket(std::vector<I3FramePtr> &packet) {
    BuildFrameRegister(packet);
    std::vector<uint> discard_these_frames = FlaggedFrames();
    ReduceEffSplitCount(discard_these_frames.size());
    FramePack pack = CleanOut(discard_these_frames);
    PushPacket(pack);
  };

  /// ACTION: clean away any frame that has the discardKey register and push remainder of the packet
  std::vector<uint> FlaggedFrames () {
    std::vector<uint> discard_these_frames;
    for (std::vector<PosFrame>::const_iterator frames_iter = FrameRegister_.SplitFrames_.begin(); frames_iter != FrameRegister_.SplitFrames_.end(); ++frames_iter) {
      if (frames_iter->second->Has(discardFlag_)) {
        log_debug("Found a frame with '%s'; will discard it; position '%d'", discardFlag_.c_str(), (uint)frames_iter->first);
        discard_these_frames.push_back(frames_iter->first);
      }
    }
    return discard_these_frames;
  };
};

I3_MODULE(FrameSentinel);
