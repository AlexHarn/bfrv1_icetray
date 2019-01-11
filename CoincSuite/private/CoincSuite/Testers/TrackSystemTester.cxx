/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file TrackSystemTester.cxx
 * @version $Revision$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date$
 */

#ifndef TRACKSYSTEMTEST_H
#define TRACKSYSTEMTEST_H

#include "CoincSuite/Testers/TesterModule.h"

/** A test if pulses are, by a certain fraction, in the in the track system of a proposed ParticleFit.
 */
class TrackSystemTester : public TesterModule {
  SET_LOGGER("TrackSystemTester");
private: // parameters
  /// Param: Name of the RecoPulseSeriesMap
  std::string recoMapName_;
  /// Param: Name of the reconstructed (I3Particle) Fit
  std::string recoFitName_;
  /// Param: Name of the hypothetical (I3Particle) Fit
  std::string hypoFitName_;
  /// Param: residual window to find pulses with
  std::pair<double,double> resTimeWindow_;
  /// Param: Radius around the HypoFit within the Pulses should be found
  double cylinderRadius_;
  /// Param: Ratio above which we accept hypothesis
  double criticalRatio_;
  /// Param: Adjust the Speed of the Particle to this value
  double particleSpeed_;

  /// PARAM: Test Mutually instead of Hypothetically
  bool mutualOpt_;

  /// Implement the Evaluation for hypo-testing
  Result HypoTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  /// Implement the evaluation for mutual-testing
  Result MutualTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  //interface for Evaluate
  ///call-back function
  static Result runHypoTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    TrackSystemTester *alPtr = (TrackSystemTester*) tester;
    return alPtr->HypoTest(hypoframe, frameA, frameB);
  };
  ///call-back function
  static Result runMutualTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    TrackSystemTester *alPtr = (TrackSystemTester*) tester;
    return alPtr->MutualTest(hypoframe, frameA, frameB);
  };
  
public:
  /// Constructor
  TrackSystemTester (const I3Context& context);
  /// Configure function
  void Configure();
  
private:
  /** Are the found pulses on the Track System within the required time residual
   *  @param track the (I3Particle)track defining the moving particle (no cascade please)
   *  @param pulses the pulses being probed
   */
  bool TrackSystemCriterum (const I3Particle& track,
                            const I3RecoPulseSeriesMap& pulses) const;
};

I3_MODULE(TrackSystemTester);

#endif // TRACKSYSTEMTEST_H


//============================IMPLEMENTATIONS===================================
#include "CoincSuite/lib/HitSorting.h"
#include "phys-services/I3Calculator.h"

//______________________________________________________________________________
TrackSystemTester::TrackSystemTester (const I3Context& context):
  TesterModule(context),
  recoMapName_(""),
  recoFitName_(""),
  hypoFitName_(""),
  resTimeWindow_(std::make_pair(-200.*I3Units::ns, 200.*I3Units::ns)),
  cylinderRadius_(50.*I3Units::m),
  criticalRatio_(0.9),
  particleSpeed_(NAN),
  mutualOpt_(false)
{
  AddParameter("RecoMapName","Name of the RecoPulseSeriesMap(Mask)", recoMapName_);
  AddParameter("RecoFitName", "Name of the (I3Particle)Fit", recoFitName_);
  AddParameter("HypoFitName", "Name of the (I3Particle)Fit in the HypoFrame to test against", hypoFitName_);
  AddParameter("ResTimeWindow", "Residual Time Window to find Pulses within", resTimeWindow_);
  AddParameter("CylinderRadius", "Raidus of cylinder where we gonna look for hits", cylinderRadius_);
  AddParameter("CriticalRatio", "Ratio above which we decise to recombine", criticalRatio_);
  AddParameter("ParticleSpeed", "Adjust the Speed of the Particle to this value", particleSpeed_);
  AddParameter("MutualCompare", "Hypo(false) -or- Mutual(true)", mutualOpt_);
};

//______________________________________________________________________________
void TrackSystemTester::Configure() {
  TesterModule::Configure();
  GetParameter("RecoMapName", recoMapName_);
  GetParameter("RecoFitName", recoFitName_);
  GetParameter("HypoFitName", hypoFitName_);
  GetParameter("ResTimeWindow", resTimeWindow_);
  GetParameter("CylinderRadius", cylinderRadius_);
  GetParameter("CriticalRatio", criticalRatio_);
  GetParameter("ParticleSpeed", particleSpeed_);
  GetParameter("MutualCompare", mutualOpt_);
  
  if (mutualOpt_ && recoFitName_=="")
    log_fatal("Configure 'RecoFitName'");
  if (!mutualOpt_ && hypoFitName_=="")
    log_fatal("Configure 'HypoFitName'");
  
  if (particleSpeed_>I3Constants::c)
    log_warn("You required that the ParticleSpeed exceeds the speed of light (c);"
      "This is allowed but might not be feasible!");
  if (particleSpeed_<=0.)
    log_fatal("Value for parameter 'ParticleSpeed' cannot be negative nor Zero");
    
  if (!mutualOpt_)
    Evaluate = runHypoTest;
  else
    Evaluate = runMutualTest;
};

