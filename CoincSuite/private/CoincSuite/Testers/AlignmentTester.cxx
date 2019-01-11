/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file AlignmentTester.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#ifndef ALIGNMENTTEST_H
#define ALIGNMENTTEST_H

#include "CoincSuite/Testers/TesterModule.h"

/** A I3Module if a track does align to a hypothetical track in angle and distance
 */
class AlignmentTester : public TesterModule {
  SET_LOGGER("AlignmentTester");
private:
  /// PARAM: Name of the reconstructed (I3Particle) Fit
  std::string recoFitName_;
  /// PARAM: Name of the hypthetical (I3Particle) Fit
  std::string hypoFitName_;
  /// PARAM: maximal angle deviations
  double criticalAngle_;
  /// PARAM: maximal distance
  double criticalDistance_;
  /// PARAM: Test Mutually instead of Hypothetically
  bool mutualOpt_;
private:
  /// Implement the evaluation for hypo-testing
  Result HypoTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  /// Implement the evaluation for mutual-testing
  Result MutualTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  //interface for Evaluate
  ///call-back function
  static Result runHypoTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    AlignmentTester *alPtr = (AlignmentTester*) tester;
    return alPtr->HypoTest(hypoframe, frameA, frameB);
  };
  ///call-back function
  static Result runMutualTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    AlignmentTester *alPtr = (AlignmentTester*) tester;
    return alPtr->MutualTest(hypoframe, frameA, frameB);
  };
  
public:
  /// Constructor
  AlignmentTester (const I3Context& context);
  /// std Configure function
  void Configure();
  
private:
  /** Test if fitA and fitB do align in angle and distance
   * @param fit_A the first (I3Particle)Fit to compare
   * @param fit_B the second (I3Particle)Fit to compare
   * @return true, if so
   */
  bool AlignmentCriterium (const I3Particle& fit_A,
                           const I3Particle& fit_B) const;
};

I3_MODULE(AlignmentTester);

#endif // ALIGNMENTTEST_H


//============================IMPLEMENTATIONS===================================
#include "phys-services/I3Calculator.h"
#include "CoincSuite/lib/I3FakeCalculator.h"

//______________________________________________________________________________
AlignmentTester::AlignmentTester (const I3Context& context):
  TesterModule(context),
  recoFitName_(""),
  hypoFitName_(""),
  criticalAngle_(25*I3Units::degree),
  criticalDistance_(20*I3Units::meter),
  mutualOpt_(false)
{
  AddParameter("RecoFitName", "Name of the <I3Particle> Fit", recoFitName_);
  AddParameter("HypoFitName", "Name of the <I3Particle> Fit in the HypoFrame to test against", hypoFitName_);
  AddParameter("CriticalAngle", "A critical angle between two reconstructions in [rad]", criticalAngle_);
  AddParameter("CriticalDistance", "A critical Distance between tracks in [m]", criticalDistance_);
  AddParameter("MutualCompare", "Hypo(false) -or- Mutual(true)", mutualOpt_);
};

//______________________________________________________________________________
void AlignmentTester::Configure() {
  TesterModule::Configure();
  GetParameter("RecoFitName", recoFitName_);
  GetParameter("HypoFitName", hypoFitName_);
  GetParameter("CriticalAngle", criticalAngle_);
  GetParameter("CriticalDistance", criticalDistance_);
  GetParameter("MutualCompare", mutualOpt_);
  
  if (recoFitName_=="")
    log_fatal("Configure 'RecoFitName'");
  if (hypoFitName_=="")
    log_fatal("Configure 'HypoFitName'");

  if (!mutualOpt_)
    Evaluate = runHypoTest;
  else
    Evaluate = runMutualTest;
};

