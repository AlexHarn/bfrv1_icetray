..  
.. @file index.rst
.. @author Michael Larson
.. 

.. _vuvuzela:

Vuvuzela
===========
Vuvuzela generates noise.

Vuvuzela is a module designed to replace noise-generator for IceCube simulation. The
old set of code was designed solely for thermal noise, but recent studies have shown
a nonthermal component of our detector noise that isn't well modeled in noise-generator.

Vuvuzela models both the thermal noise and nonthermal noise arising due to decays in
the PMT and DOM glass and subsequent scintillation. The thermal component and decays
are both drawn from uniform distributions in time while the scintillation component
is pulled from a power law distribution in the time between hits. In addition, the
number of hits due to scintillation is sampled from a Poisson distribution with a
constant mean number of hits.

.. toctree::
   :maxdepth: 3
   
   release_notes
