
Introduction
-----------------

The DeepCore detector, located around string 36 in the center of the IceCube detector, is designed to look for neutrino oscillation, atmospheric neutrinos, dark matter, and other low energy neutrino phenomena. To do this, analyses are required to reject the incident cosmic ray muon flux of approximately 3 kHz down to the atmospheric neutrino rate of around 4 mHz.

The first step of the process, the Simple Multiplicity Trigger (SMT3) requires three HLC hits within 2.5 microseconds. This is solely to accept low energy physics, but does not explicitly reject cosmic ray muons. The rate of events passing the SMT3 trigger is about 280 Hz, far above the expected low energy atmospheric neutrino rate.

The DeepCoreFilter is the first step in all low-energy analyses that is designed to reject the large muon background. Following application of the DeepCoreFilter (also known as the "DeepCoreVeto", after the name of the module in this project), the rate is reduced by roughly an order of magnitude while keeping a large fraction of the atmospheric neutrinos interacting in the DeepCore fiducial volume.
