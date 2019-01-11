.. 
.. copyright  (C) 2011
.. The Icecube Collaboration
.. 
.. $Id$
.. 
.. @version $Revision$
.. @date $LastChangedDate$
.. @author Fabian Kislat <fabian.kislat@desy.de> $LastChangedBy$

.. _topeventcleaning-main:

topeventcleaning
================

IceTray modules for IceTop event cleaning and splitting.

.. toctree::
   :maxdepth: 1
   
   release_notes


.. _I3HLCTankPulseMerger:

I3HLCTankPulseMerger
--------------------

Merges high-gain (HG) and low-gain (LG) pulses inside a tank.
If the charge in a high-gain pulse surpasses the saturation value obtained from 
VEM calibration (*HGLGCrossOver*), the charge from the best matching LG pulse is 
used.
The best-matching LG pulse is found by looking for the LG pulse that is closest
in time to the HG pulse.
If the best-matching LG time differs from the HG by more than a configurable
threshold (``MaxHGLGTimeDiff``), the LG pulse is discarded and the HG charge is
set to NAN (not-a-number).
The time is always taken from the HG pulse.

Parameters
^^^^^^^^^^

``InputVEMPulses``
   An input ``I3RecoPulseSeriesMap`` calibrated to VEM.
   This parameter must be set to a non-empty value.
``OutputTankPulses``
   Output ``I3RecoPulseSeriesMap`` containing the tank pulses.
   This parameter must be set to a non-empty value, and correspond to a frame
   object that does not yet exist.
``MaxHGLGTimeDiff``
   Maximum time difference for matching HG and LG pulses. Defaults to 40ns.
``BadDomList``
   Name of the frame object that contains a list of bad DOMs (e.g. obtained 
   from the DB).
   Specify an empty string to not use such a list.
``BadTankList``
   Name of the frame object that contains a list of bad tanks.
   Specify empty string to not use such a list.
``ExcludedTanks``
   Tanks excluded by this module because there were LG pulses without HG
   partners will be written to the frame with this name.
   If set to an empty string this list will not be written out, but a warning is
   displayed.
``Stream``
   Type of stream to be processed. The default is DAQ.


I3TopHLCClusterCleaning
-----------------------

Splits events that are separated in time.
Tank pulses these are ordered in time and events are split if subsequent pulses
do not fulfill the following condition:

\|Δt\| < \|Δ\ **x**\|/c + t\ :subscript:`tol`,

where t\ :subscript:`tol` is a time tolerance (``InterStationTimeTolerance``).
A similar condition is imposed within the two tanks in a station
(``IntraStationTimeTolerance``).
Split events are written to one or more new *P* frames.

Parameters
^^^^^^^^^^

``InputPulses``
   Input ``I3RecoPulseSeriesMap``. This should consist of merged tank pulses
   created by :ref:`I3HLCTankPulseMerger`.
``OutputPulses``
   Name of the output pulses in the *P* frame.
   The module actually creates ``I3RecoPulseSeriesMapMask``\ s.
   These, however, can be handled by all subsequent reconstruction modules
   without changes, because they are handled correctly by ``I3Frame``.
``BadTankList``
   List of bad tanks provided by previous modules.
``ExcludedTanks``
   List of tanks excluded by this module.
   For backward compatibility. Currently, a copy of ``BadTankList`` will be written;
   no additional tanks will be excluded.
``InterStationTimeTolerance``
   The t\ :subscript:`tol` in the formula above.
   Defaults to 200ns.
``IntraStationTimeTolerance``
   Equivalent of ``InterStationTimeTolerance`` when comparing pulses within one station.
   Defaults to 200ns.


KeepOnlyLargestEvent
--------------------

Module that discards all but the largest sub-event after splitting.

.. note:: This module is implemented in python.

This is how you use it::

  from icecube.topeventcleaning.modules import KeepOnlyLargestEvent
  
  tray.AddModule(KeepOnlyLargestEvent, 'keep_only_larges_event',
                 Pulses = 'NameOfPulseMaskInFrame'
                 )

Parameters
^^^^^^^^^^

``Pulses``
   ``I3RecoPulseSeriesMapMask`` in the *P* frames.
   Will be extracted from all sub-events of a *Q* frame.
   Only the *P* frame with the largest number of pulses will be kept.
