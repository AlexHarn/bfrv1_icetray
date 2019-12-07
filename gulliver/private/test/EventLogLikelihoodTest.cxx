#include <I3Test.h>

#include <icetray/I3Tray.h>

#include "I3BasicEventLogLikelihood.h"
#include <dataclasses/geometry/I3Geometry.h>

//Here we are mostly testing out the base class for the event llh
//Most of these things don't do anything, but we want to ensure
//that we get what we expect from the default behavior

TEST_GROUP(Gulliver_LikelihoodCombination);

TEST(constructor) {
  I3BasicEventLogLikelihood llh;
}

TEST(set_geometry) {
  I3BasicEventLogLikelihood llh;
  I3Geometry geom;

  llh.SetGeometry(geom);
}

TEST(set_event) {
  I3BasicEventLogLikelihood llh;
  I3Frame frame;

  llh.SetEvent(frame);
}

TEST(get_llh_direct) {
  I3BasicEventLogLikelihood llh;
  I3EventHypothesis hypothesis;

  ENSURE(llh.GetLogLikelihood(hypothesis) == 42);
}

TEST(get_llh_with_gradient) {
  I3BasicEventLogLikelihood llh;
  I3EventHypothesis hypothesis;

  try {
    llh.GetLogLikelihoodWithGradient(hypothesis, hypothesis);
    FAIL("You didn't log_fatal! This virtual function is not overloaded.");
  } catch (const std::exception&) { }
}

TEST(has_gradient) {
  I3BasicEventLogLikelihood llh;

  ENSURE(!llh.HasGradient());
}

TEST(get_llh) {
  I3BasicEventLogLikelihood llh;
  I3EventHypothesis hypothesis;

  ENSURE(llh.GetLogLikelihood(hypothesis, &hypothesis, false) == 42);
}

TEST(get_multiplicity) {
  I3BasicEventLogLikelihood llh;

  ENSURE(llh.GetMultiplicity() == 99);
}

TEST(get_diagnostics) {
  I3BasicEventLogLikelihood llh;
  I3EventHypothesis hypothesis;

  I3FrameObjectPtr pointer = llh.GetDiagnostics(hypothesis);
  ENSURE(pointer == NULL);
}