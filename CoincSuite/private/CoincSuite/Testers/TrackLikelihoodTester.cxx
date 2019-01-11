/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file TrackLikelihoodTester.cxx
 * @version $Revision: 1.00$
 * @author sflis <samuel.flis@fysik.su.se>, mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Sept 2013
 */

#ifndef TRACKLIKELIHOODTEST_H
#define TRACKLIKELIHOODTEST_H

#include "CoincSuite/Testers/TesterModule.h"

#include "gulliver/I3EventLogLikelihoodBase.h"

/** A testing module that tests the logLikelihood of the track hypothesis of either SplitFrame
 * with the combined pulses in the HypoFrame.
 * If the reduced logLikelihood value is below a set threshold the two fractures are recombined.
 */
class TrackLikelihoodTester : public TesterModule {
  SET_LOGGER("TrackLikelihoodTester");
private: // parameters
  /// PARAM: Name of event loglikelihood calculation service.
  std::string llhServiceName_;
  /// PARAM: fit name
  std::string fitName_;
  /// PARAM: Threshold value on the logLikelihood for which frames should be recombined
  double llhThreshold_;
  /// PARAM: Threshold value on the average logLikelihood for which frames should be recombined
  double averageLlhThreshold_;
  /// PARAM: Test Mutually instead of Hypothetically
  bool mutualOpt_;
  
private: // properties
  /// log-likelihood service
  I3EventLogLikelihoodBasePtr llhService_;
  
private: //algorithm
  /// Implement the Evaluation for hypo-testing
  Result HypoTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  /// Implement the evaluation for mutual-testing
  Result MutualTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  ///call-back function
  static Result runHypoTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    TrackLikelihoodTester *alPtr = (TrackLikelihoodTester*) tester;
    return alPtr->HypoTest(hypoframe, frameA, frameB);
  };
  ///call-back function
  static Result runMutualTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    TrackLikelihoodTester *alPtr = (TrackLikelihoodTester*) tester;
    return alPtr->MutualTest(hypoframe, frameA, frameB);
  };

public:
  ///constructor
  TrackLikelihoodTester (const I3Context& context);
  /// std Configure function
  void Configure();
  ///need to reimplement to set the geometry for llhService
  void Geometry (I3FramePtr frame);

private:
  /** @brief calculate the LogLikelihood value of a track fitting to the event outlined by objects in a frame
   * @param frame pointer to the frame that holds the pulses (besides other objects, which are checked in silence)
   * @param particle pointer to the Fit that should be checked
   * @return the LLh-value
   */
  double CalculateLogLikelihood (I3FrameConstPtr frame, const I3Particle &particle) const;
};

I3_MODULE(TrackLikelihoodTester);

#endif // TRACKLIKELIHOODTEST_H


//======================== IMPLEMENTATIONS =====================================

#include "gulliver/I3Gulliver.h"
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/utilities/ordinal.h"

//______________________________________________________________________________
TrackLikelihoodTester::TrackLikelihoodTester (const I3Context& context):
  TesterModule(context),
  llhThreshold_(5.0),
  averageLlhThreshold_(10.0),
  mutualOpt_(false)
{
  AddParameter( "LlhServiceName",
                "Name of the LogLikelihood service to use",
                llhServiceName_ );
  AddParameter( "LlhThreshold",
                "Threshold value on the logLikelihood for which frames should be recombined",
                llhThreshold_ );
  AddParameter( "AverageLlhThreshold",
                "Threshold value on the average logLikelihood for which frames should be recombined",
                averageLlhThreshold_ );
  AddParameter( "FitName",
                "Name of a I3Particle object for which you'd like to know the likelihood.",
                fitName_ );
  AddParameter("MutualCompare",
               "Hypo(false) -or- Mutual(true)",
               mutualOpt_);
  
  log_warn("THIS MODULE IS DEPRECATED; use the nicer and better alternative 'ReducingLikelihoodTester'!");
};

//______________________________________________________________________________
void TrackLikelihoodTester::Configure() {
  TesterModule::Configure();

  GetParameter("LlhServiceName", llhServiceName_ );
  GetParameter("LlhThreshold", llhThreshold_ );
  GetParameter("AverageLlhThreshold", averageLlhThreshold_ );
  GetParameter("FitName", fitName_ );
  GetParameter("MutualCompare", mutualOpt_);

  // never trust the user
  if ( fitName_.empty() )
    log_fatal("Specify the parameter 'FitName'");
  if ( llhServiceName_.empty() )
    log_fatal("You did not specify a llh-service, use the parameter 'LlhServiceName'");
  if ( averageLlhThreshold_<0 )
    log_fatal("Only positive values allowed for parameter 'LlhThreshold'");
  if ( llhThreshold_<0 )
    log_fatal("Only positive values allowed for parameter 'AverageLlhThreshold'");
  llhService_ = context_.Get< I3EventLogLikelihoodBasePtr >( llhServiceName_ );
  if (! llhService_ )
    log_fatal("Problem with getting llh-service <I3EventLogLikelihoodBase>('%s') from the context", llhServiceName_.c_str() );


  if ( mutualOpt_)
    Evaluate = runMutualTest;
  else
    Evaluate = runHypoTest;
};

