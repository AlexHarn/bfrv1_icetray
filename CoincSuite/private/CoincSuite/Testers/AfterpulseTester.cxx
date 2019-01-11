/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file AfterpulseTester.cxx
 * @version $Revision$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 * 
 * This is mainly derived from the module AfterpulsesDiscard
 */

#ifndef AFTERPULSETESTER_H
#define AFTERPULSETESTER_H

#include "CoincSuite/Testers/TesterModule.h"

/// A structure to store total charge and mean hit-time per DOM or event
struct Summary{
  double qtot;
  double tmean;
  ///constructor
  Summary():
    qtot(0.),
    tmean(0.)
  {};
};

typedef std::map<OMKey,Summary> HitSummaryMap;

/// A structure to save an event/frame and the information utilized to identify Afterpulses
struct SubEvent{
  Summary eventSummary;
  HitSummaryMap hitSummaries;
};

typedef std::vector<SubEvent> SubEventSeries;

///helper function to sort subevents in ascending time order
inline bool earlierTime(const SubEvent& s1, const SubEvent& s2)
  {return(s1.eventSummary.tmean < s2.eventSummary.tmean); };


/** A I3Module which Identifies subevents which are comprised of Afterpulses from a previous (sub)events ('Host-event').
 * Decision is taken by the following arguments:
 * -Afterpulses should have much less total charge than the original pulses.
 * -Afterpulses must occur after the original pulses.
 * -Afterpulses should occur on the same DOMs as the original pulses (although there are likely to be some noise pulses present as well).
 */
class AfterpulseTester : public TesterModule {
private:
  SET_LOGGER("AfterpulseTester");
  
private: //parameters
  /// PARAM: Name of the RecoPulseSeriesMap(Mask)
  std::string recoMapName_;
  /// PARAM: Maximum fraction of qTot compared to the host-event
  double qtotFraction_;
  /// PARAM: Required time offset
  double timeOffset_;
  /// PARAM: Required overlap fraction in hit DOMs to the proposed host-event
  double overlapFraction_;
  
private: //algorithm
  /// Implement the evaluation for mutual-testing
  Result Test(I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) const;
  ///call-back function
  static Result runTest (void* tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    AfterpulseTester *alPtr = (AfterpulseTester*) tester;
    return alPtr->Test(hypoframe, frameA, frameB);
  };

public:
  /// constructor
  AfterpulseTester (const I3Context& context);
  /// Configure function
  void Configure();

private:
  /** @brief create an event summary from such pulses
   * @param pulses convert these pulses
   * @return the created subevent-summary
   */
  SubEvent CreateSummary (I3RecoPulseSeriesMapConstPtr pulses) const;
};

I3_MODULE(AfterpulseTester);

#endif //AFTERPULSETESTER_H


//======================== IMPLEMENTATIONS ===========================
#include "dataclasses/I3Constants.h"
#include "icetray/I3Units.h"

#include <boost/make_shared.hpp>

