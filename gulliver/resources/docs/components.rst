Gulliver components (services)
==============================

The main components are:

Event likelihood function:
      This component contains the physics description of the relation
      between a hypothesized particle (represented by an
      :class:`I3EventHypothesis` object, see :ref:`physics-variables`)
      in the ice and the detector response. For example a loop over all
      "hits", summing the logarithms of the likelihoods of those hits
      for some given track hypothesis; or a Bayesian weight.

      .. note::

        Optionally, it could also compute the gradient of
        :math:`\log(\mathcal{L})` with respect to the relevant
        variables.

      Base class: :class:`I3EventLogLikelihoodBase`.

Minimizer algorithm:
      This is a physics-unaware algorithm to find a local -- or maybe
      even global -- minimum of any function of :math:`N` floating point
      variables.

      .. note::

        It could also use the gradient of the likelihood function if the
        likelihood function implementation supports that.

      Base class: :class:`I3MinimizerBase`.

Parametrization:
      Specification of fitting variables, step sizes, bounds, and
      transformations (e.g. parametrize energy :math:`E` as
      :math:`\log(E)`). The parametrization translates between a
      physics-describing :class:`I3EventHypothesis` object and a
      vector/array of doubles: the translation in one direction is used
      to provide the minimizer with starting values from a seed track;
      in the other direction it is used to convert the parameters of a
      function evaluation by the minimizer algorithm into an
      :class:`I3Particle` object, which is then used to evaluate the
      event likelihood function. A parametrization service *can*
      (optionally) implement the chain rule to convert gradients from
      the physics variables representation to the minimizer parameters
      representation.

      Base class: :class:`I3ParametrizationBase`.

Seeding:
    A log-likelihood fit must begin from some kind of seed hypothesis
    that is hopefully not too far off from the "truth". Sometimes you
    can directly use reconstruction results from some other module;
    sometimes you need to be more creative or careful to ensure for
    instance that the time residuals are not all negative and the vertex
    point is close to the COG of the pulses (in case of a through-going
    track). First guesses should also be stored as an
    :class:`I3EventHypothesis` object, which is trivial for hypotheses
    that require only an :class:`I3Particle` but can be more involved
    for reconstructions that use the ``nonstd`` bit of
    :class:`I3EventHypothesis` (see :ref:`physics-variables`). A seed
    service takes care of all this. If you design a new kind of
    hypothesis with its own specific ways to build a seed (from data in
    the frame), then you need to implement a new seed service.

    Base class :class:`I3SeedServiceBase`.

The design of Gulliver is such that instantiations of these components
can be so-called *services* that can be instantiated and configured
independently. A service can be used by several fit modules.

In a log-likelihood fit module, instances of each of these three
components are plugged into an :class:`I3Gulliver` object. These
instances *can* be obtained from the framework as a service, but they
can also be created within the module itself. An example of the latter
is the ``paraboloid`` module in which the specification of the fitting
variables is fixed (:math:`x, y, z` coordinates of the vertex).

Design Motivations
------------------

This design has several advantages:

* When writing reconstruction modules, you do not need to copy/redo many
  of the configuration options, likelihoods and such that are
  implemented in exactly the same way in other modules; you can reuse
  the parametrizations, minimizers, and likelihood functions that are
  implemented. You can concentrate more effort on the innovating part of
  your module.
* A Gulliver component service can be used by several modules in the
  same run script. That does not only make the run script shorter, it
  enables more efficient use of resources (the service object is shared
  among the using modules).
* For many reconstruction ideas you do not want to implement a whole new
  module. For instance, if you formulate a better likelihood
  description, then you just implement a class that derives from the
  event likelihood base class and you use that class within the existing
  Gulliver-based reconstruction modules.
* When writing run scripts, the configuration of the generic components
  and the reconstruction module specific details is separated in to
  conceptually independent parts.

Additionally, the utility had to be written such that you can also use
it to implement nontrivial reconstructions, that is, reconstructions of
event types that can not be represented by only an
:class:`I3Particle object` (see :ref:`physics-variables`).

Representation of fit results
-----------------------------

.. _physics-variables:

Physics variables
^^^^^^^^^^^^^^^^^

