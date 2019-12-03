//LCOV_EXCL_START
//This class IS the test. We don't need to test it.

////////////////////////////////////////////////////////////
////Warning, this minimizer is for testing purposes only
////////////////////////////////////////////////////////////

#include "dataclasses/I3Double.h"
#include "icetray/I3SingleServiceFactory.h"
#include "lilliput/minimizer/I3TestMinimizer.h"

// default values for options
static const unsigned int DEFAULT_MAXITERATIONS = 10000;
static const double DEFAULT_TOLERANCE = 0.1;

// option names
static const char* tolerance_optionname = "Tolerance";
static const char* maxiterations_optionname = "MaxIterations";


I3TestMinimizer::I3TestMinimizer(std::string name, const double tol,
                                 unsigned int maxi) :
  I3ServiceBase(name),
  I3MinimizerBase(),
  name_(name),
  tolerance_(tol),
  maxIterations_(maxi),
  minVal_(NAN),
  isConverged_(true),
  setDiagnostic_(false),
  fillParameters_(true),
  fillParameterErrors_(true) { }

I3TestMinimizer::I3TestMinimizer(const I3Context& context) :
  I3ServiceBase(context),
  tolerance_(DEFAULT_TOLERANCE),
  maxIterations_(DEFAULT_MAXITERATIONS),
  minVal_(NAN),
  isConverged_(true),
  setDiagnostic_(false),
  fillParameters_(true),
  fillParameterErrors_(true) {

  AddParameter("Tolerance",
               "Maximum number of iterations (may be overridden by calling "
               "module)",
               maxIterations_);
  AddParameter("MaxIterations",
               "Tolerance for minimum (may be overridden by calling module)",
               tolerance_);
  AddParameter("ValueToReturn",
               "This will be the minVal_ in the I3MinimizerResult",
               minVal_);
  AddParameter("IsConverged",
               "The convergence returned will be whatever you set here",
               isConverged_);
  AddParameter("SetDiagnostics",
               "If true, the diagnostics_ field will be filled with the number 42",
               setDiagnostic_);
  AddParameter("FillParameters",
               "If true, the result parameters will be filled with the initial parameters",
               fillParameters_);
  AddParameter("FillParameterErrors",
               "If true, the result errors will be filled with an error of 1",
               fillParameterErrors_);

}

void I3TestMinimizer::Configure() {
  log_debug("Configuring the test minimizer");

  GetParameter("MaxIterations", tolerance_);
  GetParameter("Tolerance", maxIterations_);
  GetParameter("ValueToReturn", minVal_);
  GetParameter("IsConverged", isConverged_);
  GetParameter("SetDiagnostics", setDiagnostic_);
}

I3TestMinimizer::~I3TestMinimizer() { }

I3MinimizerResult I3TestMinimizer::Minimize(
  I3GulliverBase &g,
  const std::vector<I3FitParameterInitSpecs> &parspecs) {
  log_debug("Running the test minimizer");

  int Dimension = parspecs.size();
  assert(Dimension > 0);

  // harvest results
  I3MinimizerResult minResult(Dimension);

  minResult.converged_ = isConverged_;

  minResult.minval_ = minVal_;

  if (setDiagnostic_)
    minResult.diagnostics_ = I3DoublePtr(new I3Double(42));

  if (fillParameters_) {
    for (int ipar = 0; ipar < Dimension; ipar++) {
      minResult.par_[ipar] = parspecs[ipar].initval_;
    }
  }

  if (fillParameterErrors_) {
    for (int ipar = 0; ipar < Dimension; ipar++) {
      minResult.err_[ipar] = 1;
    }
  }

  return minResult;
};

typedef I3SingleServiceFactory<I3TestMinimizer, I3MinimizerBase>
I3TestMinimizerFactory;

I3_SERVICE_FACTORY(I3TestMinimizerFactory);
//LCOV_EXCL_STOP