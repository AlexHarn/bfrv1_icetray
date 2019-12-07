#ifndef GULLIVER_I3BASICEVENTLOGLIKELIHOOD_H_INCLUDED
#define GULLIVER_I3BASICEVENTLOGLIKELIHOOD_H_INCLUDED

#include <gulliver/I3EventHypothesis.h>
#include <gulliver/I3EventLogLikelihoodBase.h>

class I3BasicEventLogLikelihood : public I3EventLogLikelihoodBase {

private:


 public:
  I3BasicEventLogLikelihood() : I3EventLogLikelihoodBase() {}
  virtual ~I3BasicEventLogLikelihood() {}

  virtual void SetGeometry(const I3Geometry &f) {}

  virtual void SetEvent(const I3Frame &f) {}

  virtual double GetLogLikelihood(const I3EventHypothesis &ehypo) {return 42;}

  using I3EventLogLikelihoodBase::GetLogLikelihood;

  virtual unsigned int GetMultiplicity() {return 99;}

  virtual const std::string GetName() const {return "TestLLH";}
};

I3_POINTER_TYPEDEFS(I3BasicEventLogLikelihood);

#endif