.. _ipdf:

====
IPDF
====

Maintainer: Kevin Meagher

.. toctree::
   :maxdepth: 1

   release_notes


Overview
========

This project provides implementations of various
"photo-electron probabilities" (PEP) and DOM-level likelihoods (e.g.
SPE1st, MPE, etc) which can be defined using any PEP.
ipdf does not provide IceTray modules but instead it provides functions for use within other I3Modules. ipdf used to to have a root based gui, this was retired.

The library is designed to be very extendible and still provide very fast
code. This is achieved through templates. This requires a certain amount
of C++ experience to read and use. Using templates allows better optimization
by the compiler. This is relevant because usually the calculation of PDFs and
likelihoods happen in the innermost loops of reconstruction algorithms.

In previous versions of this project the code could be compiled standalone and
used in ROOT. This functionality has been unmaintained for a very long time and
has therefore been cleared out. So now ipdf is a pure icetray project.  If you
are interested in making ipdf an independent project again, and/or reenable the
ROOT support, then check out a revision from SVN (older than ~124000) to see
what this might entail.

The library was originally designed by Simon Robbins in Wuppertal in 2004,
under the supervision of Christopher Wiebusch. After Simon left the
collaboration in 2005, David Boersma has maintained the code.


The ipdf package is designed to provide an interface to single photo-electron
PDFs and likelihood functions, more specifically for likelihood based muon and
shower reconstruction.  The software is intended to be extensible to any
conceivable PDF, likelihood or emission hypothesis; this is achieved with
extensive use of templates. This also ensures that the compiler can maximally
optimize the code, which is relevant as the PDFs are usually at the inner loops
of the reconstruction process. 



One drawback of heavily templated code is that the code itself does not provide
an easy definition of the interface, in contrast to designs based on inheritance where
new implementations can simply overload the virtual methods of base classes.
Also for given templates it is not always easy to guess for the novice user
which types might be used as template arguments.  Hence an attempt is made to
describe most of the interface in these docs (see next section).

A large part of the functionality implemented in the ipdf project is used and wrapped 
in the :class:`I3GulliverIPDFPandel` (which lives in the ``lilliput`` project;
for instantiation of the ipdf templates see :class:`I3GulliverIPDFPandelFactory`). 
This code is extensively used in IceCube in both online and offline processing.

The core of the code was originally written independent of any particular software framework,
but over time we have allowed icetray-specific code in some places. On the other hand
all practical examples in the original code were made using ROOT, and in order to
be compatible with CINT several core classes had preprocessor code in it in order
to make it ROOT-compatible; all this ROOT stuff has been removed, as part of an effort
to make the IceCube code as independent from ROOT as possible, in order to make it
better maintainable and more reliable.


Goals
-----

* To separate the single photo-electron PDF, likelihood and minimization implementation
* Easy to switch between different likelihoods and PDFs
* Efficiently implemented (minimize CPU time rather than memory use)
* Interactively run (at least) a sub-set of the functionality inside ROOT
* Easy to write a new PDF or likelihood
* Tabulated PDFs and likelihoods automatically produced and used

Terminology
===========

The code and the comments are written in a slightly more abstract terminology than we normaly 
use in IceCube analysis discussions.

Emission hypothesis, emitter
    A phenomenon that causes light to be emitted into a medium.
    Examples are muons and showers, described in more detail below. 

Time residual
    For a given emitter (muon, shower) and a DOM the arrival time of unscattered Cherenkov photon can be computed, which is sometimes called the direct arrival time.
    The difference between the time of an actual hit and this computed direct arrival time is called the time residual. 

PDF
    Probability Distribution Function. This is a general concept described in many statistics textbooks.
    In the context of IceCube event reconstruction it is used to refer to the normalized probability density
    of the arrival times of photons at the photocathode of the PMT in an IceCube DOM
    (or similar: detection times of photoelectrons).
    If the light emitter is far away from the DOM then the maximum and the width of this PDF increase with the distance. 
    If the light emitter (e.g. a muon or a cascade) is close to the DOM (less than an average effective scattering length,
    about 30m) then this function is strongly peaked for small values of the time residual.
    Usually we use Gauss-convoluted PDFs (sigma ~2-15 ns) to account for finite time resolution (and systematic uncertainties).

PEP
    This stands for "Photo Electron Probability". We usually call this PDF, but the original author of the ipdf project thought that PEP was a better name.

Infinite muon
    The most common "emission hypothesis".
    It represents an minimum ionizing muon passing through or close by the array of optical modules,
    emitting Cherenkov light uniformly along the track.

