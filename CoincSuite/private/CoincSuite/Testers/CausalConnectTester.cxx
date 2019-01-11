/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file CausalConnectTester.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#ifndef CAUSALCONNECTTEST_H
#define CAUSALCONNECTTEST_H

#include "gulliver/I3LogLikelihoodFitParams.h"

#include "CoincSuite/Testers/TesterModule.h"

#include "CoincSuite/lib/HitSorting.h"
#include "CoincSuite/lib/PartialCOG.h"
#include "CoincSuite/lib/I3FakeCalculator.h"

/** @brief A I3Module testing if two series of Pulses are connected topologically and in timing
 * of two fractions of Pulses, test if they:
 * are happening after each other
 * from the COG of the respective last three and respective first three Pulses evaluate
 * horizontal distance,
 * vertical distance,   
 * maximum time difference
 * and the CT-time-residual
 */
class CausalConnectTester : public TesterModule {
  SET_LOGGER("CausalConnectTester");
private: //parameters
  /// PARAM: Name of the I3RecoPulseSeriesMap that should be checked
  std::string recoMapName_;
  /// PARAM: negative time deviation from the speed of light [ns] before subseries of pulses cannot be seen as connected any more
  double timeConeMinus_;
  /// PARAM: positive time deviation from the speed of light [ns] before subseries of pulses cannot be seen as connected any more
  double timeConePlus_;
  /// PARAM: maximal time before subseries of pulses can not be seen as connected any more
  double wallTime_;
  /// PARAM: maximal vertical distance before subseries of pulses can not be seen as connected any more
  double maxVertDist_; 
  /// PARAM: maximal horizontal distance before subseries of pulses can not be seen as connected any more
  double maxHorDist_;

private: // properties
  /// holds the geometry
  std::vector<I3Position> domPos_cache_;

private: 
  /// Implement the Evaluation for Testing
  Result Test(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  ///call-back function
  static Result runTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    CausalConnectTester *alPtr = (CausalConnectTester*) tester;
    return alPtr->Test(hypoframe, frameA, frameB);
  };
  
public:
  ///constructor
  CausalConnectTester (const I3Context& context);
  /// std Configure function
  void Configure();
  /// Geometry: cash it
  void Geometry (I3FramePtr frame);

private:
  /** @brief Tests if we can causally connect the last 3 hits except one of PulsesA
   *         to the first 3 hits except one of PulsesB (if X is before Y)
   *  @param pulsesA Map of RecoPulseSeriesMap A
   *  @param pulsesB Map of RecoPulseSeriesMap B
   *  @return true if so
   */
  bool CausalConnected (const I3RecoPulseSeriesMap &pulsesA,
                        const I3RecoPulseSeriesMap &pulsesB) const;
};

I3_MODULE(CausalConnectTester);

#endif // CAUSALCONNECTTEST_H


//=========================== IMPLEMENTATIONS ==================================

#include "gulliver/I3LogLikelihoodFitParams.h"

#include "CoincSuite/lib/HitSorting.h"
#include "CoincSuite/lib/PartialCOG.h"
#include "CoincSuite/lib/I3FakeCalculator.h"

