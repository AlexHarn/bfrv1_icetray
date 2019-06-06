I3DSTModule
============

Introduction
^^^^^^^^^^^^
I3DSTModule Collects the DST information from reconstructed I3Particles as well as
DOMLaunchSeries and HitSeries/PulseSeries and I3TriggerHierarchy and with this
adds an I3DST object into the I3Frame. On configuration, I3DSTModule is given
a vector of strings corresponding to the names of I3Particles for various
reconstructions in descending order of priority. The first two reconstructions
that the module finds in the frame will be assigned to reco1 and reco2 in the
dst and the index of the reco name for each will be encoded in to a single
uint8_t byte 'reco_label' using the nth bit as a boolean to indicate whether
the nth reconstruction was selected. This is accomplished by bit-shifting 1 by
n where n is the index of the reconstruction key.

I3DST13
^^^^^^^
With the introduction of Q-frames, I3DST13 (and some earlier formats) was split into two separate modules: :cpp:class:`I3DSTDAQModule13` operates on Q-frames and records event-level (trigger-level) information such as EventID, trigger information, time, CoG, etc. and :cpp:class:`I3DSTModule13` records information from split P-frames, including reconstructions and similar variables. 




:js:data:`I3DSTDAQModule13`
---------------------------
:cpp:class:`I3DSTDAQModule13` extracts key information about triggered events and generates an :cpp:class:`I3DST13` object which is a very compact representation of an event.



:js:data:`I3DSTModule13`
------------------------
:cpp:class:`I3DSTModule13` extracts key information about reconstructed events and generates an :cpp:class:`I3DSTReco13` object which is a very compact representation of an event.

I3DST16
^^^^^^^

:js:data:`I3DSTModule16`
------------------------
With :cpp:class:`I3DSTQModule16`, treatment of both Q- and P-frames is handled by the same module which implements both the DAQ and Physics methods. It extracts key information about triggered and reconstructed events and generates an :cpp:class:`I3DST16` object which is a very compact representation of an event.



