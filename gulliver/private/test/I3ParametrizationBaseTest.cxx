#include <I3Test.h>
#include "I3TestDummyParametrization.h"
#include "I3BasicDummyParametrization.h"
#include <icetray/I3Frame.h>

TEST_GROUP(I3ParametrizationBase);

TEST(Set_N_Get_Hypothesis) {
  I3Particle theParticle;
  theParticle.SetDir(0.5, 0.5);
  theParticle.SetPos(1, 2, 3);
  theParticle.SetEnergy(1);

  I3EventHypothesisPtr hypot1(new I3EventHypothesis(theParticle));
  I3TestDummyTrackParametrization param(hypot1, 0.5, 0.5);

  vector<double> dummy(3, 0);

  I3ParticlePtr particle1 = param.GetHypothesisPtr(dummy)->particle;

  theParticle.SetEnergy(2);
  I3EventHypothesisPtr hypot2(new I3EventHypothesis(theParticle));
  param.SetHypothesisPtr(hypot2);

  I3ParticlePtr particle2 = param.GetHypothesisPtr(dummy)->particle;

  //If this first test ever fails, it could be that someone changed the
  //I3EventHypothesis definition to no longer make a copy of the input particle
  ENSURE(*particle1 != *particle2, "The particles do not have the same energy");
  ENSURE(particle1->GetPos() == particle2->GetPos(), "They DO have the same position");
  ENSURE(particle1->GetEnergy() != particle2->GetEnergy(), "The energies are different");

  I3ParticlePtr particle3 = param.GetHypothesisPtr()->particle;
  ENSURE(*particle2 == *particle3, "The vanilla getter does not match the one with the parameters");
}

TEST(Init_Specs) {
  I3Particle theParticle;
  theParticle.SetDir(0.5, 0.5);
  theParticle.SetPos(1, 2, 3);
  theParticle.SetEnergy(1);

  I3EventHypothesisPtr hypot(new I3EventHypothesis(theParticle));

  I3TestDummyTrackParametrization param(hypot, 0.5, 0.5, 33.295);
  std::vector<I3FitParameterInitSpecs> specs = param.GetParInitSpecs(hypot);

  ENSURE(specs.size() == 3, "Someone changed the number of initialized params");
  ENSURE(specs[2].stepsize_ == 33.295, "You get out what you put in");
}

TEST(SetEvent) {
  I3BasicDummyParametrization param;
  I3Frame frame = I3Frame(I3Frame::None);
  param.SetEvent(frame);
}

TEST(ChainRule) {
  I3BasicDummyParametrization param;
  ENSURE(!param.InitChainRule(true));

  try {
    param.CallProtectedApplyChainRule();
    FAIL("You didn't log_fatal! This virtual function is not overloaded.");
  } catch (const std::exception&) { }
}