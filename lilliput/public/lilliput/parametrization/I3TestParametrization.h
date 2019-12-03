#ifndef I3TESTPARAMETERIZATION_H_INCLUDED
#define I3TESTPARAMETERIZATION_H_INCLUDED

// Gulliver stuff
#include "gulliver/I3ParametrizationBase.h"

#include "icetray/I3ServiceBase.h"

class I3TestParametrization : public I3ServiceBase,
  public I3ParametrizationBase {

 public:
  I3TestParametrization(const I3Context& context);

  virtual void Configure();
  virtual void UpdatePhysicsVariables();
  virtual void UpdateParameters();
  virtual const std::string GetName() const {return I3ServiceBase::GetName();}
  virtual I3FrameObjectPtr GetDiagnostics(const I3EventHypothesis &fitresult);

  virtual ~I3TestParametrization() {}

 private:

  // stop defaults
  I3TestParametrization();
  I3TestParametrization operator= (const I3TestParametrization& rhs);

  bool setDiagnostics_;
  bool setNonStd_;

  SET_LOGGER( "I3TestParametrization" );
};

I3_POINTER_TYPEDEFS( I3TestParametrization );

#endif