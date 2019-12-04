.. _gulliver-modules:

Gulliver modules
================

**Maintainer:** Alan Coleman <alanc@udel.edu>

.. toctree::
    :maxdepth: 1

    release_notes


This project hosts three basic Gulliver-based Icetray modules:

    `I3LogLikelihoodCalculator`_
        Calculates the log-likelihood for a given :class:`I3Particle`, a
        likelihood service (derived from
        :class:`I3EventLogLikelihoodBase`), and a parametrization
        service (derived from :class:`I3ParametrizationBase`).

    `I3SimpleFitter`_
        Performs a simple generic log-likelihood reconstruction.

    `I3IterativeFitter`_
        A iteration-based extension of :class:`I3SimpleFitter`.

The general steps performed by the modules are explained in
:ref:`gulliver-fit-anatomy`. Most required services can be found in
:mod:`lilliput`.

.. _I3LogLikelihoodCalculator:
    ../../doxygen/gulliver-modules/classI3LogLikelihoodCalculator.html

.. _I3SimpleFitter:
    ../../doxygen/gulliver-modules/classI3SimpleFitter.html

.. _I3IterativeFitter:
    ../../doxygen/gulliver-modules/classI3IterativeFitter.html


.. seealso::

    `Doxygen documentation <../../doxygen/gulliver-modules/index.html>`_,
    :ref:`gulliver`, :ref:`lilliput`, :ref:`double-muon`,
    :ref:`finiteReco`, :ref:`millipede-main`, :ref:`paraboloid`.


Examples
--------

Calculate the log-likelihood for an :class:`I3LineFit` and the
:class:`I3GulliverIPDFPandelFactory` likelihood service::

    tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                    InputReadout="SRTOfflinePulses",
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    AbsorptionLength=98.*icecube.icetray.I3Units.m,
                    NoiseProbability=100.*icecube.icetray.I3Units.hertz,
                    JitterTime=15.*icecube.icetray.I3Units.ns)

    tray.AddModule("I3LineFit", "lf",
                   Name="lf",
                   InputRecoPulses="SRTOfflinePulses",
                   AmpWeightPower=1.)

    tray.AddModule("I3LogLikelihoodCalculator", "lfLOGL",
                   FitName="lf",
                   LogLikelihoodService="spe1st",
                   NFreeParameters=5)

Perform a basic :class:`I3SimpleFitter` reconstruction::

    tray.AddService("I3SimpleParametrizationFactory", "xyzza",
                    StepX=20.*icecube.icetray.I3Units.m,
                    StepY=20.*icecube.icetray.I3Units.m,
                    StepZ=20.*icecube.icetray.I3Units.m,
                    StepZenith=0.1*icecube.icetray.I3Units.radian,
                    StepAzimuth=0.2*icecube.icetray.I3Units.radian,
                    BoundsX=[-2000.*icecube.icetray.I3Units.m,
                             2000.*icecube.icetray.I3Units.m],
                    BoundsY=[-2000.*icecube.icetray.I3Units.m,
                             2000.*icecube.icetray.I3Units.m],
                    BoundsZ=[-2000.*icecube.icetray.I3Units.m,
                             2000.*icecube.icetray.I3Units.m])

    tray.AddService("I3GulliverMinuitFactory", "minuit",
                    Algorithm="SIMPLEX")

    tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                    InputReadout="SRTOfflinePulses",
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                    JitterTime=15.*icecube.icetray.I3Units.ns)

    tray.AddService("I3BasicSeedServiceFactory", "linefitseed",
                    InputReadout="SRTOfflinePulses",
                    TimeShiftType="TFirst",
                    FirstGuesses=["linefit"])

    tray.AddModule("I3LineFit", "linefit",
                   Name="linefit",
                   InputRecoPulses="SRTOfflinePulses",
                   AmpWeightPower=1.)

    tray.AddModule("I3SimpleFitter", "spefit",
                   SeedService="linefitseed",
                   Parametrization="xyzza",
                   LogLikelihood="spe1st",
                   Minimizer="minuit")

If you want to do an iterative fit instead, simple replace
:class:`I3SimpleFitter` with :class:`I3IterativeFitter`::

    tray.AddModule("I3IterativeFitter", "spefit",
                   SeedService="linefitseed",
                   RandomService="I3RandomService",
                   Parametrization="xyzza",
                   LogLikelihood="spe1st",
                   Minimizer="minuit")

The :mod:`lilliput` project provides IceTray segments for standard
vanilla reconstructions performed at standard processing.


Gulliview
---------

.. automodule:: icecube.gulliver_modules.gulliview
   :noindex:

.. autoclass:: GulliView
   :noindex:

    **Module parameters**:

    .. autoattribute:: LogLikelihood
       :noindex:
    .. autoattribute:: Parametrization
       :noindex:
    .. autoattribute:: SeedService
       :noindex:
    .. autoattribute:: WithGradients
       :noindex:
    .. autoattribute:: NSteps
       :noindex:
    .. autoattribute:: StepSize
       :noindex:
    .. autoattribute:: Filename
       :noindex:

The following example shows the basic usage of :class:`GulliView`::

    tray.Add("I3LineFit",
             Name="linefit",
             InputRecoPulses="SRTOfflinePulses",
             AmpWeightPower=1.)

    mininame, paraname, llhname, seedname, name = tray.Add(
        icecube.lilliput.I3SinglePandelFitter, "PandelFit",
        pulses="SRTOfflinePulses",
        domllh="SPE1st",
        seeds=["linefit"])

    seeder = icecube.lilliput.add_seed_service(
        tray, "SRTOfflinePulses", ["PandelFit"])

    tray.Add(icecube.gulliver_modules.gulliview.GulliView,
             SeedService=seeder,
             Parametrization=paraname,
             LogLikelihood=llhname,
             Filename=None)
