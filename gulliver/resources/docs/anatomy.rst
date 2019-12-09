.. _gulliver-fit-anatomy:

Anatomy of a Gulliver fit
=========================

Dramatis Personae
-----------------

* **module**: a Gulliver-using IceTray module; usually one of:

  + :class:`I3SimpleFitter`
  + :class:`I3IterativeFitter`
  + :class:`I3ParaboloidFitter`

* **seed service**: object of a class deriving from
  :class:`I3SeedServiceBase`
* **parametrization service**: object of a class deriving from
  :class:`I3ParametrizationBase`
* **likelihood service**: object of a class deriving from
  :class:`I3EventLogLikelihoodBase`
* **minimizer service**: object of a class deriving from
  :class:`I3MinimizerBase`
* **gulliver**: object of type :class:`I3Gulliver` (derives from
  :class:`I3GulliverBase`); data member of **module**

Act I: Configuration
--------------------

1. ``tray`` configuration

2. ``service`` instantiation and configuration

  * By *services* we mean:

    + **seed service**
    + **parametrization service**
    + **likelihood service**
    + **minimizer service**

  * Services can be created in the traditional way with a
    *service factory* added to the ``tray``.
  * Services can be also created as independent python objects, to be
    passed directly as argument to the corresponding service
    configuration parameter in the Gulliver module(s).

3. Module instantiation and configuration: an :class:`I3Gulliver` object
   is created and linked with 3 services from the context:

  * **likelihood service**
  * **parametrization service**
  * **minimizer service**

  .. note::

    The **seed service** is *not* fed to **gulliver**, it stays
    separate. 

Act II: Getting Ready For The Fit
---------------------------------

0. The likelihood service is told to run ``SetGeometry`` on the G-Frame

#. **module** arrives at the P-Frame.
#. **module** gives event data (P-frame) to the **seed service** via 
   ``SetEvent(frame)`` and asks the **seed service** how many seeds are available.
#. **module** starts loop over all available seeds.
#. **module** provides seed (``I3EventHypothesis``) and physics frame to **gulliver**.
#. **gulliver** gives event data (P-frame) to likelihood; gets the
   *multiplicity* :math:`N_{\rm mult}`.
#. **gulliver** gives event data (P-frame) to parametrization; gets
   *number of free parameters* :math:`N_{\rm par}`.
#. **gulliver** computes number of degrees of freedom
   :math:`{\rm NDF} = N_{\rm mult} - N_{\rm par}`.
#. :math:`{\rm NDF} \leq 0`:
   **gulliver** decides that the fit ``FAILs``.
#. Check if minimizer wants to use gradient:

   * **likelihood service** should be able to deliver gradients.
   * **parametrization service** should be able to compute chain rules.

   .. note::

     Minimizer parameters are not necessarily the same as the physics
     variables; e.g. :math:`\log(E)` instead of :math:`E`, or Cartesian
     direction parametrization instead of polar.

#. **gulliver** gives the seed hypothesis to the
   **parametrization service**; this updates the
   *current event hypothesis*.
#. **parametrization service** returns parameter initialization
   specifications for the fitting parameters:

   * Initial values of free parameters
   * Step sizes of free parameters
   * Bounds, if any (0 for both bounds is *unbounded*)

#. **gulliver** passes the specifications to the **minimizer**.

#. Minimization begins and iterates over the following for each *step*:

    #. The **minimizer** uses the specifications to generate a set of parameter
       values.
    #. The **minimizer** asks **gulliver** for the function value
       :math:`-\log(\mathcal{L})` corresponding to these parameter values.
    #. **gulliver** gives the parameter values to the
       **parametrization service**.
    #. **parametrization service** updates the current event hypothesis
       using the new parameter values.
    #. **gulliver** passes the current event hypothesis to the
       **likelihood service**.
    #. **likelihood service** computes :math:`+\log(\mathcal{L})`.
    #. **gulliver** returns :math:`-\log(\mathcal{L})` to the minimizer.
    #. If the minimizer wants to use gradients:

       * **gulliver** asks **likelihood service** to compute gradient.
       * **gulliver** asks **parametrization service** to apply chain rule.
       * **gulliver** returns also the parameter gradient to the minimizer.

    #. The minimizer uses function values (and gradient) to either:

       * Generate a new set of parameter values.
       * Decide that no minimum can be found (``FAIL``).
       * Decide that it has found a minimum (``SUCCESS``).

#. The minimizer returns a :class:`I3MinimizerResult` object with

   * A boolean that indicates convergence.
   * The parameter values of the best fit.
   * A pointer to an algorithm-specific object with *diagnostics*.

#. **gulliver** passes the best fit parameter values to the
   **parametrization service**.
#. **parametrization service** updates the current event hypothesis.
#. **gulliver** gives the current event hypothesis to the
   **likelihood service**; this is the best fit event hypothesis.
#. **likelihood service** computes the *best likelihood*.
#. **gulliver** returns to **module**:

   * Convergence (boolean)
   * ``logl`` = best likelihood
   * ``rlogl = logl/ndof`` = *reduced likelihood*
   * ``nmini`` = number of likelihood values computed for the minimizer
   * ``ndof`` = :math:`{\rm NDF}`; useful for failed fits, otherwise
     redundant.
   * Best fit event hypothesis
   * Minimizer-specific diagnostics, if any
   * Likelihood-specific diagnostics, if any
   * Parametrization-specific diagnostics, if any

#. **module** may or may not repeat all steps for other seeds.
#. **module** derive other quantities from the **gulliver** fit results.
#. **module** stores some or all of the results in the frame.