//______________________________________________________________________________
AfterpulseTester::AfterpulseTester (const I3Context& context):
  TesterModule(context),
  recoMapName_("MaskedOfflinePulses"),
  qtotFraction_(.1),
  timeOffset_(3.E3*I3Units::ns),
  overlapFraction_(.75)
{
  AddParameter("RecoMapName", "Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("QTotFraction", "Maximum fraction of qTot compared to the host-event", qtotFraction_);
  AddParameter("TimeOffset", "Required time difference to the host-event", timeOffset_);
  AddParameter("OverlapFraction", "Required fraction of overlap to the host-event", overlapFraction_);
};

//______________________________________________________________________________
void AfterpulseTester::Configure() {
  TesterModule::Configure();
  GetParameter("recoMapName", recoMapName_);
  GetParameter("QTotFraction", qtotFraction_);
  GetParameter("TimeOffset", timeOffset_);
  GetParameter("OverlapFraction", overlapFraction_);
  
  Evaluate = runTest;
};

//______________________________________________________________________________
SubEvent AfterpulseTester::CreateSummary (I3RecoPulseSeriesMapConstPtr pulses) const {
  SubEvent subevent;
  BOOST_FOREACH(const I3RecoPulseSeriesMap::value_type &omkey_pulses, *pulses) {
    const OMKey &omkey= omkey_pulses.first;
    BOOST_FOREACH(const I3RecoPulse &pulse, omkey_pulses.second) {
      const double charge = pulse.GetCharge();
      const double time = pulse.GetTime();
      subevent.eventSummary.qtot+=charge;
      subevent.eventSummary.tmean+=time*charge;
      subevent.hitSummaries[omkey].qtot+=charge;
      subevent.hitSummaries[omkey].tmean+=time*charge;
    }
    subevent.hitSummaries[omkey].tmean/=subevent.hitSummaries[omkey].qtot;
  }
  subevent.eventSummary.tmean/=subevent.eventSummary.qtot;
  return subevent;
}

//______________________________________________________________________________
AfterpulseTester::Result AfterpulseTester::Test(I3FrameConstPtr hypoframe,
                                                I3FrameConstPtr frameA,
                                                I3FrameConstPtr frameB) const
{
  //assume strict timeorder by the splitter: events with lower frame indexes are happening before higher ones
  I3RecoPulseSeriesMapConstPtr pulsesA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  I3RecoPulseSeriesMapConstPtr pulsesB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  if (! pulsesA || ! pulsesB) {
    log_error("Could not find the recoMap <I3RecoPulseSeriesMap>('%s') in SplitFrames; "
              "will continue with the next HypoFrame", recoMapName_.c_str());
    return UNDECIDED;
  }
  
  //fill the subevent register
  SubEvent subEvent_A=CreateSummary(pulsesA);
  SubEvent subEvent_B=CreateSummary(pulsesB);
  
  SubEvent* earlySubEvent;
  SubEvent* lateSubEvent;
  
  //figure out which is the earlier event
  if (subEvent_A.eventSummary.tmean< subEvent_B.eventSummary.tmean) {
    earlySubEvent = &subEvent_A;
    lateSubEvent = &subEvent_B;
  }
  else {
    earlySubEvent = &subEvent_B;
    lateSubEvent = &subEvent_A;
  }
  
  
    //Afterpulses should have much less total charge than the original pulses      
  const bool charge_req = (lateSubEvent->eventSummary.qtot/earlySubEvent->eventSummary.qtot <= qtotFraction_);
  log_trace_stream("ChargeReq(<"<<qtotFraction_<<") : "<<charge_req<<"; Current qtot " << lateSubEvent->eventSummary.qtot << "; earlier qtot " << earlySubEvent->eventSummary.qtot);
  if (not charge_req) {
    log_debug_stream("Too much charge on this subevent ("<<qtotFraction_<<"): early:"<<earlySubEvent->eventSummary.qtot<<" late:"<<lateSubEvent->eventSummary.qtot);
    return NO;
  }
  
  //Afterpulses must occur after the original pulses
  const bool time_req = ((lateSubEvent->eventSummary.tmean-earlySubEvent->eventSummary.tmean) >= timeOffset_);
  log_trace_stream("TimingReq(>"<<timeOffset_<<") : "<<time_req<<"; Current time " << lateSubEvent->eventSummary.tmean << "; earlier time " << earlySubEvent->eventSummary.tmean);
  if (not time_req) {
    log_debug_stream("Too less time-offset ("<<timeOffset_<<"): early:"<<earlySubEvent->eventSummary.tmean<<" late:"<<lateSubEvent->eventSummary.tmean);
    return NO;
  }
  
  //Afterpulses should occur on the same DOMs as the original pulses (although there are likely to be some noise pulses present as well)
  unsigned int overlap=0;
  BOOST_FOREACH(const HitSummaryMap::value_type &omkey_summary, lateSubEvent->hitSummaries)
    if (earlySubEvent->hitSummaries.find(omkey_summary.first)!=earlySubEvent->hitSummaries.end())
      overlap++;
    
  const bool frac_req = (((double)overlap/lateSubEvent->hitSummaries.size()) > overlapFraction_);
  log_trace_stream("FracReq(>"<<overlapFraction_<<") : "<<frac_req<<"; NChan " << lateSubEvent->hitSummaries.size() << "; overlap " << overlap<<"; size: "<<lateSubEvent->hitSummaries.size());
  if (not frac_req) {
    log_debug_stream("Not enough Overlap ("<<overlapFraction_<<"): nch_early:"<<lateSubEvent->hitSummaries.size());
    return NO;
  }
    
  //if (charge_req &&  time_req && frac_req) {
  // J'Accuse!
  return YES;
};
