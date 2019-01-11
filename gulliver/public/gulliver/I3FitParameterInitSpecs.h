/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author David Boersma <boersma@icecube.wisc.edu>

*/

#ifndef I3FITPARAMETERINITSPECS_H_INCLUDED
#define I3FITPARAMETERINITSPECS_H_INCLUDED

#include <string>
#include <cmath>

/**
 * @class I3FitParameterInitSpecs
 * @brief specs for a fitting parameter (as seen by minimizer)
 *
 * Simple auxiliary struct/class to I3GulliverBase and I3Parametrization
 *
 * @sa I3GulliverBase
 * @sa I3Parametrization
 * @sa I3ParametrizationBase
 */
class I3FitParameterInitSpecs {
public:
    I3FitParameterInitSpecs(std::string name):
        name_(name), initval_(NAN), stepsize_(NAN), minval_(NAN), maxval_(NAN) {}
    ~I3FitParameterInitSpecs(){}
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

    bool operator==(const I3FitParameterInitSpecs &o) {
        return (name_ == o.name_);
    }
};

#endif /* I3FITPARAMETERINITSPECS_H_INCLUDED */
