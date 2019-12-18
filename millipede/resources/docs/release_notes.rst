.. $Id: RELEASE_NOTES 131045 2015-04-03 19:20:35Z meike.dewith $
.. $Author: meike.dewith $
.. $Date: 2015-04-03 15:20:35 -0400 (Fri, 03 Apr 2015) $

Release Notes
=============

On the trunk
------------

* (r171368): Enforce minimum bin width in BinSigma mode
* (r171365): Fix an initialization bug in bayesian blocks rebinning
* (r170477): Use all OMTypes except IceTop and scintillators (Upgrade support)
* (r170476): Promote invalid DOM noise rate to a fatal error

June 12, 2018 Kevin Meagher
---------------------------
Release V02-01-06

* make the `LengthBounds` parameter accessible in the taupede segment

November 29, 2017 Kevin Meagher
-------------------------------
Release V02-01-05

* (r156726): Require user to explicitly set muon/cascade spacing to zero
             if no Muon/Cascade PhotonicsService is provided (#1968)
* Add `Boundry` parameter to millipede tray segment	     
* documentation updates	     
* python3 updates
* script cleanups: remove Finsih and TrashCan



March 24, 2017 Mike Richman (mike.d.richman@gmail.com)
--------------------------------------------------------------------
Release V02-01-04

* (r151307): Update tests to handle 2016 GCD (#1896)
* (r149760): Raise an error if a pulse starts _exactly_ at the end of the readout window (#1849)
* (r149503): Support multi-particle hypotheses in artist
* (r148349): Abort minimization for alpha<0 rather than crashing (#1773)
* (r146738): Fix bin counts when removing zero-width pulses (#1712)
   -- Jim Braun

May 2, 2016 Alex Olivas
--------------------------------------------------------------------
Release V02-01-03

* (r142136): Allow selection of photon tables from the GUI (#1249)
* No longer have to import icetray and gulliver,
  because you need their base classes.
  -- Jim Braun

April 3, 2015 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V02-01-02

- Correct multiplicity calculation in MillipedeLikelihood account for empty
  bins. The reduced log-likelihoods should now be slightly less insane.
- More helpful error message when Monopod is seeded with a track seed
- Small code changes to fix warnings
  -- Jakob van Santen

- Recognize I3_TESTDATA
- Fix tests
  -- Alex Olivas


November 20, 2014 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V02-01-01

- Fix to correctly handle events where a pulse has zero width, which
  can happen for sdst events that have two pulses extracted within
  0.5 ns of eachother
  -- Leif Raedel


September 23, 2014 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V02-01-00

- Add Steamshovel artist
- Fix memory-reuse bug in MillipedeDOMCacheMap
  -- Jakob van Santen


July 11, 2014 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V02-00-01

- Add some documentation of the photorec segment
  -- Chris Weaver


April 22, 2014 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V02-00-00

- Make new tests work with Python 3 
- Make conditional execution of Millipede segments work
- Switch from NMML to PCG as a minimizer (now feature complete and
  better than NMML in every way!) and solve all related problems. 
  High regularization sometimes slows convergence (it goes into the
  linear regime), but no pathologies seem to result
- Improve numerical properties of the energy optimization in PCG
- Fix segfault when gulliver hypothesis misses the detector and
  gradients are in use by just returning the noise likelihood rather
  than trying to create 0-width matrices
- Add an experimental prior option for the Bayesian binning
  -- Nathan Whitehorn 

- Make cholmod_sparse->I3Matrix conversion (and back) actually work 
- Query noise vector in PyPyMillipede 
- Pre-condition the gradient descent vector in the first iteration 
- Explicitly store the solution with the best likelihood, not just 
  the smallest gradient (not necessarily equivalent) 
- Correct sign error which effectively set MinWidth to 200, which
  was surely not the intention
- Store vector of particles for fit Foo as FooParticles instead of
  FooParams (which is not descriptive)
- Use segments in the examples
- Fill and return a real MillipedeFitParams instead of a fake one
- Fix a variety of bugs in the handling of partial exclusion windows
  (promote exclusion-window insertion to a function, increase the
  size of raw_bin_edges and raw_charges, prevent invalid bins from
  absorbing valid ones in rebin_min_entries(), only consider exclusion
  windows that overlap with the readout window) and add clauses to the
  binning unit test to exercise the above
- Add shims for named tuples and math.isnan() for Python <= 2.6
  -- Jakob van Santen 


January 21, 2014 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V01-09-00

- Teach PyPyMillipede how to evaluate gradients
- Re-enable and extend pulse binning test
- Handle an edge case that caused duplicate bin edges
- Refactor pulse binning code such that user-configurable rebinning
  can be treated separately from exclusion window rebinning
- Add tests for the energy solver and MonopodFit
- Add an optional Bayesian Blocks binning mode (can be useful in 
  situations where speed and reliability are more important than
  maximum best-case precision)
- Add option to use LBFGS in MonopodFit
- Use gulliver to put a custom FitParams in the frame instead of 
  running another Monopod module
- Integrate Patrick's Taupede parametrization and fit
- Add decorator to turn a segment containing a parametrization into
  a full-blown Millipede fit segment
- Add option to use multiple seeds
- Make binning in tests slightly coarser, for more convergence 
  -- Jakob van Santen

- Make the default parametrization and iteration count consistent:
  use the simple parametrization by default
  -- Nathan Whitehorn


August 21, 2013 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V01-08-00

- Improve pybindings for PyPyMillipede
  -- Jakob van Santen

- Update angular reconstruction examples to work with new code
- Fix errors in azimuthal gradient calculation in MuMillipede
  -- Patrick Hallen

- Make project compatible with Python 3
- Use i3_assert() for data quality checks so that they are also run 
  in release builds
  -- Nathan Whitehorn

- Add more documentation for monopod tray segment
  -- Laura Gladstone


June 29, 2013 Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-07-00

- Make the exclusion strategy used for the HESE analysis canonical.
- Add PyPyMillipede, a MillipedeService that can be instantiated
  and poked at from Python, for instance to power a steamshovel artist.
- When re-binning pulses to satisfy the PEPerBin threshold, stop merging
  bins if the duration of the new bin would exceed 200 ns.
  
  This fixes a regression introduced in r91790 that caused the re-binning
  loop to condense the time bins for DOMs with total charges less than
  PEPerBin into 2 bins: one empty bin extending from the beginning of
  the readout window to the start of the first pulse, and one containing
  the total charge and extending from the start of the first pulse to the
  end of the readout window. This bug severely degraded the performance
  of a single-source vertex fit on events with < 10 TeV deposited energy,
  but was much less noticable for high-energy events with many DOMs above
  the PEPerBin threshold, and thus went unnoticed for 9 months.
- Add pybindings for MillipedeDOMCache.
- Fixed some smaller bugs and compiler warnings.
  -- Jakob van Santen

- Fixed segfault for free cascade initial direction.
- Fixed gradients.
  -- Moriah Tobin

- Code cleanups, fixed tests
  -- Nathan Whitehorn


May 8, 2013  Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-06-01

- Fix a subtle bug for tracks that do not actually intersect the tracking
  volume where, instead of returning immediately, the code would try to
  solve for a very very very long track.
- Better input validation
- Tests use I3MCPE instead of I3MCHit.
- Code cleanup
  -- Nathan Whitehorn

- Step sizes for monopod gulliver fit are accesible as parameters
  from tray segments.
  -- Marcel Usner


February 21, 2013 Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-06-00

- Allow initial cascade direction to vary for starting or contained tracks
  in MuMillipedeParametrization
- Treat starting/stopping/contained/infinite tracks with their respective
  shapes in MuMillipede
- Update default DOM efficiency to 0.99
- Use SPE scale from dataclasses

  * NOTE: If you have been manually setting the DOMEfficiency to include the 0.85
    PE SPE scale, you MUST remove it as it is now always included (and
    may vary DOM-by-DOM in the future if we get around to measuring it)!
  * NOTE: This also means that all-default settings should produce correct
    energies with no required scalings or parameter changes. If you find
    you require any scalings, this is a bug -- please report it.

- Fix bug where NMML may (rarely) fail to converge and return a nonsense
  answer. Note that only the nonsense has been fixed; the circumstances under
  which NNLS is a terrible Poisson-likelihood seed have not yet been identified
- Add tray segments that emulate the behavior of some older reconstructions
  (e.g. Photorec) using Millipede
- The base class of MuMillipedeParametrization has been changed to
  I3SimpleParametrization, which may require some changes to option names
  in scripts using MuMillipede as a Gulliver service.


December 3, 2012 Nathan Whitehorn (nwhitehorn@icecube.wisc.edu)
--------------------------------------------------------------------
Release V01-05-00

- Fix Fisher matrix calculation to actually work
- Replace NNLS + BFGS2 tuneup with NNLS + NMML for heavy lifting, providing
  much more accurate answers
- Fix discrepancy between number of degrees of freedom calculated internally
  and reported to Gulliver
- Improve error checking, quieting some superfluous warning messages
- Require time window presence in frame for estimating readout window length
- Add regularization support back after updates to solve maximum likelihood
  explicitly
- Fix infinite loop possible in certain rare circumstances: convergence is
  now fully deterministic

September 26, 2012 Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-04-01

- Make Monopod instantly return for failed seeds.
- Support new I3FrameObject: I3Matrix they constructible from
  numpy arrays via the array protocol.
- Fully implement Fisher-matrix calculation for energy solutions
  in a new stand-alone module
  -- Jakob van Santen

- Fix for calculating time ranges from event headers
- Instead of taking the calibration errata, saturation errata, an
  bad DOMs list as separate argument, take a single vector of
  lists-of-things-to-exclude.
- Add option to use time window ranges as meant or to totally exclude DOMs
  appearing in the lists.
- Define 1-cableShadow as DOM efficiency
  -- Nathan Whitehorn


September 11, 2012 Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-04-00

- Merged with fortinbras branch new features include:

  * Support for masking out arbitrary time windows, e.g. clipped
    bits of the FADC.
  * Gradients will now pass through I3EventLogLikelihoodCombiner.
  * MillipedeFitParams::logl_ratio, the logarithm of the ratio of
    the best-fit likelihood to the maximum possible likelihood given
    the data. Unlike rlogl, this should be chi2-distributed even with
    small counts.
  * Pretty-printing for MillipedeFitParams.

- Get the range of possible pulse times explicitly rather than relying
  directly on I3EventHeader. The fallbacks to the trigger window and range
  of pulse times are now hidden inside of MillipedeBase. Note that because
  I3TimeWindow is not actually a frame object in the current dataclasses
  release, the fallback will *always* be taken for now.
  -- Jakob van Santen
- Allow importing millipede to work even if the debugger's dependencies
  aren't satisfied.
- Fix various mismerges and style errors
- Provide a fallback for adding infinite exclusion windows for event
  with old-style calibration errata
- Rewrite UpdateData() from scratch to properly handle exclusion windows.
- Initialize logl_ratio properly.
- Restore ability to skip unhit DOMs.
- Totally ignore DOMs with no valid time windows (Bug reported by Tom Feusels)
- For tracks that did not intersect the volume (i.e. tracks with no segments)
  do not try to treat the root track as a segment but fail gracefully.
- Various bugfixes
  -- Nathan Whitehorn

August 04, 2012, Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-03-00

- Support for LLH Gradients
- Better protection against negative energies in the 
  calculation of energy losses
- Add sanity checks on the input pulse stream
- Smoother handling of the case of zero sources
- Test the maximum likelihoods in the pymillipede test to be maximal
- Remove SingleEnergyGradient
- Deactivate muon and shower regularization
  -- Nathan Whitehorn
- Allow parameterization of starting tracks in MuMillipede 
  -- Claudio Kopper
- Optionally use a slant depth binning
  -- Tom Feusels


June 25, 2012, Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-02-00

- Fix PhotonsPerBin = 0, which had ended up turning on amplitudes-only
  mode at some point and would otherwise have relied on memory corruption
  to function.
- Work around broken GCD files in IC86 processing
- Remove dependency on photonics
- Refactor code to reduce code duplication.
  Note that this changes the options and default behavior of Monopod,
  which now uses PhotonsPerBin=5 as a default.
- Update example scripts to new photonics-service API
- Add Multidimensional maximum likelihood.
- Compute the negative log likelihood from the beginning.
  -- Nathan Whitehorn

- Add a debugging callback to PyMillipede
- Re-jigger pulse-binning loop so that bins no longer absorb long runs of
  zero charge. A zero-charge bin will now be added if the gap between two
  pulses is more than 10 bin widths.
- Remove reference to I3ConverterFactory
  -- Jakob van Santen

- Make using unhit DOMs optional, default behaviour should stay unchanged.
  -- Tom Feusels


March 6, 2012, Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-01-00

- Make the tolerance in SolveNoisyPoisson settable, and set it to a sanely
  small value. This signficantly improves performance on low-energy,
  noise-dominated events where the first-order solution is off by an order
  of magnitude or more.
- Make ``cableShadow_`` settable in Monopod
  -- Jakob van Santen
- Fix unit tests
  -- Nathan Whitehorn


February 7, 2012, Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-00-00
- Initial Release

  copied from sandbox/nwhitehorn/millipede
