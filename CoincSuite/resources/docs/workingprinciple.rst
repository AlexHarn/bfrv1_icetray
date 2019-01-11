Coincident Suite working principle
==================================


Preliminary Frame Removal
^^^^^^^^^^^^^^^^^^^^^^^^^

The processing machinery of CoincidentSuite is fast and powerful and can in principle handle all kinds of causes for undesiered SplitFrames. However, sometimes the recombination of certain classes of SubEvents back into a *HostEvent* is not prefered. This can be tre case for example for Afterpulse Events, because Afterpulses in general do not hold any useful information for reconstructions.
Other classes of events can pose a source of confusion for CoincidentSuite's subordinated algorithms. Here a negative example are Noise Clusters, which hold only a small multiplicity of Pulses of no conclusive character. A possible recombination of such into a HostEvent is possibly harmful.
Therefore such undesired classes of events should be removed immediately before the processing by CoincSuite. This effectively prevents negative impacts and eases the workload for a faster processing.


HypoFrames
^^^^^^^^^^

Coincident Suite relies on 1-to-1 comparisons when probing if the recombination of SplitFrames is preferable. For every such comparison a so called HypoFrame (Hypothesis) is created, which holds the Pulses from both SplitFrames in a combined mask and is put into its own subevent stream. The HypoFrame remembers from which frames it has been created. Therefore it suffices to loop over each HypoFrame to probe possible recombinations.

TesterModules
^^^^^^^^^^^^^

Probing if the Recombination of frames makes sense is the job of so called Testers. These are small modules, which probe different aspects of the Pulses and other objects in the HypoFrames and/or SplitFrame to arrive at a decision for each processed HypoFrame is the Recombination is preferred. In this different Testers can probe different properties and thereby cover a wide range spectrum of arguments, why the Recombination is preferred. The individual decision of each Tester is saved to a table in the HypoFrame, so different Testers can run after another without interference.

Decision Maker
^^^^^^^^^^^^^^

After the processing through all Testers a final decision must be made if the Recombination is over-all preferred. This is done by the 
*DecisionMaker*, which takes the individual boolean decision by specified Testers into account. Here certain Testers can be used as

* Hints, where at least one positive decision of the specified Tester leads to the recombination

* Vetos, where any negative decisions of any of the specified Testers prohibits the recombination

* Directives, where any positive decisions of any of the specified Testers leads to a recombination, regardless of Hints and Vetos.

The Decision maker puts the final decision of a positive recombination of SplitFrames also into action: It creates the new recombined *ComboEvent* at the end of the SubEventStream and removes the SplitFrames which constipate it.

Also bookkeeping objects are put into the frames so that all frames can be correctly traced to their origin.


CleanUp
^^^^^^^

This FinalStep cleans cleans away any frame or object that was created inside the processing of CoincidentSuite but is no longer needed. This exspecially involves the removal of the HypoFrames, which are now obsolete.
After this step the output stream of frames should have the same structure as before the processing by CoincSuite, however clean from  problematic SplitFrames and save for use in further analysis.
