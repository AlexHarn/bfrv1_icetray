.. _rpdf-glossary:

========
Glossary
========


The code and the comments are written in a slightly more abstract terminology than we normally 
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
