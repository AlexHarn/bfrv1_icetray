..
.. copyright  (C) 2011-2012
.. The Icecube Collaboration
..
.. $Id$
..
.. @version $Revision$
.. @date $LastChangedDate$
.. @author Nathan Whitehorn <nwhitehorn@physics.wisc.edu> $LastChangedBy$

.. _millipede-main:

Millipede
=================

Millipede is a toolkit for measuring the particle energies in IceCube,
extending previous tools like photorec by being able to solve for the
energies of multiple particles in the detector simultaneously using
an unfolding algorithm. This is designed to be used to measure the
energy loss of a muon along its track, but can also be used to reconstruct
the relative contributions of different flashers or multiple muons
simultaneously present in the detector. The algorithms here can also
efficiently reconstruct the energies of single objects (for example,
small cascades).

This project provides several modules for measuring energies (MuMillipede,
PyMillipede, and Monopod) as well as Gulliver likelihoods and parametrizations
for doing more complicated reconstructions of composite particles
(MillipedeLikelihood and MuMillipedeParametrization). In addition, two tray
segments are provided with configurations of MuMillipede optimized for high
and low energy muons.

Common Options
^^^^^^^^^^^^^^^

The various Millipede modules and services have largely similar options.
The common ones are touched on here:

1. MuonPhotonicsService
	Name of or pointer to a photonics service from which Millipede will
	look up light responses for finite bare muon tracks.

2. CascadePhotonicsService
	Name of or pointer to a photonics service from which Millipede will
	look up light responses for cascades.
	
	.. note::
	   
	   The photonics services passed to the module MUST support
	   GetProbabilityQuantiles() if using timing in the fit.

3. ExcludedDOMs
	List of names of frame objects containing DOMs or portions of readouts
	to ignore in the fit. For each entry in the list, Millipede will check
	if it is a vector of OM keys or an I3TimeWindowSeriesMap. Entries in
	an OM key list will cause those DOMs to be ignored completely. Entries
	in an I3TimeWindowSeriesMap will cause only the marked portions of the
	readout to be excluded unless PartialExclusion is set to False, in which
	case the entire readout from these DOMs will be ignored. By default,
	this is set to ["BadDomsList", "CalibrationErrata",
	"SaturationWindows"], which should be sufficient for most applications.
	Entries in this list not present in the frame are ignored. The ignored
	data are the union of all present entries.

4. PartialExclusion
	Controls the behavior of the elements in ExcludedDOMs. If set to True
	(the default), only marked regions of the readouts in DOMs referenced
	in one of the objects in ExcludedDOMs are ignored. If set to False,
	all DOMs with any time marked bad are ignored completely.

5. PhotonsPerBin
	Number of photoelectrons to include in each timeslice. If set to -1,
	timing will be ignored and fits will be based only on total amplitudes.
	Otherwise, the waveform will be divided into timing bins, each of which
	contains at least PhotonsPerBin PE, if possible. Setting to zero will
	maximally separate the data, with one timeslice for each pulse.

	Small positive values will increase the precision of the fit, but
	increase exposure to systematic errors in the photon tables and
	CPU time. Values below the 5-10 range should not be used in general
	due to the increased systematics exposure.

6. MuonRegularization (does not apply to Monopod)
	When doing multi-source fits, Millipede can apply Tikonoff
	regularization to the problem. Setting this parameter to a non-zero
	number will cause Millipede to penalize solutions that involve
	rapid variation in the light deposition of adjacent muon tracks.
	For long muon tracks, this means that the fit will try to keep
	the component of the light due to the bare muon track approximately
	constant throughout the event. The parameter itself tunes the strength
	of the regularization by setting the width of a Gaussian prior on the
	absolute energy difference between adjacent segments (actually the
	inverse of the square of the width): large values will make the muon
	components smoother at the expense of fit quality and small values will
	do the reverse. Note that setting this to a non-zero value makes the fit
	much slower.

7. ShowerRegularization
	The shower components of the fit can also be regularized. These
	are usually stochastic in nature, and so instead of penalizing
	differences between adjacent particles, setting ShowerRegularization
	to a non-zero value will cause the amplitudes of the showers to be
	kept small. This parameter is the inverse square of the with of a
	Gaussian prior centered at 0 energy -- the units are accordingly
	GeV^-2. As a result, very small values (10^-9 GeV^-2) are encouraged
	if this is used -- 10^-8 corresponds to a 10 TeV Gaussian.
	The parameter is intended to apply Occam's Razor to the problem:
	if a large cascade very far away and a small cascade nearby explain
	the observed light equally well, a non-zero value of
	ShowerRegularization will cause Millipede to prefer the small nearby
	one.

