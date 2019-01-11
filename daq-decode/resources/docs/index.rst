.. _daq-decode:

Daq-decode Documentation
========================

**Author**: Torsten Schmidt

Decoding the DAQ event format and filling the dataclasses 
structures is the job of a DAQ decoder service.

Overview
--------

Decoding the DAQ event format and filling the dataclasses structures
is the job of a DAQ decoder service.  Several older decoders have been
retired, and only the :ref:`I3PayloadParsingEventDecoderFactory` is supported.
   
The :ref:`I3PayloadParsingEventDecoderFactory` is supplied by the payload-parsing project,
which is new approach to directly decode the DAQ
event format directly from C++.
   
Both services support eleven parameters

- **Year** defines the year the data to decode is from (default: ``2006``)
- **HeaderID** defines the name of the :ref:`I3EventHeader` added to a
  frame (default: ``I3DefaultName<I3EventHeader>::value()``)
- **TriggerID** defines the name of the :ref:`I3TriggerHierarchy` added
  to a frame (default: ``I3DefaultName<I3TriggerHierarchy>::value()``)
- **SpecialDataOMs** defines optical modules whose DOM launches
  are not assigned to any IceCube or IceTop :ref:`I3DOMLaunchSeriesMap`, but
  to the special data :ref:`I3DOMLaunchSeriesMap` instead
- **SpecialDataID** defines the name of the special data
  :ref:`I3DOMLaunchSeriesMap` added to a frame
- **InIceID** defines the name affix that is pre-appended to the
  name of any IceCube :ref:`I3DOMLaunchSeriesMap` added to a frame (default: ``InIce``)
- **IceTopID** defines the name affix that is pre-appended to the
  name of any IceTop :ref:`I3DOMLaunchSeriesMap` added to a frame (default: ``IceTop``)
- **SPEDataID** defines the name of an :ref:`I3DOMLaunchSeriesMap` that
  holds DOM launches with trigger mode ``SPE_DISCRIMINATOR_TRIGGER``
  (default: ``RawData``)
- **FlasherDataID** defines the name of an :ref:`I3DOMLaunchSeriesMap`
  that holds DOM launches with trigger mode ``FLASHER_BOARD_TRIGGER``
- **CPUDataID** defines the name of an :ref:`I3DOMLaunchSeriesMap` that
  holds DOM launches with trigger mode ``CPU_REQUESTED`` and
- **TestDataID** defines the name of an :ref:`I3DOMLaunchSeriesMap` that
  holds DOM launches with trigger mode ``TEST_PATTER``

And into a given frame might create, fill and insert:

- an :ref:`I3EventHeader`
- an :ref:`I3TriggerHierarchy`
- upto nine :ref:`I3DOMLaunchSeriesMaps`


See Also
--------

.. toctree::
   :maxdepth: 1
   
   release_notes
   Python API Reference </python/icecube.daq_decode>
   C++ API Reference </doxygen/daq-decode/index>
   IceTray Inspect Reference </inspect/daq_decode>


