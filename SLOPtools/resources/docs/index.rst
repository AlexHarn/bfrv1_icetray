.. 
.. copyright  (C) 2012
.. The Icecube Collaboration
.. 
.. $Id$
.. 
.. @version $Revision: $
.. @date $Date$
.. @author Emanuel Jacobi, Last changeed by $LastChangedBy$

.. _SLOPtools-main:

SLOPtools
=========

Collection of scripts used to process the SLOP triggered data.

.. toctree::
   :maxdepth: 1
   
   release_notes

.. _TupleTagger:

TupleTagger
-----------

This module identifies the tuples of the SlowMonopoleTrigger from the pulses. It is an implementation of the SLOP trigger algorithm in Python.
Since the DAQ cannot write information about the tuples to the i3 files from pole, this module helps to reconstruct them.
The module can operate on DOMLaunches and or RecoPulses. If it operates on RecoPulses it is not guarenteed that the algorithm will find the same tuples as the trigger, since pulses cannot be converted loss free back to launches. But it's launches on what the DAQ is acting on.
When running on launches the tuples should be exactly the ones the DAQ has calculated.
The module can run on both, pulses and launches at the same time, to make comparison studies.


Parameters
^^^^^^^^^^

``PulseMapName``
	Name of the pulse series map or mask this module should act on.

``LaunchMapName``
	Name of the launch series map this module should act on.

``RunOnPulses``
    Whether the algorithm should act on pulses.

``RunOnLaunches``
    Whether the algorithm should act on launches.

.. _MPCleaner:

MPCleaner
---------

The MPCleaner, multi pulse cleaner or monopole cleaner (read it as you like), cleans SLOP triggered data for further processing.

The velocities for which the SLOP trigger has been optimized are in the range between :math:`\beta = [10^{-2}, 10^{-4}]`. Therefore a particle is in the :math:`20\,m` radius around a DOM for a time between :math:`13\,\mu s` to :math:`1.3\,ms`. Slowly moving particles are hence expected to cause subsequent launches in the same DOM.

The cleaning module writes all pulses to a PulseMapMask for DOMs which have seen multiple launches. An additional PulseMapMask is written for all DOMs which have seen multiple launches within a (configurable) time window.
Since even dim monopole tracks with :math:`\lambda = 1\,m` make multiple launches within a radius of more than :math:`50\,m` further PulseMaps are written, with the constraint that the neighbouring or next-to-neighbouring DOM on the same string has seen the minimum number of subsequent launches as well.

Output
^^^^^^
``MPClean``
    DOMs with `MinNumberOfPulses` per event

``SuperClean``
    DOMs with `MinNumberOfPulses` per event in the time interval between `MinTimeBetweenPulses` and `MaxTimeBetweenPulses`

``HyperClean``
    DOMs with `MinNumberOfPulses` per event which have neighbors fulfilling the same requirement

``UltraClean``
    DOMs with `MinNumberOfPulses` per event in the time interval between `MinTimeBetweenPulses` and `MaxTimeBetweenPulses` which have neighbors fulfilling the same requirement per event



Parameters
^^^^^^^^^^

``PulseMapName``
	Name of the pulse series map or mask this module should act on.

``MinTimeBetweenPulses``
    Minimum time between two pulses in one DOM.

``MaxTimeBetweenPulses``
    Maximum time between two pulses in one DOM.

``MinNumberOfPulses``
    Minimum number of pulses in one DOM.


