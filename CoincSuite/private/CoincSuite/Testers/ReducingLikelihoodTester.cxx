/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file ReducingLikelihoodTester.cxx
 * @version $Revision: 1.00$
 * @author sflis <samuel.flis@fysik.su.se>, mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Sept 2013
 */

#ifndef REDUCINGLIHOODTEST_H
#define REDUCINGLIHOODTEST_H

#include "CoincSuite/Testers/TesterModule.h"

#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3ParametrizationBase.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3EventHypothesis.h"

/** A Tester Module that probes if the Likelihood of a Fit is reduced when it is calculated on the
 * provisionary recombined Pulses.
 */
class ReducingLikelihoodTester : public TesterModule {
  SET_LOGGER("ReducingLikelihoodTester");
private: // parameters
  /// PARAM: name of the llhService
  std::string llhServiceName_;
  /// PARAM: name of the miniService
  std::string miniServiceName_;
  /// PARAM: name of the paraService
  std::string paraServiceName_;
    /// PARAM: Name of the reconstructed (I3Particle) Fit
  std::string recoFitName_;
  /// PARAM: Name of the hypthetical (I3Particle) Fit
  std::string hypoFitName_;
  /// PARAM: the factor by which the likelihood has to be reduced by the recombination
  double redFactor_;
  /// PARAM: Refit the hypothesis first and take the LLH from the new fit
  bool refit_;
  /// PARAM: Test Mutually instead of Hypothetically
  /// NOTE there is a difference of the meaning of this option in this module when compared to other Testers
  bool mutualOpt_;

private: // properties
  /// The llh service from context
  I3EventLogLikelihoodBasePtr llhService_;
  /// The minimizer service from context
  I3MinimizerBasePtr miniService_;
  /// The parametrization service from context
  I3ParametrizationBasePtr paraService_;

  /// The gulliver service: the work-horse
  I3GulliverPtr fitterCore_;
  
private: //algorithm
  /// Implement the evaluation for hypo-testing
  ///
  Result HypoTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  /// Implement the evaluation for mutual-testing
  Result MutualTest(I3FrameConstPtr, I3FrameConstPtr, I3FrameConstPtr) const;
  //interface for Evaluate
  ///call-back function
  static Result runHypoTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    ReducingLikelihoodTester *alPtr = (ReducingLikelihoodTester*) tester;
    return alPtr->HypoTest(hypoframe, frameA, frameB);
  };
  ///call-back function
  static Result runMutualTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    ReducingLikelihoodTester *alPtr = (ReducingLikelihoodTester*) tester;
    return alPtr->MutualTest(hypoframe, frameA, frameB);
  };

public:
  ///constructor
  ReducingLikelihoodTester (const I3Context& context);
  /// std Configure function
  void Configure();
  ///need to reimplement to set the geometry for llhService
  void Geometry (I3FramePtr frame);

private:
  /** @brief fit and calculate the LogLikelihood value of a track fitting to the event outlined by objects in a frame
   * @param frame pointer to the frame that holds the pulses (besides other objects, which are checked in silence)
   * @param particle pointer to the Fit that should be checked
   * @return the LLh-value
   */
  double FitCalcLogLikelihood (I3FrameConstPtr frame, const I3Particle &particle) const;
  
  /** @brief Just calculate the LogLikelihood value of a track fitting to the event outlined by objects in a frame
   * @param frame pointer to the frame that holds the pulses (besides other objects, which are checked in silence)
   * @param particle pointer to the Fit that should be checked
   * @return the LLh-value
   */
  double CalculateLogLikelihood(I3FrameConstPtr frame, const I3Particle &particle) const;
};

I3_MODULE(ReducingLikelihoodTester);

#endif // REDUCINGLIHOODTEST_H


//======================== IMPLEMENTATIONS =====================================

#include "gulliver/I3Gulliver.h"
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/utilities/ordinal.h"

