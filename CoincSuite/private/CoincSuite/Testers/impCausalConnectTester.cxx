/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file impCausalConnectTester.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#ifndef IMPCAUSALCONNECTTEST_H
#define IMPCAUSALCONNECTTEST_H

#include "CoincSuite/Testers/TesterModule.h"
#include "CoincSuite/lib/HitSorting.h"

#include "phys-services/I3Calculator.h"


/** @brief A I3Module testing if two series of Pulses are connected topologically and have correct timing
 * Of two fractions of pulses, test if they:
 * -are happening after each other
 * -the last/first emission point of !direct! Cherenkov-light hitting a DOM
 * -horizontal distance,
 * -vertical distance,   
 * -maximum time difference
 * -and the CT-time-residual
 */
class impCausalConnectTester : public TesterModule {
  SET_LOGGER("impCausalConnectTester");
private: // parameters
  /// PARAM: Name of the I3RecoPulseSeriesMap that should be checked
  std::string recoMapName_;
  /// PARAM: Name of the fit that should be tested
  std::string hypoFitName_;
  /// PARAM: Allowed time residual window for travel between the Emission-Points
  std::pair<double, double> timeResTravel_;
  /// PARAM: Allowed time residual window for direct hits in the consideration for EmissionPoints
  std::pair<double, double> timeResHit_;
  /// PARAM: maximal time before subseries of pulses can not be seen as connected any more
  double wallTime_;
  /// PARAM: maximal vertical distance before subseries of pulses can not be seen as connected any more
  double maxVertDist_; 
  /// PARAM: maximal horizontal distance before subseries of pulses can not be seen as connected any more
  double maxHorDist_;
  
private:
  /// Implement the evaluation for mutual-testing
  Result Test(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  ///call-back function
  static Result runTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    impCausalConnectTester *alPtr = (impCausalConnectTester*) tester;
    return alPtr->Test(hypoframe, frameA, frameB);
  };
  
  
private:
  /// short-hand for the emission point(s) along a track
  typedef std::pair<I3Position, double> Emission; //its a four-vector: (point, time)
  typedef std::vector<Emission> EmissionSeries;
  
  /// helper for sorting operation on EmissionSeries
  struct SortAlongTrack {
    I3Particle fit_;
    SortAlongTrack(const I3Particle &fit):
      fit_(fit) {};
    inline bool operator() (const Emission& lhs, const Emission& rhs)
      {return (I3Calculator::DistanceAlongTrack(fit_, lhs.first) < I3Calculator::DistanceAlongTrack(fit_, rhs.first));};
  };
  
  /** @brief 'CRAZY' function return the last or first position along a track 
   *         where Cherenkov-light hit the first or last OM in a series of first hits
   * @param hits a time ordered set of hit to find the emisson-points for
   * @param fit the track which is the sum of all possible emission points
   * @param first_last if (true) take the first (earliest) emission point (start of the track),
   *                   if (false) take the last (latest) emission point (end of the track)
   */
  Emission CherenkovPointTime (const HitSorting::TimeOrderedHitSet& hits,
                               const I3Particle &fit,
                               const bool first_last) const;

  /** @brief Tests if we can causally connect the Pulses
   * @param pulsesA Map of RecoPulseSeriesMap A
   * @param pulsesB Map of RecoPulseSeriesMap B
   * @param fit which is the particle hypothesis
   * @return true, if SplitPulses are causally connected through the fit 
   */
  bool CausalConnected (const I3RecoPulseSeriesMap &pulsesA,
                        const I3RecoPulseSeriesMap &pulsesB,
                        const I3Particle &fit) const;
  
public:
  /// constructor
  impCausalConnectTester (const I3Context& context);

  /// Configure function
  void Configure();
};

I3_MODULE(impCausalConnectTester);

#endif // IMPCAUSALCONNECTTEST_H


//=================================IMPLEMENTATIONS==============================

#include "phys-services/I3Calculator.h"

#include "CoincSuite/lib/OMKeyHash.h"
#include "CoincSuite/lib/I3FakeCalculator.h"

using namespace CoincSuite;

