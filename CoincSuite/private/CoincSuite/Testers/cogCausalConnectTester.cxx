/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file cogCausalConnectTester.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#ifndef COGCAUSALCONNECTTEST_H
#define COGCAUSALCONNECTTEST_H

#include "CoincSuite/Testers/TesterModule.h"

#include "CoincSuite/lib/PartialCOG.h"

/** @brief A I3Module testing if two series of Pulses are connected topologically and in timing
 * of two fractions of Pulses, test if they:
 * are happening after each other
 * from the COG of the respective last three and respective first three Pulses evaluate
 * horizontal distance,
 * vertical distance,
 * maximum time difference
 * and the ct time-residual
 */
class cogCausalConnectTester : public TesterModule {
private:
  SET_LOGGER("cogCausalConnectTester");
private: //parameters
  /// Param: Name of the I3RecoPulseSeriesMap that should be check
  std::string recoMapName_;
  /// Param: Name of the fit that should be tested
  std::string hypoFitName_;
  /// PARAM: Allowed time residual window for travel between the Emission-Points
  std::pair<double, double> timeResTravel_;
  /// Param: maximal time before subseries of pulses can not be seen as connected any more
  double wallTime_;
  /// Param: maximal vertical distance before subseries of pulses can not be seen as connected any more
  double maxVertDist_;
  /// Param: maximal horizontal distance before subseries of pulses can not be seen as connected any more
  double maxHorDist_;
  /// Param: maximal distance between track and cog
  double maxTrackDist_;
  /// Param: maximal distance between outmost(furthest) hit and track (perpendicular)
  double maxFurthestDist_;
private: // properties
  /// the DOMposition cache; buffering up the position of each DOM in a direct access
  PartialCOG::DomPos_cache domPos_cache_;
private: // methods  
  /// Implement the Evaluatuon for MutualTesting
  Result Test(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  ///call-back function
  static Result runTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    cogCausalConnectTester *alPtr = (cogCausalConnectTester*) tester;
    return alPtr->Test(hypoframe, frameA, frameB);
  };

public:
  ///constructor
  cogCausalConnectTester (const I3Context& context);

  /// std Configure function
  void Configure();

  ///Geometry call method to interact with Geometry frame
  void Geometry (I3FramePtr frame);

private:
  /** @brief Tests if we can causally connect the last 3 hits except one of PulsesA
   *         to the first 3 hits except one of PulsesB (if X is before Y)
   * @param pulsesA Map of RecoPulseSeriesMap A
   * @param pulsesB Map of RecoPulseSeriesMap B
   * @param hypoFit Fit from the HypoFrame
   * @return true, if so
   */
  bool CausalConnected (const I3RecoPulseSeriesMap &pulsesA,
                        const I3RecoPulseSeriesMap &pulsesB,
                        const I3Particle& hypoFit) const;
};

I3_MODULE(cogCausalConnectTester);

#endif // COGCAUSALCONNECTTEST_H


//============================ IMPLEMENTATIONS =================================

#include "phys-services/I3Calculator.h"

#include "CoincSuite/lib/OMKeyHash.h"
#include "CoincSuite/lib/HitSorting.h"
#include "CoincSuite/lib/I3FakeCalculator.h"

using namespace CoincSuite;

