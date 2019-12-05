#ifndef GULLIVER_I3BASICDUMMYPARAMETRIZATION_H_INCLUDED
#define GULLIVER_I3BASICDUMMYPARAMETRIZATION_H_INCLUDED
#include <cassert>
#include <iostream>
#include "gulliver/I3ParametrizationBase.h"


//
// This is the most basic parametrization that you can write
// It overloads none of the virtual functions that are not
// pure virtual.
//
class I3BasicDummyParametrization : public I3ParametrizationBase {
  public:

    // constructor
    I3BasicDummyParametrization() : I3ParametrizationBase(NULL), name_("BigOl'Dummy")
    {}

    // destructor
    ~I3BasicDummyParametrization() {}

    void UpdatePhysicsVariables() {}

    void UpdateParameters() {}

    void CallProtectedApplyChainRule(){
        ApplyChainRule();
    }

    const std::string GetName() const {
        return name_;
    }



  private:
    std::string name_;
};
typedef boost::shared_ptr<I3BasicDummyParametrization> I3BasicDummyParametrizationPtr;

#endif /* GULLIVER_I3BASICDUMMYPARAMETRIZATION_H_INCLUDED */