//LCOV_EXCL_START
//This class IS the test. We don't need to test it.

#include <lilliput/likelihood/I3TestLikelihood.h>
#include <icetray/I3SingleServiceFactory.h>
#include <dataclasses/I3Double.h>

I3TestLikelihood::I3TestLikelihood():
  I3ServiceBase("I3TestLikelihood"),
  I3EventLogLikelihoodBase(),
  likelihood_(NAN),
  multiplicity_(1),
  setDiagnostics_(false) {}

I3TestLikelihood::I3TestLikelihood(const I3Context& context):
  I3ServiceBase(context),
  I3EventLogLikelihoodBase(),
  likelihood_(NAN),
  multiplicity_(1),
  setDiagnostics_(false) {
  AddParameter("LikelihoodValue", "This is the LLH to be returned", likelihood_);
  AddParameter("Multiplicity", "This is the multiplicty of the event", multiplicity_);
  AddParameter("SetDiagnostics", "If true diagnostics will be returned", setDiagnostics_);
}

void I3TestLikelihood::Configure() {
  log_debug("Configuring the test likelihood");
  GetParameter("LikelihoodValue", likelihood_);
  GetParameter("Multiplicity", multiplicity_);
  GetParameter("SetDiagnostics", setDiagnostics_);
}

I3FrameObjectPtr I3TestLikelihood::GetDiagnostics(const I3EventHypothesis &fitresult){
  log_debug("Configuring the test likelihood");
  if(setDiagnostics_){
    return I3DoublePtr(new I3Double(42));
  }
  return I3FrameObjectPtr();
}

typedef I3SingleServiceFactory<I3TestLikelihood, I3EventLogLikelihoodBase>
I3TestLikelihoodFactory;

I3_SERVICE_FACTORY(I3TestLikelihoodFactory);
//LCOV_EXCL_STOP