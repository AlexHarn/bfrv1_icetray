HiveCleaning
============

Introduction
^^^^^^^^^^^^

HiveCleaning provides a classic Hit-cleaning that is based on the hexagonal description of the detector as it is used in the HiveSplitter. This enables a more precise steering of the parameters and a more powerful cleaning process.

The employed algorithm is similar to the classic RT-cleaning: Isolated hits, which do not have neighbouring hits occurring within a certain distance within a certain time window, are removed. Because Noise-hits predominantly occur randomly and show as isolated hits in the detector array, while Physics-hits occur in a clustering fashion, this algorithm is excellently suited for noise-cleaning.

I3HiveCleaning Module
^^^^^^^^^^^^^^^^^^^^^

The module ``I3HiveCleaning`` applies the HiveCleaning algorithm to a PulseSeries. It is possible to run the algorithm either on  different frames types by selecting the ``Stream``-parameter.

This is the full list of parameters to the module::
  * InputName [No Default] Name of the input PulsesSeriesMap.
  * OutputName [No Default] Name the output PulseSeriesMap the processed pulses will be written to.
  * Multiplicity [Default=1] Required multiplicity of connected neighbours for each hit, in order to remain in the PulseSeries.
  * TimeStaticMinus [Default=600 ns] Maximum nagative time difference in comparison of any two hits, in order that hits are possibly evaluated as connected [ns].
  * TimeStaticPlus [Default=600 ns] Maximum positive time difference in comparison of any two hits, in order that hits are possibly evaluated as connected [ns].
  * SingleDenseRingVicinity [Default=[100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if considering strings in the regular IceCube hexagonal structure (125m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the first ring at 125m and 100m hight.
  * DoubleDenseRingVicinity [Default=[100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled by one more string (72.2m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the first ring at72.2m and 100m hight.
  * TripleDenseRingVicinity [Default=[100.0, 100.0, 100.0, 100.0] m] Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled by six more strings (41.7m the averaged characteristic inter-string spacing); Default are hexagonal cylinders including of the first ring at 41.7m and 70m hight.
  * Stream [Default=Physics] The I3Frame-Stream to execute on.
  * If [Default=lambda f: True] An optional function for conditional execution.

The module delivers output as an PulseSeriesMapMask in under the key ``OutputName`` in the target frame.


How to specify the RingLimits
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

See the section in :ref:`ringlimits`


Best Practices
^^^^^^^^^^^^^^

Despite the fact that this module has been developed for in the hindsight of noise-cleaning, it is also very well suited for the removal of confusing hits, which might be problematic for reconstructions.

These might include, but are not limited to::
  * Early/Late Hits which are in fact Physics-hits, but distort the start/end times of events.
  * Hits which are in fact Physics, but occur at the border of the Cherenkov-cone. These originate from long-propagated survivor photons, which have scattered multiple times. Thereby they fuzz-out the otherwise sharp characteristics of the Cherenkov cone. Their removal and reduction of the PulseSeries to more direct hits, the bright central core, can significantly improve reconstruction results.
  * Afterpulses, which are instrumentation artefacts, occur long after the causal (Physics) from a through-going particle has long since concluded.
  * Isolated hits of great pull to reconstruction. Some reconstructions are very vulnerable if the PulseSeries presented to them is not clean from any artefacts, which are not well described by the modelled event topology. These can introduce a significant pull to the computed track-hypothesis and should be removed beforehand.
