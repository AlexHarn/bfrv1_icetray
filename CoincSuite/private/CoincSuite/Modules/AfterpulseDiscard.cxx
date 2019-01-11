/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file AfterpulseDiscard.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se> (Chrisopher Weaver <c.weaver@icecube.wisc.edu>)
 * @date $Date: 20 Mar 2013
 *
 * The code core is copied from Chris Weaver's AfterpulseSpotter and adapted for Coincident Suite
 * http://code.icecube.wisc.edu/svn/sandbox/cweaver/RandomStuff/private/RandomStuff/AfterpulseSpotter.cxx
 */

#ifndef AFTERPULSEDISCARD_H
#define AFTERPULSEDISCARD_H

#include <boost/make_shared.hpp>

#include "CoincSuite/Modules/FrameCombiner.h"
#include "icetray/I3Units.h"

/// A struct to store total charge and mean hit-time per DOM and/or event
struct Summary{
  double qtot;
  double tmean;
  //constructor
  Summary():
    qtot(0.),
    tmean(0.)
  {};
};

typedef std::map<OMKey,Summary> HitSummaryMap;

/// A struct to save an event/frame and the information utilized to identify Afterpulses
struct SubEventSummary{
  I3FramePtr frame;
  Summary eventSummary;
  HitSummaryMap hitSummaries;
  //constructor
  SubEventSummary(const I3FramePtr frame):
    frame(frame) {};
};

typedef std::vector<SubEventSummary> SubEventSummarySeries;

///helper function to sort subevents in ascending time order
inline bool earlierTime(const SubEventSummary& s1, const SubEventSummary& s2)
  { return(s1.eventSummary.tmean < s2.eventSummary.tmean); };

/** A I3Module which Identifies subevents which are comprised of Afterpulses from a previous (sub)events ('Host-event').
 * Decision is taken by the following arguments:
 * -Afterpulses should have much less total charge than the original pulses.
 * -Afterpulses must occur after the original pulses.
 * -Afterpulses should occur on the same DOMs as the original pulses (although there are likely to be some noise pulses present as well).
 */
class AfterpulseDiscard : public FrameCombiner {
private:
  SET_LOGGER("AfterpulseDiscard");
protected: //parameters
  /// PARAM: Name of the RecoPulseSeriesMap
  std::string recoMapName_;
  /// PARAM: Maximum fraction of qTot compared to the host-event
  double qtotFraction_;
  /// PARAM: Required TimeOffset
  double timeOffset_;
  /// PARAM: Maximum time difference to the host-event
  double maxTimeOffset_;
  /// PARAM: Required OverlapFraction in hit DOMs to the proposed host-event
  double overlapFraction_;
	/// PARAM: Immediately discard identified AfterpulseEvents as such
	bool discard_;
private: //bookkeeping
  ///Count the number of Afterpulses tagged
  uint n_afterpulses_;
public:
  ///constructor
  AfterpulseDiscard (const I3Context& context);
  /// std Configure function
  void Configure();
  /// Finish to tell me stuff
  void Finish();
  ///where stuff should happen
  void FramePacket(std::vector<I3FramePtr> &packet);
};

I3_MODULE(AfterpulseDiscard);

#endif //AFTERPULSEDISCARD_H


