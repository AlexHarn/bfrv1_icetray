import numpy
import scipy.optimize

from icecube import icetray, gulliver


class SciPyMinimizer(gulliver.I3Minimizer):
    """Multi-dimensional minimizers in SciPy

    Parameters
    ----------
    name : str
        Name for minimizer service
    method : str
        Name of minimization method
    tolerance : float
        Tolerance for minimization termination
    max_iterations : int
        Maximum number of iterations
    options : dict
        Options that are passed to the minimizer; see `scipy.optimize`.

    """
    # Tuple specifies if the minimizer uses gradients/Hessians.
    methods = {
        "Nelder-Mead": (False, False),
        "Powell": (False, False),
        "CG": (True, False),
        "BFGS": (True, False),
        "Newton-CG": (True, True),
        "Anneal": (False, False),
        "L-BFGS-B": (True, False),
        "TNC": (True, False),
        "COBYLA": (False, False),
        "SLSQP": (True, False),
        "dogleg": (True, True),
        "trust-ncg": (True, True)
        }

    def __init__(self, name, method="L-BFGS-B", tolerance=1e7,
                 max_iterations=100, options=None):
        super(SciPyMinimizer, self).__init__()

        if method not in self.methods:
            raise ValueError(
                "Unknown algorithm {0}.".format(method))

        if self.methods[method][1]:
            raise ValueError(
                "{0} uses Hessians, which we don't know how to "
                "calculate.".format(method))

        self.name = name
        self.method = method
        self.tolerance = tolerance
        self.max_iterations = max_iterations
        self.options = options

    def GetName(self):
        return self.name

    def UsesGradient(self):
        return self.methods[self.method][0]

    def Minimize(self, llh, parspecs):
        icetray.logging.log_trace(
            "{0} is going to minimize...".format(self.name),
            unit="SciPyMinimizer")

        if self.method in ["L-BFGS-B", "TNC", "SLSQP"]:
            bounds = []
        else:
            bounds = None

        seeds = []

        for spec in parspecs:
            seeds.append(spec.initval)

            if bounds is None:
                continue

            parbounds = [None, None]

            if spec.minval != spec.maxval:
                if numpy.isfinite(spec.minval):
                    parbounds[0] = spec.minval

                if numpy.isfinite(spec.maxval):
                    parbounds[1] = spec.maxval

            bounds.append(parbounds)

        def evaluate(x, grad):
            pars = icetray.vector_double()
            pars.extend(x)

            icetray.logging.log_trace(
                "{0}: call llh with grad={1}.".format(self.name, grad),
                unit="SciPyMinimizer")

            if grad:
                value, grads = llh(pars, grad)
                grads = numpy.array(grads)

                icetray.logging.log_trace(
                    "Pars: {0}, LLH: {1}, grads: {2}".format(x, value, grads),
                    unit="SciPyMinimizer")

                return grads
            else:
                value = llh(pars, grad)

                icetray.logging.log_trace(
                    "Pars: {0}, LLH: {1}".format(x, value),
                    unit="SciPyMinimizer")

                return value

        def fmin(x):
            return evaluate(x, grad=False)

        def fgrad(x):
            return evaluate(x, grad=True)

        if self.UsesGradient():
            jac = fgrad
        else:
            jac = None

        result = scipy.optimize.minimize(
            fmin, seeds, method=self.method, jac=jac, bounds=bounds,
            tol=self.tolerance, options=self.options)

        minresult = gulliver.I3MinimizerResult(len(seeds))
        minresult.converged = bool(result.success)
        minresult.minval = float(result.fun)

        for i, x in enumerate(result.x):
            minresult.par[i] = x

        return minresult
