Introduction
================

Coincident Suite was originally conceived to deal with the problems left behind by the use of event splitters in Icetray. The most prominent problem with event splitters is that they produce at a certain probability false splits, thereby ripping apart an event and delivering two subevents where one is expected. This has negative impact in physics-analysis (loss of pulse-multiplicity, sensitivity, misreconstruction, double counting). All processing has to commence in the series of subevents that is the output of the generic splitter, which can be huge and poses a challenge in the aspects of decision making, consistency, conformity and bookkeeping.

Definitions and Abbreviations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The working aspects of this project are complicated and require some previous knowledge of the processing pipeline of IceCube and IceTray. This section tries to outline these and make the reader familiar with the concepts and abbreviations used in the remainder of the documentation.

The Definition of an Event
^^^^^^^^^^^^^^^^^^^^^^^^^^

Since 2011 IceCube is recording and storing its data in a so called ``QPP`` frame-structure. Where a ``Q``-frame (DAQ) frame stores only raw or mildly processed detector data, and is the container of what the **DAQ** refers to as an *event*, the readout of the detector after triggers. From a ``Q``-frame event splitters can produce a number of ``P``-frames (Physics), where each ``P``-frame contains only a portion of the information in the ``Q``-frame and which is considered interesting. A ``P``-frame is the container of what the **Analyser** refers to as an *event*. So the analysier will be most interested to find particles traversing the detector, while the DAQ is interested in not missing any occurrence where possibly something (one **or more** particles) are present in the detector. Still the detector is in the end operated by the DAQ and therefore delivers events which contain none, one or multiple particles to the analyser who is in charge from this stream of ``Q``-frames to select its preferred selection.   

Streams and SubEventStreams of frames
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Event Splitters are the analysers tool to extract and select the useful parts of information and put them into one or multiple ``P``-frames, which are then called subevents. This most often means a different selection of Pulses, which are already the highest abstraction of the detector information. 

However, analysers preferences are different, while all analyses have to be conducted from the same stream of ``Q``-frames. Therefore each event splitter is allocated its own *SubEventStream* under which its ``P``-frames are stored. Thus multitude of ``P``-frames of multiple SubEventStreams can fledge from a single ``Q``-frame.

Problems of Event Splitters
^^^^^^^^^^^^^^^^^^^^^^^^^^^

As already mentioned the event splitting process is not perfect, and deficiencies in the algorithm might deliver ``P``-frames which have different content than anticipated or desired by the analyser. This includes the following cases:

* A frame contains no interesting Physics but only Noise is present; So called *NoiseClusters*

* A frame contains Physics of no value; for example only *Afterpulses* or events below a certain imposed threshold.

* One or multiple frames contain only a fraction of Pulses from an event; or in other words: *fractures* of pulses from a single event are spread out over a multitude of subframes.

These things are not desired and have to be possibly corrected or removed if not possible otherwise.

Event Recombination
^^^^^^^^^^^^^^^^^^^

Event Recombination solutions like CoincSuite are addressing the problem of SplitFrames. They try to identify occurences where such SplitFrames occur and possibly merge and consolidate the event fractures back to a complete single event.
This task is done by comparing every frame containing a SubEvent from the splitter, which is referred to as an SplitFrame, to each other. It is evaluated if it is preferable to keep the SplitFrames separated, because they indeed represent independent SubEvents, or if they should be recombined into a single SubEvent.


More reading
^^^^^^^^^^^^

  `Licentiate thesis <http://internal.icecube.wisc.edu/reports/details.php?type=report&id=icecube%2F201407001>`_


Abbreviations and Glossary
^^^^^^^^^^^^^^^^^^^^^^^^^^

There must be some definitions and abbreviations introduced:

SplitCount
  Number of subevents/splits found by EventSplitter, this includes splits of PhysicsClusters (correct splits), NoiseClusters, and fractured tracks; i.e. identical to the number of subevent frames generated
ReducedCount
  Number of successful recombinations, removed NoiseClusters and Afterpulses, or other measures which reduced the number of original subevents/splits
EffectiveCount (EffCount)
  Number of effective splits after recombinations (== SplitCount – ReducedCount)
Subevent
  Part of a bigger Hit/Pulse-Series, which was split off by e.g. a splitter representing a individual event seperated to others e.g in time or space. Multiple subevents can i.g. be associated to ``Q``-frame
Fracture
  Deficit of splitters can cause that e.g. single track events are fractured into two or more subevents, which is than a wrong split
Recombination
  The unification of two or more subevents into one single subevent.
Split
  Synonym to a subevent found and written out by a SplitterModule
NoiseCluster
  A subevent that contains only noise hits and no Physics. Their creation is a consequence from the fact that noise can also cluster mimicking low multiplicity events.
RecoFit
  A fit performed on individual subevents
HypoFit
  A fit performed on he recombination of two subevents
SplitFrame
  A frame of a split-off subevent
HypoFrame
  A frame containing the recombination of two subevents and associated objects; is used for hypothesis testing
COG (Center of gravity)
  The averaged position and time considering a series of Hits/Pulses
FramePackage
  A series of frames which all belong to one mother-(``Q``)frame. Mostly associated with a leading ``Q``-frames and a series of ``P``-(subevent)frames.
