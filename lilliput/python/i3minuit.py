from numpy import isfinite
from icecube import icetray, gulliver

try:
    import iminuit
except ImportError:
    iminuit = None


class IMinuitMinimizer(gulliver.I3Minimizer):
    """Wrapper for the iminuit minimization algorithm for use with the
    Gulliver suite.

    It is intended to be a drop-in replacement for the ROOT-based
    minimizers :js:data:`I3GulliverMinuitFactory` and
    :js:data:`I3GulliverMinuit2Factory` found in the :ref:`Gulliver`
    suite. `iminuit`_ is a stand alone Python package which, contains
    SEAL Minuit, the original port of Minuit to C++. Comparted to the
    standalone Minuit2, it is relativly well maintained and unlike the
    version which comes with ROOT, it does not print anything to stdout
    when you set the print level to zero.

    If you don not have iminuit, it can be insalled very easily using
    the following command: ``pip install iminuit``

    Parameters
    ----------
    name : string, optional
        String for the Gulliver module to identify the minimizer
    tolerance: float, optional
        Tolerance for finding the minimum
    MaxIterations: int, optional
        Maximum number of iterations to perform before giving up
    MinuitPrintLevel: int, optional
        Set print level: 0 = quiet, 1 = normal, 2 = paranoid,
        3 = really paranoid
    MinuitStrategy: int,optional
        0 = fast, 1 = default, 2 = slow but accurate

    See also
    --------
    Further reading: `Wikipedia`_, `iminuit documentation`_,
    `Minuit user's guide`_, `Minuit CERN page`_, `Minuit paper`_

    .. _iminuit:
        https://github.com/iminuit/iminuit

    .. _Wikipedia:
        https://en.wikipedia.org/wiki/MINUIT

    .. _iminuit documentation:
        http://iminuit.readthedocs.org/en/latest/

    .. _Minuit user's guide:
        http://seal.web.cern.ch/seal/documents/minuit/mnusersguide.pdf

    .. _Minuit CERN page:
        https: www.cern.ch/minuit

    .. _Minuit paper:
        http://dx.doi.org/10.1016/0010-4655(75)90039-9

    """
    def __init__(self, name="iminuit", Tolerance=0.001, MaxIterations=1000,
                 MinuitStrategy=2, MinuitPrintLevel=0,):
        super(IMinuitMinimizer, self).__init__()

        if not iminuit:
            icetray.logging.log_fatal(
                "IMinuitMinimizer requires the python package iminuit, which "
                "does not appear to be installed. You can install it "
                "with pip.", unit="IMinuit")

        self.name = name
        self.tolerance = Tolerance
        self.max_iterations = MaxIterations
        self.print_level = MinuitPrintLevel
        self.strategy = MinuitStrategy

    def GetName(self):
        return self.name

    def UsesGradient(self):
        return False

    def Minimize(self, llh, parspecs):
        # Define a function that iminuit can understand which llh.
        def minfunc(*vals):
            # Convert vals into the data format that llh wants.
            vec = icetray.vector_double()
            vec.extend(vals)

            # Call the log-liklihood function.
            return llh(vec, False)

        # List of the names of parameters to the log-liklihood function
        param_names = []
        # Additional arguments to Minuit
        minuit_args = {}

        # Loop over the all of the parameters givin to us by gulliver and
        # translate that information into a format that iminuit can understand.
        for spec in parspecs:

            # Add the parameter name to the list of parameter names.
            param_names.append(spec.name)

            # Set the initial value of the parameter.
            minuit_args[spec.name] = spec.initval

            # Set the stepsize of the parameter.
            minuit_args["error_" + spec.name] = spec.stepsize

            # If this parameter has any boundries set those as well.
            if (spec.minval != spec.maxval and
                    isfinite(spec.minval) and
                    isfinite(spec.maxval)):
                minuit_args["limit_" + spec.name] = (spec.minval, spec.maxval)

        icetray.logging.log_debug(
            "%s: begining minimization with parameters %r %r" % (
                self.name, param_names, minuit_args),
            unit="IMinuit")

        # Setup the minimization with minuit.
        minimizer = iminuit.Minuit(minfunc,
                                   print_level=self.print_level,
                                   errordef=0.5,
                                   forced_parameters=param_names,
                                   **minuit_args)

        minimizer.tol = self.tolerance
        minimizer.set_strategy(self.strategy)

        # Perform the actual minimization.
        minimizer.migrad(ncall=self.max_iterations)

        icetray.logging.log_debug(
            "%s: migrad finished after %d calls with OK=%d with params=%r "
            "and fval=%r" % (
                self.name, minimizer.ncalls, minimizer.migrad_ok(),
                minimizer.values, minimizer.fval),
            unit="IMinuit")

        # Create a data object that gulliver can understand.
        result = gulliver.I3MinimizerResult(len(param_names))

        # Tell it if the minimizer converged on a real minimum.
        result.converged = minimizer.migrad_ok()

        # Tell it the value of the log-lilkihood at the minimum.
        result.minval = minimizer.fval

        # Tell it the value of each of the parameaters of the minimum.
        for i, x in enumerate(param_names):
            result.par[i] = minimizer.values[x]

        return result
