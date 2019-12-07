#include <I3Test.h>

#include <icetray/I3Tray.h>

#include "I3BasicSeedService.h"
#include <dataclasses/geometry/I3Geometry.h>

//Here we are mostly testing out the base class for the event llh
//Most of these things don't do anything, but we want to ensure
//that we get what we expect from the default behavior

TEST_GROUP(Gulliver_I3SeedServiceBase);

TEST(constructor) {
  I3BasicSeedService seedServ;
}

TEST(set_event) {
  I3BasicSeedService seedServ;
  I3Frame frame;

  ENSURE(seedServ.SetEvent(frame) == 42);
}

TEST(get_seed) {
  I3BasicSeedService seedServ;

  I3EventHypothesis ehypo = seedServ.GetSeed(13);

  ENSURE(ehypo.particle->GetEnergy() == 42);
}

TEST(get_seed_ptr) {
  I3BasicSeedService seedServ;

  I3EventHypothesisPtr ehypo = seedServ.GetSeedPtr(13);

  ENSURE(ehypo->particle->GetEnergy() == 42);
}

TEST(get_dummy) {
  I3BasicSeedService seedServ;

  I3EventHypothesis ehypo = seedServ.GetDummy();

  I3Particle comparisonParticle;

  //The dummy should just return an I3EventHypothesis with a default particle
  ENSURE(ehypo.particle->GetShape() == comparisonParticle.GetShape(), "The particle should be a default one");
  ENSURE(ehypo.particle->GetType() == comparisonParticle.GetType(), "The particle should be a default one");
}

TEST(get_dummy_ptr) {
  I3BasicSeedService seedServ;

  I3EventHypothesisPtr ehypo = seedServ.GetDummyPtr();

  I3Particle comparisonParticle;

  //The dummy should just return an I3EventHypothesis with a default particle
  ENSURE(ehypo->particle->GetShape() == comparisonParticle.GetShape(), "The particle should be a default one");
  ENSURE(ehypo->particle->GetType() == comparisonParticle.GetType(), "The particle should be a default one");
}

TEST(get_copy) {
  I3BasicSeedService seedServ;

  I3Particle particle;
  particle.SetEnergy(42);
  particle.SetPos(1,2,3);
  particle.SetDir(0.5,0.6);
  I3EventHypothesis ehypo(particle);

  I3EventHypothesis comparisonEHypo = seedServ.GetCopy(ehypo);

  ENSURE(*comparisonEHypo.particle == *ehypo.particle, "By default, just makes a copy");
}

TEST(get_copy_ptr) {
  I3BasicSeedService seedServ;

  I3Particle particle;
  particle.SetEnergy(42);
  particle.SetPos(1,2,3);
  particle.SetDir(0.5,0.6);
  I3EventHypothesis ehypo(particle);

  I3EventHypothesisPtr comparisonEHypo = seedServ.GetCopyPtr(ehypo);

  ENSURE(*comparisonEHypo->particle == *ehypo.particle, "By default, just makes a copy");
}

TEST(tweak) {
  I3BasicSeedService seedServ;
  I3EventHypothesis ehypo;
  seedServ.Tweak(ehypo);
}

TEST(fill_in_the_blanks) {
  I3BasicSeedService seedServ;
  I3EventHypothesis ehypo;
  seedServ.FillInTheBlanks(ehypo);
}