//______________________________________________________________________________
AlignmentTester::Result AlignmentTester::HypoTest(I3FrameConstPtr hypoframe,
                                                  I3FrameConstPtr frameA,
                                                  I3FrameConstPtr frameB) const 
{
  //now the action can start: buffer up necessary frame objects
  // get all HypoFrame objects

  I3ParticleConstPtr hypoFit = hypoframe->Get<I3ParticleConstPtr>(hypoFitName_);
  if (not hypoFit) {
    log_error("Could not find the HypoFit <I3Particle>('%s') in the Hypoframe; "
              "will continue with next HypoFrame", hypoFitName_.c_str());
    return UNDECIDED;
  }
  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_warn("Fit Status not OK; will continue with next HypoFrame");
    return UNDECIDED; // to loop over all Hypoframes
  }

  I3ParticleConstPtr recoFitA = frameA->Get<I3ParticleConstPtr>(recoFitName_);
  I3ParticleConstPtr recoFitB = frameB->Get<I3ParticleConstPtr>(recoFitName_);
  if (not recoFitA || not recoFitB) {
    log_error("Could not find the RecoFit <I3Particle>('%s') in the SplitFrame;"
              " will continue with next HypoFrame", recoFitName_.c_str());
    return UNDECIDED; // to loop over all Hypoframes
  }

  log_debug("all necessary objects are found in the frames to begin hypothesis testing");

  bool liketorecombine = false;
  bool vetotorecombine = false;

  // test recoFitA against Pulses B
  if (recoFitA->GetFitStatus() != I3Particle::OK) {
    log_debug("FitA Status not good; cannot test");
  } else {
    const bool criterium = AlignmentCriterium(*recoFitA, *hypoFit);
    liketorecombine = liketorecombine || criterium;
    vetotorecombine = vetotorecombine || !criterium;
  }

  // test recoFitB against Pulses A
  if (recoFitB->GetFitStatus() != I3Particle::OK) {
    log_debug("FitB Status not good; cannot test");
  } else {
    const bool criterium = AlignmentCriterium(*recoFitB, *hypoFit);
    liketorecombine = liketorecombine || criterium;
    vetotorecombine = vetotorecombine || !criterium;
  }

  return (Result)(liketorecombine && ! (vetotorecombine));
}

//______________________________________________________________________________
AlignmentTester::Result AlignmentTester::MutualTest(I3FrameConstPtr hypoframe,
                                                    I3FrameConstPtr frameA,
                                                    I3FrameConstPtr frameB) const{
  //now the action can start: buffer up necessary frame objects

  I3ParticleConstPtr recoFitA = frameA->Get<I3ParticleConstPtr>(recoFitName_);
  I3ParticleConstPtr recoFitB = frameB->Get<I3ParticleConstPtr>(recoFitName_);
  if (not recoFitA || not recoFitB) {
    log_error("Could not find the RecoFits <I3RecoPulseSeriesMap>('%s') in the SplitFrame;"
              "will continue with next HypoFrame", recoFitName_.c_str());
    return UNDECIDED; // to loop over all HypoFrames
  }
  
  log_debug("all necessary objects are found in the frames to begin hypothesis testing");

  if (recoFitA->GetFitStatus() != I3Particle::OK || recoFitB->GetFitStatus() != I3Particle::OK ) {
    log_debug("Fit Status not good; can not test");
    return UNDECIDED;
  }
  
  return (Result)(AlignmentCriterium(*recoFitA, *recoFitB));
}

bool AlignmentTester::AlignmentCriterium (const I3Particle& fit_A,
                                          const I3Particle& fit_B) const
{
  log_debug("Entering AlignmentCriterium()");
  
  if (fit_A.GetFitStatus()== I3Particle::OK && fit_B.GetFitStatus()== I3Particle::OK) {
    log_debug("Evaluate Fit");
    const double angle = I3Calculator::Angle(fit_A, fit_B);
    const double distance = I3FakeCalculator::TrackDistance(fit_A, fit_B);
    log_trace_stream("Angle: "<<angle<<" rad ["<<(angle*360./3.1416)<<" deg];  Distance: "<<distance<<" m;");
    if ((angle <= criticalAngle_) && (distance <= criticalDistance_)) {
      log_debug("Fits do align");
      return true;
    }
    else {
      log_debug("Fits do NOT align");
      return false;
    }
  }
  else{
    log_error("Function AlignmentCriterium called with a not OK fit");
    return false;
  }
};