Point cascade
    Something which emits its light practically isotropically from a point, e.g. the particle shower from a CC interaction
    by 100 GeV electron neutrino. That shower is so small that its 
    small that all . Could also be used for simple flasher reconstruction.

Directional cascade
    A shower that is energetic enough that we can hope to reconstruct also its direction.

Likelihood
    In ipdf the "likelihood" refers to the likelihood to observe the measured pulses in *one* given optical module and given emission hypothesis (track, cascade, etc.).
    Examples are SPE, MPE, PSA.

"All OMs likelihood"
    This is the likelihood of the whole event.


Acronyms
--------

* PDF: Probability Density Function, Usually referring to PEP (see below). likelihood: A mathematical combination of PDFs for the whole event (usually a product of all hits in all OMs in the detector).
* PEP: single PhotoElectron Probability density function. The probability density of the arrival time of each single photo-electron
* SPE: Single PhotoElectron likelihood function. Likelihood of the arrival-time of each photo-electron. Usually mathematically only a product of PEPs but conceptually different.
* SPEAll: SPE likelihood for all photo-electrons, formed from the product of all the PEPs.
* SPE1st: SPE applied to only the first photo-electron at each OM, formed from the product of all the 1st PEPs at each OM. This is actually an mathematically ill-formed likelihood.
* MPE: Multi-PhotoElectron likelihood function. The likelihood of the first photo-electron's arrival time, given that in total N photoelectrons arrive at the OMs.
* PSA: Poisson Saturated Amplitude likelihood function. This is the MPE likelihood convoluted with Poisson distributed amplitudes.

Implementation Details
======================

