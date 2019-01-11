.. _dipolefit:

=========
dipolefit
=========

Dipolefit is a first guess algorithm used by AMANDA and Icecube-9. Its performance relative to other first guess algorithms did not improve for larger detectors and it has not been used since.

**Original Authors**: Hagar Landsman, contributions from Dusan Turcan

**Currnet Maintainer**: Alex Olivas

.. toctree::
   :maxdepth: 1
   
   release_notes


Overview
========

Dipole Fit is a first guess algorithm. It does not generate as good an initial track as the direct walk, but it is less vulenrable to specific class of background events: almost coincident atmospheric muons from independent air showers in which the first muon hit the bottom and the second muon hits the top of the detector. A big improvement to dipole fit performance is obtained by calculating the dipole moment not between two closest (in time) hits, but hits with a distance of N hits between them. 
The best performance were obtained using ``N=4,5`` or ``N`` =half the number of hits in the event (set ``DipoleStep=0`` in the steering file).

The Dipole algorithm considers the unit vector from one hit OM to an other hit OM as an individual dipole moment. Averaging over all individual dipole moments yields the global moment ``M``. 

Algorithm
=========

- Get all first leading edge hits times, amplitudes and positions.
- Sort them according to time (there is a slight different between Siegmunds and this module results concerning the secondary order of same time hits).
- Calculate for each pair (determined by ``N=DipoleStep`` parameter) the dipole moment: :math:`\vec{M}=\frac{1}{N_{ch}-1}\sum\frac{\vec{r_i}-\vec{r}_{i-N}}{|\vec{r_i}-\vec{r}_{i-N}|}`
    
- Or in case a weight was assigned (``AmpWeightPower>0``): :math:`w_i=\left(\frac{amp_i+amp_{i-N}}{2}\right)^{AmpWeightPower}`
   
- obtain directions using ``M``. :math:`\vec{M}=\frac{1}{\sum{w_i}}\sum\frac{w_i*(\vec{r_i}-\vec{r}_{i-N})}{|\vec{r_i}-\vec{r}_{i-N}|}`
   
Output Variables
================

- **zenith** - calculated from dipole moment.
- **azimuth** - calculated from dipole moment.
- **vertex** (rx,ry,rz) - weighted average on all hits positions: :math:`\vec{R}=\frac{1}{\sum{w_i}}\sum\frac{w_i*(\vec{r_i}+\vec{r}_{i-N})}{2}`
   
- **T** - :math:`T=\frac{1}{\sum{w_i}}\sum\frac{w_i*(t_i+t_{i-N})}{2}`
- **ampsum** - :math:`w_i` as described above.
- **NHits** - Number of pairs used.
- **MaxAmp** - maximum value of :math:`\frac{amp_i+amp_{i-N}}{2}`

Input Variables
===============

- **Name** - [DEFAULT="DipoleFit"] Name to give the fit results the module adds to the event.  
- **MinHits** - [DEFAULT=5]  Minimum number of hits. Event with fewer hits will not be reconstructed. 
- **DipoleStep** - [DEFAULT=0]  The distance between two time sorted hits that used to calulate the dipole. For ``DipoleStep=x``, the 'dipole' direction will be calculated using position(i) and position(i+x). Use ``DipoleStep=0`` to use dynamically changing N/2 (recommended)
- **AmpWeightPower** - [DEFAULT=0] Hits are weighted with the amplitude raised to this power. Usually =0 so hits_weight=1, or =1 so hits_weight=amplitude
- **InputRecoHits** - [DEFAULT=""] RecoHits to use for input
- **InputRecoPulses** - [DEFAULT=""] RecoPulses to use for input

Examples and Tests
==================

To run tests execute the scripts in resources/scripts. 
The following tests will be performed:

- ``general`` - a General test, reads events and do a line fit reconstructions with the default values to all parameters. The following variables are checked: Zenith and Azimuth values between 0..2*pi, :math:`V^{2}=v_{x}^{2}+v_y^2,v_z^2`.
- ``MinHit`` - Same as test "general". but with MinHits set to 99999999 (so no event will be reconstructed).
- ``recoos`` - Comparison between I3DipoleFit and recoos' linefit. EventWeight is set to 0.0 and hitselection is the same. (disabled - about 0.5% of reconstructions are different).
- ``AmpWeightPower`` - Same as test ``general``. AmpWeightPower is changed. 
- ``Step`` - Same as test ``general``. DipoleStep is changed. 

Also see
========
-   `Doxygen Documentation <../../doxygen/dipolefit/index.html>`_
-   `icetray inspect <../../inspect/dipolefit.html>`_
