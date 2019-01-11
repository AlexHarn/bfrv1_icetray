
#ifndef I3GULLIVERLBFGSB_H_INCLUDED
#define I3GULLIVERLBFGSB_H_INCLUDED

/* Gulliver stuff */
#include "gulliver/I3MinimizerBase.h"

#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"


class I3GulliverLBFGSB : public I3ServiceBase, public I3MinimizerBase {
public:
    /// constructor for service factory
    I3GulliverLBFGSB(const I3Context& context);

    /// destructor
    virtual ~I3GulliverLBFGSB();

    /// configuration
    void Configure();

    /// set tolerance (minuit manual is not very clear on what "tolerance" exactly means)
    double GetTolerance() const { return tolerance_; }
    void SetTolerance(double newtol ){ tolerance_ = newtol; }
    unsigned int GetMaxIterations() const { return maxIterations_; }
    void SetMaxIterations(unsigned int newmaxi ){ maxIterations_ = newmaxi; }
    const std::string GetName() const {
        return I3ServiceBase::GetName();
    }
    bool UsesGradient() { return true; };

    /// core method: minimizer a given function with given initial conditions
    I3MinimizerResult Minimize(I3GulliverBase &g, const std::vector<I3FitParameterInitSpecs> &parspecs );
private:
    SET_LOGGER("I3GulliverLBFGSB");

    double tolerance_;
    double gradientTolerance_;
    int maxIterations_;
};

#endif
