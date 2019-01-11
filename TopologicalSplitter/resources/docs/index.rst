.. _TopologicalSplitter:

====================
Topological Splitter
====================

	**Maintainer** : Chris Weaver

.. toctree::
   :maxdepth: 3
   :glob:

   release_notes

Overview
--------
This project is a re-implementation of the topological trigger algorithm by D. Chirkin. Its focus it to produce identical results for a subset of the modes supported by the original project. 

The basic algorithm is to construct all subsets of the input pulses such that each pulse in the subset is causally connected to at least N other, earlier pulses in the set, within a time window T, and then to form subevents by merging all causal subsets which overlap. 

Supported Modes from the Original TTrigger
..........................................

This project deliberately does not implement all of the modes which were present in the original implementation. The reasons for this are that these modes were seldom, if ever, used, and they substantially complicated the code. 

The original ttrigger had a parameter Topo which controlled the mode used for the main clustering algorithm. Accepted modes were 0, 1, 2, and 3. This project implements support only for what was formerly mode 1. 

The original ttrigger gave the option to compute whether hits met a local coincidence criterion and exclude those which did not. A hit was considered to have local coincidence if there was another hit on the same string within LCSpan DOMs and within LCWindow nanoseconds. This project does not implement this mode, so its behavior corresponds to that of the original with LCWindow set to zero (suppressing the use of local coincidence logic). 

Setting the CBWindow parameter in the original ttrigger to a non-zero time caused hits closer together than that time to be treated as a single hit. This implementation does not support this behavior. 

As a result of these simplifications, the behavior of I3TopologicalSplitter is equivalent to that of the original TTriggerSplitter with Topo = 1, LCWindow = 0, and CBWindow = 0. 

Modules
-------

I3TopologicalSplitter
.....................

This is the primary module provided in this project, and should be used in new scripts in preference to the modules listed below. 
This module operates as a splitter, so it takes pulses in Q frames as input and places subsets of the input pulses in new P frames as output. It can optionally save a count of the number of P frames produced into the Q frame. 

:InputName: [No Default] Name of the pulses to split
:OutputName: [No Default] Name of the pulses to put in the split frames
:Multiplicity: [Default=4] Required number of connected hits to form a subevent
:TimeWindow: [Default=4000 ns] Time span within which the multiplicity requirement must be met
:XYDist: [Default=500 m] Maximum horizontal distance within which hits will be considered connected
:ZDomDist: [Default=30] Maximum vertical distance within which hits on the same string will be considered connected, expressed as a number of DOM spacings (unitless)
:TimeCone: [Default=1000 ns] Maximum deviation from speed of light travel time which will allow a pair of hits to be considered connected
:SaveSplitCount: [Default=False] Whether to save an integer in the frame indicating the number of subevents generated

TTriggerSplitter
................

This module is provided for backwards compatibility with the original ttrigger project. It performs exactly the same function as I3TopologicalSplitter, but accepts certain additional, deprecated parameters, and warns if any of those deprecated parameters is specified with a value which is not supported. 

:InputName: [No Default] Name of the pulses to split
:OutputName: [No Default] Name of the pulses to put in the split frames
:Multiplicity: [Default=4] Required number of connected hits to form a subevent
:TimeWindow: [Default=4000 ns] Time span within which the multiplicity requirement must be met
:XYDist: [Default=500 m] Maximum horizontal distance within which hits will be considered connected
:ZDomDist: [Default=30] Maximum vertical distance within which hits on the same string will be considered connected, expressed as a number of DOM spacings (unitless)
:TimeCone: [Default=1000 ns] Maximum deviation from speed of light travel time which will allow a pair of hits to be considered connected
:SaveSplitCount: [Default=False] Whether to save an integer in the frame indicating the number of subevents generated
:Topo: [DEPRECATED, Default=1] Setting this parameter to a non-default value will result in an error
:LCSpan: [DEPRECATED, Default=-1]
:LCWindow: [DEPRECATED, Default=0 ns] Setting this parameter to a non-default value will result in an error
:CBWindow: [DEPRECATED, Default=0 ns] Setting this parameter to a non-default value will result in an error

ttrigger<I3RecoPulse>
.....................

This module is provided for backwards compatibility with the original ttrigger project. Unlike I3TopologicalSplitter it does not generate a new P frame for each subevent, but instead places each subevents pulses into the original P frame with a numeric suffix. Like TTriggerSPlitter it accepts certain additional, deprecated parameters, and warns if any of thos deprecated parameters is specified with a value which is not supported. 

:InputName: [No Default] Name of the pulses to split
:OutputName: [No Default] Name of the pulses to put in the split frames
:Multiplicity: [Default=4] Required number of connected hits to form a subevent
:TimeWindow: [Default=4000 ns] Time span within which the multiplicity requirement must be met
:XYDist: [Default=500 m] Maximum horizontal distance within which hits will be considered connected
:ZDomDist: [Default=30] Maximum vertical distance within which hits on the same string will be considered connected, expressed as a number of DOM spacings (unitless)
:TimeCone: [Default=1000 ns] Maximum deviation from speed of light travel time which will allow a pair of hits to be considered connected
:SaveSplitCount: [Default=False] Whether to save an integer in the frame indicating the number of subevents generated
:Topo: [DEPRECATED, Default=1] Setting this parameter to a non-default value will result in an error
:LCSpan: [DEPRECATED, Default=-1]
:LCWindow: [DEPRECATED, Default=0 ns] Setting this parameter to a non-default value will result in an error
:CBWindow: [DEPRECATED, Default=0 ns] Setting this parameter to a non-default value will result in an error

`Generated doxygen documentation for this project <../../doxygen/TopologicalSplitter/index.html>`_