The *physics* variables describe whatever it is in the ice that emits
light, and are relevant for computing light yields and probability
distributions functions for photon arrival times. In Gulliver modules
and services, these variables are collected in an
:class:`I3EventHypothesis` object. This simple object has two members,
namely ``particle`` (an object of class :class:`I3Particle`) and
``nonstd`` (an object of any class deriving from
:class:`I3FrameObject`).

The :class:`I3Particle` class (defined in the ``dataclasses`` project)
is a very generic class, used both for simulated particles and for
reconstructed tracks and showers. But the class is not extendible to all
possible event types and it contains only one diagnostic data member:
the fit status (whether the fit was successful or not). This is
sufficient for standard track and shower reconstructions, which make up
the bulk of reconstruction in online and offline data processing.

So only if you are interested in fitting a more complicated hypothesis
then you will be using the ``nonstd`` side of the
:class:`I3EventHypothesis`. You need to identify which "extra" physics
variables (in addition to what you can store in an :class:`I3Particle`)
are suitable to characterize the event hypothesis. You may be able to
host these extra variables using an existing class from the
``dataclasses`` project (e.g. :class:`I3Double` to add just one
variable, or :class:`I3Particle` to add a complete second particle), or
you may choose to write your own. The object with the "extra" physics
variables is stored in the ``nonstd`` member of the
:class:`I3EventHypothesis`.

Examples cases in which the event hypothesis needs a ``nonstd`` part are
reconstructions for a muon bundle, two coincident muons (see the
``double-muon`` project), or a high energy muon with many stochastic
showers (``millipede``).

If you would like to develop a new reconstruction idea, then you'll
typically try to reuse as many existing components as possible. However,
if the event hypothesis that you are trying to fit is so different that
you need to define your own new class for the ``nonstd`` part, then you
need to write implementations for three Gulliver components:
parametrization, likelihood and seeding.

Gulliver modules usually store ``particle`` and ``nonstd`` separately
into the frame, rather than storing the :class:`I3EventHypothesis`. For
vanilla reconstructions the ``nonstd`` is empty and does not need to be
stored at all.

Diagnostic variables
^^^^^^^^^^^^^^^^^^^^

*Diagnostic* variables refer to any information that is related to the
reconstruction but is not an input variable for the calculation of the
likelihood. Usually this information can be interesting for event
selection in an analysis or for studies of reconstruction algorithms.

* Gulliver itself provides a diagnostics object of class
  :class:`I3LogLikelihoodFitParams` with just four numbers:

  + The likelihood of the hypothesis with the fitted parameter values
  + The *number of degrees of freedom* as reported by the likelihood
    service.

    .. note::

      This depends on the details of the likelihood, but typically it is
      the number of hit DOMs used in the event.

  + The *reduced log-likelihood*, a quantity that you will not find in
    any serious textbook about statistics; it was invented during AMANDA
    times in analogy with *reduced chi-squared* and is defined as the
    likelihood divided by the number of degrees of freedom (so in
    principle it is even redundant).
  + The number of times that the minimizer asked Gulliver to compute the
    event likelihood. This is rarely used, but if this number is
    relatively large it could indicate that the event was difficult to
    reconstruct (e.g. unlucky seed or complicated likelihood behavior).

* Each service *can* (but does not have to) produce its own specific
  diagnostics (e.g. a minimizer service might provide its own method
  specific error information; a parametrization service might provide
  data on how much of the available parameter space was covered during
  the fit; a likelihood service might specify how many DOMs in the event
  have hits that do not at all fit to the hypothesis and are regarded as
  random noise).

For vanilla Gulliver reconstructions (Pandel, muons) performed with the
:class:`I3SimpleFitter` and :class:`I3IterativeFitter` modules, you get
just one :class:`I3Particle` object and one
:class:`I3LogLikelihoodFitParams` in the frame.

Specialized reconstruction modules such as :class:`I3ParaboloidFitter`,
which have more specific reconstruction details, such as the paraboloid
error ellipse, can derive a fit parameters class from
:class:`I3LogLikelihoodFitParams`; :class:`I3Gulliver` will take care of
the above-mentioned four variables, and the module fills in the specific
details.
