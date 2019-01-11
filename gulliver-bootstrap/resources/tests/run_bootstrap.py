#!/usr/bin/env python

import os
import os.path
import sys
import numpy as np

from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import gulliver, lilliput
import icecube.lilliput.segments
from icecube import gulliver_bootstrap

testpath = os.environ.get('I3_TESTDATA')
if (testpath is None) or (not os.path.exists(testpath)):
	print("$I3_TESTDATA does not point to a proper folder.")
	print("Please set up your environment correctly.")
	sys.exit(1)

gcd_file = os.path.join(testpath, 'sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz')
i3_file  = os.path.join(testpath, 'sim/Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2')
out_file = '/tmp/bootstrap_test_output'
pulses   = "TWSRTHVInIcePulses"

# build tray
tray = I3Tray()

# configure file reader
tray.AddModule('I3Reader', 'reader', FilenameList=[gcd_file, i3_file])

# filter events, which have a proper seed
tray.Add(lambda f: f.Has('SPEFit2_HV'))

# do a simple MPE fit
llhname  = lilliput.segments.add_pandel_likelihood_service(tray, pulses, "MPE")
mininame = lilliput.segments.add_minuit_simplex_minimizer_service(tray)
paraname = lilliput.segments.add_simple_track_parametrization_service(tray)

tray.AddService("I3BasicSeedServiceFactory", "TestSeedPrep",
        AddAlternatives = "None",
        FirstGuesses =  [ "SPEFit2_HV" ],
        InputReadout =  pulses,
        MaxMeanTimeResidual = 1000*I3Units.ns,
        NChEnergyGuessPolynomial = [],
        OnlyAlternatives = False,
        SpeedPolice = True,
        TimeShiftType =  "TNone",
        )

tray.AddModule("I3SimpleFitter",
        OutputName      = "TestMPEFit",
        SeedService     = "TestSeedPrep",
        Parametrization = paraname,
        LogLikelihood   = llhname,
        Minimizer       = mininame
        )


### now do the actual bootstrapping ###

if 'I3RandomService' not in tray.tray_info.factories_in_order:
    tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")

# seed with previous MPE fit
tray.AddService("I3BasicSeedServiceFactory", "TestMPESeed",
        FirstGuesses =                 [ "TestMPEFit" ],
        ChargeFraction =               0.9,                      # Default
        FixedEnergy =                  float( "nan" ),           # Default
        MaxMeanTimeResidual =          1000.0*I3Units.ns,        # Default
        NChEnergyGuessPolynomial =     [],                       # Default
        SpeedPolice =                  True,                     # Default
        AddAlternatives =              "None",                   # Default
        OnlyAlternatives =             False                     # Default
        )

# configure bootstrapping (6 iterations here)
tray.AddService("BootstrappingLikelihoodServiceFactory", "TestMPEBootstrapLLH",
        Pulses            = pulses,
        Bootstrapping     = gulliver_bootstrap.BootstrapOption.Multinomial,
        Iterations        = 6,
        WrappedLikelihood = llhname,
        RandomService     = "I3RandomService"
        )

# seed the fits...
tray.AddService("BootStrappingSeedServiceFactory", "TestMPEBootstrapSeed",
        WrappedSeed             = "TestMPESeed",
        BootstrappingLikelihood = "TestMPEBootstrapLLH"
        )

# do the actual fits
tray.AddModule("I3SimpleFitter",
        OutputName        = "TestMPEBootstrap",
        SeedService       = "TestMPEBootstrapSeed",
        Parametrization   = paraname,
        LogLikelihood     = "TestMPEBootstrapLLH",
        Minimizer         = mininame,
        StoragePolicy     = "AllFitsAndFitParams"
        )

# calculate containment radius
tray.AddModule("BootstrapSeedTweak", "TestMPEBootstrapTweakSeeds",
        BootstrappedRecos = "TestMPEBootstrapVect",
        ContainmentLevel  = 0.5,
        AngularError      = "TestMPE_Bootstrap_Angular"
        )

### bootstrapping ends ###


# evaluate result
def ResultExists(frame):
	if frame.Has('TestMPE_Bootstrap_Angular'):
		result = frame['TestMPE_Bootstrap_Angular'].value
		if not (0. <= result <= np.pi):
			print("Bootstrapping result outside of allowed range:")
			print(" {1:f} not in [0, Pi]".format(result))
			sys.exit(1)
		if frame.Has("TestMPE_Bootstrap_AngularParams"):
			params=frame.Get("TestMPE_Bootstrap_AngularParams")
			if(params.successfulFits>params.totalFits):
				print("Number of successful fits larger than total number of fits")
				sys.exit(1)
		else:
			print("No bootstrapping status information found in frame.")
			sys.exit(1)
	else:
		print("No bootstrapping result found in frame.")
		sys.exit(1)

tray.Add(ResultExists)





# run tray
tray.Execute()


