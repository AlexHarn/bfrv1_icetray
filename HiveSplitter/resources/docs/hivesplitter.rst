The event splitter HiveSplitter
================================

Overview
^^^^^^^^
The origin of the project is a re-implementation of the '''TopologicalTrigger''' algorithm by D. Chirkin into '''TopologicalSplitter''' rewritten and coded by C. Weaver. In a number of consecutive steps the code has been refined and expended with better understanding on the background  physics and implementation mechanics. The result is a general clustering algorithm, that can be used as both, cleaning and splitter.

The main objective of the algorithm is to construct (all) subsets from the input pulses such that each pulse in the subset is causally connected to at least N other, earlier pulses in the set, within a time window T, and then to form subevents by merging all causal subsets which overlap.

The novelty with this module is that the detector is described by hexagonal cylinders (honey combs) which build up the detector. In the final detector geometry thereby around each string surrounding rings of strings can be defined, so called rings.

By this segmentation of the detector for each DOM the surrounding rings can be identified and on each of them a certain region of included DOMs defined, the so called eligibility volume.

In this way the individual active volume can be steered much more exactly and the global active Volume kept to a minimum so that splitting power and physics against noise separation power is increased.

Modules
^^^^^^^
'''I3HiveSplitter''' is the primary and only '''I3Module''' provided in this project and takes the parameters::
  * InputName [No Default] Name of the pulses to split
  * OutputName [No Default] Name of the pulses to put in the split frames
  * Multiplicity [Default=4] Required multiplicity of hit DOMs in any subevent
  * TimeWindow [Default=4000 ns] Time span within which the multiplicity requirement must be met. Any setting above TimeConeMinus+TimeConePlus renders this parameter ineffective.
  * TimeConeMinus [Default=1000 ns] Maximum negative residual in time.<BR> For Norm==1(Lorentz): Maximum negative deviation from speed of light travel time which will allow a pair of hits to be considered connected;<BR> Norm==2(Euclidean(): Not Used; <BR> Norm==3(Static): Not Used
  * TimeConePlus [Default=1000 ns] Maximum positive residual in time.<BR> For Norm==1(Lorentz): Maximum positive deviation from speed of light travel time which will allow a pair of hits to be considered connected;<BR> Norm==2(Euclidean(): Maximum deviation from speed of light which will allow a pair of hits to be considered connected; ; <BR> Norm==3(Static): Static time-window within all pairs of hits to be considered connected;
  * UseDOMspacings CURRENTLY DISABLED [Default=false] RingLimits are specified as inter-DOM spacings (True) instead of meters (False). CURRENTLY DISABLED
  * ICRingLimits [Default=[300.,300.,272.7,272.7,165.8,165.8] m] Limits for IceCube (125m the averaged characteristic inter-string spacing) on-ring distances in pairs Minus/Plus; Default describes spheres of 300m radius. Units are either meter or inter-DOM spacings (nominal 17m) if 'UseDOMspacings' is specified.
  * DCRingLimits [Default=[150.,150.,131.5,131.5,40.8, 40.8] m] Limits for DeepCore (72.2m the averaged characteristic inter-string spacing) on-ring distances in pairs Minus/Plus; Default describes spheres of 150m radius. Units are either meter or inter-DOM spacings (nominal 10m) if 'UseDOMspacings' is specified.
  * PinguRingLimits [Default=[150.,150.0,144.1,144.1,124.7,124.7, 82.8, 82.8] m] Limits for PINGU (41.7m the averaged characteristic inter-string spacing) on-ring distances in pairs Minus/Plus; Default describes sphere of 150m radius. Units are either meter or inter-DOM spacings (nominal 10m) if 'UseDOMspacings' is specified.
  * SaveSplitCount [Default=False] Whether to save an integer in the frame indicating the number of subevents generated
  * Mode [Default=1] The Mode to use for the causal Argument: 1=Lorentz, 2=Euclidean, 3=Static
  * NoSplitMode [Default=False] Do not issue splits in separate P-frames than rather a cleaned PulseSeriesMask with the unification of all clusters to the Q frame;

RingLimits
^^^^^^^^^^
The number of rings configurable is right now limited to 2 in IC, 4 in DC and 8 in PINGU, because HiveLib LookUp-Tables only extend that far.
It is possible for each value to take the following values:

  * positive floating point = connect that high p/m on that ring),
  * NAN = connect nothing on that ring,
  * INF = connect everything on that ring.

The parameter is a list of the distance/number of DOMs which are to be included up (Plus) and down (Minus) on each outward going ring (RingN) around a central string (C). In this context 'RingN' means the N smallest regular hexagon of strings with C restring in its center. For example considering central string 36 this would yield: Ring0==C=(36), Ring1=(26,27,35,37,45,46), Ring2=(17,18,19,25,28,34,38,44,47,54,55,56) ...<BR>
The RingLimits thereby have the form [Ring0_Minus, Ring0_Plus, Ring1_Minus, Ring1_Plus, ... ] and can only be configured in pairs of Minus/Plus for each ring. For example a configuration of 'ICRingLimits=[10,10, 15, 5, NAN, INF]' does imply for an early Hit (E) on OMkey(36,30) that the following OMKeys are eligible to find neighboring hits (L): {(36, {20-40}), ({Ring1}, {15-35}), ({Ring2}, {30-60})<BR>

The different designation suffixes IC (IceCube), DC (DeepCore), PINGU refer to special regions of inter-string spacing where more stringent parameters can be applied.

Clustering
^^^^^^^^^^
The clustering of HiveSplitter works by multiplicity arguments imposed on overlapping causal connected hits.
The causal connection of any two hits is checked evaluating the earlier hit (E) against the later one (L).
The evaluation is done by the requirement that L is in the eligibility volume of E and fulfills a causal argument. Hence both checks enforce time-ordering.

Eligibility volume
^^^^^^^^^^^^^^^^^^
The eligibility volume is shaped exclusively by the RingLimits passed on by the user. It does include different numbers of DOMs up and down on the strings organized in rings around the earlier hit (E). Each DOM in this eligibility volume can be a potential neighbor hit, which can contribute to fulfilling the multiplicity requirement and thereby forming a cluster. The eligibility volume is not mutually inclusive (hit1 including hit2 in eligibility volume, does not automatically imply hit2 includes hit1), if RingLimits parameters are not configured symmetrically. This is actually one of the main features of this approach. By different shapes of this eligibility volume (more/less rings, asymmetries in plus/minus, more/less included DOMs per ring) the acceptance of different track directions can be exactly steered.

However, it is wise to stick to a rather isotropic neutral 4pi acceptance configuration, if not entirely sure what one is doing.

Causal argument
^^^^^^^^^^^^^^^
If two hits are compared and found to be included in the eligibility volume they are checked if the earlier hit does have a causal argument to cause the later hit. This argument takes parameters of distance and timing and returns a binary expression. The causal Arguments in HiveSplitter, which are Lorentz, Euclidean and Static are steered by the associated parameter name 'TimeCone'.
('dr' is the spacial distance, 'dt' the positive time difference between hits and 'c' the speed of light in vacuum):

  * Lorentz Mode (1 Default): The Lorentz Mode is used for physical causality in a 3+1 space-time vector-space. It is the mode of choice. In a graphical Interpretation it is representing an expanding sphere shell by light-speed with thickness 'TimeConeMinus'+'TimeConePlus'. It has the general form:

        alpha_minus <= dt - dr/c <= alpha_plus

    If the inner bracket of the equation is an expression of a time-residual. If it equals 0 the hits are exactly on each other light.cones / event horizons; if it is negative the hits are space-separated, e.g. the are not in each other event horizon; if it is positive the hits are time separated, e.g. are in each other event horizon.
  * Eucledean Mode (2): The Euclidean Mode it taken up for completeness and compatibility to CoincEventSplitter (M. Wellons). Its graphical interpretations are receding spheres with lightspeed and a predefined radius. Its general form is:

        sqrt(dt + dr/c) <= alpha

    The left side expression is always bigger than zero, why only a symmetrical residual alpha is established.
  * Static Mode (3): The Static Mode is establishing a static time window around each hit, where any further hit is considered to be connected, as long as they are not too far apart (which is however separately steered). This mode is especially used in SeededRTcleaning. Its general form is:

        dt <= alpha (dr <= max_dist)

Further modes can be added and implemented in the code easily, however for the inexperienced user it is highly recommended to stay with either mode 1 or 3 and shrink the parameters until a certain desired performance is achieved and than describe it by an approximating new argument and parameters.

Multiplicity
^^^^^^^^^^^^
The parameter Multiplicity is the requirement on the number of overlapping causal connected hits to any one other hit found, in order for it accepted into a cluster. The multiplicity must be fulfilled within a time limit set by parameter TimeWindow.
So to say for a Multiplicity of 4(=default) and a TimeWindow of 2ms: A cluster does form, if 4 hits are found within a 2ms, where each of them is causally connected (within the time ordering). Once a cluster is formed more hits are added to, if the new (later) hit does causally connect to at least 4 (former) hits from that cluster, within the time-window.

Backwards Compatibility
^^^^^^^^^^^^^^^^^^^^^^^
You can simulate the behaviour of passed splitters and cleanings by choosing the right parameters:

SRTCleaning Compatibility
-------------------------
Compatibility to SRTCleaning if seeded by all (SLC) Hits, 150m spheres::
  * ICRingLimits: [150., 150., 125., 125., NAN, NAN]
  * DCRingLimits: [150., 150., 131.5, 131.5, 40.8, 40.8, NAN, NAN]
  * PinguRingLimits: [150., 150., 144.1, 144.1, 124.7, 124.7, 82.8, 82.8]
  * NormMode: 3 (Static)
  * TimeConePlus: 1000
  * NoSplitMode: True

Coincident Event Detection Clustering Routine Compatibility
-----------------------------------------------------------
Compatibility to CoincEventSplitter 400m spheres::
  * ICRingLimits: [400., 400., 380., 380., 312., 312., 139., 139.]
  * DCRingLimits: [400., 400., 391.2, 391.2, 363.6, 363.6, 312.1, 312.1, 220.7, 220.7]
  * PinguRingLimits: [400, 400, 397.8, 397.8, 391.2, 391.2, 379.9, 379.9, 363.6, 363.6, 341.4, 341.4, 312.1, 312.1, 273.5, 273.5, 220.7, 220.7, 138.4, 138.4]
  * NormMode: 2 (Euclidean)
  * TimeConePlus: 1000

TopologicalSplitter Compatibility
---------------------------------
Compatibility to TopologicalSplitter: Center:+-15DOMs; Ring1 all; Ring2 all ::
  * ICRingLimits: [15*17., 15*17., INF, INF, INF, INF]
  * DCRingLimits: [15*17., 15*17., INF, INF, INF, INF]
  * PinguRingLimits: [15*17., 15*17., INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF]
  * NormMode: 1 (Lorentz)
  * TimeConePlus: 1000
  * TimeConeMinus: 1000

MaxDistSplitter Compatibility
-----------------------------
Compatibility to MaxDist 300m spheres, 150m in DC ::
  * ICRingLimits: [300., 300., 263., 263., 166., 166.]
  * DCRingLimits: [150., 150., 131.5, 131.5, 40.9, 40.9]
  * PinguRingLimits: [150., 150.0, 144.1, 144.1, 124.7, 124.7, 82.8, 82.8]
  * NormMode: 1 (Lorentz)
  * TimeConePlus: 1000
  * TimeConeMinus: 1000

HiveLib
^^^^^^^
HiveLib is the Driving core of this module: It is a library that provides fast look-up of the IceCube detector configuration expressed in terms of Rings of strings surrounding a chosen central string. Right now a maximum number of 2 rings are indexed for IceCube, 4 for DeepCore and 8 for PINGU-spaced strings.

All necessary functionality is provided through interfaces, so there is no need for direct class interactions.

Normally, you should choose the representation of rings that suffice your needs, so do not describe inter-IceCube spacings by a DeepCore-spaced configuration, as it will be slow and have gap rings (every second one). However, if you really want to, there is the possibility to abstract a given configuration to the next lower level.


