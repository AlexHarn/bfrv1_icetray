/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file CylinderPulsesTester.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#ifndef CYLINDERPULSESTEST_H
#define CYLINDERPULSESTEST_H

#include "CoincSuite/Testers/TesterModule.h"

/** A I3Module testing a hypothesis that a fraction of the pulses are contained around any hypothetical track
 * Input are: A track \<I3Particle\> and a PulseSeries \<I3RecoPulseSeriesMap(Mask)\>
 * Requirements: The Fit Status needs to be Okay,
 * This Module has a 'Hypo' and a 'Mutual' mode of operation
 */
class CylinderPulsesTester : public TesterModule {
  SET_LOGGER("CylinderPulsesTester");
private: // parameters
  /// Param: Name of the RecoPulseSeriesMap
  std::string recoMapName_;
  /// Param: Name of the Fit I should test for
  std::string hypoFitName_;
  /// Param: Name of the (I3Particle) Fit
  std::string recoFitName_;
  /// Param: Radius around the HypoFit within the Pulses should be found
  double cylinderRadius_;
  /// Param: Ratio above which we accept hypophesis
  double criticalRatio_;
  /// PARAM: Test Mutually instead of Hyothetically
  bool mutualOpt_;
  
private: // algorithm
  /// Implement the evaluation for hypo-testing
  Result HypoTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  /// Implement the evaluation for mutual-testing
  Result MutualTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  ///call-back function
  static Result runHypoTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    CylinderPulsesTester*alPtr = (CylinderPulsesTester*) tester;
    return alPtr->HypoTest(hypoframe, frameA, frameB);
  };
  ///call-back function
  static Result runMutualTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    CylinderPulsesTester*alPtr = (CylinderPulsesTester*) tester;
    return alPtr->MutualTest(hypoframe, frameA, frameB);
  };

public:// icetray interaction
  /// Constructor
  CylinderPulsesTester (const I3Context& context);
  /// Configure function
  void Configure();

private:
  /** Tests if the Pulses are found in a cylinder around the recoFit
  * @param recoFitA test around this track
  * @param pulsesB test this pulses
  * @return true, if so
  */
  bool CylinderPulsesCritera (const I3Particle& recoFitA,
                              const I3RecoPulseSeriesMap& pulsesB) const;
};

I3_MODULE(CylinderPulsesTester);

#endif // CYLINDERPULSESTEST_H


//================================== IMPLEMENTATIONS ===========================
#include "phys-services/I3Calculator.h"

//______________________________________________________________________________
CylinderPulsesTester::CylinderPulsesTester (const I3Context& context):
  TesterModule(context),
  recoMapName_("MaskedOfflinePulses"),
  hypoFitName_(""),
  recoFitName_(""),
  cylinderRadius_(50*I3Units::m),
  criticalRatio_(0.9),
  mutualOpt_(false)
{
  log_warn_stream("================================================================"<<std::endl
                <<" module 'CylinderPulsesTester' is DEPRICATED:"<<std::endl
                <<" use identical behaving module 'TrackSytemTest' instead,"<<std::endl
                <<"    with parameter ResTimeWindow=[-INF, INF]"<<std::endl
                <<"================================================================");
  AddParameter("RecoMapName", "Name of the <I3RecoPulseSeriesMap(Mask)>", recoMapName_);
  AddParameter("HypoFitName", "Name of the <I3Particle>Fit in the HypoFrame to test against", hypoFitName_);
  AddParameter("RecoFitName", "Name of the <I3Particle>Fit in the SplitFrames", recoFitName_);
  AddParameter("CylinderRadius", "Radius of cylinder in which hits have to included", cylinderRadius_);
  AddParameter("CriticalRatio", "Ratio of Pulses above which recombination is preferred", criticalRatio_);
  AddParameter("MutualCompare", "Hypo(false) -or- Mutual(true)", mutualOpt_);
};

//______________________________________________________________________________
void CylinderPulsesTester::Configure() {
  TesterModule::Configure();
  GetParameter("RecoMapName", recoMapName_);
  GetParameter("HypoFitName", hypoFitName_);
  GetParameter("RecoFitName", recoFitName_);
  GetParameter("CylinderRadius", cylinderRadius_);
  GetParameter("CriticalRatio", criticalRatio_);
  GetParameter("MutualCompare", mutualOpt_);

  if ( !mutualOpt_ && hypoFitName_=="")
    log_fatal("Configure 'HypoFitName'");
  if ( mutualOpt_ && recoFitName_=="")
    log_fatal("Configure 'RecoFitName'");

  if (!mutualOpt_)
    Evaluate = runHypoTest;
  else
    Evaluate = runMutualTest;
};

