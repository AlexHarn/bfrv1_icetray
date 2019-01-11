Starting/Stopping Likelihood
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The idea here is to create a parameter to check whether a track is starting and/or stopping within the detector. The method has to be seeded with a finite muon track, reconstructed typically by the `I3StartStopPoint <I3StartStopPoint.html>`_ module. The likelihood values for a starting and stopping assumption are calculated and compared. While in principle different likelihood functions can be used, the chosen one has to depend on the positions of the start and stop point of the muon. FiniteReco comes with a likelihood function based on the hit distribution inside the detector, implemented in `I3GulliverFinitePhPnhFactory <CalculationOfProbabilities.html>`_, which has this property and is used here.

How to use
^^^^^^^^^^

The calculations are implemented within the module :cpp:class:`I3StartStopLProb`::

   tray.AddModule("I3StartStopLProbPoint","FiniteRecoStartStopLLH", 
      Name        = "LineFit_Finite",
      ServiceName = "FinitePhPnh"
   )

* ``Name``: name of the input I3Particle
* ``ServiceName``: name of a likelihood service that is sensitive to the length of the track (typically provided by `I3GulliverFinitePhPnhFactory <CalculationOfProbabilities.html>`_)

Output
^^^^^^

The module returns an I3StartStopParams object with the name ``"Name"_StartStopParams``" to the frame. This object is a container for three quantities:

* ``LLHInfTrack``: log-likelihood value assuming an infinite muon track
* ``LLHStartingTrack``: log-likelihood value assuming no track before the seeded start point.
* ``LLHStoppingTrack``: log-likelihood value assuming no track after the seeded stop point.

The difference of ``LLHStartingTrack`` (``LLHStoppingTrack``) and ``LLHInfTrack`` is a measure for the probability of the track to be starting (stopping). Note that only for contained tracks all three values are returned. For a starting (stopping) track, ``LLHStoppingTrack`` (``LLHStartingTrack``) is NAN.
