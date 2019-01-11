Python API
==========

IceTray segments
----------------

.. automodule:: icecube.lilliput.segments
   :noindex:

.. autofunction:: I3SinglePandelFitter
   :noindex:

.. autofunction:: I3IterativePandelFitter
   :noindex:

.. autofunction:: I3ParaboloidPandelFitter
   :noindex:

.. autofunction:: add_minuit_simplex_minimizer_service
   :noindex:

.. autofunction:: add_simple_track_parametrization_service
   :noindex:

.. autofunction:: add_pandel_likelihood_service
   :noindex:

.. autofunction:: add_seed_service
   :noindex:


Minimizer
---------

.. automodule:: icecube.lilliput.scipymin
   :noindex:

.. autoclass:: SciPyMinimizer
   :members:
   :show-inheritance:
   :noindex:

.. automodule:: icecube.lilliput.i3minuit
   :noindex:

.. autoclass:: IMinuitMinimizer
   :members:
   :show-inheritance:
   :noindex:

Example
-------

Example how to use the minimizers in an IceTray script::

    # Add the standard gulliver service factories.
    tray.AddService("I3SimpleParametrizationFactory", "param", ...)
    tray.AddService("I3GulliverIPDFPandelFactory", "pandel", ...)
    tray.AddService("I3BasicSeedServiceFactory", "seed", ...)

    # Add the python class directly into the tray's context
    # (no need for a service factory).
    tray.context["iminuit"] = lilliput.IMinuitMinimizer(tolerance=0.001)

    # Now call the gulliver module in the normal way.
    tray.AddModule("I3SimpleFitter", "singlefit",
                   SeedService="seed",
                   Parametrization="param",
                   LogLikelihood="pandel",
                   Minimizer="iminuit")