//______________________________________________________________________________
CausalConnectTester::CausalConnectTester (const I3Context& context):
  TesterModule(context),
  recoMapName_("MaskedOfflinePulses"),
  timeConeMinus_(1000.*I3Units::ns),
  timeConePlus_(1000.*I3Units::ns),
  wallTime_(2000.*I3Units::ns),
  maxVertDist_(400.*I3Units::m),
  maxHorDist_(400.*I3Units::m)
{
    log_warn_stream("================================================================"<<std::endl
                <<" module 'CausalConnectTest' is DEPRICATED:"<<std::endl
                <<" use identical behaving module 'cogCausalConnectTest' instead,"<<std::endl
                <<"    which has stringer discrimination power"<<std::endl
                <<"================================================================");
  
  AddParameter("RecoMapName","Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("TimeConeMinus", "negative allowed time residual in [ns] to still reconnect", timeConeMinus_);
  AddParameter("TimeConePlus", "positive allowed time residual in [ns] to still reconnect", timeConePlus_);
  AddParameter("WallTime", "max Time in [ns] between Pulses fractions to still reconnect", wallTime_);
  AddParameter("MaxVerticalDist", "max vertical Distance in [m ]to still reconnect", maxVertDist_);
  AddParameter("MaxHorizontalDist", "max horizontal Distance in [m ]to still reconnect", maxHorDist_);
};

//______________________________________________________________________________
void CausalConnectTester::Configure() {
  TesterModule::Configure();
  GetParameter("RecoMapName", recoMapName_);
  GetParameter("TimeConeMinus", timeConeMinus_);
  GetParameter("TimeConePlus", timeConePlus_);
  GetParameter("WallTime", wallTime_);
  GetParameter("MaxVerticalDist", maxVertDist_);
  GetParameter("MaxHorizontalDist", maxHorDist_);
  
  if (recoMapName_.empty())
    log_fatal("Configure parameter 'RecoMapName'");
  if (timeConeMinus_<0. || timeConePlus_<0.)
    log_fatal("Parameters 'TimeConeMinus' and 'TimeConePlus' have to be positive");
  if (wallTime_<0.)
    log_fatal("Parameter 'WallTime' has to be positive");
  if (maxVertDist_<0.)
    log_fatal("Parameter 'MaxVerticalDist' has to be positive");
  if (maxHorDist_<0.)
    log_fatal("Parameter 'MaxHorizontalDist' has to be positive");
  
  Evaluate= runTest;
};

//______________________________________________________________________________
void CausalConnectTester::Geometry(I3FramePtr frame){
  log_debug("Entering Geometry()");

  geometry_ = frame->Get<I3GeometryConstPtr>();
  if (!geometry_)
    log_fatal("Unable to find <I3Geometry>('I3Geometry')  in Geometry-frame!");

  domPos_cache_ = PartialCOG::BuildDomPos_cache(*geometry_);
  
  PushFrame(frame);
  log_debug("Leaving Geometry()");
}

//______________________________________________________________________________
CausalConnectTester::Result CausalConnectTester::Test(I3FrameConstPtr hypoframe,
                                                      I3FrameConstPtr frameA,
                                                      I3FrameConstPtr frameB) const
{
  // get all splitframe objects needed
  I3RecoPulseSeriesMapConstPtr PulsesA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  I3RecoPulseSeriesMapConstPtr PulsesB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

  if (not PulsesA || not PulsesB) {
    log_fatal("Could not find the RecoMap <I3RecoPulseSeriesMap>('%s') in the SplitFrames;", recoMapName_.c_str());
  }

  log_info("All necessary objects are found in the frames to begin hypothesis testing");
  
  return (Result)(CausalConnected(*PulsesA, *PulsesB));
}

//______________________________________________________________________________
bool CausalConnectTester::CausalConnected (const I3RecoPulseSeriesMap &pulsesA,
                                           const I3RecoPulseSeriesMap &pulsesB) const {
  log_debug("Entering CausalConnected()");

  HitSorting::TimeOrderedHitSet hitsA= HitSorting::ExtractHits<HitSorting::TimeOrderedHitSet, I3RecoPulse>(pulsesA, HitSorting::Extract_TotalChargeToFirstHit);
  HitSorting::TimeOrderedHitSet hitsB= HitSorting::ExtractHits<HitSorting::TimeOrderedHitSet, I3RecoPulse>(pulsesB, HitSorting::Extract_TotalChargeToFirstHit);

  //figure out which portion happens first
  HitSorting::TimeOrderedHitSet* hits_early;
  HitSorting::TimeOrderedHitSet* hits_late;
  double timediff;
  if (hitsA.begin()->time < hitsB.begin()->time) {// A is before B
    log_debug("PulsesA start before PulsesB: %f < %f", hitsA.begin()->time, hitsB.begin()->time);
    timediff = std::abs((hitsA.rbegin())->time - hitsB.begin()->time);

    hits_early = &hitsA;
    hits_late = &hitsB;
  }
  else {// B is before A
  log_debug("PulsesA start after PulsesB: %f >= %f", hitsA.begin()->time, hitsB.begin()->time);
    timediff = std::abs((hitsB.rbegin())->time - hitsA.begin()->time);

    hits_early = &hitsB;
    hits_late = &hitsA;
  }
  if (timediff>wallTime_) {
    log_debug("Max time difference between these events is too big : %f", timediff);
    return false; // vetotorecombine=true;
  }

  // test if the last three of the one and the first three pulses of the other series are causally connected inside time window
  std::pair < I3Position, double > cogE_last3_postime = PartialCOG::PartialCOGexclude(*hits_early, domPos_cache_, false, 3, 1);
  const double& cogE_last3_time = cogE_last3_postime.second;
  const I3Position& cogE_last3_pos = cogE_last3_postime.first;
  
  std::pair < I3Position, double > cogL_first3_postime = PartialCOG::PartialCOGexclude(*hits_late, domPos_cache_, true, 3, 1);
  const double& cogL_first3_time = cogL_first3_postime.second;
  const I3Position& cogL_first3_pos = cogL_first3_postime.first;
  
  //differences
  const double cog_diff_time = cogL_first3_time - cogE_last3_time;
  const double cog_diff_dist = I3FakeCalculator::DistanceTowardsPoint(cogL_first3_pos, cogE_last3_pos); // distance between both cog
  const double cog_diff_distx = std::fabs(cogL_first3_pos.GetX()-cogE_last3_pos.GetX());
  const double cog_diff_disty = std::fabs(cogL_first3_pos.GetY()-cogE_last3_pos.GetY());
  const double cog_diff_distz = std::fabs(cogL_first3_pos.GetZ()-cogE_last3_pos.GetZ());
  const double cog_diff_hordist = sqrt(cog_diff_distx*cog_diff_distx + cog_diff_disty*cog_diff_disty);

  // test if:
  // last and first are not time-inverted
  // within vertical distance
  // within horizontal distance
  // within walltime
  // causality is within the time residual [timeconeMinus, timeconePlus]
  if (cogE_last3_time <= cogL_first3_time
    && cog_diff_distz <= maxVertDist_
    && cog_diff_hordist <= maxHorDist_
    && cog_diff_time <= wallTime_
    && cog_diff_dist/I3Constants::c-timeConeMinus_ <= cog_diff_time
    && cog_diff_time <= cog_diff_dist/I3Constants::c+timeConePlus_)
  {
    log_info("Could causally connect A to B");
    return true;
  }
  else {
    log_info("Could NOT causally connect A to B");     
    return false;
  }
};