//______________________________________________________________________________
void TrackLikelihoodTester::Geometry (I3FramePtr frame) {
  TesterModule::Geometry (frame); //there is a frame-push
  llhService_->SetGeometry(*geometry_);
};

//______________________________________________________________________________
TrackLikelihoodTester::Result TrackLikelihoodTester::HypoTest(I3FrameConstPtr hypoframe,
                                                              I3FrameConstPtr frameA,
                                                              I3FrameConstPtr frameB) const
{
  log_debug("Entering HypoTest()");

  // get all splitframe objects needed
  I3ParticleConstPtr hypoFit = hypoframe->Get<I3ParticleConstPtr>(fitName_ );

  if (! hypoFit) {
    log_error("Could not find the Fit <I3Particle>('%s') in the HypoFrame; will continue with next hypoframe", fitName_.c_str());
    return UNDECIDED;
  }

  double llhA = NAN; //LLh of pulsesA included in TrackB
  double llhB = NAN; //LLh of pulsesB included in TrackA

  // test recoFitA against Pulses B
  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_debug("HypoFit Status not good");
    return UNDECIDED;
  } else {
    llhA = CalculateLogLikelihood(frameA, *hypoFit);
    llhB= CalculateLogLikelihood(frameB, *hypoFit);
  }
 
  const double avg_llh = (llhA+llhB)/2;
  log_trace_stream("llhA= "<<llhA<<"; llhB= "<<llhB);
  //NOTE likelihood returns NAN if it does not exist in a meaningful way
  
  return (Result)(llhA < llhThreshold_ || llhB < llhThreshold_ || avg_llh < averageLlhThreshold_);
}

//______________________________________________________________________________
TrackLikelihoodTester::Result TrackLikelihoodTester::MutualTest(I3FrameConstPtr hypoframe,
                                                                I3FrameConstPtr frameA,
                                                                I3FrameConstPtr frameB) const
{
  log_debug("Entering MutualTest()");

  // get all splitframe objects needed
  I3ParticleConstPtr recoFitA = frameA->Get<I3ParticleConstPtr>(fitName_ );
  I3ParticleConstPtr recoFitB = frameB->Get<I3ParticleConstPtr>(fitName_ );

  if (! recoFitA) {
    log_error("Could not find the Fit <I3Particle>('%s') in frame A; will continue with next hypoframe", fitName_.c_str());
    return UNDECIDED;
  }

  if (! recoFitB) {
    log_error("Could not find the Fit <I3Particle>'%s' in frame B; will continue with next hypoframe", fitName_.c_str());
    return UNDECIDED;
  }

  double llhAB = NAN; //LLh of pulsesA included in TrackB
  double llhBA = NAN; //LLh of pulsesB included in TrackA

  // test recoFitA against Pulses B
  if (recoFitA->GetFitStatus() != I3Particle::OK) {
    log_debug("FitA Status not good");
  } else {
    llhBA = CalculateLogLikelihood(frameB, *recoFitA);
  }
  // test recoFitB against Pulses A
  if (recoFitB->GetFitStatus() != I3Particle::OK) {
    log_debug("FitB Status not good");
  } else {
    llhAB= CalculateLogLikelihood(frameA, *recoFitB);
  }
  const double avg_llh = (llhAB+llhBA)/2.;
  log_trace_stream("llhBA= "<<llhBA<<"; llhAB= "<<llhAB);
  //NOTE likelihood returns NAN if it does not exist in a meaningful way
  
  return (Result)(llhAB < llhThreshold_ || llhBA < llhThreshold_ || avg_llh < averageLlhThreshold_);
}

//_____________________________________________________________________________
double TrackLikelihoodTester::CalculateLogLikelihood(I3FrameConstPtr frame,
                                                     const I3Particle &particle) const
{
  log_debug("Entering CalculateLogLikelihood");
  I3EventHypothesis hypothesis( particle );
  llhService_->SetEvent(*frame);
  const int multiplicity = llhService_->GetMultiplicity();
  const double llh = llhService_->GetLogLikelihood(hypothesis);
  return llh/multiplicity;
}