//========================IMPLEMENTATIONS===============
AfterpulseDiscard::AfterpulseDiscard (const I3Context& context):
  FrameCombiner(context),
  recoMapName_("MaskedOfflinePulses"),
  qtotFraction_(.1),
  timeOffset_(3.E3*I3Units::ns),
  maxTimeOffset_(15.E3*I3Units::ns),
  overlapFraction_(.75),
	discard_(false),
  n_afterpulses_(0)
{
  AddParameter("RecoMapName", "Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("QTotFraction", "Maximum fraction of qTot compared to the host-event", qtotFraction_);
  AddParameter("TimeOffset", "Required time difference to the host-event", timeOffset_); //TODO FIXME rename to MinTimeOffset
  AddParameter("MaxTimeOffset", "Maximum time difference to the host-event", maxTimeOffset_);
  AddParameter("OverlapFraction", "Required fraction of overlap to the host-event", overlapFraction_);
  AddParameter("Discard", "Immediately discard identified AfterpulseEvents as such", discard_);
};
//___________________________________________________________________________
void AfterpulseDiscard::Configure() {
  FrameCombiner::Configure();
  GetParameter("recoMapName", recoMapName_);
  GetParameter("QTotFraction", qtotFraction_);
  GetParameter("TimeOffset", timeOffset_);
  GetParameter("MaxTimeOffset", maxTimeOffset_);
  GetParameter("OverlapFraction", overlapFraction_);
  GetParameter("Discard", discard_);
};

void AfterpulseDiscard::Finish() {
  FrameCombiner::Finish();
  log_notice_stream("Afterpulse events tagged :" << n_afterpulses_ << std::endl);
}

void AfterpulseDiscard::FramePacket(std::vector<I3FramePtr> &packet) {
  log_debug("Entering FramePacket()");

  BuildFrameRegister(packet);
  
  //test if recombination attempt makes sense
  if (FrameRegister_.GetSplitCount() < 2) {
    log_debug("There is only one or no split; Can not run; will push everything");
    PushFrameRegister();
    log_debug("Leaving FramePacket()");
    return;
  }
  
  //BEGIN MAIN ALGORITHM PREPARATION
  SubEventSummarySeries subevents;
 
  for (uint splitframe_index=0; splitframe_index < FrameRegister_.GetNumberSplitFrames(); splitframe_index++) {
    I3FramePtr splitframe = FrameRegister_.SplitFrames_[splitframe_index].second;

    I3RecoPulseSeriesMapConstPtr pulses = splitframe->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

    if (! pulses) {
      log_error("Could not find the RecoMap <I3RecoPulseSeriesMap>('%s') in the SplitFrames;", recoMapName_.c_str());
      continue;
    }

    if ((int)pulses->size()==0) {
      log_debug("PulseSeriesMap is empty; skipping this subevent in the consideration");
      continue;
    }
    
    //create and fill fill the subevent register
    subevents.push_back(SubEventSummary(splitframe));
    BOOST_FOREACH(const I3RecoPulseSeriesMap::value_type &omkey_pulses, *pulses) {
      const OMKey &omkey= omkey_pulses.first;
      
      if (omkey_pulses.second.size() == 0) {
        log_warn_stream ("found an empty PulseSeries at "<<omkey);
        continue;
      }
      
      BOOST_FOREACH(const I3RecoPulse &pulse, omkey_pulses.second) {
        const double charge = pulse.GetCharge();
        const double time = pulse.GetTime();
        subevents.back().eventSummary.qtot+=charge;
        subevents.back().eventSummary.tmean+=time*charge;
        subevents.back().hitSummaries[omkey].qtot+=charge;
        subevents.back().hitSummaries[omkey].tmean+=time*charge;
      }
      subevents.back().hitSummaries[omkey].tmean/=subevents.back().hitSummaries[omkey].qtot;
    }
    subevents.back().eventSummary.tmean/=subevents.back().eventSummary.qtot;
  }
  //sort subevents by means of time
  std::sort(subevents.begin(),subevents.end(),&earlierTime);
  //END MAIN ALGORITHM PREPARATION
  
  //BEGIN MAIN ALGORITHM CORE
  uint id_afterpulses=0;

  //Loop backwards through subevents, from latest to earliest
  for(SubEventSummarySeries::reverse_iterator frame=subevents.rbegin(), frameEnd=subevents.rend(); frame!=frameEnd; frame++){
  //Compare each afterpulse candidate (subevent) to all earlier subevents
    for(SubEventSummarySeries::reverse_iterator earlierFrame=frame+1; earlierFrame!=frameEnd; earlierFrame++){
      //Afterpulses must occur after the original pulses
      const double t_diff = (frame->eventSummary.tmean-earlierFrame->eventSummary.tmean);
      const bool time_req = (timeOffset_ <= t_diff && t_diff <= maxTimeOffset_);
      log_trace_stream("TimingReq(>"<<timeOffset_<<") : "<<time_req<<"; Current time " << frame->eventSummary.tmean << "; earlier time " << earlierFrame->eventSummary.tmean);
      if (not time_req)
        continue; //not an afterpulse-event; jump out
      
      //Afterpulses should have much less total charge than the original pulses      
      const bool charge_req = (frame->eventSummary.qtot/earlierFrame->eventSummary.qtot <= qtotFraction_);
      log_trace_stream("ChargeReq(<"<<qtotFraction_<<") : "<<charge_req<<"; Current qtot " << frame->eventSummary.qtot << "; earlier qtot " << earlierFrame->eventSummary.qtot);
      if (not charge_req)
        continue; //not an afterpulse-event; jump out
      
      //Afterpulses should occur on the same DOMs as the original pulses (although there are likely to be some noise pulses present as well)
      unsigned int overlap=0;
      BOOST_FOREACH(const HitSummaryMap::value_type &omkey_summary, frame->hitSummaries)
        if (earlierFrame->hitSummaries.find(omkey_summary.first)!=earlierFrame->hitSummaries.end())
          overlap++;
        
      const bool frac_req = (((double)overlap/frame->hitSummaries.size()) > overlapFraction_);
      log_trace_stream("FracReq(>"<<overlapFraction_<<") : "<<frac_req<<"; NChan " << frame->hitSummaries.size() << "; overlap " << overlap<<"; size: "<<frame->hitSummaries.size());
      if (not frac_req)
        continue; //not an afterpulse-event; jump out
      
      if (charge_req &&  time_req && frac_req) {
        // J'Accuse! it is an afterpulse event
        n_afterpulses_++;
        id_afterpulses++;
        frame->frame->Put(GetName(), boost::make_shared<I3Int>(CoincSuite::GetSubEventID(earlierFrame->frame)));
        log_debug("Identified Afterpulse event!");
        break;
      }
    }
  }
  //END MAIN ALGORITHM CORE
  
  ReduceEffSplitCount(id_afterpulses); //NOTE should this be only invoked if frames are really discarded?
	
	//have to postpone the discard until here
	if (discard_) {
    //search for any occurrence where a Afterpulse event has been identified
    std::vector<I3FramePtr> outframes = FrameRegister_.RetrieveAllContent();
    BOOST_FOREACH(const I3FramePtr &outframe, outframes) {
      if (! outframe->Has(GetName())) 
        PushFrame(outframe);
    }
  }
  else
    PushFrameRegister(); //just push everything
  
  log_debug("Leaving FramePacket()");
};