As discussed in the AMANDA Reconstruction Paper\ [#]_ the core method for muon track reconstruction in AMANDA uses maximum likelihood techniques. IPDF is designed to provide an interface to the likelihood and pdfs that is flexible enough to accomodate different:

* hypotheses (e.g. muon, cascade)
* likelihood types (e.g. SPE, MPE, PSA)
* pdf types (e.g. Pandel, UPatched Pandel, Gaussian convoluted Pandel)

Please see the AMANDA Reconstruction Paper for more details.

The pdfs in ipdf are calculated as a function of time, with reference to the "geometrical time". The geometrical time is calculated as the expected time of arrival, relative to the given T0, for light travelling along the Cherenkov cone without scattering or jitter. This includes the time taken for the particle to travel from the "T0 point" to the light emission point. For muons this defines the following geometry:

The calculated pdfs take into account different effects:

* jitter (i.e. PMT jitter and timing accuracy of leading edge determination)
* showering
* scattering

These are shown below. Noise is not easy to implement in mathematically correct way. In the short term we plan to add a simple "noisy pdf" by simply adding a small constant to the pdf; this is however not a wholly satisfactory way of accounting for noise.

The ipdf likelihoods assume that photo-electron hits are ordered in time. The I3HitOm class assumes that the I3AnalogReadout returns that leading-edges time ordered.

Pandel
------

The ipdf project can be used to implement any pdf, but we mostly use the Pandel pdf, in particular the gauss convoluted Pandel pdf as described in this paper\ [#]_. 
The original Pandel function was written down by Dirk Pandel, an undergrad student at DESY Zeuthen in the mid 90s\ [#]_.
This function has a very simple analytical form, and it worked surprisingly well, except when tracks pass very close to the optical modules.
To avoid the very sharp peaks in the pdf due to such close tracks the pdf had to be convoluted with a gaussian.
It turns out that there is again a closed analytical form for this, in terms of so-called hypergeometric functions. But to actually use this in a numerical context, like reconstruction, turns out to be tricky.
That is what the above mentioned paper is about. It gives a recipe to compute this in a numerically robust and fast way.

The Gaussian-convoluted Pandel calculation is broken down into five regions.  In region 1 (small distances, small time residuals), the analytic representation can be used exactly.  In the other regions, the analytic representation results in a GSL over/underflow error, and an approximation is used.  Approximately 90% of the evaluations (for IceCube data) fall into region 1.  To improve performance in this region, we use three different techniques to evaluate the PDF:

1.  For large time residuals and large distances, we use the existing calculation of the Gaussian-convoluted Pandel PDF.  This calculation takes the difference of 1F1 confluent hypergeometric functions computed by GSL.
2.  For negative time residuals (eta > 1.35), the two terms in the above computation are almost equal, resulting in poor conditioning.  For this portion of region 1, we instead use Tricomi's hypergeometric function, which is well-conditioned and more precise.
3.  For small time residuals and small distances, we can efficiently compute 1F1 hypergeometric functions using a power series by precomputing the denominators in the hypergeometric power series terms.  This is approximately 10x faster than GSL with similar accuracy.


Conventions and Units
---------------------

Following the IceTray unit convention, the ipdf project uses meters and nanoseconds as its base unit system; this is assumed throughout the code internal to ipdf. The conversion to this system of units must be made as all the input points into ipdf. This is demonstrated for the I3 classes:

* ``InfiniteMuon``
* ``I3OmReceiver``
* ``I3OmHit``

The majority of the conversions occur inside ``InfiniteMuon``. This class has two constructors for I3 framework classes:

.. code-block:: c++

 explicit InfiniteMuon(const I3TrackPtr);

 InfiniteMuon(const I3Position&,
                   const I3Direction&,
                   double energy=-1.e99);

These will be extended in the future in order to use more complicated likelihoods.

The ``InfiniteMuon`` class is also where the majority of the other conventions on coordinate systems come to bear. The most significant is that of the muon direction. Internally ipdf uses both the directional cosines and the spherical polar coordinates of the muon propagation direction. That is, ipdf defines tracks in the direction that the particle is moving. This is different from much of the I3 code, which instead uses the spherical polar coordinates with respect to the source of the particle, i.e. much like astronomy.

``InfiniteMuon`` provides methods to access the track direction in both these conventions, see `IPDF::InfiniteMuon <../../doxygen/ipdf/classIPDF_1_1InfiniteMuon.html>`_.


Example Use of IPDF
===================

The tutorial demonstrates the basic syntax for using the IPDF interface (see private/tutorial/main.cxx).

.. code-block:: bash

   cd $I3_BUILD
   ./bin/ipdf-tutorial

The standard interface to ipdf is AllOMsLikelihood, which provides the interface to the likelihood functions. ``public/ipdf/AllOMsLikelihood.h`` demonstrates the basic interface:

.. code-block:: c++

   like_result getLikelihood(const Response& response,
                             const EmissionHypothesis& emitter) const;
   like_result getLogLikelihood(const Response& response,
                                const EmissionHypothesis& emitter) const;
   like_result getIntLikelihood(const Response& response,
                                const EmissionHypothesis& emitter) const;

The code in ``public/ipdf/Simple`` is provided to show the basic IPDF interface for the classes implementing hits, OMs, etc.

The basic PEP (photo-electron PDFs) interface is shown in ``public/ipdf/Simple/SimplePEP.h``. This includes a constructor (in this case the default ctor) and the 5 methods provided by all PEPs to access the PDF, hit probability and expected number of photo-electrons:

.. code-block:: c++

   double getPdf(const PEHit& pehit, const InfMuonHypothesis& eh) const;
   double getLogPdf(const PEHit& pehit, const InfMuonHypothesis& eh) const;
   double getIntPdf(const PEHit& pehit, const InfMuonHypothesis& eh) const;
   double getHitProb(const OmReceiver& omr, const InfMuonHypothesis& eh) const;
   double expectedNPE(const OmReceiver& omr, const InfMuonHypothesis& eh) const;



Use Within the IceTray Environment
==================================

The IPDF core code does not depend on any other IceTray projects but uses the GSL libraries. All ROOT dependencies have been removed.

The IPDF project is bundled with extensive test suites, which test the majority of the IPDF functionality. By running `make ipdf-test` these tests can be performed; they are also a good place to begin learning about IPDF.

A small number of examples are provided with IPDF to demonstrate typical usage, see Example Use of IPDF.

Using ipdf in IceTray
----------------------

The physics classes in ipdf (likelihood, emission hypothesis, etc.) have been designed independent of IceCube.
The detector configuration and light receivers of IceCube are necessarily IceCube specific.

`I3OmReceiver`
    The ipdf class to represent the IceCube DOMs: position, orientation, relative sensitivity, noise rate.

`I3PEHit`
    This is *not* what it says, because it is not a single photoelectron hit. Instead, it is basically equivalent to the `I3RecoPulse` class.
    It represents a hit time and some estimate of the number of photo electrons recorded at or around that time (the "charge").  

`I3HitOm`
    This *is* what it says: a hit optical module. It's characterized by an OM receiver object and a list of "PE hits", like above.
    
`I3DetectorConfiguration`
    A list of all I3OmReceivers in the IceCube array. It basically contains the same information as I3Geometry, but with a different API.

`I3DetectorResponse`
    A list of all I3HitOms in an event. It is basically equivalent to the `I3RecoPulseSeriesMap` class, but then including the geometry information of all the hit DOMs. 

`I3MediumPropertiesFile` and `I3MediumProperties`
    This is a facility to read text files with optical properties of the ice: scattering in absorbtions length for various layers.
    The I3MediumPropertiesFile class deals with reading the IceFile in photonics format, and the I3MediumProperties class lets you query this information at any depth, it takes care of the interpolation.

IPDF Interface to I3 Framework
-------------------------------

The IPDF project interfaces to the I3 framework in order to obtain information regarding:

* Detector geometry and hardward
* Event information - 'hits' and 'pulses'
* Seed hypothesis

This interface is provided by the classes in the directory public/ipdf/I3. The use of this is demonstrated in the example likelihood reconstruction ``I3IPdfLikelihoodModule.h``.

The "points of contact" to the framework are:

Construction of the detector geometry/hardware configuration:

.. code-block:: c++

  IPDF::I3DetectorConfiguration(const I3InIceGeometry&)

For example:

.. code-block:: c++

  IPDF::I3DetectorConfiguration dgeom(GetGeometry(frame).GetInIceGeometry());

Construction of the detector response:

.. code-block:: c++

  IPDF::I3DetectorResponse(const I3OMResponseMap&, const IPDF::I3DetectorConfiguration&)

For example:

.. code-block:: c++

  IPDF::I3DetectorResponse dresponse(event.GetOMResponseMap(),dgeom);

Creation of "emission hypothesis" (e.g. infinite muon):

.. code-block:: c++

  IPDF::InfiniteMuon(const I3Track&)
  IPDF::InfiniteMuon(const I3Position&, const I3Direction&)

For example:

.. code-block:: c++

  IPDF::InfiniteMuon muon( hypothesis(i3track) );
  IPDF::InfiniteMuon muon( posn, dirn );

Create the likelihood and PEP (photo-electron PDF) objects, e.g.:

.. code-block:: c++

  template<class IceModel> 
    IPDF::Pandel::UnConvolutedPEP(double absorptionLength, double jitter)
  template<PEProb> SPEAll(const PEProb& spp)

For example:

.. code-block:: c++

  typedef IPDF::Pandel::UnConvolutedPEP<IPDF::Pandel::H2> PandelPDF;
  IPDF::AllOMsLikelihood< IPDF::Likelihood::SPEAll<PandelPDF> > likelihood;

Calculate the likelihood:

.. code-block:: c++

  template<class Response, class EmissionHypothesis>
    double getLikelihood(const Response& response,
                         const EmissionHypothesis& emitter) const

For example:

.. code-block:: c++

  double result = likelihood.getLikelihood(dresponse,muon);

This can be seen in the code for ``I3IPdfLikelihoodModule`` and the ``IPDF::TMinuitMinimizer``, which is an example minimization scheme.


Using ipdf with PhotoSpline
---------------------------

The ipdf project can be used to implement any pdf, not just Pandel.
So we have a class that uses a ``I3PhotoSpline`` service to compute pdf values and expected number of photoelectrons (given a DOM position and a track or cascade, of course). 
This has so far only been used for reconstruction studies (master thesis by Michael Soiron\ [#]_).
The spline-reco project has a dedicated likelihood implementation using photospline.

See Also
========

.. toctree::
   :maxdepth: 1
	      
   Python API Reference </python/icecube.ipdf>
   C++ API Reference </cpp/ipdf>

References
==========

.. [#] \J. Ahrens, et al., "Muon Track Reconstruction and Data Selection Techniques in AMANDA" , NIM A524, p.169-194 (2004). `astro-ph/0407044 <http://arxiv.org/abs/astro-ph/0407044>`_

.. [#] \N. van Eijndhoven, O. Fadiran, G. Japaridze, "Implementation of a Gauss convoluted Pandel PDF for track reconstruction in neutrino telescopes," Astroparticle Physics 28, 456 (2007). `arXiv:0704.1706 <http://arxiv.org/abs/0704.1706>`_
 
.. [#] \D. Pandel, "Bestimmung von Wasser- und Detektorparametern und Rekonstruktion von Myonen bis 100 TeV mit dem Baikal Neutrinoteleskop NT-72," Diploma Thesis, Humboldt-Universität zu Berlin, 1996 (in German). https://docushare.icecube.wisc.edu/dsweb/Services/Document-74814

.. [#] \M. Soiron, "Untersuchungen zur Verbesserung der Myonspur-Rekonstruktion in IceCube,"
       Diploma Thesis, Rheinisch-Westfälische Technische Hochschule (RWTH) Aachen, 2012 (in German).
       http://wwwo.physik.rwth-aachen.de/fileadmin/user_upload/www_physik/Institute/Inst_3B/Forschung/IceCube/publications/thesis_MS.pdf
