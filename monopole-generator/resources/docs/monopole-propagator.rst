Monopole Propagator
===================

This is a project providing the module to propagate magnetic monopoles of every velocity through IceCube.
It is designed to take the output of monopole-generator as input,
that means an I3MCTree containing a particle with a randomized position and direction
and a info dictionary MPInfoDict which contains important information how to handle this particle in different simulation steps.

The module decides depending on the true velocity saved in MPInfoDict which mode to use to handle the different interaction types:
 - slow: catalyse of proton decay
 - fast: luminescence, indirect and direct Cherenkov light

 To learn more about the classes and variables in the source code have a look in to the doxygen_ documentation.


I3MonopolePropagator
--------------------

Main:
The module reads the simulated velocity from the dict frame and decides depending on this which mode (slow/fast) to use.

Fast:
Calculates the energy loss of the particle depending on the mass (given by the dict frame).
Calculates the velocity change for every stepsize and uses this as a new start-velocity for the next step.
Returns a MCTree containing many particles lined up after each other tracking the velocity changes inside/near the detector.

Slow:
The propagator adds a poisson distributed number of cascades at a uniform distributed position along the monopole track. By default for each catalyzed proton decay on EPlus is produced, carrying the whole energy from the proton. Though this is not correct physics-wise, the light output is the same, as the direction of each cascade is randomized as well. Nevertheless there is the option to use the correct decay into a 460 Mev EPlus and a 480 MeV Pi0 at the cost of twice as much secondary particles in the MCTree, which have to be propagated thorough the detector.
To save further computing resources the option ScaleEnergy allows to reduce the number of cascades (lambda=1m) while increasing their energy. This is helpful for short mean free paths.



Main Parameters
^^^^^^^^^^^^^^^

**InputTreeName**    (*I3MCTree*)
	Name of the I3MCTree containing the monopole simulated by the monopole-generator.

**OutputTreeName**    (*I3MCTree*)
	Name of the I3MCTree containing the monopole after propagation.

**InfoName**    (*MPInfoDict*)
        Name of the monopole info dictionary, containing all necessary information about the generation parameters.

**BetaThreshold** (*0.09c*) 
	Threshold wether to handle a monopole of speed Gamma/BetaRange as fast or slow monopole.

Parameters for mode: fast
^^^^^^^^^^^^^^^^^^^^^^^^^

**CalculateEnergy** (*true*)
    Wether to use calculate the energy loss of monopoles (else the monopole velocity is not decreasing).

**MaxDistanceFromCenter** (800m)
    How far beyond the detector to propagate the monopole. If the start of the Monopole is further from the detector than this value, propagator will IGNORE the parameter and propagate until it reaches same distance away on far side of detector

**Profiling** (*false*)
    If true adds a profile (type I3VectorDouble) of the monopole speed for each track segment to the frame

**SpeedMin** (*0.09c*)
    The speed at which the propagator should stop propagating. The value should not be set below ~0.1c if one is using the Calculate Energy loss, since Ionization formula assumes it above this range. If you use a lower speed min, please make sure you turn off energy calculation and treat this as a track segmentor only.

**StepSize** (*NaN*)
    Allows user to set the length of monopole track segments. If set this will override any length setting done in the module. Otherwise, Max/MinLength can be used to set the upper/lower bounds on the track segment lengths

**MaxLength** (*10m*)
    Assuming stepsize is NAN, this represents the largest segment the propagator will generate

**MinLength** (*0.001m*)
    Assuming stepsize is NAN, this represents the smallest segment the propagator will generate


Parameters for mode: slow
^^^^^^^^^^^^^^^^^^^^^^^^^

**MeanFreePath** (*NaN*)
    Mean free path lambda between catalyzed proton decays.

**ScaleEnergy** (*False*)
    Whether to set the mean free path to 1 meter and scale up the energy by 1/lambda. The overall light output stays comparable, while the number of secondary particles in the MCTree is reduced. This saves computing resources especially for short mean free paths. 

**UseCorrectDecay** (*False*)
    Wheter to simulate back to back EPlus (460 MeV) and Pi0 (480 MeV) instead of just one EPlus. This option cannot be used together with ScaleEnergy or EnergyScaleFactor, since the energies are hard coded. The overall light output is similar, but correct decay has twice as much secondary particles in the MCTree.

**EnergyScaleFactor** (*1.0*)
    Scale down the cascade energy in order to test the influence of other decay channels.
    


.. _doxygen: ../../doxygen/monopole-propagator/index.html


