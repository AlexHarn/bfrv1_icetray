/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id:$
 *
 * @file NoiseClusterDiscard.cxx
 * @version $Revision:$
 * @author mzoll marcel.zoll@fysik.su.se
 * @date $Date: Nov 30 2011
 */

#ifndef NOISECLUSTERDISCARD_H
#define NOISECLUSTERDISCARD_H

#include "CoincSuite/Modules/FrameCombiner.h"

#include <boost/make_shared.hpp>

#include <vector>
#include <string>
#include <limits>

#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3String.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "icetray/I3Bool.h"
#include "icetray/I3PacketModule.h"

#include "icetray/I3Units.h"

/// A Module that does ID noise events in a very crude way
class NoiseClusterDiscard : public FrameCombiner {
private:
  SET_LOGGER("NoiseClusterDiscard");
  /// Param: Name of the I3RecoPulseSeriesMap i should check
  std::string recoMapName_;
  /// Param: Maximal number of DOMs that can participate in an NoiseCluster events
  uint nChanLimit_;
  /// Param: Minimal duration of a NoiseClusterEvent
  uint tExtLimit_;
  /// Param: Discard the frame right away (True) or just mark it with a key 'NoiseCluster' (False)
  bool discardOpt_;
public:
  ///constructor
  NoiseClusterDiscard(const I3Context& context);
  ///ConfigureMethode: to bind new Parameters
  void Configure();
  ///Action: Find arguments why this is noise and mark to discard, increase the 'ReducedCount'
  void FramePacket(std::vector<I3FramePtr> &packet);
};

I3_MODULE(NoiseClusterDiscard);

#endif // NOISECLUSTERDISCARD_H


//================================= IMPLEMENTATIONS ===============================
using namespace std;
using namespace CoincSuite;

//___________________________________________________________________________________
NoiseClusterDiscard::NoiseClusterDiscard(const I3Context& context):
  FrameCombiner(context),
  recoMapName_("MaskedOfflinePulses"),
  nChanLimit_(4),
  tExtLimit_(1700.*I3Units::ns),
  discardOpt_(false)
{
  AddParameter("RecoMapName","Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("NChanLimit", "Maximal number of DOMs that can participate in an NoiseCluster event", nChanLimit_);
  AddParameter("TExtLimit", "Minimal duration of a NoiseClusterEvent", tExtLimit_);
  AddParameter("Discard", "Discard the frame right away (True) or just mark it with a key 'NoiseCluster' (False)", discardOpt_);
};

void NoiseClusterDiscard::Configure() {
  FrameCombiner::Configure();
  GetParameter("RecoMapName", recoMapName_);
  GetParameter("NChanLimit", nChanLimit_);
  GetParameter("TExtLimit", tExtLimit_);
  GetParameter("Discard", discardOpt_);
};

void NoiseClusterDiscard::FramePacket(std::vector<I3FramePtr> &packet) {
  log_debug("Entering FramePacket()");
  BuildFrameRegister(packet);

  std::vector<uint> discard_these_frames;

  for (uint split_iter = 0; split_iter < FrameRegister_.SplitFrames_.size() ; split_iter++) {
    I3FramePtr splitframe = FrameRegister_.SplitFrames_[split_iter].second;
    
    I3RecoPulseSeriesMapConstPtr pulses = splitframe->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
    
    if (not pulses) {
      log_error("Could not find the RecoMap <I3RecoPulseSeriesMap>('%s') in the SplitFrame;", recoMapName_.c_str());
      continue;
    }
    
    log_debug("All neccessary objects found");

    //do hitsorting and extract timeExtension
    uint nchan = 0;
    double min_hittime = std::numeric_limits<double>::max();
    double max_hittime = std::numeric_limits<double>::min();
    for (I3RecoPulseSeriesMap::const_iterator om_pulses_iter=pulses->begin(); om_pulses_iter!=pulses->end(); om_pulses_iter++){
      if (om_pulses_iter->second.size()==0) {
        log_warn("empty PulseSeries");
        continue;
      }
      nchan++;
      const double t_firsthit = om_pulses_iter->second[0].GetTime();
      min_hittime=std::min(min_hittime, t_firsthit);
      max_hittime=std::max(max_hittime, t_firsthit);
    }
    const double text = max_hittime - min_hittime;

    if ( nchan <= nChanLimit_ && text >= tExtLimit_) {
      log_debug("NOISECLUSTER");
      discard_these_frames.push_back(FrameRegister_.SplitFrames_[split_iter].first);
      splitframe->Put(GetName(), boost::make_shared<I3Bool>(true));
    }
  }
  
  //all frames are marked, time to remove them from the equation
  ReduceEffSplitCount(discard_these_frames.size());
  
  if (discardOpt_) {
    std::vector<I3FramePtr> outframes = FrameRegister_.RetrieveAllContent();
    BOOST_FOREACH(const I3FramePtr &outframe, outframes) {
      if (! outframe->Has(GetName())) 
        PushFrame(outframe);
    }
  }
  else
    PushFrameRegister();
  
  log_debug("Leaving FramePacket()");
};
