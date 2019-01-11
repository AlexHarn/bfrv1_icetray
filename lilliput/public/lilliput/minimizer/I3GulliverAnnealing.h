/**
 *
 * @brief declaration of the I3GulliverAnnealing class
 *
 * @file I3GulliverAnnealing.h
 * @version $Revision$
 * @author Holger Motz <holger.motz@physik.uni-erlangen.de>
 */

#ifndef I3GULLIVERANNEALING_H_INCLUDED
#define I3GULLIVERANNEALING_H_INCLUDED

// standard library stuff
#include <vector>
#include <string>
#include <algorithm>
#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"

#include "phys-services/I3RandomService.h"

// Gulliver stuff
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3MinimizerResult.h"
#include "gulliver/I3FitParameterInitSpecs.h"

// Forward declaration of the minimizer class:
class DSimulatedAnnealing;

/**
 * @class I3GulliverAnnealing
 * @brief A minimizer class which wraps DSimulated such that it can be used in
 *        Gulliver-based reconstruction.
 */
class I3GulliverAnnealing : public I3ServiceBase,
                            public I3MinimizerBase {

private:
    I3GulliverAnnealing();
    I3GulliverAnnealing( const I3GulliverAnnealing& );
    I3GulliverAnnealing operator= (const I3GulliverAnnealing& rhs);

    const std::string name_;
    I3RandomServicePtr random_;
    unsigned int maxIterations_;
    double tolerance_;
    bool shiftBoundaries_;
    double quenchingfactor_;  
    double starttemp_;
    int scycle_;
    int tcycle_;
    int ncycleseps_;

public:
    /// constructor with full initialization (for unit tests)
    I3GulliverAnnealing( std::string name,
                         I3RandomServicePtr rndptr,
                         unsigned int maxi=DEFAULT_MAXITERATIONS,
                         double tol= DEFAULT_TOLERANCE,
                         bool shiftBoundaries=DEFAULT_SHIFTBOUNDARIES,
                         double quench=DEFAULT_QUENCHINGFACTOR,
                         double temp0=DEFAULT_STARTTEMP,
                         int scylce=DEFAULT_SCYCLE,
                         int tcycle=DEFAULT_TCYCLE,
                         int ncycleseps=DEFAULT_NCYCLESEPS );

    /// constructor for service factory
    I3GulliverAnnealing(const I3Context& context);

    /// destructor
    virtual ~I3GulliverAnnealing();

    /// configuration
    void Configure();

    /// core method: minimize a given function with given initial conditions
    I3MinimizerResult Minimize(
            I3GulliverBase &g,
            const std::vector<I3FitParameterInitSpecs> &parspecs ) ;
    /// set tolerance (what is "tolerance" supposed to be anyway ?)
    // double GetTolerance() const { return 10*sqrt(tolerance_); } // kinda crazy, but maybe it works
    // void SetTolerance(double newtol ){ tolerance_ = 0.01 * newtol * newtol; } // kinda crazy, but maybe it works
    double GetTolerance() const { return tolerance_; }
    void SetTolerance(double newtol ){ tolerance_ = newtol; }
    unsigned int GetMaxIterations() const { return maxIterations_; }
    void SetMaxIterations(unsigned int newmaxi ){ maxIterations_ = newmaxi; }
    const std::string GetName() const {
        return I3ServiceBase::GetName();
    }

    SET_LOGGER( "I3GulliverAnnealing" );

    /// default values for options
    static const unsigned int DEFAULT_MAXITERATIONS;
    static const double DEFAULT_TOLERANCE;
    static const bool DEFAULT_SHIFTBOUNDARIES;
    static const double DEFAULT_QUENCHINGFACTOR;
    static const double DEFAULT_STARTTEMP;
    static const int DEFAULT_SCYCLE;
    static const int DEFAULT_TCYCLE;
    static const int DEFAULT_NCYCLESEPS;

};

#endif
