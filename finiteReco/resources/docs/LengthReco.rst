Length reconstruction of finite tracks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The energy of contained low-energy events is directly correlated to the
length of the muon track. With a reconstruction of the length one gains
therefore immediately an energy proxy. This page shows how to reconstruct
the length of a track using a chain of FiniteReco modules. Note that for
all practical purposes, the
`tray segment <http://code.icecube.wisc.edu/projects/icecube/browser/IceCube/projects/finiteReco/trunk/python/segments.py>`_
shipped with FiniteReco should be used. An example script for how to use
that segment can be found `here <example_script.html>`_. 

.. warning::

   Do not copy and paste code from this page, unless you know exactly what
   you are doing! Use the `example script <example_script.html>`_ instead!

How to use
^^^^^^^^^^

To reconstruct the length of a track with FiniteReco and gulliver, a chain
of modules is needed. Before that, a first-guess contained track is
needed. Typically this is delivered by I3StartStopPoint. Then, a few
services need to be installed::

   tray.AddService("I3PhotonicsServiceFactory", "PhotonicsService",
      PhotonicsLevel2DriverFile  = "SPICEMie_i3coords_level2_muon_resampled.list",
      PhotonicsTableSelection    = 2,
      PhotonicsTopLevelDirectory = "/data/sim/scratch/test_pt/photonics-prod/tables/SPICEMie_i3coords/",
      DriverFileDirectory        = "/data/sim/scratch/test_pt/photonics-prod/tables/SPICEMie_i3coords/driverfiles/",
      UseDummyService            = False,
      ServiceName                = "PhotonicsService"
   )

The Photonics service provides the photonics tables that will be needed to
obtain the hit probabilities. Note that FiniteReco is quite picky about
the Photonics tables. If you select the wrong tables, it will complain.
Next up are the likelihood service and the minimizer::

   tray.AddService("I3GulliverFinitePhPnhFactory", "FinitePhPnh",
      InputReadout  = "PulseSeriesMap",
      NoiseRate     = 700.0*I3Units.hertz,
      RCylinder     = 200*I3Units.m,
      ProbName      = "PhPnhPhotorec",
      PhotorecName  = "PhotonicsService"
   )

   tray.AddService("I3GulliverMinuitFactory", "minuit",
      MaxIterations = 1000,
      Tolerance     = 0.01,
      Algorithm     = "SIMPLEX"
   )

For a description of the parameters of the LLH service see the
`Calculation of Probabilities <CalculationOfProbabilities.html>`_ page.
Now comes the actual length reconstruction. It is performed in two
steps. Starting from the first guess, in the first step, the start point
is kept fixed and the length is varied until the optimal stop point is
found::

   tray.AddService("I3SimpleParametrizationFactory","simparStartVertex",
      StepLinL   = 10.0*I3Units.m,
      BoundsLinL = [0,2*I3Units.km],
      VertexMode = "Start"
   )

   tray.AddService("I3BasicSeedServiceFactory", "seedserveStartVertex",
      FirstGuess    = "Linefit_Finite",
      InputReadout  = "PulseSeriesMap",
      TimeShiftType = "TNone"
   )

   tray.AddModule("I3SimpleFitter", "LineFit_FiniteStop",
      Parametrization = "simparStartVertex",
      SeedService     = "seedserveStartVertex",
      LogLikelihood   = "FinitePhPnh",
      Minimizer       = "minuit",
      StoragePolicy   = "OnlyBestFit"
   )

In a second step, the newly found stop point is kept fixed and the start
point is varied until the best likelihood is found::

   tray.AddService("I3SimpleParametrizationFactory","simparStopVertex",
      StepLinL   = 10.0*I3Units.m,
      BoundsLinL = [0,2*I3Units.km],
      VertexMode = "Stop"
   )

   tray.AddService("I3BasicSeedServiceFactory", "seedserveStopVertex",
      FirstGuess    = "Linefit_FiniteStop",
      InputReadout  = "PulseSeriesMap",
      TimeShiftType = "TNone"
   )

   tray.AddModule("I3SimpleFitter", "LineFit_Contained",
      Parametrization = "simparStopVertex",
      SeedService     = "seedserveStopVertex",
      LogLikelihood   = "FinitePhPnh",
      Minimizer       = "minuit",
      StoragePolicy   = "OnlyBestFit"
   )


Output
^^^^^^

Since two separate fits are performed, this chain returns two new I3Particle
objects to the frame. Their names are determined by the names of the two
I3SimpleFitter modules. In the above example:

LineFit_FiniteStop :
  The result of the first fit for the stop point. This is an intermediate
  result and can be ignored or deleted from the frame.

LineFit_Contained :
  The final result of the length reconstruction.

Additionally, the fitter also puts the corresponding FitParams into the frame.
