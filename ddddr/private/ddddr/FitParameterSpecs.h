#ifndef FITPARAMETERSPECS_H_INCLUDED
#define FITPARAMETERSPECS_H_INCLUDED

#include <string>
#include <math.h>

/**
 * @brief Class based on (=copied from) I3FitParameterInitSpecs in lilliput/gulliver 
 * that holds all information about a function parameter needed by the minimizer.
 */
class FitParameterSpecs {
public:
    FitParameterSpecs(std::string name):
        name_(name), initval_(NAN), stepsize_(NAN), minval_(NAN), maxval_(NAN) {}
    FitParameterSpecs(std::string name, double initv, double steps, 
			double minv, double maxv):
        name_(name), initval_(initv), stepsize_(steps), minval_(minv), maxval_(maxv) {}
    ~FitParameterSpecs(){}

    /// parameter name (some minimizers want that)
    std::string name_;

    /// initval start value
    double initval_;

    /// stepsize step size (expected order of magnitude for initial variation)
    double stepsize_;

    /// minimum value of parameter (NAN if unlimited)
    double minval_;

    /// maximum value of parameter (NAN if unlimited)
    double maxval_;
};

#endif 
