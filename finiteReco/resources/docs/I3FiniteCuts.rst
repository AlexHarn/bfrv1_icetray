FiniteCuts
~~~~~~~~~~

This modules implements some pattern-based cut parameters that can help to identify finite tracks in an event selection.

How to use
^^^^^^^^^^

The calculations are implemented in the module :cpp:class:`I3FiniteCutsModule`::

   tray.AddModule("I3FiniteCutsModule","FiniteCuts", 
      Name            = "LineFit",
      InputRecoPulses = "PulseSeriesMap",
      CylinderRadius  = 200*I3Units.m
   )

* ``Name``: name of the input I3Particle
* ``InputRecoPulses``: input pulse series map
* ``CylinderRadius``: only DOMs inside this radius are used for the calculations

Output
^^^^^^

The module returns an I3FiniteCuts object with the name ``"Name"_FiniteCuts``" to the frame. This object is a container for several cut parameters:

* ``Sdet``: A parameter similar to a smoothness. It depends on the distribution of the Cherenkov light emission points for the given hits along the track.
* ``finiteCut``: Sum of the signed distances between the middle of an assumed infinite track and the Cherenkov emission points on the track corresponding to the given hits. The sum is normalized by the number of hits.
* ``Length``: The estimated length of the track.
* ``Lend``: Distance between the stop point of the track and the border of the detector. As border of the detector the last potential Cherenkov emission point on the track is used. Light emitted up to this point can reach a DOM without scattering.
* ``Lstart``: Similar to Lend, but for the start point.