8. DOMEfficiency
	Fraction of the lab-measured PMT effective area available for light
	collection (e.g. unshadowed by the cable). This parameter scales the
	light-collection efficiencies stored in the calibration. The default
	is 0.99, following the simulation defaults.

9. Pulses
	The pulse series to use for input. The algorithm depends critically
	on no-hit information and expects noise, so the input pulse series
	SHOULD NOT be hit-cleaned in any way. Pulses must be accompanied by
	an I3TimeWindow object containing the first and last possible pulse
	time in the event.

10. ReadoutWindow
	Name of the I3TimeWindow object in the frame giving the first and last
	possible times of a pulse.
	Default: Use the name of the pulse series with "TimeRange" appended.

11. BinSigma
	If set to a finite number, PhotonsPerBin will be ignored, and
	time bins will be combined using the Bayesian Blocks algorithm
	until the mean PE rate in each bin differs from its neighbors at
	this significance level.
	Default: NAN

12. MinTimeWidth
	Timing structure less than this width should be ignored in the fit.
	Implemented as a hard cutoff when using fixed binning and a prior
	for Bayesian Blocks.
	Default: 8 ns
	
13. UseUnhitDOMs
	Use unhit DOMs when unfolding shower amplitudes.  This is slower
	but more accurate.
	Default: true


Cast of Characters
^^^^^^^^^^^^^^^^^^

MuMillipede
-----------

From an input muon track, MuMillipede will divide it into a set of equally
spaced finite bare muon tracks and EM showers, solving for the amplitudes of
all of them and storing the result in the frame as a vector of particles.
Parameters of interest besides the common ones:

1. MuonSpacing/ShowerSpacing
	Controls the spacing of the sub-muons and sub-showers spaced along the
	track. Smaller values provide increased resolution at the risk of
	increased fit time and possible overfitting due to having many
	degrees of freedom. 10-15 meters seems to be a reasonable number
	for IceCube-86 and high-energy particles.

.. note::
	In general, MuonSpacing MUST match the length of the finite muon
	segments provided by MuonPhotonicsService.

2. SeedTrack
	Input track to divide up. The exact behavior of the division is
	controlled by the particle's shape property. For infinite tracks,
	it will divide the track in segments within the detector boundary
	(see Boundary option). Semi-infinite (starting/stopping) tracks will
	extend from/to the (x,y,z,t) coordinates defined by the particle
	and go to/from the place they exit/enter the boundary cube. Contained
	tracks extend from their start point to their stop point and are
	not affected by the Boundary option.

3. Output
	Frame object name of the output vector of I3Particles. MuMillipede will
	also write an object of name OutputFitParams containing the likelihood
	and reduced likelihood of the fit, as well as the Chi-Squared value,
	the total charge, and the predicted total charge.

4. Boundary
	MuMillipede defines a detector boundary in the shape of a cube centered
	on the coordinate system origin and extending from -Boundary to Boundary
	on all axes. The default is 600 meters. Infinite or semi-infinite tracks
	will be clipped when they cross the edge of the cube.

MuMillipedeParametrization provides a Gulliver likelihood service for use with
iterative muon fits, and inherits all options from the
I3SimpleParametrization. In addition, it adds an additional parameter
(StartingCascadeStepSize). If this is set to a non-zero value, it allows
the initial cascade on starting or contained tracks to point in a direction
different from the main track direction. This is intended to be used to
separately fit the hadronic cascade produced by a neutrino from the direction
of the outgoing muon, which may be significantly different at low energies.

PyMillipede
-----------

PyMillipede provides an interface to solve for the energies of an arbitrary
collection of particles.  It takes one argument, Hypothesis, which should
be a python function. For every event, the python function is called and
passed a copy of the frame. It is then expected to return an I3VectorI3Particle
containing the list of I3Particles the energies of which are to be solved.
Upon completion, PyMillipede will write this vector, containing the final
energies, to the frame along with two fit parameters objects (see MuMillipede).
The first of these, written to OutputFitParams, contains the final likelihood
parameters of the fit. The second, written to OutputSeedParams, contains
the likelihood parameters associated with the initial energies passed to
Millipede by Hypothesis.

