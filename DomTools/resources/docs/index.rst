.. _DomTools:

DomTools
========

**Maintainer:** Alex Olivas

.. toctree::
   :maxdepth: 1
   
   release_notes

The Modules
~~~~~~~~~~~

* :cpp:class:`I3IsolatedHitsCutModule` - This selection module rejects all responses (I3RecoHit, I3RecoPulse, I3MCHit or I3DOMLaunch, see I3OMSelection) which are temporal and/or spatially isolated.
* :cpp:class:`I3FirstPulsifier` - This module takes a regular pulse series map as input and returns a pulse series map that has only the first pulses in it.
* :cpp:class:`I3LCCleaning` - This module splits an I3DOMLaunchSeriesMap into two maps.  One of which contains only HLC launches, the other of which contains only SLC launches.
* :cpp:class:`I3DOMLaunchCleaning` - Cleans out launches according to user define input like "CleanedKeys" and "CleanedKeysList" which are lists of OMKeys (the latter being a frame object).  This is mostly for cleaning out bad DOMs.
* :cpp:class:`I3TimeWindowCleaning` - For a user defined time window this module maximizes the number of hits in that time window and cleans out any outside hits.
* :cpp:class:`I3OMSelection` - Seems to perform a similar task to I3DOMLaunchCleaning except it has instantiatons for both I3DOMLaunches and I3RecoPulses, though it's not used for I3DOMLaunches.
* :cpp:class:`I3Coincify` -  This module applies Hard Local Coincidence to all time-like responses : I3RecoPulse, I3RecoHit, I3MCHit, and I3DOMLaunch.


