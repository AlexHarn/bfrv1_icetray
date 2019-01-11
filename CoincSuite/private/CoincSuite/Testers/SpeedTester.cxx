/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file SpeedTester.cxx
 * @version $Revision: 1.0$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#ifndef SPEEDTEST_H
#define SPEEDTEST_H

#include "CoincSuite/Testers/TesterModule.h"

/// A Tester-Module testing if the ParticleFit speed of a hypoFit is comparable to light speed in vacuum
class SpeedTester : public TesterModule {
private:
  SET_LOGGER("SpeedTester");
  //parameters
  /// PARAM: Name of the hypoFit i should check
  std::string hypoFitName_;
  /// PARAM: Upper Cut value on the particle's speed in [m/ns]
  double speedUpperCut_;
  /// PARAM: Lower Cut value on the particle's speed in [m/ns]
  double speedLowerCut_;
private: // methods
  /// Implement the evaluation for mutual-testing
  Result Test(I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) const;
  ///call-back function
  static Result runTest (void *tester, I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) {
    SpeedTester *alPtr = (SpeedTester*) tester;
    return alPtr->Test(hypoframe, frameA, frameB);
  };

public:
  /// Constructor
  SpeedTester (const I3Context& context);
  /// Configure
  void Configure();

private:
  /** Tests if the hypoFit-speed is within the given limits
   * @return true, if speed is within ...[speedLowerCut_, speedUpperCut_]...
   */
  bool SpeedCriterium(const I3Particle &hypoFit) const;
};

I3_MODULE(SpeedTester);

#endif // SPEEDTEST_H


//=========================== IMPLEMENTATIONS ==================================

#include "dataclasses/I3Constants.h"
#include <boost/lexical_cast.hpp>

//______________________________________________________________________________
SpeedTester::SpeedTester (const I3Context& context): TesterModule(context),
  hypoFitName_(),
  speedUpperCut_(0.2995*I3Units::m/I3Units::ns),
  speedLowerCut_(.3*I3Units::m/I3Units::ns)
{
  AddParameter("HypoFitName","Name of the Fit in the HypoFrame to test against", hypoFitName_);
  AddParameter("SpeedUpperCut","Maximum allowed particle speed in [m/ns]", speedUpperCut_);
  AddParameter("SpeedLowerCut","Minimum allowed particle speed in [m/ns] (choose wisely)", speedLowerCut_);
}

//______________________________________________________________________________
void SpeedTester::Configure() {
  TesterModule::Configure();
  GetParameter("HypoFitName", hypoFitName_);
  GetParameter("SpeedUpperCut", speedUpperCut_);
  GetParameter("SpeedLowerCut", speedLowerCut_);

  if (hypoFitName_.empty())
    log_fatal("Configure 'HypoFitName'!");
  if (speedLowerCut_ < 0 || speedUpperCut_ < 0 )
    log_fatal("SpeedUpperCut and SpeedLowerCut must be greater than zero!");
  if (speedLowerCut_ > speedUpperCut_)
    log_fatal("SpeedUpperCut must be greater or equal than SpeedLowerCut!");
  if (speedLowerCut_ > I3Constants::c || I3Constants::c > speedUpperCut_)
    log_warn("SpeedLowerCut and SpeedUpperCut should mutually include the light-speed c=%f", I3Constants::c);

  Evaluate = runTest;
}

//______________________________________________________________________________
SpeedTester::Result SpeedTester::Test(I3FrameConstPtr hypoframe, I3FrameConstPtr frameA, I3FrameConstPtr frameB) const {
  log_debug("Entering SpeedTester::Test()");
  I3ParticleConstPtr hypoFit = hypoframe->Get<I3ParticleConstPtr>(hypoFitName_);
  if (! hypoFit) {
    log_error("Could not find the HypoFit <I3Particle>('%s') in the HypoFrame;"
              " will continue with next HypoFrame", hypoFitName_.c_str());
    return UNDECIDED;
  }

  if (hypoFit->GetFitStatus() != I3Particle::OK) {
    log_warn("FitStatus not OK; will continue with next HypoFrame");
    return UNDECIDED; // to loop over all HypoFrames
  }

  log_debug("All necessary objects are found in the frames to begin hypothesis testing");

  return Bool2Result(SpeedCriterium(*hypoFit));
}

//______________________________________________________________________________
bool SpeedTester::SpeedCriterium(const I3Particle &hypoFit) const {
  log_debug("Entering SpeedCriterium");

  const double speed = hypoFit.GetSpeed();
  const bool passed = (speedLowerCut_ <= speed) && (speed <= speedUpperCut_);
  log_trace_stream((passed?"Passed":"Failed")<<" SpeedCriterium(["<<speedLowerCut_<<","<<speedUpperCut_<<"]): "<<speed);
  
  return ( passed );
}
