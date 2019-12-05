#include <I3Test.h>

#include <gulliver/I3EventHypothesis.h>
#include <dataclasses/I3Double.h>

TEST_GROUP(I3EventHypothesis);

TEST(default_constructor) {
  I3EventHypothesis hypot;
  ENSURE(hypot.particle != NULL);
  ENSURE(!hypot.nonstd);
}

TEST(initialized_constructor_particle) {
  I3Particle aParticle;
  aParticle.SetEnergy(42);
  aParticle.SetPos(I3Position(0,1,2));

  I3EventHypothesis hypot(aParticle);

  ENSURE(*hypot.particle == aParticle);
  ENSURE(!hypot.nonstd);
}

TEST(initialized_constructor_nonstd) {
  I3ParticlePtr aParticle(new I3Particle());
  aParticle->SetEnergy(42);
  aParticle->SetPos(I3Position(0,1,2));

  I3DoublePtr value(new I3Double(-42.));

  I3EventHypothesis hypot(aParticle, value);

  ENSURE(*hypot.particle == *aParticle);
  I3Double newVal = *boost::dynamic_pointer_cast<I3Double>(hypot.nonstd);
  ENSURE(newVal == *value);
}

