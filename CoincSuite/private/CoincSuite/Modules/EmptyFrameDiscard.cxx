/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file EmptyFrameDiscard.cxx
 * @version $Revision$
 * @author mzoll marcel.zoll@fysik.su.se
 * @date $Date: 20 Mar 2013
 *
 */

#ifndef EMPTYFRAMEDISCARD
#define EMPTYFRAMEDISCARD

#include <boost/make_shared.hpp>

#include "CoincSuite/Modules/FrameCombiner.h"
#include "icetray/I3Units.h"
#include "icetray/I3Bool.h"



/** A I3Module which Identifies empty frames where the pulse-series only contains only few hit DOMs
 */
class EmptyFrameDiscard : public FrameCombiner {
private:
  SET_LOGGER("EmptyFrameDiscard");
  //parameters
  /// Param: Name of the RecoPulseSeriesMap
  std::string recoMapName_;
  /// Param: min hit doms
  int minDOMshit_;
  //bookkeeping
  ///Count the number of afterpulses tagged
  uint n_fewpulses;
public:
  ///constructor
  EmptyFrameDiscard (const I3Context& context);
  /// std Configure function
  void Configure();
  /// Finish to tell me stuff
  void Finish();
  ///where stuff should happen
  void FramePacket(std::vector<I3FramePtr> &packet);
};

I3_MODULE(EmptyFrameDiscard);

#endif

//***************************IMPLEMENTATIONS*************
EmptyFrameDiscard::EmptyFrameDiscard (const I3Context& context):
  FrameCombiner(context),
  recoMapName_("MaskedOfflinePulses"),
  minDOMshit_(1),
  n_fewpulses(0)
{
  AddParameter("RecoMapName","Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("MinHitDOMS","Discard frames which have less than this many hit DOMs", minDOMshit_);
  FrameRegister_.clear();
};
//___________________________________________________________________________
void EmptyFrameDiscard::Configure() {
  FrameCombiner::Configure();
  GetParameter("recoMapName", recoMapName_);
  GetParameter("MinHitDOMS", minDOMshit_);
};

void EmptyFrameDiscard::Finish() {
  FrameCombiner::Finish();
  log_info_stream("EmptyPulses tagged :" << n_fewpulses << std::endl);
}

void EmptyFrameDiscard::FramePacket(std::vector<I3FramePtr> &packet) {
  log_debug("Entering FramePacket()");

  BuildFrameRegister(packet);

  unsigned int id_emptypulses=0;
  //loop over all hypothesis frames and find their originators
  for (uint splitframe_index=0; splitframe_index < FrameRegister_.GetSizeSplitFrames(); splitframe_index++) {
    I3FramePtr splitframe = FrameRegister_.SplitFrames_[splitframe_index].second;

    I3RecoPulseSeriesMapConstPtr pulses = splitframe->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

    if (! pulses) {
      log_error("Could not find the recoMap '%s' in the splitframes;", recoMapName_.c_str());
      continue;
    }

    if ((int)pulses->size()< minDOMshit_) {
      n_fewpulses++;
      id_emptypulses++;
      splitframe->Put("EmptyPulseSeries",boost::make_shared<I3Bool>(true));
      //std::cout << " Afterpulses!" << std::endl;
      break;
    }
  }

  ReduceEffSplitCount(id_emptypulses);

  PushFrameRegister();
  log_debug("Leaving FramePacket()");
  return;
};