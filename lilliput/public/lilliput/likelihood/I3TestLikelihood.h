
//LCOV_EXCL_START
//This class IS the test. We don't need to test it.

#ifndef I3TESTLIKELIHOOD_H
#define I3TESTLIKELIHOOD_H

#include <icetray/I3ServiceBase.h>
#include <gulliver/I3EventLogLikelihoodBase.h>

class I3TestLikelihood: public I3ServiceBase, public I3EventLogLikelihoodBase {
 public:
  I3TestLikelihood(const I3Context& context);
  virtual ~I3TestLikelihood() {}

  virtual void Configure();

  virtual void SetGeometry(const I3Geometry &f) {}
  virtual void SetEvent(const I3Frame &f) {}
  virtual double GetLogLikelihood(const I3EventHypothesis &ehypo){return likelihood_;}
  virtual bool HadGradient() {return true;}
  virtual double GetLogLikelihoodWithGradient( const I3EventHypothesis &ehypo,
      I3EventHypothesis &gradient, double weight = 1 ) {
    return GetLogLikelihood(ehypo);
  }
  virtual unsigned int GetMultiplicity(){return multiplicity_;}
  virtual I3FrameObjectPtr GetDiagnostics(const I3EventHypothesis &fitresult);
  virtual const std::string GetName() const {return I3ServiceBase::GetName();}

 private:
  I3TestLikelihood();

  double likelihood_;
  unsigned int multiplicity_;
  bool setDiagnostics_;
};

#endif
//LCOV_EXCL_STOP