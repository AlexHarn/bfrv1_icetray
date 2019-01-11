.. 
.. copyright  (C) 2011
.. The Icecube Collaboration
.. 
.. $Id$
.. 
.. @version $Revision$
.. @date $LastChangedDate$
.. @author Fabian Kislat <fabian.kislat@desy.de> $LastChangedBy$

.. _tpx-main:

tpx
================

Modules for IceTop pulse extraction and calibration.

This project is maintained by Timo Karg <timo.karg@desy.de>.

.. toctree::
   :titlesonly:
   
   release_notes

I3TopHLCPulseExtractor
----------------------

Extract pulses from HLC waveforms and calibrate to PE and/or VEM.

The pulse charge is determined by integrating the whole waveform.
Pulse time is defined by the "leading edge", i.e. the slope between the bins
where 10 and 90% of the first peak are reached extrapolated to the baseline.
The trailing edge of the pulse is defined as the point where the waveform
drops below 10% of the first pulse for the last time.
This is used to calculate the pulse width.
Furthermore the rise time of the 10-90% slope and the pulse amplitude are
calculated.

Parameters
^^^^^^^^^^

``Waveforms``
   Calibrated input waveforms.
   Use ``I3WaveformSplitter`` from :ref:`WaveCalibrator-main` to extract HLC ATWD
   waveforms from WaveCalibrator output.
``PEPulses``
   Name of the ``I3RecoPulseSeriesMap`` with pulses calibrated in PE.
   Set to an empty string if you do not need these pulses.
   They are still calculated, however, because they are the bases of the VEM
   pulses.
``VEMPulses``
   Name of the pulses calibrated in VEM.
   Set to an empty string if you do not need them.
   In that case they are never determined (e.g. in vemcal).
``PulseInfo``
   Pulse info objects.
   These contain additional information that cannot be stored in
   :cpp:class:`I3RecoPulse`\ s, like amplitude and rise-time.
   Writes and :cpp:class:`I3TopPulseInfoSeriesMap` to the frame.
``BadDomList``
   A list of bad DOMs.
   All waveforms from DOMs on this list are ignored.

I3TopSLCPulseExtractor
----------------------

Treats SLC charge stamps.

Parameters
^^^^^^^^^^

``Waveforms``
   Calibrated input waveforms.
   Use ``I3WaveformSplitter`` from :ref:`WaveCalibrator-main` to extract HLC ATWD
   waveforms from WaveCalibrator output.
``PEPulses``
   Name of the ``I3RecoPulseSeriesMap`` with pulses calibrated in PE.
   Set to an empty string if you do not need these pulses.
   They are still calculated, however, because they are the bases of the VEM
   pulses.
``VEMPulses``
   Name of the pulses calibrated in VEM.
   Set to an empty string if you do not need them.
   In that case they are never determined (e.g. in vemcal).
``BadDomList``
   A list of bad DOMs.
   All waveforms from DOMs on this list are ignored.

I3IceTopBaselineModule
----------------------

Calculates baseline parameters from a given range of ATWD/FADC bins for monitoring:

* Average value of waveform. Can be used to monitor baseline subtraction.

* Slope of waveform in given range (simple linear fit). Can be used to monitor droop correction.

* Waveform RMS in range. Should be small if there are no pulses in given range.

Parameters
^^^^^^^^^^

``Waveforms``
   Calibrated input waveforms. Ideally, you should use all waveforms output by
   :ref:`WaveCalibrator-main`, don't use ``WaveformSplitter``.
``BaselineRange``
   Range of bins used for baseline calibration, specified as a list of two bins.
   Numbers are inclusive, i.e. the last bin given will be included.
   Negative values are counted from the end of the waveform with -1 corresponding to the last bin.
``Source``
   An I3Waveform.Source such as I3Waveform.ATWD (default) or I3Waveform.FADC.
``Output``
   Name of the output. The module writes an :cpp:class:`I3IceTopBaselineSeriesMap` to the frame.


I3VEMConverter
--------------