Monopod
-------

Monopod does a simple one-particle cascade energy fit, much like the
AtmCascadeEnergyReco project, except faster and within the Millipede framework.
It takes only two arguments: Output and Seed. Monopod will solve for the
energy of the particle found in the frame at Seed and save the result in Output.

Taupede
-------

Taupede solves the deposited energy for a double-bang tau neutrino hypothesis
with two cascades at a given position and direction.

The TauMillipedeParametrization can be used with Gulliver do fit the 7
remaining parameters of the double-bang hypothesis by likelihood maximization:
Vertex position (3 parameters), time (1 parameter), direction (2 parameters)
and tau decay length, i.e. distance between the two cascades (1 parameter). The
direction and position of the second cascade is determined by the parameters of
the first cascade and the tau decay length, since both cascades are assumed to
have the same direction.

There is the TaupedeFit traysegment which implements this full 9-parameters
double-bang reconstruction using Gulliver and TauMillipedeParametrization.

1. Tau
	Input tau track for which the two energy depositions are solved. The
	vertex of the track determines the vertex of the first cascade and the
	length of the track is taken as the tau decay length, i.e. a second cascade
	is placed at the end of the track with the same direction as the track and
	the first cascade.

2. Output
	Frame object name of the output vector of exactly two I3Particles. Both
	I3Particles will be cascades with the direction and vertices defined by the
	Tau input I3Particle. The energies of these two cascades are the fitted
	energy depositions of the double-bang hypothesis. Taupede will
	also write an object of name OutputFitParams containing the likelihood
	and reduced likelihood of the fit, as well as the Chi-Squared value,
	the total charge, and the predicted total charge.

photorec
--------

'photorec' is a mimic for the old I3PhotorecEnergyEstimator provided by the 
photorec-llh project. It is essentially the same as Monopod but for muons, using a 
single, infinite track source as its hypothesis. To do this it requires a 
PhotonicsService which reads from a special photorec muon table, which assuming one 
has such a table, is accomplished by adding an I3PhotonicsServiceFactory with parameter 
IsPhotorecTable set to true. 

The parameters of this segment are:

1. PhotonicsService
	An I3PhotonicsTableService which will evaluate the light expectation from 
	an infinite muon track. 

2. RecoParticleName
	The name of the seed track which will supply the geometric reconstruction 
	information.

3. RecoPulsesName
	The name of the input pulses to use for the reconstruction. 

4. ResultParticleName
	The name for the output reconstructed particle. 

How it Works
^^^^^^^^^^^^

Millipede uses PhotonicsService to estimate deposited energies. For only
one source, the deposited energy from that source is proportional to the
light yield. Knowing the yield from the photon simulation for a particle
at a reference energy and the amount of light in the detector then lets
you trivially compute the actual deposited energy.

For the one-source case (Monopod), Millipede follows approximately this
algorithm, but solves a Poisson likelihood instead of a simple linear fit.
This has no analytic solution, but is easily obtained by root finding. At
high energies, the solution is asymptotically convergent to the direct
proportional solution and is not very different at low energies. The root
solver can thus use the linear solution as a seed and then converges very
quickly.

The more interesting case is the multi-source one. The light yield for
one source can be thought of as the particle energy times a response vector,
with one entry for the light yield in each DOM or DOM readout timeslice at
a reference energy. This then yields a vector of the expected light yields
in each DOM or timeslice. For multiple sources, we can generalize this to
a matrix multiplication, where the energy is replaced by a vector of energies
and the response vector is replaced by a response matrix.

To solve the multi-source problem, then, all we need to do is to invert this
matrix multiplication, starting from the observed light yields. A simple
inversion or linear least squares approach will fail due to the possibility
of negative coefficients (meaning negative energy depositions), so Millipede
uses an algorithm that is guaranteed to give only non-negative values (Lawson
and Hanson, 1973). The resulting vector can then be interpreted as an energy
depositions from each particle in the set.

See Also
^^^^^^^^

.. toctree::
   :maxdepth: 1

   release_notes
   Python API Reference </python/icecube.millipede>
   C++ API Reference </doxygen/millipede/index>
   IceTray Inspect Reference </inspect/millipede>

