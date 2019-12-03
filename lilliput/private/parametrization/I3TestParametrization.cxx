#include <lilliput/parametrization/I3TestParametrization.h>
#include <icetray/I3SingleServiceFactory.h>
#include <dataclasses/I3Double.h>

I3TestParametrization::I3TestParametrization():
  I3ServiceBase("I3TestParametrization"),
  I3ParametrizationBase(I3EventHypothesisPtr()),
  setDiagnostics_(false),
  setNonStd_(false) {

}

I3TestParametrization::I3TestParametrization(const I3Context& context):
  I3ServiceBase(context),
  I3ParametrizationBase(I3EventHypothesisPtr()),
  setDiagnostics_(false),
  setNonStd_(false) {
  AddParameter("SetDiagnostics", "If true diagnostics will be returned", setDiagnostics_);
  AddParameter("SetNonStd", "If true a nonstd will be added to the hypothesis", setNonStd_);
}

void I3TestParametrization::Configure() {
  log_debug("Configuring the test parameterization");

  GetParameter("SetDiagnostics", setDiagnostics_);
  GetParameter("SetNonStd", setNonStd_);

  //This is needed because the parameterization will immediately
  //check for par_.size() != 0 && par.size() == parspecs_.size()
  par_.push_back(1);
  parspecs_.push_back(I3FitParameterInitSpecs("DummyInitSpec"));
}

/// this should calculate datamembers of EmissionHypothesis from the values in par_
void I3TestParametrization::UpdatePhysicsVariables() {
  log_debug("Updating the physics variables");

  I3Particle& particle = *(hypothesis_->particle);

  particle.SetPos(1, 2, 3);
  particle.SetDir(0.5, 0.5);
}

void I3TestParametrization::UpdateParameters() {
  log_debug("Updating the parameters");

  I3Particle particle;
  particle.SetPos(1, 2, 3);
  particle.SetDir(0.5, 0.5);
  particle.SetShape(I3Particle::Primary);

  *hypothesis_->particle = particle;

  if(setNonStd_ && !hypothesis_->nonstd){
    log_debug("Adding a nonstd element to the hypothesis");
    hypothesis_->nonstd = I3DoublePtr(new I3Double(42));
  }

}

I3FrameObjectPtr I3TestParametrization::GetDiagnostics(const I3EventHypothesis &fitresult) {
  if (setDiagnostics_) {
    return I3DoublePtr(new I3Double(42));
  }
  return I3FrameObjectPtr();
}

typedef I3SingleServiceFactory<I3TestParametrization, I3ParametrizationBase>
I3TestParametrizationFactory;

I3_SERVICE_FACTORY(I3TestParametrizationFactory);