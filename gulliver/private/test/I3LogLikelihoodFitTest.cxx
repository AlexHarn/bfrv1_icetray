#include <I3Test.h>

#include <icetray/I3Tray.h>

#include <gulliver/I3LogLikelihoodFit.h>

TEST_GROUP(Gulliver_I3LogLikelihoodFit);

TEST(constructor) {
  I3LogLikelihoodFit llhFit;

  ENSURE(llhFit.hypothesis_ != NULL);
  ENSURE(llhFit.fitparams_ != NULL);
  ENSURE(llhFit.llhdiagnostics_ == NULL);
  ENSURE(llhFit.minidiagnostics_ == NULL);
  ENSURE(llhFit.paradiagnostics_ == NULL);
}

TEST(initialized_constructor_pointer) {

  I3EventHypothesisPtr ehypo(new I3EventHypothesis());
  ehypo->particle->SetEnergy(42);
  I3LogLikelihoodFitParamsPtr params(new I3LogLikelihoodFitParams(42, 41, 99, -15));

  I3LogLikelihoodFit llhFit(ehypo, params);

  ENSURE(llhFit.hypothesis_ != NULL);
  ENSURE(llhFit.hypothesis_->particle->GetEnergy() == 42);
  ENSURE(llhFit.fitparams_ != NULL);
  ENSURE(llhFit.fitparams_->logl_ == 42);
  ENSURE(llhFit.fitparams_->rlogl_ == 41);
  ENSURE(llhFit.fitparams_->ndof_ == 99);
  ENSURE(llhFit.fitparams_->nmini_ == -15);
}

TEST(initialized_constructor_reference) {

  I3EventHypothesis ehypo;
  ehypo.particle->SetEnergy(42);
  I3LogLikelihoodFitParams params(42, 41, 99, -15);

  I3LogLikelihoodFit llhFit(ehypo, params);

  ENSURE(llhFit.hypothesis_ != NULL);
  ENSURE(llhFit.hypothesis_->particle->GetEnergy() == 42);
  ENSURE(llhFit.fitparams_ != NULL);
  ENSURE(llhFit.fitparams_->logl_ == 42);
  ENSURE(llhFit.fitparams_->rlogl_ == 41);
  ENSURE(llhFit.fitparams_->ndof_ == 99);
  ENSURE(llhFit.fitparams_->nmini_ == -15);
}

TEST(initialized_constructor_only_hypot) {

  I3EventHypothesis ehypo;
  ehypo.particle->SetEnergy(42);

  I3LogLikelihoodFit llhFit(ehypo);

  ENSURE(llhFit.hypothesis_ != NULL);
  ENSURE(llhFit.hypothesis_->particle->GetEnergy() == 42);
}

TEST(initialized_constructor_particle) {
  I3Particle particle;
  particle.SetEnergy(42);
  I3LogLikelihoodFitParams params(42, 41, 99, -15);

  I3LogLikelihoodFit llhFit(particle, params);

  ENSURE(llhFit.hypothesis_ != NULL);
  ENSURE(llhFit.hypothesis_->particle->GetEnergy() == 42);
  ENSURE(llhFit.fitparams_ != NULL);
  ENSURE(llhFit.fitparams_->logl_ == 42);
  ENSURE(llhFit.fitparams_->rlogl_ == 41);
  ENSURE(llhFit.fitparams_->ndof_ == 99);
  ENSURE(llhFit.fitparams_->nmini_ == -15);
}

TEST(comparison_operators) {
  I3Particle particle;
  particle.SetEnergy(42);

  I3LogLikelihoodFitParams params(42, 41, 99, -15);

  params.rlogl_ = 1;
  params.logl_ = 2;
  I3LogLikelihoodFit llhFit1(particle, params);
  params.rlogl_ = 2;
  params.logl_ = 3;
  I3LogLikelihoodFit llhFit2(particle, params);
  params.rlogl_ = 3;
  params.logl_ = 1;
  I3LogLikelihoodFit llhFit3(particle, params);

  ENSURE(llhFit1 < llhFit2, "The order is decided by rlogl");
  ENSURE(llhFit3 > llhFit2, "The order is decided by rlogl");
}