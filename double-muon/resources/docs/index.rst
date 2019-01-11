.. _double-muon:

===========
Double muon
===========

This a project to collect attempts to identify and possibly
reconstruct double muon events (or coincident muons, or whatever
you'd like to call them). This can be for the purpose of eliminating
background as well as finding interesting physics signatures.

  **Maintainer** : Kevin Meagher

.. toctree::
   :maxdepth: 1
   
   release_notes


Introduction
=================================

The goal of this project is to provide tools to facilitate the
identification and reconstruction of events in which two coincident
muon tracks triggered the In-Ice detector. This can be both for the
purpose of background reduction (flagging coincident downgoing muons)
and for identifying interesting physics events (the study of high
p_T muons produced in UHECR air showers).

Currently we have a module for splitting hitsets and a set of
gulliver-services to perform a 10 parameter fit to a 2-muon event.

Splitting events
================

The two (or more) tracks of an event with coincident muons
are typically separated in time and/or space. If they are, then
that offers a simple way to disentangle the event. The I3ResponseSplitter
module implements a few very basic methods to split an event
into two subevents:

- Average time: determine the average time of the pulses, and split
  the event in pulses before and after the average time.
  (Jargon: time-splitting). UPDATE: since it is intrinsically
  bad to do anything that depends on individual pulses, a new option
  is added to configure the pulse weighting. The 'old' weighting weights
  all pulses equal, the 'Charge' weighting weights them by pulse charge
  and the 'DOM' weighting uses only the first pulse in each DOM
  (no charge weighting). Moreover, you another option allows you to
  split by the *median* of the pulse times (with your favorite weighting
  method) instead of the *mean*.
- Brightest string: determine which string has the most pulses
  (ignoring charge for now); select the strings within some horizontal
  distance from the brightest string; and keep the pulses on those
  selected strings in one subevent, the rest in the other subevent.
- Geometrical: Use a single-muon fit to the full event, find the plane
  through the COG of the pulses and perpendicular to the fitted track.
  This plane separates the two subevents.
  (Jargon: geo-splitting).
- Time residual: Use a single-muon fit, find all pulses which have
  a time residual within some (configurable) interval and put them in
  one subevent, put the remaining pulses in the other subevent.

Finding high-pT muons in showers
================================

The brightest string method is intended for the distinction of two separated
parallel tracks, specifically a cosmic ray shower which includes a muon with
a large lateral separation. Typically the main muon bundle will be much
brighter in the array than the individual muon, so the string with the most
hits usually corresponds to the bundle (for a showers with zenith angles
below about 30).

Once the track of the bundle has been found, the pulses can be sorted
according to their time residual relative to the fit. Pulses from the
separated muon will mostly have a negative time residual (because they are
coincident in time, but laterally separated from the fit).

Rejecting coincident downgoing muons
====================================

2x5par fit
----------

For the purpose of rejecting coincident downgoing muon events (for which a
single fit might result in an upgoing track), the event is split into two
subevents and regular single-muon fits (e.g. linefit and loglikelihood fits)
are performed on both subevents (this is sometimes called a 2x5 parameter
fit).  For a given full event that is actually a nice upgoing neutrino
event, the fits to both subevents should also result in upgoing tracks,
while for coincident downgoing muons at least one of the fits might result
in a downgoing track.
   
This procedure is typically done with both the "time-splitting" algorithm
and the "geo-splitting" algorithm.  We expect that both algorithms
successfully eliminate only a subset of the coincident muons, and that these
subsets are overlapping with eachother only partially. So using both
algorithms and combining the results in some way seems to be the most
effective strategy.  For instance in the IC-22 diffuse analysis, for each
event the combined likelihood of the subevent-fits is computed for both
algorithms, and the best one is then used for the double muon cut (for the
event in question).

full 10-parameter double muon fit
---------------------------------

When two coincident muons cross close-by, then for several pulses it is not
obvious to which track they "belong", and the above-described pattern
recognition methods may not work so well. One can try to fit events with a
10-parameter double muon fit, in which both muons are varied simultaneously
and pulses are not "allocated" exclusively to one of the muon tracks.

- A seed service (I3DoubleMuonSeedService) builds an initial two-muon
  seed using two other (regular) single-track seed services, which in turn
  might get their first guess tracks from fits performed on subevents
  obtained with I3ResponseMapSplitter.
- A parametrization service (I3DoubleMuonParametrization) translates 
  between 10 parameters from the minimizer and two I3Particle objects (muons)
  to be used by the I3DoubleMuonLogLikelihoodService.
- The I3DoubleMuonLogLikelihood service computes for each DOM and for
  each track the expected number of photoelectrons (NPE) arriving at the DOM,
  and computes the total PDF as the NPE-weighted sum of the individual PDFs.
  
These services can be used with a gulliver module (typically I3SimpleFitter)
to perform a 10-parameter fit. Just like for 2x5-parameter fits, good
upgoing events should result in two upgoing subtracks while for coincident
events at least one subtrack should be down-going.  And for the same reasons
stated in the previous section it is probably a good idea to do the fit
seeds obtained with both time-splitting and geo-splitting.


See Also
--------

.. toctree::
   :maxdepth: 1
	      
   Python API Reference </python/icecube.double_muon>
   C++ API Reference </cpp/double-muon>
   IceTray Inspect Reference </inspect/double_muon>

Further Reading
===============

* `Reconstruction and identification of coincident muon events (wiki) <http://wiki.icecube.wisc.edu/index.php/Reconstruction_and_identification_of_coincident_muon_events>`_      
* :ref:`gulliver-modules`
* :ref:`ipdf`
   