//______________________________________________________________________________
TrackSystemTester::Result TrackSystemTester::HypoTest(I3FrameConstPtr hypoframe,
                                                      I3FrameConstPtr frameA,
                                                      I3FrameConstPtr frameB) const
{
  //now the action can start: buffer up necessary frame objects
  // get all HypoFrame objects

  I3ParticleConstPtr hypoFit = hypoframe->Get<I3ParticleConstPtr>(hypoFitName_);
  if (not hypoFit) {
    log_error("Cannot find the HypoFit <I3Particle>('%s') in the HypoFrame;"
              " will continue with next HypoFrame", hypoFitName_.c_str());
    return UNDECIDED;
  }
  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_warn("Fit Status not good; will continue with next HypoFrame");
    return UNDECIDED; // to loop over all HypoFrames
  }

  I3RecoPulseSeriesMapConstPtr pulsesA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  I3RecoPulseSeriesMapConstPtr pulsesB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

  if (not pulsesA || not pulsesB) {
    log_fatal("Could not find the <I3RecoMapSeriesMap>('%s') in the SplitFrames;", recoMapName_.c_str());
  }
  
  log_debug("all necessary objects are found in the frames to begin hypothesis testing");

  //if requested augment the speed of the particle
  I3Particle emitterParticle = *hypoFit;
  if (!std::isnan(particleSpeed_))
    emitterParticle.SetSpeed(particleSpeed_);
  
  const bool criteriumA = TrackSystemCriterum(emitterParticle, *pulsesA);
  const bool criteriumB = TrackSystemCriterum(emitterParticle, *pulsesB);
  
  return (Result)(criteriumA || criteriumB);
}

//______________________________________________________________________________
TrackSystemTester::Result TrackSystemTester::MutualTest(I3FrameConstPtr hypoframe,
                                                        I3FrameConstPtr frameA,
                                                        I3FrameConstPtr frameB) const
{
  I3ParticleConstPtr recoFitA = frameA->Get<I3ParticleConstPtr>(recoFitName_);
  I3ParticleConstPtr recoFitB = frameB->Get<I3ParticleConstPtr>(recoFitName_);
  if (not recoFitA || not recoFitB) {
    log_error("Could not find the RecoFits <I3Particle>('%s') in the SplitFrame;"
              "will continue with next HypoFrame", recoFitName_.c_str());
    return UNDECIDED; // to loop over all Hypoframes
  }
  
  I3RecoPulseSeriesMapConstPtr pulsesA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  I3RecoPulseSeriesMapConstPtr pulsesB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

  if (not pulsesA || not pulsesB) {
    log_fatal("Could not find the <I3RecoMapSeriesMap>('%s') in the SplitFrames;", recoMapName_.c_str());
  }
  
  log_debug("all necessary objects are found in the frames to begin hypothesis testing");

  //if requested augment the speed of the particle
  I3Particle emitterParticleA = *recoFitA;
  I3Particle emitterParticleB = *recoFitB;
  if (!std::isnan(particleSpeed_)) {
    emitterParticleA.SetSpeed(particleSpeed_);
    emitterParticleB.SetSpeed(particleSpeed_);
  }
  
  if (recoFitA->GetFitStatus() != I3Particle::OK && recoFitB->GetFitStatus() != I3Particle::OK ) {
    log_debug("Fit Status not good; cannot test");
    return UNDECIDED;
  }
  
  bool liketorecombine = false;
  bool vetotorecombine = false;
  
  // test recoFitA against Pulses B
  if (emitterParticleA.GetFitStatus() != I3Particle::OK) {
    log_debug("FitA Status not good; cannot test");
  } else {
    const bool criterium = TrackSystemCriterum(emitterParticleA, *pulsesB);
    liketorecombine = liketorecombine || criterium;
    vetotorecombine = vetotorecombine || !criterium;
  }

  // test recoFitB against Pulses A
  if (emitterParticleB.GetFitStatus() != I3Particle::OK) {
    log_debug("FitB Status not good; cannot test");
  } else {
    const bool criterium = TrackSystemCriterum(emitterParticleB, *pulsesA);
    liketorecombine = liketorecombine || criterium;
    vetotorecombine = vetotorecombine || !criterium;
  }
  
  return (Result)(liketorecombine && ! (vetotorecombine));
}


//______________________________________________________________________________
bool TrackSystemTester::TrackSystemCriterum (const I3Particle& track,
                                             const I3RecoPulseSeriesMap& pulses) const {
  log_debug("Entering TrackSystemCriteria()");

  unsigned int nOnTrackSystem =0; //counter
  unsigned int nOffTrackSystem =0; //counter
  
  HitSorting::HitSeries hits= HitSorting::ExtractHits<HitSorting::HitSeries, I3RecoPulse>(pulses, HitSorting::Extract_TotalChargeToFirstHit);
  
  BOOST_FOREACH(const HitSorting::Hit &hit, hits) {
    const OMKey omkey = OMKeyHash::SimpleIndex2OMKey(hit.domIndex);
    const I3Position ompos = (geometry_->omgeo.find(omkey)->second).position;
    
    const double timeRes = I3Calculator::TimeResidual(track, ompos, hit.time);
    const bool onTime = ((resTimeWindow_.first<=timeRes) && (timeRes<=resTimeWindow_.second));
    const bool onTrack = I3Calculator::IsOnTrack(track, ompos, cylinderRadius_);
    
    if (onTime && onTrack)
      nOnTrackSystem++;
    else
      nOffTrackSystem++;
  }

  //Probe the multiplicity requirement
  log_debug_stream("On track:"<<nOnTrackSystem<<" Off track:"<<nOffTrackSystem);
  if ((nOnTrackSystem+nOffTrackSystem)>0) {
    if (double(nOnTrackSystem)/(nOnTrackSystem+nOffTrackSystem) >= criticalRatio_) {
      log_debug("Met ratio-requirement");
      return true;
    }
    else {
      log_debug("Could not meet ratio-requirement");
      return false;
    }
  }
  else {
    log_warn("Pulses are probably empty!");
    return false;
  }
};