//______________________________________________________________________________
CylinderPulsesTester::Result CylinderPulsesTester::HypoTest(I3FrameConstPtr hypoframe,
                                                            I3FrameConstPtr frameA,
                                                            I3FrameConstPtr frameB) const
{
  log_debug("Entering CylinderPulsesTester::HypoTest");
  //now the action can start
  I3ParticleConstPtr hypoFit = hypoframe->Get<I3ParticleConstPtr>(hypoFitName_);
  if (not hypoFit) {
    log_error("Could not find the HypoFit <I3Particle>('%s') in the HypoFrame;"
              "will continue with next HypoFrame", hypoFitName_.c_str());
    return UNDECIDED;
  }
  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_debug("Fit Status not good; will continue with next HypoFrame");
    return UNDECIDED;
  }

  I3RecoPulseSeriesMapConstPtr PulsesA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  I3RecoPulseSeriesMapConstPtr PulsesB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

  if (not PulsesA || not PulsesB) {
    log_fatal("Could not find the RecoMap <I3RecoPulseSeriesMap>('%s') in the SplitFrames;", recoMapName_.c_str());
  }
  // All needed objects have been found
  log_debug("all necessary objects are found in the frames to begin hypothesis testing");
  
  bool liketorecombine = false;
  bool vetotorecombine = false;

  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_debug("Fit Status not good");
    vetotorecombine = true;
  } else {
    log_debug("test hypoFit against Pulses A");
    bool criteria = CylinderPulsesCritera (*hypoFit, *PulsesA);
    liketorecombine = liketorecombine || criteria;
    vetotorecombine = vetotorecombine || !criteria;
    log_debug("test hypoFit against Pulses B");
    criteria = CylinderPulsesCritera (*hypoFit, *PulsesB);
    liketorecombine = liketorecombine || criteria;
    vetotorecombine = vetotorecombine || !criteria;
  }

  return (Result)(liketorecombine && !(vetotorecombine));
}

//______________________________________________________________________________
CylinderPulsesTester::Result CylinderPulsesTester::MutualTest(I3FrameConstPtr hypoframe,
                                                              I3FrameConstPtr frameA,
                                                              I3FrameConstPtr frameB) const
{
  log_debug("Entering CylinderPulsesTester::MutualTest");
  // get all splitframe objects needed
  I3RecoPulseSeriesMapConstPtr PulsesA = frameA->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);
  I3RecoPulseSeriesMapConstPtr PulsesB = frameB->Get<I3RecoPulseSeriesMapConstPtr>(recoMapName_);

  if (not PulsesA || not PulsesB) {
    log_fatal("Cannot find the RecoMap <I3RecoPulseSeriesMap>('%s') in the SplitFrames;", recoMapName_.c_str());
  }

  I3ParticleConstPtr recoFitA = frameA->Get<I3ParticleConstPtr>(recoFitName_);
  I3ParticleConstPtr recoFitB = frameB->Get<I3ParticleConstPtr>(recoFitName_);
  if (not recoFitA || not recoFitB) {
    log_error("Cannot find the RecoFits <I3Particle>('%s') in the SplitFrames;"
              "will continue with next HypoFrame", recoFitName_.c_str());
    return UNDECIDED; // to loop over all HypoFrames
  }

  log_debug("All necessary objects are found; begin hypothesis testing");

  bool liketorecombine = false; // should frames be recombined according to this decision?
  bool vetotorecombine = false; // are there objections to recombine?

  if (recoFitA->GetFitStatus() != I3Particle::OK) {
    log_debug("FitA Status not good");
  } else {
    log_debug("test recoFitA against Pulses B");
    liketorecombine = liketorecombine || CylinderPulsesCritera(*recoFitA, *PulsesB);
  }
  if (recoFitB->GetFitStatus() != I3Particle::OK) {
    log_debug("FitB Status not good");
  } else {
    log_debug("test recoFitB against Pulses A");
    liketorecombine = liketorecombine || CylinderPulsesCritera(*recoFitB, *PulsesA);
  }

  return Bool2Result(liketorecombine && !(vetotorecombine));
}

//______________________________________________________________________________
bool CylinderPulsesTester::CylinderPulsesCritera (const I3Particle& recoFit,
                                                  const I3RecoPulseSeriesMap& pulses) const {
  log_debug("Entering CylinderPulsesCriteria()");
  //now loop OMKeys Pulses A; and try cylinder containment criteria
  uint nOnTrackPulses=0; //counter
  uint nOffTrackPulses=0; //counter
  for (I3RecoPulseSeriesMap::const_iterator reco_iter = pulses.begin(); reco_iter != pulses.end(); ++reco_iter) {
    const OMKey om = reco_iter->first;
    const I3Position ompos = (geometry_->omgeo.find(om)->second).position;

    if (I3Calculator::IsOnTrack(recoFit, ompos, cylinderRadius_))
      nOnTrackPulses+=1;
    else
      nOffTrackPulses+=1;
  }

  log_trace_stream("nOnTrackPulses"<<nOnTrackPulses<<" nOffTrackPulses"<<nOffTrackPulses);
  //Probe the multiplicity requirement
  if ((nOnTrackPulses+nOffTrackPulses)>0) {
    if (float(nOnTrackPulses)/(nOnTrackPulses+nOffTrackPulses) >= criticalRatio_) {
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
