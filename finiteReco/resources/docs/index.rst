.. _finiteReco:

FiniteReco
==========

**Maintainer:** Sebastian Euler <sebastian.euler@icecube.wisc.edu>

.. toctree::
   :maxdepth: 1
   
   release_notes
   example_script

Overview
~~~~~~~~
FiniteReco is a tool to identify and reconstruct finite (starting, stopping
or fully contained) muon tracks based on the hit/no-hit probability of the
DOMs.

Idea
~~~~
IceCube is a huge detector with |1km3| volume, measuring mainly
muon-induced events. Most analyses are searching for muons produced by
neutrino interactions in the ice around or the rock below the detector. Due
to the huge size of IceCube, many of those interactions happen actually
inside the detector. With the same reasoning muons can also stop in the
detector. These different possibilities lead to different event topologies
in the detector, referred to as shape in the IceCube software. For muon
tracks there are four different shapes that can be distinguished:

.. |1km3| replace:: 1km\ :sup:`3`

* InfiniteTrack: through-going events neither starting nor stopping in the detector.
* StartingTrack: muon tracks starting inside of the detector
* StopingTrack: muon tracks which stop in the detector
* ContainedTrack: muon tracks stopping and starting inside of the detector

.. note::

   The inside and outside of the detector is not well defined. IceCube
   has no boundaries as such. An experimental definition is fine most
   of the time (but it has to be stated).

These different shapes are important for different physics topics.
InfiniteTracks are in some sense the standard signature. Most of the tracks
above 10 TeV are of this type. The other three types have smaller effective
volumes because the start and/or stop point are required to be inside the
detector. Thus, these tracks have on average lower energies. This is
especially true for ContainedTracks. Here the length is confined to be below
approximately 1km. This directly limits the energy. Due to their special
topology finite tracks have to be treated differently from through-going
tracks. In addition to the direction, the start and stop point have to be
reconstructed. This project provides tools for the selection and analysis
of finite tracks. This documentation describes all parts of the software
including information on how the algorithms are implemented and how to use
them.

General
~~~~~~~
The FiniteReco project is implemented within the IceCube software framework.
It provides modules and services for the data processing as well as classes
for special calculations and output. It depends on several other projects
in the IceCube software framework, the most important of which are the
:ref:`gulliver` project for the calculation of LLH algorithms and
:ref:`photonics-service` to access the photorec tables.

First Guess
~~~~~~~~~~~
FiniteReco provides a dedicated module for getting a first guess of the
start and stop point of a finite track. It does a simple calculation based
on the first and last hit along a provided track.

.. toctree::
   :maxdepth: 2

   I3StartStopPoint

LLH Algorithm
~~~~~~~~~~~~~
The LLH algorithm is the core of FiniteReco. It is based on the
probabilities of the individual DOMs to see a hit or not, given
different track hypotheses. The calculation of these probabilities
is most critical and therefore discussed first.

.. toctree::
   :maxdepth: 2
   
   CalculationOfProbabilities
   LengthReco
   I3LengthLLH
   StringLLH

Cut Parameters
~~~~~~~~~~~~~~
Besides the start and stop point (and thus the length) of a reconstructed
track, FiniteReco also provides a few parameters which can be used as cut
parameters in an event selection.

.. toctree::
   :maxdepth: 2
   
   I3StartStopLProb
   I3FiniteCuts

See also...
~~~~~~~~~~~
* The `doxygen <../../doxygen/finiteReco/index.html>`_ docs.
* The `wiki <https://wiki.icecube.wisc.edu/index.php/FiniteReco>`_ page.