//______________________________________________________________________________
ReducingLikelihoodTester::ReducingLikelihoodTester (const I3Context& context):
  TesterModule(context),
  llhServiceName_(),
  miniServiceName_(),
  paraServiceName_(),
  recoFitName_(),
  hypoFitName_(),
  redFactor_(0.9),
  refit_(false),
  mutualOpt_(false)
{
  AddParameter("RecoFitName", "Name of the <I3Particle> Fit", recoFitName_);
  AddParameter("HypoFitName", "Name of the <I3Particle> Fit in the HypoFrame to test against", hypoFitName_);
  AddParameter("LlhName", "LogLikelihood service to use", llhServiceName_);
  AddParameter("MiniName", "Minimizer service to use", miniServiceName_);
  AddParameter("ParaName", "Parametrization service to use", paraServiceName_);
  AddParameter("ReductionFactor", "Factor by which the likelihood has to be reduced by the recombination", redFactor_);
  AddParameter("Refit", "Refit the track before calculating the LLH", refit_);
  AddParameter("MutualCompare",
               "Hypo(false) -or- Mutual(true): Hypo compares recombined Pulses with the HypoFit VS individual Pulses with the HypoFit, "
               "Mutual compares recombined Pulses with the HypoFit VS individual Pulses with the RecoFit",
               mutualOpt_);
};

//______________________________________________________________________________
void ReducingLikelihoodTester::Configure() {
  TesterModule::Configure();
  
  GetParameter("RecoFitName", recoFitName_);
  GetParameter("HypoFitName", hypoFitName_);
  GetParameter("LlhName", llhServiceName_);
  GetParameter("MiniName", miniServiceName_);
  GetParameter("ParaName", paraServiceName_);
  GetParameter("ReductionFactor", redFactor_ );
  GetParameter("Refit", refit_);
  GetParameter("MutualCompare", mutualOpt_);

  // never trust the user
  if (recoFitName_=="")
    log_fatal("Configure 'RecoFitName'");
  if (hypoFitName_=="")
    log_fatal("Configure 'HypoFitName'");
  if ( 0.> redFactor_  || redFactor_>1. )
    log_fatal("Only positive values [0,1) allowed for parameter 'ReducingFactor'");

  if ( llhServiceName_.empty() )
    log_fatal( "You did not specify a llh service, use the 'LlhName' option");
  if ( miniServiceName_.empty() )
    log_fatal( "You did not specify a minimizer service, use the 'MiniName' option");
  if ( paraServiceName_.empty() )
    log_fatal( "You did not specify a parametrization service, use the 'ParaName' option");
  
  if ( ! context_.Has<I3EventLogLikelihoodBase>( llhServiceName_ ) )
    log_fatal( "Problem with getting llh service \"%s\"", llhServiceName_.c_str() );
  if ( ! context_.Has<I3MinimizerBase>( miniServiceName_ ) )
    log_fatal( "Problem with getting mini service \"%s\"", miniServiceName_.c_str() );
  if ( ! context_.Has<I3ParametrizationBase>( paraServiceName_ ) )
    log_fatal( "Problem with getting parametrization service \"%s\"", paraServiceName_.c_str() );

  //instantise the needed services and the fitter
  llhService_ = context_.Get< I3EventLogLikelihoodBasePtr >( llhServiceName_ );
  miniService_ = context_.Get< I3MinimizerBasePtr >( miniServiceName_ );
  paraService_ = context_.Get< I3ParametrizationBasePtr >( paraServiceName_ );
  fitterCore_ = I3GulliverPtr(new I3Gulliver(paraService_, llhService_, miniService_, GetName()));
  
  if (!mutualOpt_)
    Evaluate = runHypoTest;
  else
    Evaluate = runMutualTest;
};

//______________________________________________________________________________
void ReducingLikelihoodTester::Geometry (I3FramePtr frame) {
  TesterModule::Geometry (frame); //there is a frame-push
  llhService_->SetGeometry(*geometry_);
};

