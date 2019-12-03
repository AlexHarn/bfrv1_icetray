//LCOV_EXCL_START
//This class IS the test. We don't need to test it.

#ifndef I3TESTMINIMIZER_H
#define I3TESTMINIMIZER_H

#include "icetray/I3ServiceBase.h"

#include "gulliver/I3MinimizerBase.h"

class I3TestMinimizer : public I3ServiceBase, public I3MinimizerBase {
 public:
  /// constructor with full initialization (for unit tests)
  I3TestMinimizer(std::string name, const double tol, unsigned int maxi);

  /// constructor for service factory
  I3TestMinimizer(const I3Context& context);

  virtual ~I3TestMinimizer();

  virtual void Configure();

  I3MinimizerResult Minimize(
    I3GulliverBase &g,
    const std::vector<I3FitParameterInitSpecs> &parspecs);

  double GetTolerance() const {return tolerance_;}
  void SetTolerance(double newtol) {tolerance_ = newtol;}

  unsigned int GetMaxIterations() const {return maxIterations_;}
  void SetMaxIterations(unsigned int newmaxi) {maxIterations_ = newmaxi;}

  const std::string GetName() const {return I3ServiceBase::GetName();}

  SET_LOGGER("I3TestMinimizer");

 private:
  I3TestMinimizer();
  I3TestMinimizer(const I3TestMinimizer&);
  I3TestMinimizer operator= (const I3TestMinimizer& rhs);

  const std::string name_;
  double tolerance_;
  unsigned int maxIterations_;
  double minVal_;
  bool isConverged_;
  bool setDiagnostic_;
  bool fillParameters_;
  bool fillParameterErrors_;
};

#endif
//LCOV_EXCL_STOP