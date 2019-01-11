.. _shield:

Shield
======

.. toctree::
   :maxdepth: 1
   
   release_notes

Overview
--------
Shield implements the basis for a simple veto, by computing the time residual of hits in the detector with respect to a track hypothesis. 

The canonical use for this algorithm is to attempt to identify muon events in the IceCube detector which have associated pulses in the IceTop detector due to belonging to a cosmic ray induced airshower. Assuming that the direction and timing of the event is well reconstructed in-ice, the time of each pulse in IceTop is compared against the time at which a shower front moving along the track would have passes the IceTop tank in which that pulse occured. A pulse which occured close to the time at which the hypothetical shower front passed is an indication that there was in fact a shower, so the time residuals may be used in an analysis to cut events which are likely to have been air showers, or to assign a probability that a given event is an airshower. Such a use is described on http://wiki.icecube.wisc.edu/index.php/EHE_IT_Veto_Analysis . 

For each pulse two quantities are reported: the time residual, as described above, and the lateral distance from the tank in which the pulse occured to the track hypothesis. The results are reported using the standard I3Units. 

The shower front is treated as a simple plane, perpedicular to the hypothesis track, and moving at the speed of light. No correction is applied for the shower front being curved, as this has been deemed to be a small effect. However, a user can choose to use the per-pulse lateral distance information to make such a correction, if desired. 

Module Parameters
-----------------
The information described above is computed by the I3ShieldDataCollector module, which accepts the following parameters:
InputRecoPulses [No Default] The pulses to be tested for coincidence with the track
InputTrack [No Default] I3Particle to use for input track reconstruction (position, direction, and time)
OutputName [DEFAULT="ShieldResults"] Name for the output coincidence data, in the form of a list of I3ShieldHitRecord%s
ReportUnhitDOMs [DEFAULT=false] Whether to also generate a list of distances to unhit DOMs. Note that this is only usful if the input hits are complete, without cleaning, such as the union of all SLC hits. This module has no knowledge of the cleanings applied for IceTop Tank pulses, for instance.
BadDOMList [DEFAULT="BadDomsList"] List of DOMs not expected to trigger, or to be ignored
ReportDOMs [DEFAULT=false] Whether to report the list of DOMs in the output
