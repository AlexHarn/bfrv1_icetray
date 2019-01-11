
Introdution
===========

In the IC79 season, the DeepCoreFilter was introduced as a way to identify events in the energy range 10-100 GeV. Rates were calculated and shown to disagree between data and MC by up to 30% with data showing a large peak at low values of the HLC nChannel.

After much work, the source was discovered: the lowered threshold of the SMT3 allowed for accidental triggers ("Noise triggers") to propagate into the analysis chain. Simulation of the noise triggers was produced in IceSim2 and IceSim3, but discrepancies persisted. 

NoiseEngine is an attempt to reduce the number of noise triggers in the analyses of the low energy and oscillation groups. The concept derives from earlier attempts at identifying tracks with the TrackEngine algorithm (the source of the name of this module). 

Details about the efficacy of this module can be found in the original presentation during the Berkeley meeting: 

.. _presentation: https://events.icecube.wisc.edu/contributionDisplay.py?sessionId=32&contribId=114&confId=43
