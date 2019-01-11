.. _STTools_algorithms_seededRT_icetray_modules:

Icetray Modules
===============

The following icetray modules, utilizing the seededRT ST algorithm, are
provided:

- I3SeededRTCleaningModule
- I3RTVetoModule

These icetray modules are derived from the :ref:`STTools_framework_I3STModule`
icetray module and are C++ class templates, so they can be used for different
hit types and output types.

.. _STTools_algorithms_seededRT_I3SeededRTCleaningModule:

I3SeededRTCleaningModule
------------------------

This icetray module provides the seededRT hit cleaning functionality.
This module template comes with three different hit-output-type
specializations:

- **I3SeededRTCleaning_RecoPulse_Module**: Takes an I3RecoPulseSeriesMap or
  I3RecoPulseSeriesMapMask as input and outputs I3RecoPulseSeriesMaps.
- **I3SeededRTCleaning_RecoPulseMask_Module**: Takes an I3RecoPulseSeriesMap
  or I3RecoPulseSeriesMapMask as input and outputs I3RecoPulseSeriesMapMasks.
- **I3SeededRTCleaning_DOMLaunch_Module**: Takes an I3DOMLaunchSeriesMap as
  input and outputs I3DOMLaunchSeriesMaps.

Parameters
^^^^^^^^^^

The I3SeededRTCleaningModule extends the list of parameters of the
:ref:`STTools_framework_I3STModule` icetray module with seededRT hit cleaning
specific parameters:

- **SeedProcedure**: Possible string values are:
  ``"AllHLCHits"``, ``"AllCoreHits"``, ``"HLCCoreHits"``,
  ``"OMKeyHits"``, ``"FirstOMKeyHits"``, ``"HitSeriesMapHitsFromFrame"``
- **NHitsThreshold**
- **AllowNoSeedHits**
- **SeedOMKeyList**
- **SeedHitSeriesMapName**
- **MaxNIterations**
- **OutputDiscardedHLCHitSeriesMapName**
- **OutputDiscardedSLCHitSeriesMapName**

To see all available parameters and their description type into your env-shell::

    icetray-inspect STTools

Example
^^^^^^^

.. sourcecode:: python

    from icecube import icetray, STTools
    from icecube.icetray import I3Units
    from I3Tray import I3Tray

    # Create a ST configuration service that will be used by the icetray module.
    stConfigService = STTools.seededRT.configuration_services.I3DOMLinkSeededRTConfigurationService(
        allowSelfCoincidence    = False,            # Default: False.
        useDustlayerCorrection  = True,             # Default: True.
        dustlayerUpperZBoundary =    0*I3Units.m,   # Default: 0m.
        dustlayerLowerZBoundary = -150*I3Units.m,   # Default: -150m.
        ic_ic_RTTime            = 1000*I3Units.ns,  # Default: 1000m.
        ic_ic_RTRadius          =  150*I3Units.m    # Default: 150m.
    )

    tray = I3Tray()

    # ...

    tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", "MySeededRTCleaning",
        InputHitSeriesMapName  = "OfflinePulses",
        OutputHitSeriesMapName = "MyCleanedPulses",
        STConfigService        = stConfigService,
        SeedProcedure          = "HLCCoreHits",
        NHitsThreshold         = 2,
        Streams                = [icetray.I3Frame.DAQ]
    )

    # ...

    tray.Execute()
    

I3RTVetoModule
--------------

This is a special module that utilizes the seededRT algorithm. For each hit of
the input hit map, a new seededRT cleaning is performed with this hit as the
seed. The cleaned map with the most hits is then selected and written to the
frame. This allows to find a cluster in a given input hit map. If for example
an I3RecoPulseSeriesMap is split into a DeepCore pulse map and a "rest of
IceCube" (veto) pulse map, an event may be discarded if the size of the cluster
inside the veto pulse map is above a certain threshold.

This module template comes with three different hit-output-type
specializations:

- **I3RTVeto_RecoPulse_Module**: Takes an I3RecoPulseSeriesMap or
  I3RecoPulseSeriesMapMask as input and outputs an I3RecoPulseSeriesMap.
- **I3RTVeto_RecoPulseMask_Module**: Takes an I3RecoPulseSeriesMap or
  I3RecoPulseSeriesMapMask as input and outputs an I3RecoPulseSeriesMapMask.
- **I3RTVeto_DOMLaunch_Module**: Takes an I3DOMLaunchSeriesMap as
  input and outputs an I3DOMLaunchSeriesMap.

Parameters
^^^^^^^^^^

This module adds no extra parameters to the already existing
:ref:`STTools_framework_I3STModule` parameters.
