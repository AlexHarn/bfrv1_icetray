=================================================
How to implement your own parametrization service
=================================================

Overview
--------

A parametrization service acts as a translator between the
seed/likelihood services (information is an event hypothesis with
physics variables) and the minimizer (information is an array of
floating point numbers, the fit parameters). It also takes care of
translation the seed into initial fit parameter values, setting the
initial step sizes, imposing bounds on fit parameters, and performing
coordinate transformations for both the fit parameters and their
gradients.

Physics variables versus fit parameters
---------------------------------------

The type of event hypothesis usually determines which physics variables
we would like to fit. For instance, for a typical minimum ionizing
through-going muon we want to fit a position (location of the muon at
some arbitrary reference time) and a direction; the fit is characterized
by five variables (e.g.: :math:`x,y,z`, zenith, azimuth). For a high
energy cascade we probably want to know the interaction time, the
interaction vertex, direction, and energy: seven variables. For fitting
two randomly coincident through-going muons we need to fit ten
variables.

For the likelihood service, the event hypothesis is represented by an
event hypothesis consisting of an :class:`I3Particle` and optionally
some ``nonstd`` part. This representation may be convenient for the
physics calculations in the likelihood service, but the generic
minimizer does not care about physics, it treats the fit parameters just
as an array of numbers. The simplest way to translate from the physics
representations to the numerical one is to just read the variable values
(e.g. :math:`x`-, :math:`y`- and :math:`z`-components of the vertex) and
push them into a single vector. When the minimizer would like to
evaluate the function-to-be-minimized with some other vector of values,
then we need to translate the other way round, plugging the numbers
(probably slightly different from the ones we started with) back into
the event hypothesis. Some data members of the event hypothesis might
remain completely unaffected during this process (e.g. the "time" data
member in a typical through-going muon track fit). The thus updated
event hypothesis will then probably be used by the likelihood service to
compute the likelihood. This is in a nut shell what the
:class:`I3SimpleParametrizationService` (implemented in ``lilliput``)
does.

However, some variables that make sense for a physicist are not very
practical in a numerical context. For instance, the energy variable may
vary over several orders of magnitude, and numerical minimization
algorithms may work more robustly if we would use the logarithm of the
energy. Another example is the direction: for a physicist the zenith and
azimuth angles may be very useful but these variables suffer from
coordinate singularities (at the poles) and periodicity. If we are
willing to constrain to just a part of the sphere, e.g. a hemisphere,
then we can project that onto some flat surface and let the minimizer
work with Cartesian coordinates on that surface instead of the usual
polar coordinates. This is exactly what the
:class:`I3HalfSphereParametrizationService` (also implemented in
``lilliput``) does.

Of course the physics user would like to specify initial step sizes and
bounds in terms of the physics variables. A parametrization service
which performs coordinate transformations should also take care of
transforming the step sizes and bounds.

Gradients
---------

Gradients are computed (if wanted) by the likelihood service in terms of
physics variables, and may be used by a minimizer to find a function
minimum more efficiently. If the parametrization performs any coordinate
transformations, then it needs to transform the gradients as well, i.e.
it should know how to apply the chain rule.

A parametrization service may or may not support gradients. It is
important that this is documented clearly for each parametrization
service. Before actually performing any fit Gulliver will check if the
minimizer wants to use gradients, and configures the parametrization
service accordingly (see `API`_).

API
---

Quite a bit of work is done by the base class,
:class:`I3ParametrizationBase`. The implementation and API is not really
according to a traditional OO style, in particular the implementation of
the subclasses have to access directly the following data members:

``hypothesis_``
  Pointer to the current event hypothesis (:class:`I3EventHypothesis`,
  for the likelihood service)
``gradient_``
  Pointer to the current gradient (:class:`I3EventHypothesis`, for the
  likelihood service)
``parspecs_``
  The vector of fit parameter initialization specifications
  (:class:`I3ParameterInitSpec`, for the minimizer): initial value,
  initial step size, name (for log messages) and bounds (if any).
``par_``
  The vector of fit parameter values (set by the minimizer)
``par_gradient_``:
  Gradient w.r.t. fit parameters (for the minimizer)

The subclass constructor (or if the subclass also derives from
:class:`I3ServiceBase`, its ``Configure()`` method) should initialize
``parspecs_`` and ``par_``.

The two crucial methods to be implemented in the subclass are
``UpdateParameters()`` and ``UpdatePhysicsVariables()``.

The ``UpdateParameters()`` method should use the information from
``hypothesis_`` to update ``par_``. This step gets executed only at the
start of the fit and serves to initialize the fit parameters with the
seed; the base class will copy ``par_`` to the initial values of
``parspecs_``. If bounds and step sizes depend on the seed then
``UpdateParameters()`` should update those as well.

In the implementation of the ``UpdatePhysicsVariables()`` method the
``par_`` values should be used to update ``hypothesis_``.
