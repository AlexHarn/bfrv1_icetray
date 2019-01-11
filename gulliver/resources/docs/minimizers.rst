===========================================
How to implement your own minimizer service
===========================================

Overview
--------

With *minimizer* we mean a generic minimizer algorithm, that is, given a
function :math:`f(x)` of :math:`n` parameters, starting values :math:`x
= (x_{1},...,x_{n})` and initial step sizes, the algorithm will try to
find a set of numerical values for :math:`x` that consists of a close
enough approximation of a *local* minimum for :math:`f`, which is
hopefully also the *global* minimum. (See below in the Tolerance_
section for a discussion of what "close enough" might mean.)

For example, if we can set bounds for all :math:`n` parameters, then we
could simply fill the :math:`n`-dimensional parameter space uniformly
with a dense grid of points, evaluate :math:`f` on all those points, and
pick the one at which the function value is lowest. For the level of
precision that we usually aim for, this "brute force" approach is
usually computationally prohibitive, and we use algorithms that try to
be "smart" and we rely on "seeds" (starting values) that are obtained
with some "first-guess" method that hopefully does a reasonably good job
at picking a phase space close enough to the likelihood function
extremum.

Implementations of minimization algorithms are available from a variety
of libraries (e.g. GSL, ROOT, :mod:`scipy`), which all come with
different interfaces. In Gulliver, yet another minimizer interface is
defined. Most minimizers currently available with Gulliver are actually
*wrappers* for the external implementations.

If you would like to add a wrapper to yet another minimizer, or maybe
implement your own method, then this documentation is for you.

Language choice
---------------

The job of implementing a (wrapper of some external) minimizer for use
with Gulliver amounts to implementing a subclass of the
:class:`I3MinimizerBase` base class.

You can do this in C/C++ or in Python. Pick one.

C/C++
  Implement a subclass of `I3MinimizerBase`_. For inspiration you might
  have a look at an existing implementation, such as the GSL simplex
  wrapper: the `header`_ and `implementation`_.

Python
  In "principle", the base class is defined in lines 186-225 and 262-268
  of Gulliver's `ServiceWrappers.cxx`_; however, it may be more helpful
  to have a look at an example such as the `scipy minimizer wrapper`_ in
  the ``lilliput`` project (that project was actually originally meant
  exclusively for minimizer implementations).

.. _I3MinimizerBase:
    http://code.icecube.wisc.edu/svn/projects/gulliver/trunk/public/gulliver/I3MinimizerBase.h

.. _header:
    http://code.icecube.wisc.edu/projects/svn/projects/lilliput/trunk/public/lilliput/minimizer/I3GSLSimplex.h

.. _implementation:
    http://code.icecube.wisc.edu/projects/svn/projects/lilliput/trunk/private/lilliput/minimizer/I3GSLSimplex.cxx

.. _ServiceWrappers.cxx:
    http://code.icecube.wisc.edu/svn/projects/gulliver/trunk/private/pybindings/ServiceWrappers.cxx

.. _scipy minimizer wrapper:
    http://code.icecube.wisc.edu/svn/projects/lilliput/trunk/python/__init__.py

Tolerance
---------

There does not seem to be a universally adopted definition of
"tolerance" for a minimizer, so you can pick your own. The Gulliver
interface assumes that it can be represented by a single number, that's
all. In the documentation and example scripts for your minimizer
wrapper, you should inform your users about its meaning/definition (if
any) and some reasonable values for your definition of the tolerance.

One kind of tolerance refers to the estimated distance of the
*function value* (with the current best set of parameter values) to the
absolute minimum function value. In Minuit, Minuit2, and ROOT this is
called "EDM" or "Estimated Distance to Minimum".

Another kind of tolerance refers to the estimated distance of the
(current best set of) *parameter values* to the parameter values for the
absolute minimum. For example, `Nelder and Mead's simplex minimizer`_
characterizes this with the "size of the simplex". In an
:math:`N`-dimensional phase space, a "simplex" is the volume enclosed by
a set of :math:`N+1` points and usually the "size" of the simplex is
defined as the maximum of the distances of these :math:`N+1` points to
the center of gravity of this simplex.

.. note::

  This assumes that a given numerical distance has roughly the same
  significance for all dimensions in phase space and that is why in the
  GSL simplex wrapper the values of the fit parameters are divided by
  the step sizes.

Since Gulliver does not know which definition of "tolerance" is used, it
will not check that the result presented by the minimizer is indeed
within the configured tolerance. However, see Convergence_.

.. _Nelder and Mead's simplex minimizer:
    http://en.wikipedia.org/wiki/Nelder%E2%80%93Mead_method

Maximum number of iterations
----------------------------

Resources are finite, so it must be possible to put a limit to how long
your minimizer keeps on searching. Like with "tolerance" there is no
universal definition of "iteration", but it should be strongly
correlated with the number of likelihood function evaluations. It is up
to the algorithm/wrapper to enforce this limit; Gulliver does not do
that. Gulliver does count the number of likelihood evaluations and
exports this information as the ``nmini`` data member of the
:class:`I3LogLikelihoodFitParams` output object.

Convergence
-----------

Most minimizer algorithms claim they "converged" if they managed to
find, using less than the maximum number of iterations, a point in the
allowed phase space at which the gradient is practically zero and in an
environment around this point the function values are larger or equal to
the function value in that point. There is a special case where the
definition is kind of ambiguous, namely when the function in the said
environment is perfectly constant. In other lingo, the "likelihood
landscape" is "flat". There may be scientific applications in which it
is acceptable or even desirable to label such solutions as "converged",
but fitting the parameters of an event hypothesis for an IceCube event
is not one of them. If the likelihood landscape near the solution is
flat, then we want to consider the fit as "not converged" or "failed",
because most likely the minimizer got lost in a part of phase space
where all recorded hits in the array look like random noise hits.

