/**
 * @brief Interface to Low-memory Broyden-Fletcher-Goldfarb-Shanno
 *        (with Bounds) minimization algorithm.
 *
 * References for L-BFGS-B:
 * - R. H. Byrd, P. Lu and J. Nocedal. A Limited Memory Algorithm for Bound Constrained Optimization, (1995), SIAM Journal on Scientific and Statistical Computing , 16, 5, pp. 1190-1208.
 * - C. Zhu, R. H. Byrd and J. Nocedal. L-BFGS-B: Algorithm 778: L-BFGS-B, FORTRAN routines for large scale bound constrained optimization (1997), ACM Transactions on Mathematical Software, Vol 23, Num. 4, pp. 550 - 560.
 * - J.L. Morales and J. Nocedal. L-BFGS-B: Remark on Algorithm 778: L-BFGS-B, FORTRAN routines for large scale bound constrained optimization (2011), to appear in ACM Transactions on Mathematical Software.
 *
 * Author: Jakob van Santen <vansanten@wisc.edu>
 */

#include <boost/foreach.hpp>
#include "gulliver/I3GulliverBase.h"

#include "gulliver/I3MinimizerResult.h"
#include "gulliver/I3FitParameterInitSpecs.h"
#include "icetray/I3SingleServiceFactory.h"
#include "minimizer/I3GulliverLBFGSB.h"
#include "minimizer/I3MinimizerUtils.h"
#include "minimizer/lbfgsb/lbfgsb.h"

I3GulliverLBFGSB::I3GulliverLBFGSB(const I3Context& context) : I3ServiceBase(context)
{
    AddParameter("Tolerance", "Terminate if the last iteration improved the minimum value "
        "by less than Tolerance. "
        "Recommended values are 1e-14/1e-8/1e-3 for high/moderate/low precision.", 1e-3);
    AddParameter("GradientTolerance", "Terminate if the magnitude of the largest gradient "
        " element (times the step size) is smaller than this.", 1e-3);
    AddParameter("MaxIterations", "Terminate if the number of function iterations "
        "exceeds this limit.", 1000);
}

I3GulliverLBFGSB::~I3GulliverLBFGSB() {}

void
I3GulliverLBFGSB::Configure()
{
    GetParameter("Tolerance", tolerance_);
    GetParameter("GradientTolerance", gradientTolerance_);
    GetParameter("MaxIterations", maxIterations_);
}

template <typename T>
std::ostream& operator<<(std::ostream &s, const std::vector<T> &v)
{
    s << "[";
    if (v.size() > 0) {
        typename std::vector<T>::const_iterator it = v.begin();
        typename std::vector<T>::const_iterator end = v.end()-1;
        for ( ; it != end; it++)
            s << *it << ", ";
        s << *it;
    }
    s << "]";

    return s;
}

I3MinimizerResult
I3GulliverLBFGSB::Minimize(I3GulliverBase &g, const std::vector<I3FitParameterInitSpecs> &parspecs )
{
    // Maximum number of terms in the approximation to the Hessian matrix
    int mmax = 10;
    // Number of dimensions
    int nmax = parspecs.size();
    // Parameter constraints
    std::vector<doublereal> upper(nmax, 0),
                            lower(nmax, 0),
                            scale(nmax, 1);
    // by default, all parameters are unbounded
    std::vector<integer> bound_type(nmax, 0);
    // Current parameter vector
    std::vector<doublereal> x(nmax);

    // Set bounds and initial values
    for (unsigned i=0; i < parspecs.size(); i++) {
        const I3FitParameterInitSpecs &par = parspecs[i];
        if (std::isfinite(par.minval_) && par.minval_ != par.maxval_) {
            lower[i] = par.minval_;
            // only lower bound
            bound_type[i] = 1;
        }
        if (std::isfinite(par.maxval_) && par.minval_ != par.maxval_) {
            upper[i] = par.maxval_;
            // both bounds if lower bound is defined, otherwise upper bound only
            bound_type[i] = (bound_type[i] == 1) ? 2 : 3;
        }
        if (std::isfinite(par.stepsize_))
            scale[i] = std::abs(par.stepsize_);
        x[i] = par.initval_;
    }

    // Disable spew (doesn't work in C port anyhow)
    integer iprint = 0;
    // Function value and gradient
    doublereal fval;
    std::vector<doublereal> fgrad(nmax);

    // Workspaces whose size depends on the problem
    std::vector<doublereal> workspace((2*mmax+5)*nmax + 11*mmax*mmax + 8*mmax);
    std::vector<integer> int_workspace(3*nmax);

    // Various constant-size workspaces
    char task[60];
    char csave[60];
    logical lsave[4];
    integer isave[44];
    doublereal dsave[29];

    memset(task, 0, 60);
    strncpy(task, "START", 5);
    bool done = false;
    I3MinimizerResult result(nmax);
#ifndef NDEBUG
    double bestval = 0;
#endif
    while (!done) {
        setulb_(&nmax, &mmax, &x[0], &lower[0], &upper[0], &bound_type[0], &scale[0], &fval, &fgrad[0],
            &tolerance_, &gradientTolerance_, &workspace[0], &int_workspace[0], task,
            &iprint, csave, lsave, isave, dsave);

        switch (task[0]) {
            case 'C': // CONVerged
                result.converged_ = true;
                result.minval_ = fval;
                result.par_ = x;
                done = true;
                break;
            case 'F': // Function and Gradient
                fval = g(x, fgrad);
#ifndef NDEBUG
                if (strncmp(task, "FG_START", 8) == 0)
                    bestval = fval;
#endif
                break;
            case 'N': // NEW value of X
                if (isave[33] > maxIterations_)
                    done = true;
                break;
            case 'A': // ABNOrmal termination
                result.minval_ = fval;
                result.par_ = x;
            default:
                done = true;
                break;
        }
#ifndef NDEBUG
        log_trace_stream("Iteration " << isave[33] << ": "<< task << " x = "<<x<<" f = " << fval << " (" <<(fval-bestval)<< ") df = "<<fgrad<<" => Linf = " << dsave[12]);
#endif
    }

    bool checkflat = false;
    for (unsigned i=0; i < parspecs.size(); i++) {
        if (result.converged_ && x[i] == parspecs[i].initval_)
            checkflat = true;
    }
    if (checkflat)
        I3MinimizerUtils::CheckMinimum(*this, result, parspecs, g);

    return result;
}

typedef
I3SingleServiceFactory<I3GulliverLBFGSB,I3MinimizerBase>
I3GulliverLBFGSBFactory;
I3_SERVICE_FACTORY( I3GulliverLBFGSBFactory );
