The event splitter IceHive
==========================

Introduction
^^^^^^^^^^^^
IceHive and its splitter component HiveSplitter are the central drivers of this project. IceHive, when used correctly, is a very powerful tool in IceCube data processing at stages where only rudimentary data-treatment has been applied. In fact the performance of HiveSplitter is improved the more complete the PulseSeries are that are delivered to it. This, paired with its low noise vulnerability, makes it in fact a good candidates for earliest event splitting and cleaning.


Driving Principle of HiveSplitter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

HiveSplitter works by forming clusters of hits which evolve and die at the end of their lifetime.
If these clusters reach a certain threshold requirement on their size they will in the end be written out as an subevent.
Because individual particles are expected to create clusters in separate places of the detector, so that the hits do not connect, the clusters are kept separate from each other during processing: they establish, evolve and die individually. This is the splitting capacity of HiveSplitter. 

Clusters are formed by hits connecting to each other and fulfilling a given multiplicity requirement on the number of DOMs each hit can connect to at each point in time. This connection argument is an amalgam of physics-causal arguments::
  * Hits should be too distant to each other (eligible volume)
  * Hits can be seen as originated from one and the same particle (particle-causal)
  * Hits can be seen as caused by photon of the same light-wavefront (photon-causal)
  * Hits are at different topological points of the same light-topology *Cherenkov-cone* (vicinity-causal) 

This Connection argument and the parameters on the individual sub-aspects efficiently establish a global volume and time limitation on the hits it can connect to. Thus ever hit defines a  volume, where it can connect to further hits, the *hit's active volume (IAV)*. Each cluster, thus collection of hits, has a overlapping volume of active volumes, the *summed active Volume (SAV)*, with only certain regions fulfilling the multiplicity. Only in these region further hits can be added to the cluster, it is the *cluster's active Volume (CAV)*. With each hit the cluster adds to itself, the SAV is expanded by the additional IAV of that hit and the CAV is altered. As the IAV is time-dependent and has a finite lifetime, during the evolution of the cluster certain hits will not participate any more in the SAV. As a consequence the CAV is decreasing accordingly. In conclusion clusters expand with each added hit and the CAV is found where the most recent hits were added.

Because hits are created through a traversing particle in the detector, the CAV will follow the path of further created light and hits, and thus the particle itself as long as the particle create enough hits to nourish the clusters evolution.


I3IceHive Module
^^^^^^^^^^^^^^^^

The module ``I3IceHive`` applies the HiveSplitter and TriggerSplitter algorithms to any PulseSeries found in the ``Q``-frame. It is possible to also apply information from the ``I3TriggerHierarchy`` to accellerate the splitting process. Output are a series of subevent ``P``-frames.

This is the full list of parameters to the module, whch are later explained in more detail:

