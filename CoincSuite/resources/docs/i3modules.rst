.. highlight: sh

I3Modules
=========

This is a short list and description the I3Modules that are contained in CoincSuite. For a full description of the functionality and parameters of an module you can always use::

   icetray-inspect CoincSuite | grep "[ModuleName]"

in the loaded icetray environment.

Discard-Modules
^^^^^^^^^^^^^^^

Modules that mark and remove certain frames from the stream of SplitFrames in order to improve purity, processing reliability or simply speed:

* **AfterpulseDiscard**:
  A module that is able to identify Afterpulse-events, which are mainly comprised of Afterpulses in the Subevent stream (associated to any preceding host event). Identification is done by arguments of total charge and an sufficient overlap in hit DOMs.

* **NoiseClusterDiscard**:
  A module that is able to identify NoiseClusters. Identification is done by a small number of hit Doms and an extreme extension in time.


HypoFrame-Modules
^^^^^^^^^^^^^^^^^

* **HypoFrameCreator**:
  Creates for a series of HypoFrames through all possible 2 permutations of SplitFrames. This can be restricted to a certain time-window or frames marked by a certain flag (useful for re-evaluations).

* **HypoFrameFaker**:
  In case exactly 2 effective splits are found this module can modify every other *single* P frame (e.g. 'in_ice') to act as a HypoFrame.

* **HypoFrameFakerKiller**:
  Reverts all changes done by HypoFrameFaker.

Tester-Modules
^^^^^^^^^^^^^^

* **AfterpulseTester**:
  This is a Tester implementation of the AfterpulseDiscard Module, when Afterpulse events should rather be recombined into the host-event instead of them being discarded. It is recommended to immediately run a recombining step (DecisionMaker) straight after this step, because Afterpulse events left in the subevent stream can highly confuse other downstream Testers. Identification is done by arguments of total charge and an sufficient overlap in hit DOMs.

* **impCausalConnectTester**:

  * Tests if two islands of pulses are connected topologically and have correct timing with the through-going reconstructed track.
  * Uses the emission points of the last and first emitted photon on the track that is still direct.
  * Tested are the ct-time residual, vertical distance, horizontal distance, besides others.

* **cogCausalConnectTester**:

  * Tests the causal connection between two islands of pulses.
  * Uses the COG of the third and the second quarter of hits in the respective pulse-series.
  * Tested are the ct-time residual, vertical distance, horizontal distance, besides others.

* **AlignmentTester**:

  * Tests if tracks of the fractures of pulses do align to a Fit
  * Tracks must be found within a critical angle and a critical distance (closest approach) to the Fit 

* **TrackLikelihoodTester**:

  * Tests if the combination of pulses can be described as origination from one single Fit with a sensible Likelihood-value.

* **ReducingLikelihoodTester**:
  See if the combination of pulses has a better likelihood reconstruction (smaller LLH-value) as the separate pulses.

* **SpeedTester**:
  Test if a proposed reconstruction connecting two pulse-series with scalable particle-speed (LineFit) has a velocity which is comparable to the speed of light.

* **TrackSystemTester**:
  Tests if the pulses are found in the track-system of a given track, e.g. if the pulses are found within a certain radius around the track and within a permitted time residual window. If a sufficient fraction of the pulse-series should be found to be within the rack system the recombination is preferred.

Recombine-Modules
^^^^^^^^^^^^^^^^^

* **DecisionMaker**:
  A final decision module if frames should be recombined or not. Decision is taken by boolean evaluations of the TesterModules.

Additional Modules
^^^^^^^^^^^^^^^^^^

You will find a number of additional modules in the project CoincSuite. These are helper modules for not so general purposes but can be very helpful in the one or the other situation. Look at their documentation through::

  icetray-inspect CoincSuite

Frame-Objects
^^^^^^^^^^^^^

There are a number of I3Objects through which the communication of this software suite is realized. These are either written by the CoincSuite-Modules themselves, or must be provided or created by the user or previous processes for successful operation:

* **SplitFrames** (I3Frame) in FramePackage after QFrame: ``P``-frames written out by the Splitter Module, which hold the subevent split pulse-series; must have an "I3EventHeader" with subevent-stream by the name of the SplitterModule (if you are using phys-services.GetNextSubEvent() this is automatic)<br>

* **HypoFrames** (I3Frame) in FramePackage after QFrame and (usally after P-Frames) : ``P``-frames created by the HypoFrameCreator Module, which hold the preliminary recombined pulse-series from two SplitFrames and act as the unsplit hypothesis of these frames; must have an "I3EventHeader" with subevent-stream by the name of the 'HypoFrame' (automatic) and have an 'CreatedFrom' object telling about the parents of this HypoFrame. 

* **SplitCount** (I3Int) in QFrame : Holds the number of SplitFrames written out by the Splitter. Must go by the name "[SplitterModuleName]SplitCount". (Most Splitters allow the option 'WriteSplitCount' to be set or use the module CoincSuite.SplitCountMaker)

* **ReducedCount** (I3Int) in QFrame: Holds the number of removed SplitFrames from the FramePackage. This variable must be created before CoincSuite starts computation. Must go by the name "[SplitterModuleName]ReducedCount". (Use the module CoincSuite.ReducedCountPlacer to create it)

* **CS_RecombSuccess** (I3MapStringBool) in HypoFrame : A map that will be created and/or augmented by and Tester module that is run. Entries are made if that specific Tester voted in favor or against a recombination of the SplitFrames the HypoFrame was created from. 

* **CS_RecombAttempts** (I3VectorString) in QFrame : A list that will be created and/or augmented by and Tester module that is run. Entries are made for each Tester that has been run.

* **CS_CreatedFrom** (I3MapStringVectorDouble) in HypoFrame : Holds the parent's subevent-ID of this HypoFrame; is created by HypoFrameCreator or HypoFrameFaker

* **CS_ComboFrom** (I3MapStringVectorDouble) in recombined ComboFrame : In SplitsFrames which are the result of a successful recombination, tracking the parent's subevent-ID.

* **CS_Reducing** (I3MapStringVectorDouble) in HypoFrame : upon successful recombination put into the HypoFrame to signal the frames which are the fractures leading to this recombined event

* **CS_ReducedWith** (I3MapStringVectorDouble) in recombined SplitFrame : upon successful recombination put into the Splitframe pointing to the partner SplitFrame which are fractures of the recobined events

* **CS_ReducedBy** (I3MapStringVectorDouble) in SplitFrame : upon successful recombination put into the SplitFrame pointing to the Hypoframe which is the host event of this fraction

* **Discard** (I3Bool) in SplitFrame: A marker that this frame can be discarded, because it has either been identified as nuisance or as a fracture of a recombined event, which has already been dealt with
