MUEX
===========

The muex module of mue implements the most recent version of the in-ice track and cascade
energy reconstruction of the project mue. A likelihood function consists of both statistical
(Poisson) and systematic (describing errors in the photon flux description) terms.

Photon parametrization relies on ice files icemodel.dat and icemodel.par that describe the ice
scattering and absorption coefficients. These should be contained within the directory specified
with parameter icedir. Optionally if files tilt.dat and tilt.par exist they will be used to describe
the tilt of the ice layers. These files are exactly the same as in the ice description used by
direct photon propagators (e.g., ppc). For best results most recent ice description should be used,
but different ice models will obviously lead to a varition in the result.

The systematic part of the likelihood function describing the errors of the photon parametrization
is a wider log-normal gaussian (a 1-degree student's t-distribution) when used for cascade
reconstruction of in a compatibility mode for track reconstruction. If the compatibility mode is
not used the newer skewed description is used in track reconstruction that allows for large
over-fluctuations (that happen in a form of, e.g., bremsstrahlung losses of muons). This improves
resolution in the muon energy reconstruction to the levels comparable with TE.

In detailed mode a particle vector is produced that containes reconstructed losses at equal distance
intervals along the muon track, similar to what is done by millipede. Since only time-integrated
analytical description of the photon flux is used, the precision is obviously lower than that
achieved by millipede, with an error level of less than about 10% in both the described charge
deposition and total energy deposition.

If the input track name is empty the module will calculate its own track, and as such can be
used as a simple-and-quick stand-alone track/cascade reconstuction tool. In this mode use parameter
usempe to switch between the SPE (using the full time-series) and MPE (using only the first
photon and total charge in each DOM) descriptions.

If a parameter repeat is set then the input recopulse series will be re-sampled according to a
multinomial distribution (preserving total charge in the event) and uncertainties in some
parameters (energy, angular resolution if rectrk is empty) will be calculated by a bootrapping
resampling method. Additionally, the precision of the reconstruction will usually go up with more
resamplings as the result of each iteration is checked against the full recopulse series to see
if it yields an improved solution.

Finally, this module is not a toy. Please use responsibly!


Usage
^^^^^

Usage is straightforward.  Available parameters are listed below (from icetray-inspect mue)::

  muex
    badoms
      Description : list of clipped/saturated OMs
      Default     : ''

    compat
      Description : compatibility with older pre-wreco trunk
      Default     : False

    detail
      Description : detailed energy losses
      Default     : False

    energy
      Description : estimate energy
      Default     : False

    icedir
      Description : icemodel directory
      Default     : '$I3_BUILD/mue/resources/ice/mie'

    lcspan
      Description : lc span in input data
      Default     : 0

    pulses
      Description : input pulse series
      Default     : 'InPulses'

    rectrk
      Description : input track name
      Default     : ''

    rectyp
      Description : track(True) or cascade(False) reconstruction
      Default     : True

    repeat
      Description : number of iterations for uncertainty estimatation
      Default     : 0

    result
      Description : name of the result particle
      Default     : 'MuEx'

    usempe
      Description : mpe(True) or spe(False) reconstruction
      Default     : False

Output
^^^^^^

This module can write several output objects to the frame. 

- result: An I3Particle. The position, time and direction of this particle will either be copied from 
  'rectrk, if it exists, or otherwise calculated by this module. If the 'energy' option is set, this 
  module will also compute an energy estimate which will be stored as the particle's energy. 

- result + "_r": Written only when 'detail' is set to True. A number indicating the ratio of charge
  predicted by the energy loss unfolding to the actually observed total charge. 

- result + "_list": Written only when 'detail' is set to True. A list of particles representing the
  energy losses which were unfolded. 

- result + "_rllt": Only written when 'rectrk' is not set, so this module does its own reconstruction.
  A number representing the likelihood value obtained from the directional/postional fit. 

- result + "_rlle": Only written when 'rectrk' is not set, so this module does its own reconstruction,
  and 'energy' is set to True. A number representing the likelihood value obtained from the energy fit.  

- result + "_Sigma": Only written when 'rectrk' is not set, so this module does its own reconstruction,
  and 'repeat' is larger than zero. A number representing the median angular difference, in radians, 
  between the bootstrapped repetitions of the directional reconstruction and the average of those 
  reconstructions. This can be interpreted as a measure of the uncertainty of the directional fit. 

- result + "_EnUnc": Only written when 'rectrk' is not set, so this module does its own reconstruction,
  'energy' is set to True, and 'repeat' is larger than zero. A number representing the median absolute 
  value of the difference between the natural logarithm of the energies obtained from the bootstrapped 
  repetitions of the energy reconstruction and the average natural logarithm of those energies. This
  can be interpreted as a measure of the uncertainty of the energy estimate. 

