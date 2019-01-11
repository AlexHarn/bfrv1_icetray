.. _truncated_energy:

Truncated Energy
================

This project is maintained by Joulien Tatar <jtatar@lbl.gov>.

.. toctree::
   :maxdepth: 1
   
   release_notes

Introduction
^^^^^^^^^^^^

The overall philosophy of this module uses the relationship between dE/dx (energy loss) of the muon, and the actual energy of the muon.  Muons have a stochastic nature about their energy losses and sometimes a very large energy loss will occur along its track.  This large burst of energy will skew the dE/dx value of the muon, which will skew the result of the muon energy calculation.  The Truncated Energy project (officially named truncated_energy), removes the highest energy losses and averages the remaining energy losses to obtain a more accurate result.  This module uses two methods to calculate that energy:  Bins method and DOMs method.  In addition, Truncated Energy also includes the original Photorec energy reconstruction that can be used for comparison (note: there is no reason to run Photorec separately from Truncated Energy).

The Bins Method
^^^^^^^^^^^^^^^

The Bins Method places the photoelectrons (PE) seen by each DOM into 120m bins, based upon the perpendicular location of the DOM along the reconstructed track.  Only the DOMs within 10m to 80m of the track (the "cylinder") are included in the totals.  The energy resolution was optimized by looking at the cylinder cuts and finding the best distances to keep.  The actual PE are compared to the expected PE from Photonics and a ratio is created (actual/expected), which is the dE/dx value for the bin.  A minimum of 3 bins is required to do the calculation using the Bins Method (about 300m track length).  The highest dEdx values are cut (40% is currently the hard-coded optimized value), and a new dEdx ratio for the event, and the remaining PE are added to create a new dEdx ratio for the event.  This truncated dE/dx value is then plugged into the appropriate energy equation to get the muon energy.


The DOMs Method
^^^^^^^^^^^^^^^

The DOMs Method calculates a dE/dx ratio for each individual DOM, instead of binning.  The actual PE in each DOM is divided by the expected PE, based on the reconstructed track and Photonics.  Thus, each DOM gets its own dE/dx value.  Only DOMs within 0m to 60m of the track are used (this distance was optimized for).  The highest 50% of these DOMs' dE/dx values are cut (this was optimized and hard-coded as well).  The remaining DOMs are averaged to create a new dE/dx value for the event.  The DOMs method uses a quality cut of 8 DOMs minimum within the cylinder for the event to qualify.  This reconstructed dE/dx value for DOMs method is then plugged into the appropriate energy equation to get the muon energy.

Precision of Energy Resolution
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Quality cuts are used to determine the energy and precision of the energy resolution.  Six different possibilities exist in this version for each method, with 3 energy regions and 2 quality parameters based on number of participating DOMs.  If the event has more than 20 DOMs, then the method is much more precise and uses different fitting equations.

* Truncated Energy has lowest energy reconstruction precision for low energy events.  These are events that have a truncated dE/dx value < 1.6 GeV/m (approx 8.5TeV).

* The core region (mid energy) consists of events with truncated 1.6GeV/m =< dE/dx =< 500GeV/m (approx 44PeV).

* The high energy region events have truncated dE/dx >= 500GeV/m.

With the DOM count either more than or less than 20 DOMs, there are 6 situations with different fitted equations.

The equations Truncated Energy use vary based on the dE/dx value.  Originally, the equation was linear over the full range:

dE/dx = A + B * E_muon or E_muon = (dE/dx - A) / B

With the dE/dx value, and the fitted values of A and B from a log10(dE/dx) vs. log10(E_muon) plot, you can back out the E value.  This is the way that Original Photorec does its calculation.  It is also the way the ORIG values are calculated within Truncated Energy, using the same constants.

However, for Truncated Energy, the fits need to be more elaborate for the best resolution.  In this case, the module uses a 2nd order polynomial for most of the dE/dx range (the low and high energy regions listed above):

log10(E_muon) = A + B * log10(dE/dx) + C * log10(dE/dx) * log10(dE/dx)

For the mid energy range, the module uses a linear dE/dx equation similar to ORIG Photorec, but with an added constant C as intercept:

log10(E_muon) = log10[(dE/dx - A) / B] + C

These equations have resulted in the best fit to the simulated data, with one exception.  The lower energy region (dE/dx < 1.6 GeV/m) demonstrates a steep drop off such that the dE/dx value becomes nearly constant over a wide range of E values.  The best fit to this data is the linear equation similar to Photorec, but that fit also eliminates most low-energy events (returns a value of 0 for energy), and thus the fit was changed to a 2nd order polynomial to include a calculation for these events as well.


Simulations Details
^^^^^^^^^^^^^^^^^^^
This analysis used 85,000 events (that contained reco tracks and pulses) from IC-79 E^-1 simprod NuGen 5377 and 5561 with SPICE1 tables, 1GeV-100PeV, in order to get the A, B, C constants.  Only about 42,000 events qalified for the Bins and DOMs methods.  A parallel analysis was performed on IC-59 simprod to compare AHA to SPICE1 tables, to compare the performance of the module on different ice models.  There was no discernible difference between AHA and SPICE1, ut a difference was noted between IC-59 SPICE1 and IC-79 SPICE1 (about 0.13 in log10(E_muon).  This difference is still under investigation?...

Truncated Energy also calculates the neutrino energy based upon the De/dx value, using similar fitting methods described above for the muon energy.  New A, B, C constants were created from the fits to the log10(E_nu) vs log10(dE/dx) plots.  This is a linear fit with no other adjustments.

The program also "calculates" the energy resolution for each event, based on the number of bins (for Bins method) or the number of DOMs (for DOMs method).  The energy resolution was plotted for each Bin/DOM value, and a 1 sigma resolution was recorded.  These values were then plotted to determine the relationship between the energy resolution and the Bin/DOM count for each method.  A best fit equation was created for each method, and the energy resolution could possibly be used as a weighting factor, since events with more Bins/DOMs would be more accurately calculated.


Output
^^^^^^

The output of this program includes the following:
Bins muon energy (within I3Particle)
Bins muon energy resolution (within I3Double, in log10(E))
Bins muon neutrino energy (within I3Particle)
DOMs muon energy (within I3Particle)
DOMs muon energy resolution (within I3Double, in log10(E))
DOMs muon neutrino energy (within I3Particle)
Orig muon energy (within I3Particle, without truncation)
Orig muon neutrino energy (within I3Particle)
Orig dE/dx value (within I3Double, without truncation)   


Misc
^^^^

Truncated Energy does not use the Cascade or BALL_OM options from Photorec, and they have been eliminated from the code.  In addition, the track length calculator has been eliminated, as well as adding noise, since these were not used in Truncated Energy.  The program does currently contain the ability to change the cylinder parameters, although the equations for the energy calculations are fit to the default cylinders, so the energy calculations would no longer be accurate if the cylinder distances were changed.  The functionality might useful in the future.


Future Improvements
^^^^^^^^^^^^^^^^^^^

Future studies to improve Truncated Energy may want to focus on gaining more precision by using qualifiers on the event, such as well contained events.  A well-contained event is one that comes within 400m of the center of the detector.  Well-contained events have a much better energy resolution than side-clippers.  Also, the bin size was not varied during the analysis, and it is possible that this might improve the energy resolution as well.  Also, the amount of improvement between 40% and 50% cuts, for example, in the DOMs method was not very large, and additionally between 60m and 80m cylinder distance.  It might be worthwhile to study the tradeoff between changing these parameters and getting more events to qualify for an energy calculation with this method.
