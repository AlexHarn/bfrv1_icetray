.. _gulliver-bootstrap:

Gulliver-Bootstrapping
======================

A tool to apply the bootstrapping technique to Gulliver-based fits.
This is useful e.g. to extract an estimation of the angular uncertainty
of track reconstructions.

.. toctree::
    :maxdepth: 1

    release_notes


This project hosts two Gulliver-based Icetray modules:

* :class:`BootstrappingLikelihoodService`
	Takes an existing likelihood service and feeds it with
	resampled pulses.

* :class:`BootstrappingSeedService`
	Uses the original best fit (before resampling) as the seed
	for each iteration of the resampling.

A third Icetray module is:

* :class:`BootstrapSeedTweak`
	Calculation of a given containment radius,
	in which a certain fraction of the bootstrapped fits falls.
	The result can serve as an estimator of angular resolution.

.. seealso::
    `gulliver <../gulliver/index.html>`_,
    `lilliput <../lilliput/index.html>`_,
    `paraboloid <../paraboloid/index.html>`_.


Background
----------

Bootstrapping is a statistical method
aimed at providing estimates of distributions of statistics
in cases where the underlying theoretical distribution is unknown.
It does so by employing random resampling with replacement to the given dataset.

The method was first described by B. Efron in
`Bootstrap Methods: Another Look at the Jackknife <https://dx.doi.org/10.1214%2Faos%2F1176344552>`_.

Given a sample :math:`X_0 = (x_1, \dots, x_n)` defining the
empirical distribution function :math:`\hat{F}`: to the theoretical distribution :math:`F`,
the approach generates the bootstrap samples :math:`X_b = (x_1^*, \dots, x_n^*), b=1 \dots B`
by resampling with replacement :math:`n` times from the given distribution :math:`X_0`.

The distribution of the statistical measure of interest :math:`T(F)` is then approximated
by :math:`T_b=T(X_b)`.

In the case of IceCube, this approach can be used to obtain an estimate for the angular error
of a track reconstruction.
In the language used before, :math:`T` corresponds to the direction of the track
and :math:`x` can be understood as the pulses seen in the detector from a given track.
Modelling the detector behaviour,
it is possible to choose between resampling the pulses from a Poisson or Multinomial distribution.
From the empirical distribution of fits after resampling
it is finally possible to obtain e.g.
the desired containment radius as a measure of the angular uncertainty.

Parameters
----------

* :class:`BootstrappingLikelihoodService`
	- ``Pulses``: original set of pulses, that was also used for the initial track fit.
	- ``Bootstrapping``: The type of resampling to use. Options are Multinomial (default) and Poisson.
	- ``Iterations``: The number of resamplings to perform. Set to 4 by default.
		Increasing to 8 yields narrower pull distributions at the expense of longer runtime.
	- ``RandomService``: The random service to use for the resampling.
	- ``WrappedLikelihood``: The likelihood service used in the initial track fit.

* :class:`BootstrappingSeedService`
    - ``WrappedSeed``: A seed service pointing to the result of the initial track fit.
    - ``BootstrappingLikelihood``: The name of the aforementioned likelihood service.

* :class:`BootstrapSeedTweak`
    - ``BootstrappedRecos``: The name of the output of the fitter.
    - ``ContainmentLevel``: The size of the desired containment radius. Default: 50%.
    - ``AngularError``: Where to store the angular size corresponding to the containment radius.


Application
-----------

The general sequence of applying this method is as follows.

1. Do the initial track fit (like SPE, MPE, SplineMPE, etc.).
2. Create a :class:`BootstrappingLikelihoodService` and a :class:`BootstrappingSeedService`
   pointing to the pulses and and likelihood configuration used in step 1.
3. Use an :class:`I3SimpleFitter` to fit each set of resampled pulses provided
   by the classes from step 2.
   Set the output to ``AllFitsAndFitParams`` so that the result from each iteration is
   accessible afterwards.
4. Use the :class:`BootstrapSeedTweak` to calculate the desired containment radius
   given the output of step 3.

Example
-------

The following code shows just the application of the bootstrapping itself.
It assumes that you have done a track fit (``firstguess``) before::

    mininame = lilliput.segments.add_minuit_simplex_minimizer_service(tray)
    paraname = lilliput.segments.add_simple_track_parametrization_service(tray)

    pulses = "CleanedMuonPulses"        # <- The pulses used in the initial fit
    firstguess = "OnlineL2_SplineMPE"   # <- The name of the initial fit
    firstllh = "SplineMPEllh"           # <- The name of the llh of the initial fit

    tray.AddService("I3BasicSeedServiceFactory", "MySplineMPESeed",
            FirstGuesses =                 [ firstguess ],
            ChargeFraction =               0.9,                      # Default
            FixedEnergy =                  float( "nan" ),           # Default
            MaxMeanTimeResidual =          1000.0*I3Units.ns,        # Default
            NChEnergyGuessPolynomial =     [],                       # Default
            SpeedPolice =                  True,                     # Default
            AddAlternatives =              "None",                   # Default
            OnlyAlternatives =             False                     # Default
            )

    if 'I3RandomService' not in tray.tray_info.factories_in_order:
        tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")

    tray.AddService("BootstrappingLikelihoodServiceFactory", "MyBootstrapLLH",
            Pulses            = pulses,
            Bootstrapping     = gulliver_bootstrap.BootstrapOption.Multinomial,
            Iterations        = 8,
            WrappedLikelihood = firstllh,
            RandomService     = "I3RandomService"
            )

    tray.AddService("BootstrappingSeedServiceFactory", "MyBootstrapSeed",
            WrappedSeed             = "MySplineMPESeed",
            BootstrappingLikelihood = "MyBootstrapLLH"
            )

    tray.AddModule("I3SimpleFitter", "SplineMPE_Bootstrap",
            SeedService       = "MyBootstrapSeed",
            Parametrization   = paraname,
            LogLikelihood     = "MyBootstrapLLH",
            Minimizer         = mininame,
            StoragePolicy     = "AllFitsAndFitParams",
            If = If
            )

    tray.AddModule("BootstrapSeedTweak", "MyBootstrapTweakSeeds",
            BootstrappedRecos = "SplineMPE_BootstrapVect",
            ContainmentLevel  = 0.5,
            AngularError      = "SplineMPE_Bootstrap_Angular",
            If = If
            )

For a more thorough example, take a look at the example script provided
along with the project.