For this reason most minimizer wrappers for Gulliver use the "flatness
check", a utility function implemented in the
:cpp:any:`I3MinimizerUtils` namespace (which is private in ``lilliput``,
at the time of writing these docs, but it should really move to
``gulliver`` public), to accept or reject a converged minimizer result.

Local versus global minimum
---------------------------

Ideally the minimizer will find the "global minimum", that is, no other
point in the allowed phase space has a lower function value. However,
some minimizers (e.g. simplex) will just find the local minimum that is
closest to the first guess. For fast fits (online filtering) that is
good enough, and for "easy events" (obvious down-going muons or textbook
neutrino events) the likelihood function has only one real minimum.
However, there are also many tricky cases and in higher level analysis
we want to try harder to make sure that we really get the fit with the
best likelihood.

There are several approaches towards trying to find the global minimum.
The method most commonly used in IceCube involves the "iterative fit":
after performing a regular fit, we randomly change the direction of the
fit track or shower, and then we use that as a new seed for a new fit to
the same event. On the result of this new fit we again randomize the
direction, providing the seed for the next iteration. So after 1 regular
fit and :math:`N` iterations we have :math:`N+1` results. If the
likelihood function has multiple local minima then hopefully at least
one of these :math:`N+1` results points close to the global minimum.

Another method is to employ a minimizer algorithm that aims to do better
than finding local minima, for instance the so-called "MultiNest"
algorithm [Feroz]_. This minimizer is used for the low energy "hybrid
reconstruction" and it would be interesting if someone could make a
Gulliver wrapper for this minimization method. However, this algorithm
typically needs a very large number of function evaluations.

Gradients
---------

Some minimizers may use gradients. Not all log-likelihood services
provide gradients, and not all parametrization services support
gradients.

.. note::

  If the parametrization service performs any coordinate transformations
  then it should apply the chain rule in order to get the correct
  gradient.

But if an analytical gradient is available, then this can sometimes speed
up the minimization substantially.

The documentation for a minimizer service should clearly state whether
it will use the gradient or not. The minimizer service base class also
has a virtual method ``UsesGradient()`` and before attempting a fit
:class:`I3Gulliver` will check that all services are in agreement with
each other about the question of whether gradients will be used or not.

Initialization (start of the run)
---------------------------------

Check the `IceTray documentation <../icetray/services.html>`_ on how to
write services: how to deal with a *context*, add/get configuration
parameters (not to be confused with "fit parameters"), and all that, so
that your wrapper class can be "added" to a ``tray`` and you can
configure it. Studying the code of other minimizer wrappers can be
helpful, but try to understand what you are doing and why, and avoid the
"cargo cult" style of coding.

Minimization
------------

The minimization is requested by Gulliver by invoking the ``Minimize``
method, providing a :class:`I3GulliverBase` reference (actually just a
reference to itself!) and a vector of "fit parameter initialization
specifications" (:class:`I3ParameterInitSpecs`) as input. The fit
parameter initialization specifications specify (for each fittable
parameter):

* **initial value**: seed value
* **name**: e.g. for logging messages
* **step size**: users sometimes confuse the "step size" with
  "tolerance". It is important to realize that these are two very
  different things, because with too small step sizes the minimizer will
  very easily get stuck in local minima. Usually parameters have some
  reasonable range of values; as a rule of thumb the step size should be
  about 5-10% of that range.
* **bounds** (if any):
  Gulliver specifies the bounds with a lower and upper limit
  (inclusive). If these values are identical, then the implication is
  that there are no bounds. It can happen that some parameters have
  bounds while others are boundless. For a parameter with bounds, the
  parametrization service should make sure that the initial value is at
  least one step size away from either bound, and the parametrization
  service may call ``log_fatal`` if the minimizer suggests a value that
  is outside of the given bounds.

Some minimizers perform better with bounds, others cannot be used
without them. Recommendations for bounds should be clearly specified in
the documentation of your minimizer wrapper.

The function values (and gradients, if available and desired) can be
evaluated through the :class:`I3GulliverBase` functor. This functor
really is actually just the :class:`I3Gulliver` object that can talk to
both the parametrization and the likelihood service. In order to request
function values, call the functor object with a vector of parameter
values. The length of the vector must of course be exactly as long as
the length of fit parameter specifications, and obey bounds (if any).
Parameter values should also never be ``NAN``. If gradients are desired
(and supported), then call the functor with a second vector of the same
length; after the call that array should be filled with the values of
the partial derivatives corresponding to each parameter.

Result
------

The result of the minimization should be reported through the return
value, an object of the struct-like type :class:`I3MinimizerResult`:

``converged_`` (boolean)
    This is the most basic data member. You should claim convergence
    only if the algorithm claimed it did converge **and** the likelihood
    landscape around the solution is *not* perfectly flat.
``minval_``
    Keep this ``NAN`` if the minimizer did not converge. Otherwise: set
    it to the function value at the minimum.
``par_``
    Parameter values which yielded the found minimum.
``err_``
    This data member is *obsolete*, Gulliver completely ignores whatever
    you store here. Many minimizer do not report uncertainty values.
    Uncertainty estimates should be reported in the diagnostics data
    member.
``diagnostics_``
    This is where uncertainty and other algorithm-specific information
    should be made available. You can use any data type you like, as
    long as it derives from :class:`I3FrameObject`.

References:

.. [Feroz]
  Feroz et al. "Importance Nested Sampling and the MultiNest Algorithm"
  In arXiv: http://arxiv.org/abs/1306.2144 (2013)