//______________________________________________________________________________
ReducingLikelihoodTester::Result ReducingLikelihoodTester::MutualTest(I3FrameConstPtr hypoframe,
                                                                      I3FrameConstPtr frameA,
                                                                      I3FrameConstPtr frameB) const
{
  log_debug("Entering MutualTest()");

  I3ParticleConstPtr recoFitA = frameA->Get<I3ParticleConstPtr>(recoFitName_ );
  I3ParticleConstPtr recoFitB = frameB->Get<I3ParticleConstPtr>(recoFitName_ );

  if (! recoFitA) {
    log_error("Could not find the Fit <I3Particle>('%s') in frame A; will continue with next hypoframe", recoFitName_.c_str());
    return UNDECIDED;
  }

  if (! recoFitB) {
    log_error("Could not find the Fit <I3Particle>('%s') in frame B; will continue with next hypoframe", recoFitName_.c_str());
    return UNDECIDED;
  }

  if ((recoFitA->GetFitStatus() != I3Particle::OK) && (recoFitB->GetFitStatus() != I3Particle::OK)) {
    log_error("Both fit's FitStatus not OK");
    return UNDECIDED;
  }
  
  double llhA_native = NAN;
  double llhB_native = NAN;
  double llhA_hypo = NAN;
  double llhB_hypo = NAN;

  // test recoFitA against Pulses B
  if (recoFitA->GetFitStatus() != I3Particle::OK) {
    log_debug("FitA Status not good");
  } else {
      llhA_native = ( refit_ ? FitCalcLogLikelihood(frameA, *recoFitA) : (CalculateLogLikelihood(frameA, *recoFitA)));
      llhA_hypo = ( refit_ ? FitCalcLogLikelihood(hypoframe, *recoFitA) : (CalculateLogLikelihood(hypoframe, *recoFitA)));
  }
  // test recoFitB against Pulses A
  if (recoFitB->GetFitStatus() != I3Particle::OK) {
    log_debug("FitB Status not good");
  } else {
    llhB_native = ( refit_ ? FitCalcLogLikelihood(frameB, *recoFitB) : (CalculateLogLikelihood(frameB, *recoFitB)));
    llhB_hypo = ( refit_ ? FitCalcLogLikelihood(hypoframe, *recoFitB) : (CalculateLogLikelihood(hypoframe, *recoFitB)));
  }
 
  if ((std::isnan(llhA_native) || std::isnan(llhA_hypo)) && (std::isnan(llhB_native) || std::isnan(llhB_hypo))) {
    log_debug("Both fit procedures did fail");
    return UNDECIDED;
  }
 
  const bool A_reduced = (llhA_hypo<= redFactor_*llhA_native);
  const bool B_reduced = (llhB_hypo<= redFactor_*llhB_native);
  
  return (Result)(A_reduced || B_reduced);
}


//______________________________________________________________________________
ReducingLikelihoodTester::Result ReducingLikelihoodTester::HypoTest(I3FrameConstPtr hypoframe,
                                                                    I3FrameConstPtr frameA,
                                                                    I3FrameConstPtr frameB) const
{
  log_debug("Entering HypoTest()");

  I3ParticleConstPtr hypoFit = hypoframe->Get<I3ParticleConstPtr>(hypoFitName_ );

  if (! hypoFit) {
    log_error("Could not find the Fit <I3Particle>('%s') in frame A; will continue with next hypoframe", hypoFitName_.c_str());
    return UNDECIDED;
  }

  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_debug("HypoFit FitStatus not OK");
    return UNDECIDED;
  }
  
  double llhA_hypo = NAN;
  double llhB_hypo = NAN;
  double llhH_hypo = NAN;

  llhA_hypo = ( refit_ ? FitCalcLogLikelihood(frameA, *hypoFit) : (CalculateLogLikelihood(frameA, *hypoFit)));
  llhB_hypo = ( refit_ ? FitCalcLogLikelihood(frameB, *hypoFit) : (CalculateLogLikelihood(frameB, *hypoFit)));
  llhH_hypo = ( refit_ ? FitCalcLogLikelihood(hypoframe, *hypoFit) : (CalculateLogLikelihood(hypoframe, *hypoFit)));
 
  if (std::isnan(llhA_hypo) || std::isnan(llhB_hypo) || std::isnan(llhH_hypo)) {
    log_debug("Some fit procedures did fail! testing can not commence");
    return UNDECIDED;
  }
 
  const bool A_reduced = (llhH_hypo<= redFactor_*llhA_hypo);
  const bool B_reduced = (llhH_hypo<= redFactor_*llhB_hypo);
  
  return (Result)(A_reduced && B_reduced);
}

//_____________________________________________________________________________
double ReducingLikelihoodTester::FitCalcLogLikelihood(I3FrameConstPtr frame,
                                                      const I3Particle &particle) const
{
  log_debug("Entering CalculateLogLikelihood");
  I3EventHypothesis hypothesis( particle );
  I3LogLikelihoodFitPtr fitptr( new I3LogLikelihoodFit(hypothesis) ); // <-- here the new fit will end up

  fitterCore_->Fit(*frame, fitptr ); // <-- here the fit occures
  
  return fitptr->fitparams_->rlogl_;
}

double ReducingLikelihoodTester::CalculateLogLikelihood(I3FrameConstPtr frame,
                                                        const I3Particle &particle) const
{
  log_debug("Entering CalculateLogLikelihood");
  I3EventHypothesis hypothesis( particle );
  llhService_->SetEvent(*frame);
  const int multiplicity = llhService_->GetMultiplicity();
  const double llh = llhService_->GetLogLikelihood(hypothesis);
  return llh/multiplicity;
}