//______________________________________________________________________________
cogCausalConnectTester::cogCausalConnectTester (const I3Context& context):
  TesterModule(context),
  recoMapName_("MaskedOfflinePulses"),
  hypoFitName_(""),
  timeResTravel_(std::make_pair(-1000.*I3Units::ns, 1000.*I3Units::ns)),
  wallTime_(2000*I3Units::ns),
  maxVertDist_(400*I3Units::m),
  maxHorDist_(400*I3Units::m),
  maxTrackDist_(200*I3Units::m),
  maxFurthestDist_(600*I3Units::m)
{
  AddParameter("RecoMapName", "Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("HypoFitName", "Name of the Fit in the HypoFrame to test against", hypoFitName_);
  AddParameter("TravelTimeResidual", "Allowed time residual window for travel between the EmissionPoints", timeResTravel_);
  AddParameter("WallTime", "Maximal time difference in [ns] between Pulses fractions to still reconnect", wallTime_);
  AddParameter("MaxVerticalDist", "Maximal vertical distance in [m] to still reconnect", maxVertDist_);
  AddParameter("MaxHorizontalDist", "Maximal horizontal distance in [m] to still reconnect", maxHorDist_);
  AddParameter("MaxTrackDist", "Maximal distance of COG to particle track in [m] to still reconnect", maxTrackDist_);
  AddParameter("MaxFurthestDist", "Maximal perpendicular distance of the outmost hit towards the track in each hit-series", maxFurthestDist_);
};

//______________________________________________________________________________
void cogCausalConnectTester::Configure() {
  TesterModule::Configure();
  GetParameter("RecoMapName", recoMapName_);
  GetParameter("HypoFitName", hypoFitName_);
  GetParameter("TravelTimeResidual", timeResTravel_);
  GetParameter("WallTime", wallTime_);
  GetParameter("MaxVerticalDist", maxVertDist_);
  GetParameter("MaxHorizontalDist", maxHorDist_);
  GetParameter("MaxTrackDist", maxTrackDist_);
  GetParameter("MaxFurthestDist", maxFurthestDist_);
  
  if (recoMapName_.empty())
    log_fatal("Configure parameter 'RecoMapName'");
  if (hypoFitName_.empty())
    log_fatal("Configure parameter 'HypoFitName'");
  if (timeResTravel_.second<timeResTravel_.first)
    log_fatal("Time Residuals have to define a non negative interval");
  if (wallTime_<0.)
    log_fatal("Parameter 'WallTime' has to be positive");
  if (maxVertDist_<0.)
    log_fatal("Parameter 'MaxVerticalDist' has to be positive");
  if (maxHorDist_<0.)
    log_fatal("Parameter 'MaxHorizontalDist' has to be positive");
  if (maxTrackDist_<0.)
    log_fatal("Parameter 'MaxTrackDist' has to be positive");
  if (maxFurthestDist_<0.)
    log_fatal("Parameter 'MaxFurthestDist' has to be positive");

  Evaluate = runTest;
};

//______________________________________________________________________________
void cogCausalConnectTester::Geometry(I3FramePtr frame){
  log_debug("Entering Geometry()");

  geometry_ = frame->Get<I3GeometryConstPtr>("I3Geometry");
  if (!geometry_)
    log_fatal("Unable to find <I3Geometry>(I3Geometry) in the Geometry-frame!");

  domPos_cache_ = PartialCOG::BuildDomPos_cache(*geometry_);

  PushFrame(frame, "OutBox");
  log_debug("Leaving Geometry()");
}

cogCausalConnectTester::Result cogCausalConnectTester::Test(I3FrameConstPtr hypoframe,
                                                            I3FrameConstPtr frameA,
                                                            I3FrameConstPtr frameB) const {
  I3ParticleConstPtr hypoFit = hypoframe->Get<I3ParticleConstPtr>(hypoFitName_);
  if (not hypoFit) {
    log_error("Could not find the HypoFit <I3Particle>('%s') in the HypoFrame; will continue with next HypoFrame", hypoFitName_.c_str());
    return UNDECIDED;
  }
  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_warn("Fit Status not good; will continue with next HypoFrame");
    return UNDECIDED;
  }

  // get all splitframe objects needed
  I3RecoPulseSeriesMapConstPtr PulsesA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  I3RecoPulseSeriesMapConstPtr PulsesB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

  if (not PulsesA || not PulsesB) {
    log_error("Could not find the RecoMap <I3RecoPulseSeriesMap>('%s') in the SplitFrames;", recoMapName_.c_str());
    return UNDECIDED;
  }
  log_debug("All necessary objects are found; begin hypothesis testing");

  // apply causal connection criteria
  return (Result)(CausalConnected(*PulsesA, *PulsesB, *hypoFit));
}

