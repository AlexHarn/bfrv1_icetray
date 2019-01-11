The project IceHive
=========================


Overview
^^^^^^^^

The projects contains the two major modules::
  * IceHive, an advanced event splitter
  * HiveCleaning, an hit-cleaning removing isolated pulses

It is supported by the universal libraries::
  * OMKeyHash, a way to compactly hash OMKeys and construct fast acces look-up tables
  * HitSorting, a way to cache PulseMaps into ordered vectors of compact objects
  * Hive-lib, a library which can describe the hexagonal structured detector and provide some additional inforamtion about strings geometry
  * TriggerSplitter, a break-out from the project under same name with generalized functionality

The novelty with IceHive is that the detector is described by hexagonal cylinders (honey combs) which build up the detector. In the final detector geometry thereby around each string surrounding rings of strings can are defining, so called rings.


General UseCase
^^^^^^^^^^^^^^^

The project has a general usecase for any incident that one desires to perform lowest level data treatment in Pulses. This expecitally includes its use in basprocessing.
The event splitter module HiveSplitter is able to select from a series Pulses these which are clustering. Thereby it is able to separate Physics from Noise Pulses. Simulataniously this implements the splitting of coincident events by the same algorithm. This algorithm performs in general fast and dependently even in high noise environments. The algorithm scales linearly with the number of injeted Pulses to work off, so even processing of *very* long events is possible.

If there is demand for an even higher work-rate the IceHive module supports the use of TriggerSplitter routines in order to preselect time-frames in the input Pulses where most likely Physics Pulses are located. The work-load on the HiveSplitter is thereby reduced. This path of action is advisable if the input pulses are dominated in time-occupancy and absolute numbers by NoisePulses. However if permitted by the applied performance the solemn use of IceHive should be given the advantage.

Additionaly HiveCleaning can support the further useage of the processed data when reconstruction and simmlar should applied.


Hits vs. Pulses
^^^^^^^^^^^^^^^

Within this project terms ``Pulse`` and ``Hit`` will be used quite often and in a not very differentiated way. In principle both terms refer to the occurence of a physical response in instrument after a certain physical incident. In IceCube where we have the DOMs as instruments this holds true and the physical incident can always be rooted back to a photon hitting the photocathode, generating photoelectron, electron avalange, photocurrent, digitization, pulse-templating. So the terms Hits and Pulse just refer to the stage at which the corresponding physical response from DOM is processed. However, both still stand for the same concept and within this common concept they are used in the context of this project.
Thereby a Hit or Pulse is just the information of a certain *DOM* processing a signal with arbitray characteristics at a certain *time*.

Furthermore the term ``PulseSeries`` should refer to any ordered series of Pulses in arbitray format;
and the term ``PulseSeriesMap`` referrs to the  storeage represenation in I3-format, which is a ordered map of DOMs and the ordered series of Pulses which were registered at them.


Templating
^^^^^^^^^^

IceHive is fully templated, which means that it should in principle be able to run with any kind of input PulseSeriesMap. The only requirement is that the object has the signature ``I3Map<OMKey,vector<Response>>``, where Response is the defining Hit/Pulse I3Object.
All to do is to specify the desired Hit/Pulse object to the I3Module in the icetray, for example::
  
  from icecube import IceHive
  tray.AddModule("IecHive<I3RecoPulse>", "IceHive")
  
The standart output of the IceHive-modules are bit-masks of the same type as the specified PulseSeriesMap.

WARNING: However, because icetray currently implements only the specialized ``I3RecoPulseSeriesMapMask`` the module has been restricted to use of ``<I3RecoPulse>`` as template-argument only.

