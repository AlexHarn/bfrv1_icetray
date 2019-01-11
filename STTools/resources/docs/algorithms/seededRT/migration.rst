.. _STTools_algorithms_seededRT_migration:

Migrating old script for using STTools
======================================

This section shows how to migrate old scripts, that use the old
*SeededRTCleaning* project, to use the STTools project instead.

Assuming the old script contains a ``tray.AddModule()`` statement that looks
similar to this:

.. sourcecode:: python

    tray.AddModule("I3SeededRTHitCleaningModule<I3DOMLaunch>", "hit_cleaning",
        InputResponse    = "SLC_InIceRawData",
        OutputResponse   = "SLC_RT_InIceRawData",
        RTRadius         = 150,  #default
        RTTime           = 1000, #default
        DeepCoreRTRadius = 90,   #default = -1: same as RTRadius
        DeepCoreRTTime   = 700,  #default = -1: same as RTTime
        MaxIterations    = -1,   #default = -1: infinite
        Seeds            = "HLC" #default
    )

The script snippet above would do a seededRT cleaning of DOM launches with
different RT settings for IceCube and DeepCore DOMs. The seed hits would be all
HLC DOM launches.

In order to do the same thing but using STTools, the script snippet above would
have to be change to:

.. sourcecode:: python

    # Create a ST configuration service that will be used by the icetray module.
    # Note: IceCube-DeepCore DOM links will get the DeepCore-DeepCore DOM link
    #       ST configuration automatically, if not specified seperately.
    stConfigService = STTools.seededRT.configuration_services.I3DOMLinkSeededRTConfigurationService(
        ic_ic_RTRadius =  150*I3Units.m,
        ic_ic_RTTime   = 1000*I3Units.ns,
        dc_dc_RTRadius =   90*I3Units.m,
        dc_dc_RTTime   =  700*I3Units.ns,
        treat_string_36_as_deepcore = False,
        useDustlayerCorrection      = False,
        allowSelfCoincidence        = True
    )

    tray.AddModule("I3SeededRTCleaning_DOMLaunch_Module", "hit_cleaning",
        InputHitSeriesMapName  = "SLC_InIceRawData",
        OutputHitSeriesMapName = "SLC_RT_InIceRawData",
        STConfigService        = stConfigService,
        MaxNIterations         = -1,
        SeedProcedure          = "AllHLCHits",
        Streams                = [icetray.I3Frame.DAQ]
    )

It is worth noting, that once a ST configuration service has been created (i.e.
the ``stConfigService`` object in the example above) it can be used for several
seededRT hit cleaning modules. For a full list of options for constructing the
DOM link SeededRT configuration service see section
:ref:`STTools_algorithms_seededRT_I3DOMLinkSeededRTConfigurationService`.

In cases, where I3RecoPulses rather than DOMLaunches are going to be seededRT
cleaned, the ``I3SeededRTCleaning_RecoPulse_Module`` or
``I3SeededRTCleaning_RecoPulseMask_Module`` must be used instead (see also
:ref:`STTools_algorithms_seededRT_I3SeededRTCleaningModule`).