//______________________________________________________________________________
bool cogCausalConnectTester::CausalConnected (const I3RecoPulseSeriesMap &pulsesA,
                                              const I3RecoPulseSeriesMap &pulsesB,
                                              const I3Particle &hypoFit) const
{
  log_debug("Entering CausalConnected()");
  //now test for timing
  if (pulsesA.size()==0 || pulsesB.size()==0) {
    log_debug("PulsesA (size=%d) or PulsesB (size=%d) are empty;"
              " cannot evaluate CausalConnection: return false", (int)pulsesA.size(), (int)pulsesB.size());
    return false;
  }

  HitSorting::TimeOrderedHitSet hitsA= HitSorting::ExtractHits<HitSorting::TimeOrderedHitSet, I3RecoPulse>(pulsesA, HitSorting::Extract_TotalChargeToFirstHit);
  HitSorting::TimeOrderedHitSet hitsB= HitSorting::ExtractHits<HitSorting::TimeOrderedHitSet, I3RecoPulse>(pulsesB, HitSorting::Extract_TotalChargeToFirstHit);

  //figure out which portion happens first
  HitSorting::TimeOrderedHitSet* hits_early;
  HitSorting::TimeOrderedHitSet* hits_late;

  const double starttime_diff = hitsA.begin()->time - hitsB.begin()->time;
  const double endtime_diff  = (hitsA.rbegin())->time - (hitsB.rbegin())->time;

  if (starttime_diff<0) {// A is before B
    log_debug("PulsesA start before PulsesB: %f < %f", hitsA.begin()->time, hitsB.begin()->time);
    if (endtime_diff>0 && std::fabs(endtime_diff)>std::fabs(starttime_diff)) {
      log_debug("but extents far beyond; going to switch their order");
      hits_early = &hitsB;
      hits_late = &hitsA;
    } else {
      hits_early = &hitsA;
      hits_late = &hitsB;
    }
  }
  else {// B is before A
    log_debug("PulsesA start after PulsesB: %f >= %f", hitsA.begin()->time, hitsB.begin()->time);
    if (endtime_diff<0 && std::fabs(endtime_diff)>std::fabs(starttime_diff)) {
      log_debug("but extents far beyond; going to switch their order");
      hits_early = &hitsA;
      hits_late = &hitsB;
    } else {
      hits_early = &hitsB;
      hits_late = &hitsA;
    }
  }
  //figured out which one is early and which one is late

  
  /// take the THIRD quartile (at least 4 hits) of pulses from the EARLY pulses and calculate cog
  const PartialCOG::PosTime Early_postime = PartialCOG::PartialCOGhit(*hits_early, domPos_cache_, 4, 3, false, 4, 1000);
  /// take the SECOND quartile (at least 4 hits) of pulses from the LATE pulses and calculate cog
  const PartialCOG::PosTime Late_postime = PartialCOG::PartialCOGhit(*hits_late, domPos_cache_, 4, 2, true, 4, 1000);

  const I3Position& Early_pos = Early_postime.first;
  const double& Early_time = Early_postime.second;

  const I3Position& Late_pos = Late_postime.first;
  const double& Late_time = Late_postime.second;

  const double Early_distToTrack = I3Calculator::ClosestApproachDistance(hypoFit, Early_pos);
  const double Late_distToTrack = I3Calculator::ClosestApproachDistance(hypoFit, Late_pos);

  //which hit is furthest outside
  double Early_distFurthestHit(0);
  double Late_distFurthestHit(0);

  for (HitSorting::TimeOrderedHitSet::const_iterator hits_iter= hits_early->begin(); hits_iter!=hits_early->end(); ++hits_iter) {
    const I3Position& dompos = domPos_cache_[hits_iter->domIndex];
    Early_distFurthestHit = std::max(Early_distFurthestHit, I3Calculator::ClosestApproachDistance(hypoFit, dompos));
  }
  for (HitSorting::TimeOrderedHitSet::const_iterator hits_iter= hits_late->begin(); hits_iter!=hits_late->end(); ++hits_iter) {
    const I3Position& dompos = domPos_cache_[hits_iter->domIndex];
    Late_distFurthestHit = std::max(Late_distFurthestHit, I3Calculator::ClosestApproachDistance(hypoFit, dompos));
  }

  log_trace("computing differences");
  //differences
  const double diff_time = Late_time-Early_time;
  const double diff_dist = I3FakeCalculator::DistanceTowardsPoint(Late_pos, Early_pos); // distance between both cog
  const double diff_distx = std::fabs(Late_pos.GetX()-Early_pos.GetX());
  const double diff_disty = std::fabs(Late_pos.GetY()-Early_pos.GetY());
  const double diff_distz = std::fabs(Late_pos.GetZ()-Early_pos.GetZ());
  const double diff_hordist = sqrt(diff_distx*diff_distx + diff_disty*diff_disty);

  const double res = diff_time - diff_dist/I3Constants::c;
  
  //NOTE some of these could move up, so that decision are taken early and computation time is saved
  const bool passed_timeInv = (Early_time <= Late_time);
  const bool passed_maxVertDist = (diff_distz <= maxVertDist_);
  const bool passed_maxHorDist = (diff_hordist <= maxHorDist_);
  const bool passed_walltime = (diff_time <= wallTime_);
  const bool passed_travelres = (timeResTravel_.first <= res) && (res <= timeResTravel_.second);
  const bool passed_distToCOGearly = (Early_distToTrack <= maxTrackDist_);
  const bool passed_distToCOGlate = (Late_distToTrack <= maxTrackDist_);
  const bool passed_distToFHearly = (Early_distFurthestHit <= maxFurthestDist_);
  const bool passed_distToFHlate = (Late_distFurthestHit <= maxFurthestDist_);
  
  log_trace_stream((passed_timeInv?"Passed":"Failed")<<" time-inversion: "<<Early_time<<" vs. "<<Late_time);
  log_trace_stream((passed_maxVertDist?"Passed":"Failed")<<" vertical distance("<< maxVertDist_<<"): "<<diff_distz);
  log_trace_stream((passed_maxHorDist?"Passed":"Failed")<<" horizontal distance("<<maxHorDist_<<"): "<<diff_hordist);
  log_trace_stream((passed_walltime?"Passed":"Failed")<<" walltime("<<wallTime_<<"): "<<diff_time);
  log_trace_stream((passed_travelres?"Passed":"Failed")<<" travel time residual (["<<timeResTravel_.first<<","<<timeResTravel_.second<<"]): "<<res);
  log_trace_stream((passed_distToCOGearly?"Passed":"Failed")<<" distance to Track COG Early("<<maxTrackDist_<<"): "<<Early_distToTrack);
  log_trace_stream((passed_distToCOGlate?"Passed":"Failed")<<" distance to Track COG Late("<<maxTrackDist_<<"): "<<Late_distToTrack);
  log_trace_stream((passed_distToFHearly?"Passed":"Failed")<<" distance to Track Furtherst Hit Early("<<maxFurthestDist_<<") "<< Early_distFurthestHit);
  log_trace_stream((passed_distToFHlate?"Passed":"Failed")<<" distance to Track Furtherst Hit Late("<<maxFurthestDist_<<") "<< Late_distFurthestHit);

  const bool connected = (passed_timeInv
    && passed_maxVertDist 
    && passed_maxHorDist
    && passed_walltime
    && passed_travelres
    && passed_distToCOGearly
    && passed_distToCOGlate
    && passed_distToFHearly
    && passed_distToFHlate);
  
  log_debug_stream("Could "<<( !connected ? "NOT ":"")<<"causally connect");
  return connected;
};
