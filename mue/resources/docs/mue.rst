MUE
===========

The mue module mue implements an older version of the in-ice event energy reconstruction.
It relies on the pure Poisson likelihood function (though the code implementing the systematic
deviations is there, it is by default disabled). There are known typos in the flux functions that
describe the photon flux at a distance from the track. These are fixed and updated in the newer
module muex, which completely replaces mue and should be used instead. The module mue is left
here for backwards compatibility only.

Usage
^^^^^

Usage is straightforward.  Available parameters are listed below (from icetray-inspect mue)::

  I3mue
    DataReadoutName
      Description : name of the recopulses vector
      Default     : 'RawData'

    OutputParticle
      Description : name of the output particle
      Default     : ''

    RecoIntr
      Description : cpandel parameterization mode
      Default     : 0

      Set to 1 to parameterize the cpandel.
      Set to 2 to additionally save/read parameterization tables to/from disk.
      This disables the multi-layered ice reconstruction even if the icemodel.* files are present.

    RecoPulseSeriesNames
      Description : Reco Pulses to process
      Default     : ['InitialPulseSeriesRecoNames']

    RecoResult
      Description : name of the reconstruction result to use
      Default     : 'RecoResult'

    RecoRnum
      Description : required trigger multiplicity
      Default     : 8

    Verbose
      Description : Enable event printout on cout
      Default     : 0