Converts :cpp:class:`I3RecoPulse`\ s from PE to VEM. Use this module to calibrate pulses transferred as
superdst. Pulse time and all other properties of the pulses will remain unchaged.

Parameters
^^^^^^^^^^

``PEPulses``
   Input :cpp:class:`I3RecoPulseSeriesMap` containing pulses with charge in units of PE.
``VEMPulses``
   Output :cpp:class:`I3RecoPulseSeriesMap` with the same pulses calibrated in VEM.

I3IceTopSLCCalibrator
---------------------

I3Module to apply a correction to SLC charges. The correction is applied to the already VEM-calibrated pulses (the VEM calibration constants are correct for HLCs).

Parameters
^^^^^^^^^^

``SLCPulses``
   ID of input calibrated SLC pulse list (this gets replaced by default)
``SLCPulsesOut``
   ID of output calibrated SLC pulse list. If it is not specified, it is set equal to SLCPulses. If an object with this name is present, it will be replaced.
``InputWaveforms``
   Name of input I3WaveformSeriesMap (required only to know the ATWD channel)
``Config``
   Configuration file with the parameters for each OM/chip/ATWD

CalibrateSLCs
-------------

   Segment to calibrate SLC pulses. The current implementation of SLC calibration is done after VEM
   calibration. It needs to know the ATWD channel used and therefore
   needs access to the waveforms generated by
   I3WaveformSplitter. Since these are discarded, they have to be
   produced again (hence this segment).

   This segment also includes 'merging' the SLC pulses into the tank
   pulses container.

Parameters
^^^^^^^^^^

``Launches``
   Input: raw data
``SLCVEMPulses``
   Input: name of the ``I3RecoPulseSeriesMap`` with pulses calibrated in VEM. The output calibrated pulses have the same name, with 'Calibrated' appended.
``SLCTankPulses``
   Output: name of the ``I3RecoPulseSeriesMap`` with pulses calibrated in VEM. One per tank per launch.


Classes
-------

.. cpp:class:: I3IceTopBaselineSeriesMap

   A typedef for an :cpp:class:`I3Map\<OMKey, std::vector\<I3IceTopBaseline> >` storing
   :cpp:class:`I3IceTopBaseline` information about waveforms.

.. cpp:class:: I3IceTopBaseline

   I3IceTopBaseline stores information about the IceTop-style baseline of a waveform.
   These parameters are calculated by I3IceTopBaselineModule.

  .. cpp:member:: I3Waveform::Source source

     Waveform source (ATWD or FADC)

  .. cpp:member:: uint8_t channel

     ATWD channel, undefined in case of FADC

  .. cpp:member:: uint8_t sourceID

     ATWD chip ID, undefined in case of FADC

  .. cpp:member:: float baseline

     Average baseline (voltage)

  .. cpp:member:: float slope

     Baseline slope (voltage/time)

  .. cpp:member:: float rms

     Variation of waveform baseline (voltage)


.. cpp:class:: I3TopPulseInfoSeriesMap

   A typedef for :cpp:class:`I3Map\<OMKey, std::vector\<I3TopPulseInfo> >` storing
   :cpp:class:`I3TopPulseInfo` objects.


.. cpp:class:: I3TopPulseInfo

   Stores additional parameters of an IceTop pulse.

   .. cpp:enum:: Status

      Pulse status. Can be OK, Saturated, BadCharge, BadTime.

   .. cpp:member:: double amplitude

      Amplitude of the waveform (voltage)

   .. cpp:member:: double risetime

      Risetime of the first peak of the waveform

   .. cpp:member:: double trailingEdge

      Trailing edge of the waveform

   .. cpp:member:: Status status

      Waveform status

   .. cpp:member:: uint8_t channel

      ATWD channel

   .. cpp:member:: uint8_t sourceID

      ATWD Chip, from :cpp:func:`I3Waveform::GetSourceIndex()`



`Generated doxygen for this project <../../doxygen/tpx/index.html>`_
