.. 
.. copyright  (C) 2012
.. The Icecube Collaboration
.. 
.. $Id$
.. 
.. @version $Revision: $
.. @date $Date: 2012-10-18 16:01:43 +0200 (Do, 18. Okt 2012) $
.. @author $LastChangedBy: zierke $

.. _KalmanFilter-main:

KalmanFilter
============

.. toctree::
   :maxdepth: 1
   
   release_notes

KalmanFilter Module
-------------------

The Kalman filter is a set of mathematical equations that provides an efficient computational way to estimate the state of a process and can incorporate process noise and track evolution.
This Kalman filter module was developed for the tracking of subrelativistic magnatic monopoles.
The Kalman filter is used for filtering a given pulse series map or mask.
In the following process all pulses are taken into account, which are closer than *CutSize* to the estimated position.



Parameters
^^^^^^^^^^

``InputMapName``
    Name of the pulse series map or mask this module should act on.

``InputTrack``
    Name of the I3Particle used for seeding the Kalman filter.

``OutputTrack``
    Name of the resulting Track.

``CutRadius``
    Radius of the sphere around the estimated positions.

``Iterations``
    Iterations per event.

``NoiseQ``
    Initial value for the process noise Q.

``NoiseR``
    Initial value for the observation noise R.

``IterationMethod``
    Method used for the calculation of the resulting track.

    * 1 - Use last estimation as track

    * 2 (default) - LineFit fitted to the selected pulses

    * 3 - LineFit fitted to the estimations of the Kalman filter

``UseAcceleration``
    Tries to estimate the track by an accelerated movement.

``AdditionalInformation``
    Additional output


.. _MyKalmanSeed:

MyKalmanSeed
------------

Creates a Linefit for seeding the Kalman filter based on the given cleaned pulse series map or mask.

Parameters
^^^^^^^^^^

``InputMapName``
    Name of the pulse series map or mask this module should act on.

``OutputTrack``
    Name of the resulting LineFit.