The Module IceHive takes the parameters::
  * **InputName** [No Default] Name of the input PulsesSeriesMap.
  * **OutputName** [No Default] Name the output PulseSeriesMap the processed pulses will be written to.
  * **Multiplicity** [Default=4] Required multiplicity of connected DOMs in each cluster to any hit, in ordered that the hit is assigned to that cluster.
  * **TimeWindow** [Default=2000 ns] Time span within which the multiplicity requirement must be met.
  * **TimeStatic** [Default=200 ns] Maximum time span within a close-by pair of hits can be considered connected (Static term).
  * **TimeCVMinus** [Default=200 ns] Maximum negative time residual at speed of light in vacuum travel at which a pair of hits can be considered connected (Particle-propagation term).
  * **TimeCVPlus** [Default=200 ns] Maximum positive time residual at speed of light in vacuum travel at which a pair of hits can be considered connected (Particle-propagation term).
  * **TimeCNMinus** [Default=200 ns] Maximum negative time residual at speed of light in ice/water travel at which a pair of hits can be considered connected (Photon-propagation term).
  * **TimeCNPlus** [Default=200 ns] Maximum positive time residual at speed of light in ice/water travel at which a pair of hits can be considered connected (Photon-propagation term).
  * **SingleDenseRingLimits** [Default=[300.0, 300.0, 272.7, 272.7, 165.8, 165.8] m] Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if considering strings in the regular IceCube hexagonal structure (125m the averaged characteristic inter-string spacing); Default describes spheres of 300m radius.
  * **DoubleDenseRingLimits** [Default=[150.0, 150.0, 131.5, 131.5, 40.8, 40.8] m] Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if the hexagonal structure is regularly filled by one more string (72.2m the averaged characteristic inter-string spacing); Default describes spheres of 150m radius.
  * **TripleDenseRingLimits** [Default=[150.0, 150.0, 144.1, 144.1, 124.7, 124.7, 82.8, 82.8] m] Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if the hexagonal structure is regularly filled with six more strings (41.7m the averaged characteristic inter-string spacing); Default describes sphere of 150m radius.
  * **SingleDenseRingVicinity** [Default=[100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if considering strings in the regular IceCube hexagonal structure (125m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the first ring at 125m and 200m hight.
  * **DoubleDenseRingVicinity** [Default=[100.0, 100.0, 100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled by one more string (72.2m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the second ring at 125m and 200m hight.
  * **TripleDenseRingVicinity** [Default=[100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled with six more string (41.7m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the third ring at 125m and 200m hight.
  * **SaveSplitCount** [Default=False] Save an integer in the frame indicating the number of subevents generated
  * **ReadoutWindowMinus** [Default=4000 ns] Length of readout window to pad before trigger [ns]. This time is used in the TriggerSplitter pulse extraction and is added to the '_Noised' pulse output for every subevent.
  * **ReadoutWindowPlus** [Default=6000 ns] Length of readout window to pad after end of trigger [ns]. This time is used in the TriggerSplitter pulse extraction and is added to the '_Noised' pulse output for every subevent.
  * **UseTriggerSplitter** [Default=False] Use TriggerSplitter prior to HiveSplitter (A speed-up)
  * **TrigHierName** [Default="I3TriggerHierarchy"] Name of the input TriggerHierarchy
  * **TriggerConfigIDs** [Default=[1006 (SMT8), 1007 (sting), 1011 (SMT3-DC), 21001 (volume)] Vector of config IDs for the relevant triggers
  * **NoSplitDt** [Default=10000 ns] Don't split for trigger time difference lower than this (end of previous trigger to the start of next).


.. _ringlimits:

RingLimits/Vicinity
^^^^^^^^^^^^^^^^^^^

Here the inclusion volume in the comparison of two hits is defined.

  .. figure:: rings.png

     (Regular single dense strings in green, double dense strings in red, and triple dense strings in blue; compare to the surface assembly of IC86 detector)

The parameter is a list of the vertical distance which is to be included up (first value) and down (second value) on each ring *RingN* around a central string *C*. In this context *RingN* means the N-th smallest regular hexagon of strings with C residing in its center. For example considering central string 36 this would yield: ::

  Ring0 = (36) == C
  Ring1 = (26,27,35,37,45,46),
  Ring2 = (17,18,19,25,28,34,38,44,47,54,55,56) etc.

each entry in the list can take the following values:
* ``(double)`` = connect that DOMs in this vertical distance on this ring,
* ``NAN`` = connect nothing on this ring,
* ``INF`` = connect everything on that ring.
These denote the range of included DOMs. Thus in order to be included a DOM has to fulfill::
  
  limit_down <= z_dist <= limit_up

The RingLimits thereby have the form [(Ring0_Down, Ring0_Up), (Ring1_Down, Ring1_Up), ... ] and can only be configured in pairs of (up, down) for each ring. For example a configuration of::
  
  ICRingLimits=[(-10,10), (-15, 5), (NAN, INF)]

does imply for an early Hit (E) on OMKey(36,30) that the following OMKeys are eligible to find neighbouring hits (L) (furthermore assume for now that DOMs are spaced 1m apart):
::

  {(36, {20-40}), ({Ring1}, {15-35}), ({Ring2}, {30-60}); 

**NOTE**: As per default the hit DOM itself is always an eligible neighbour.


The different designation SingleDense, DoubleDense and TripleDense refer to the regular different hexagonal geometries.
* SingleDense refers to the IceCube geometry (characteristic spacing 125 meter).
* DoubleDense refers to a regular geometry added one more strings to every triangular cell, aka one further string in between any three other strings characteristic spacing 72 meter)
* TripleDense refers to a regular geometry with four more strings added to every triangular cell, aka 2 strings on each arm of the triangular cell and one in the middle

  ..image::geo.png
  (Regular single dense strings in green, double dense strings in red, and triple dense strings in blue; compare to the surface assembly of IC86 detector)
  

Output
^^^^^^

The output of IceHive according to the selection of parameters are following objects:

in the Q-frame:
* <I3RecoPulseSeriesMapMask>([OutName]_Physics) containing ALL found clusters
* <I3RecoPulseSeriesMapMask>([OutName]_Noise) the inverse of [OutName]_Physics taken in respect to [InputName]
* <I3Int>([ModuleName]_SplitCount) The number of found clusters

in the P-frame: (one for each found cluster in time-order fashion)
* <I3RecoPulseSeriesMapMask>([OutName]) containing the pulses of the cluster
* <I3TimeWindow>([OutName]_TimeRange) containing the start and end-time of the cluster
* <I3RecoPulseSeriesMapMask>([OutName]_Noised) containing the cluster and the noise around the specified time-window
* <I3EventHeader>(I3EventHeader) correct with the timing information of the start and end-time of the cluster, eventually plus the trigger-readout windows; subevent stream is the [ModuleName]
* <I3TriggerHierarchy>(I3TriggerHierarchy) the updated Trigger Hierarchy to the requested trigger-windows and selections


How to optimize
^^^^^^^^^^^^^^^

**There is always room for improvement.**

The default parameters in this project are not optimized. They are first guess values fuelled by experience from previous approaches to event splitting or hit-recognition/cleaning. However they work quite well and the performance with default settings is more than satisfying.

However these parameters might want to be optimized in the future to strengthen the splitting capability or increase the selectiveness on Physics-hits. Here are some pointers how to approach this:
* First, you will need to do your quantifications on MC datasets and compare them to data as a verification. In these MC-datasets you need to have the hit truth present for those objects you plan to operate on. For I3RecoPulses you can use the projects MCHitSeparator or MCPulseSeparator to recover this information from simulation.
* For any change, you need to observe two properties very closely: The splitting capability vs the number of wrong splits, and the physics-purity vs noise-contamination in the output. There are tools in MCPulseSeparator that can help you with this.
* Optimize eligible volume first: its tempting to manipulate the simple time-settings first, however you might over-optimize this faster than  the eligible volume.
* Geometrical analysis of your proposed settings in the topology of the detector can get you a long way: for example observe blind pathes, track-length between DOMs for certain angles
* Currently the inclusion volumes are symmetrical up and down on each string, however the DOM sensitivity is much higher for up-going particle movement. This means that in principle fewer DOMs have to be included upwards on the rings than down-wards.
* Before removing whole rings, think about the blind paths and probability of any track still making enough hits so that the driving principle is not impaired.
* On the central string the numbers of included DOMs can always be smaller than on the first ring: this is a property of the detector geometry where inter-DOM spacing is much lower than inter-string spacing.
* Do not overoptimize the time-settings to their respective purpose: It has shown that somewhat looser settings can help the algorithm with deficiencies, which arise from irregularities in the geometrical setup, the detector-response and even physical properties.
* Be more easy on the positive residual boundary(arriving to late) for photon propagation than on the negative one (arriving too early): This has to do that this parameter considers direct photon-hits. However we know there is scattering in the ice, which delay photons and increase their time-residual. this is also an effect which increases over travelled distance. The future might bring that such model can be better integrated into the algorithm.
* Optimize always without the TriggerSplitter option.
* It is highly discouraged to experiment with settings which are infinite (INF) or not-a-number (NAN). Such can break the driving principle and cause other bad things to happen.

