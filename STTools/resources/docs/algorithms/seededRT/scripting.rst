.. _STTools_algorithms_seededRT_scripting:

Python Scripting Interface
==========================

The seededRT algorithm of STTools provides a Python scripting interface for
seededRT hit cleaning without icetray. This feature is an automatic (positive)
result from the utility function approach used in STTools.

The ``seededRT`` Python module of STTools provides (based on the intended
output type in analogy with the
:ref:`STTools_algorithms_seededRT_I3SeededRTCleaningModule` icetray module)
three Python functions for doing seededRT hit cleaning in a Python script or
console:

.. module:: icecube.STTools.seededRT

.. autofunction:: doSeededRTCleaning_RecoPulse
   :noindex:
.. autofunction:: doSeededRTCleaning_RecoPulseMask
   :noindex:
.. autofunction:: doSeededRTCleaning_DOMLaunch
   :noindex:

Seed Procedure Classes
----------------------

The function described above take objects of seed procedure classes, which
define the hit seed procedure. The following seed procedure classes exists:

.. _seed_with_all_HLC_hits_I3RecoPulse:
.. autoclass:: seed_with_all_HLC_hits_I3RecoPulse
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_all_HLC_hits_I3DOMLaunch:
.. autoclass:: seed_with_all_HLC_hits_I3DOMLaunch
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_all_core_hits_I3RecoPulse:
.. autoclass:: seed_with_all_core_hits_I3RecoPulse
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_all_core_hits_I3DOMLaunch:
.. autoclass:: seed_with_all_core_hits_I3DOMLaunch
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_HLC_core_hits_I3RecoPulse:
.. autoclass:: seed_with_HLC_core_hits_I3RecoPulse
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_HLC_core_hits_I3DOMLaunch:
.. autoclass:: seed_with_HLC_core_hits_I3DOMLaunch
   :noindex:

    .. automethod:: __init__
       :noindex:

.. .. _seed_with_HLC_COG_ST_hits_I3RecoPulse:
.. .. autoclass:: seed_with_HLC_COG_ST_hits_I3RecoPulse
..
..     .. automethod:: __init__

.. .. _seed_with_HLC_COG_ST_hits_I3DOMLaunch:
.. .. autoclass:: seed_with_HLC_COG_ST_hits_I3DOMLaunch
..
..     .. automethod:: __init__

.. _seed_with_OMKey_hits_I3RecoPulse:
.. autoclass:: seed_with_OMKey_hits_I3RecoPulse
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_OMKey_hits_I3DOMLaunch:
.. autoclass:: seed_with_OMKey_hits_I3DOMLaunch
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_Nth_OMKey_hits_I3RecoPulse:
.. autoclass:: seed_with_Nth_OMKey_hits_I3RecoPulse
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_Nth_OMKey_hits_I3DOMLaunch:
.. autoclass:: seed_with_Nth_OMKey_hits_I3DOMLaunch
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_hit_series_map_hits_from_frame_I3RecoPulse:
.. autoclass:: seed_with_hit_series_map_hits_from_frame_I3RecoPulse
   :noindex:

    .. automethod:: __init__
       :noindex:

.. _seed_with_hit_series_map_hits_from_frame_I3DOMLaunch:
.. autoclass:: seed_with_hit_series_map_hits_from_frame_I3DOMLaunch
   :noindex:

    .. automethod:: __init__
       :noindex:

Example
-------

An example Python script can be found at
``$I3_BUILD/STTools/resources/examples/SeededRTCleaningScripting.py``.

Four steps need to be performed in order to do a seededRT hit cleaning:

#. Create a ST configuration service object.
#. Setup the spatial context of the ST configuration object using an I3OMGeo
   object.
#. Create a seed procedure object.
#. Finally, call the seededRT hit cleaning utility function passing it the ST
   configuration service object, the seed procedure object, an I3Frame object,
   the name of the hit map inside that frame object, and optional parameters.

.. sourcecode:: python

    from icecube import STTools

    # Create the ST configuration service using default values.
    stConfigService = STTools.seededRT.configuration_services.I3DOMLinkSeededRTConfigurationService()

    # Setup the spatial context for the detector geometry.
    stConfigService.SetupSContext(frame['I3Geometry'].omgeo)

    seedProcedure = STTools.seededRT.seed_with_HLC_core_hits_I3RecoPulse(
        stConfigService = stConfigService,
        nHitsThreshold  = 2,
        allowNoSeedHits = False)

    srtCleanedPulseMap = STTools.seededRT.doSeededRTCleaning_RecoPulse(
        stConfigService       = stConfigService,
        seedProcedure         = seedProcedure,
        frame                 = frame,
        inputHitSeriesMapName = "OfflinePulses",
        maxNIterations        = 0,    # Optional, default is 0.
        allowNoSeedHits       = False # Optional, default is False.
    )

    # Put the cleaned pulse map into the frame.
    frame['MySRTCleanedPulseMap'] = srtCleanedPulseMap
