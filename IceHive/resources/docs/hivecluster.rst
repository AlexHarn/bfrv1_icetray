The clustering-selector HiveCluster
===================================

Introduction
^^^^^^^^^^^^
HiveCluster is a module which stands in between IceHive and HiveCleaning. It is a hit-cleaning in its full rights, but based on the selection of Pulses by the algorithms of IceHive. While the same functionality can in principle be obtained from the IceHive module and the assessment of its ``[OutputName]_physics`` PulseSeries, IceHive still remains an event splitter in its application and is thereby limited to the application on Q-frames only. HiveCluster can be applied to any PulseSeriesMap in any frame and writes out just the one single object of the cleaned PulseSeriesMap.


I3IceCluster Module
^^^^^^^^^^^^^^^^^^^

The module ``I3HiveCluster`` applies the HiveSplitter algorithm to any PulseSeries. It is possible to run the algorithm either on Q-frames or P-frames by selecting the ``Stream`` parameter.

This is the full list of parameters to the module:

The Module IceHive takes the parameters::
  * InputName [No Default] Name of the input PulsesSeriesMap.
  * OutputName [No Default] Name the output PulseSeriesMap the processed pulses will be written to.
  * Multiplicity [Default=4] Required multiplicity of connected DOMs in each cluster to any hit, in ordered that the hit is assigned to that cluster.
  * TimeWindow [Default=2000 ns] Time span within which the multiplicity requirement must be met.
  * TimeStatic [Default=200 ns] Maximum time span within a close-by pair of hits can be considered connected (Static term).
  * TimeCVMinus [Default=200 ns] Maximum negative time residual at speed of light in vacuum travel at which a pair of hits can be considered connected (Particle-propagation term).
  * TimeCVPlus [Default=200 ns] Maximum positive time residual at speed of light in vacuum travel at which a pair of hits can be considered connected (Particle-propagation term).
  * TimeCNMinus [Default=200 ns] Maximum negative time residual at speed of light in ice/water travel at which a pair of hits can be considered connected (Photon-propagation term).
  * TimeCNPlus [Default=200 ns] Maximum positive time residual at speed of light in ice/water travel at which a pair of hits can be considered connected (Photon-propagation term).
  * SingleDenseRingLimits [Default=[300.0, 300.0, 272.7, 272.7, 165.8, 165.8] m] Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if considering strings in the regular IceCube hexagonal structure (125m the averaged characteristic inter-string spacing); Default describes spheres of 300m radius.
  * DoubleDenseRingLimits [Default=[150.0, 150.0, 131.5, 131.5, 40.8, 40.8] m] Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if the hexagonal structure is regularly filled by one more string (72.2m the averaged characteristic inter-string spacing); Default describes spheres of 150m radius.
  * TripleDenseRingLimits [Default=[150.0, 150.0, 144.1, 144.1, 124.7, 124.7, 82.8, 82.8] m] Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if the hexagonal structure is regularly filled with six more strings (41.7m the averaged characteristic inter-string spacing); Default describes sphere of 150m radius.
  * SingleDenseRingVicinity [Default=[100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if considering strings in the regular IceCube hexagonal structure (125m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the first ring at 125m and 200m hight.
  * DoubleDenseRingVicinity [Default=[100.0, 100.0, 100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled by one more string (72.2m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the second ring at 125m and 200m hight.
  * TripleDenseRingVicinity [Default=[100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled with six more string (41.7m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the third ring at 125m and 200m hight.
  * Stream [Default=Physics] The I3Frame-Stream to execute on.
  * If [Default=lambda f: True] An optional function for conditional execution.