//______________________________________________________________________________
impCausalConnectTester::impCausalConnectTester (const I3Context& context):
  TesterModule(context),
  recoMapName_("MaskedOfflinePulses"),
  hypoFitName_(""),
  timeResTravel_(std::make_pair(-1000.*I3Units::ns, 1000.*I3Units::ns)),
  timeResHit_(std::make_pair(-15.*I3Units::ns, 150.*I3Units::ns)),
  wallTime_(2000.*I3Units::ns),
  maxVertDist_(400.*I3Units::m),
  maxHorDist_(400.*I3Units::m)
{
  AddParameter("RecoMapName", "Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("HypoFitName", "Name of the Fit in the HypoFrame to test against", hypoFitName_);
  AddParameter("TravelTimeResidual", "Allowed time residual window for travel between the EmissionPoints", timeResTravel_);
  AddParameter("HitTimeResidual", "Allowed time residual window for direct hits in the consideration for EmissionPoints", timeResHit_); 
  AddParameter("WallTime", "Maximal time difference in [ns] between Pulses fractions to still reconnect", wallTime_);
  AddParameter("MaxVerticalDist", "Maximal vertical distance in [m] to still reconnect", maxVertDist_);
  AddParameter("MaxHorizontalDist", "Maximal horizontal distance in [m] to still reconnect", maxHorDist_);
  
};

//______________________________________________________________________________
void impCausalConnectTester::Configure() {
  TesterModule::Configure();
  
  GetParameter("RecoMapName", recoMapName_);
  GetParameter("HypoFitName", hypoFitName_);
  GetParameter("TravelTimeResidual", timeResTravel_);
  GetParameter("HitTimeResidual", timeResHit_); 
  GetParameter("WallTime", wallTime_);
  GetParameter("MaxVerticalDist", maxVertDist_);
  GetParameter("MaxHorizontalDist", maxHorDist_);
  
  if (recoMapName_.empty())
    log_fatal("Configure parameter 'RecoMapName'");
  if (hypoFitName_.empty())
    log_fatal("Configure parameter 'HypoFitName'");
  if (timeResTravel_.second<timeResTravel_.first || timeResHit_.second<timeResHit_.first)
    log_fatal("Time Residuals have to define a non negative interval");
  if (wallTime_<0.)
    log_fatal("Parameter 'WallTime' has to be positive");
  if (maxVertDist_<0.)
    log_fatal("Parameter 'MaxVerticalDist' has to be positive");
  if (maxHorDist_<0.)
    log_fatal("Parameter 'MaxHorizontalDist' has to be positive");
  
  Evaluate = runTest;
};

//______________________________________________________________________________
impCausalConnectTester::Result impCausalConnectTester::Test (I3FrameConstPtr hypoframe,
                                                             I3FrameConstPtr frameA,
                                                             I3FrameConstPtr frameB) const
{
  //now the action can start: buffer up necessary frame objects

  // get all SplitFrame objects needed
  I3RecoPulseSeriesMapConstPtr PulsesA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  I3RecoPulseSeriesMapConstPtr PulsesB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

  if (not PulsesA || not PulsesB) {
    log_warn("Could not find the recoMap <I3RecoPulseSeriesMap>('%s') in the SplitFrames;", recoMapName_.c_str());
    return UNDECIDED;
  }

  I3ParticleConstPtr hypoFit = hypoframe->Get<I3ParticleConstPtr>(hypoFitName_);
  if (not hypoFit) {
    log_error("Could not find the HypoFit <I3Particle>('%s') in the HypoFrame; will continue with next HypoFrame", hypoFitName_.c_str());
    return UNDECIDED;
  }
  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_warn("Fit Status not good; will continue with next HypoFrame");
    return UNDECIDED; // to loop over all HypoFrames
  }
  
  log_trace("all necessary objects are found in the frames to begin hypothesis testing");
  // apply causal connection criteria
  return (Result)(CausalConnected(*PulsesA, *PulsesB, *hypoFit));
}

//______________________________________________________________________________
bool impCausalConnectTester::CausalConnected (const I3RecoPulseSeriesMap &pulsesA,
                                              const I3RecoPulseSeriesMap &pulsesB,
                                              const I3Particle &hypoFit) const
{
  log_debug("Entering impCausalConnected()");
  //now test for timing

  if (pulsesA.size()==0 || pulsesB.size()==0) {
    log_error("PulsesA (size=%d) or PulsesB (size=%d) are empty;"
    "cannot evaluate CausalConnected(): return false", (int)pulsesA.size(), (int)pulsesB.size());
    return false;
  }

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
    return false; //NOTE make an early exit here to spare computation time
  }

  // use the last emission point (the one furthest down the track) on the early portion of pulses 
  // and the first emission point (the one furthest up the track) on the early portion of pulses 
  std::pair < I3Position, double > Early_postime = CherenkovPointTime(*hits_early, hypoFit, false);
  std::pair < I3Position, double > Late_postime = CherenkovPointTime(*hits_late, hypoFit, true);

  if (std::isnan(Early_postime.second) || std::isnan(Late_postime.second)) {
    log_debug("Not connected because one no Emission points could be found");
    return false;
  }
  
  const double& Early_time = Early_postime.second;
  const I3Position& Early_pos = Early_postime.first;
  
  const double& Late_time = Late_postime.second;
  const I3Position& Late_pos = Late_postime.first;

  //differences
  const double diff_time = Late_time-Early_time;
  const double diff_dist = I3FakeCalculator::DistanceTowardsPoint(Late_pos, Early_pos); // distance between both cog
  const double diff_distx = std::fabs(Late_pos.GetX()-Early_pos.GetX());
  const double diff_disty = std::fabs(Late_pos.GetY()-Early_pos.GetY());
  const double diff_distz = std::fabs(Late_pos.GetZ()-Early_pos.GetZ());
  const double diff_hordist = sqrt(diff_distx*diff_distx + diff_disty*diff_disty);

  const double res = diff_time - diff_dist/I3Constants::c;

  //calculate the subcriteria
  const bool passed_timeInv = (Early_time <= Late_time);
  const bool passed_maxVertDist = (diff_distz <= maxVertDist_);
  const bool passed_maxHorDist = (diff_hordist <= maxHorDist_);
  const bool passed_walltime = (diff_time <= wallTime_);
  const bool passed_travelres = (timeResTravel_.first <= res) && (res <= timeResTravel_.second);
  
  log_trace_stream((passed_timeInv?"Passed":"Failed")<<" time-inversion: "<<Early_time<<" vs. "<<Late_time);
  log_trace_stream((passed_maxVertDist?"Passed":"Failed")<<" vertical distance("<< maxVertDist_<<"): "<<diff_distz);
  log_trace_stream((passed_maxHorDist?"Passed":"Failed")<<" horizontal distance("<<maxHorDist_<<"): "<<diff_hordist);
  log_trace_stream((passed_walltime?"Passed":"Failed")<<" walltime("<<wallTime_<<"): "<<diff_time);
  log_trace_stream((passed_travelres?"Passed":"Failed")<<" travel time residual (["<<timeResTravel_.first<<","<<timeResTravel_.second<<"]): "<<res);

  // test if:
  // last and first are not inverted in time
  // within walltime
  // within vertical distance
  // within horizontal distance
  // causality is within the time residual [timeconeMinus, timeconePlus]
  const bool connected =(passed_timeInv
    && passed_maxVertDist 
    && passed_maxHorDist
    && passed_walltime
    && passed_travelres);
  
  log_debug_stream("Could "<<( !connected ? "NOT ":"")<<"causally connect A to B");
  return connected;
};

//______________________________________________________________________________
std::pair<I3Position, double> impCausalConnectTester::CherenkovPointTime(const HitSorting::TimeOrderedHitSet& hits,
                                                                         const I3Particle &hypoFit,
                                                                         const bool first_last) const
{
  log_debug("Entering CherenkovPointTime()");

  EmissionSeries emission_series;

  BOOST_FOREACH(const HitSorting::Hit &hit, hits) {
    I3Position chpos; //Emission on track
    double chtime; //total travel time
    double chdist; //distance between chpos and dompos
    double changle; //unnecessary z angle   

    I3Position dompos = geometry_->omgeo.find(OMKeyHash::SimpleIndex2OMKey(hit.domIndex))->second.position;
    I3Calculator::CherenkovCalc(hypoFit, dompos, chpos, chtime, chdist, changle);

    const double t_res =  hit.time - hypoFit.GetTime() - chtime; // the time res
    
    if (!(timeResHit_.first <=t_res && t_res <= timeResHit_.second)) {
      log_debug_stream("this hit "<<hit<<" and its emission point is rejected, because its TimeResidual is too big: "<<t_res);
      continue; //hop over everything that is not in the time residual
    }
    const I3Position& emission_point = chpos;
    const double emission_time = hit.time - chdist/ I3Constants::c;

    emission_series.push_back(std::make_pair(emission_point, emission_time));
  }

  const SortAlongTrack sortalongtrack(hypoFit);
  sort(emission_series.begin(), emission_series.end(), sortalongtrack); 
  
  //catch bad behaviours
  if (emission_series.size()==0) {
    log_warn("No Emission points could be found on this track");
    return std::make_pair(I3Position(), NAN);
  }
  if (first_last)
    return emission_series.front();
  else
    return emission_series.back();
};
