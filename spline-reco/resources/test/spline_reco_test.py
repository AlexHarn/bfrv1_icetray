#!/usr/bin/env python
# spline_reco_test.py

__doc__ = """Try to run spline-reco on test data and see if the results are as expected."""

import os
from glob import glob

# more general stuff:
from I3Tray import *
from icecube import icetray, dataclasses, dataio, linefit, gulliver, gulliver_modules, lilliput, paraboloid, phys_services
import icecube.lilliput.segments
# more specific to spline-reco:
from icecube import photonics_service, spline_reco

inputfiles = sorted(glob(os.environ['I3_TESTDATA']+'/reco-toolbox/I3TestEvent_Pulse_*'))
pulses = "Pulses"
test_data_zens = [0.629593664068, 2.30465925233, 0.0347026311]
test_data_azis = [1.14170201371, 5.5749804806, 3.05733228674]

tray = I3Tray()

tray.AddModule("I3Reader", "reader", filenamelist=inputfiles)

# first guess needed, use linefit
tray.AddSegment(linefit.simple, "imprv_LF",
                inputResponse = pulses,
                fitName = "LF_FirstGuess"
                )

# paths to bare muon splines
splineDir = os.environ['I3_TESTDATA']+'/photospline/'
timingSpline = splineDir+'ems_z0_a0.pt.prob.fits'
amplitudeSpline = splineDir+'ems_z0_a0.pt.abs.fits'

### next services provide spline access

# bare muon spline with 4 ns convolution
tray.AddService("I3PhotoSplineServiceFactory", "spline-reco-test_SplineService4",
     AmplitudeTable = amplitudeSpline, # path to amplitude spline (abs)
     TimingTable    = timingSpline, # path to timing spline (prob)
     TimingSigma    = 4, # pdf convolution in ns
    )

# seed service without vertex time shift, PandelMPE as first guess
# used for the SplineMPE fit
tray.AddService( "I3BasicSeedServiceFactory", "spline-reco-test_MPESeedNoShift",
        FirstGuesses =                ["LF_FirstGuess"],
        ChargeFraction =              0.9,                      # Default
        FixedEnergy =                 float( "nan" ),           # Default
        MaxMeanTimeResidual =         1000.0*I3Units.ns,        # Default
        NChEnergyGuessPolynomial =    [],                       # Default
        SpeedPolice =                 True,                     # Default
        AddAlternatives =             "None",                   # Default
        OnlyAlternatives =            False,                    # Default
        )

# the spline reco likelihood doing a default MPE reco --> faster and worse than with all modifications
tray.AddService("I3SplineRecoLikelihoodFactory", "spline-reco-test_SplineMPEllh",
       PhotonicsService = "spline-reco-test_SplineService4", # spline services
       Pulses = pulses,
       Likelihood = "MPE",
       NoiseRate = 10*I3Units.hertz,
       )

mininame = lilliput.segments.add_minuit_simplex_minimizer_service(tray)
paraname = lilliput.segments.add_simple_track_parametrization_service(tray)

tray.AddModule("I3SimpleFitter", "spline-reco-test_SplineMPE",
        SeedService = "spline-reco-test_MPESeedNoShift",
        Parametrization = paraname,
        LogLikelihood = "spline-reco-test_SplineMPEllh",
        Minimizer = mininame,
        NonStdName =  "Params",                 # Default
        StoragePolicy =  "OnlyBestFit"         # Default
        )

framecount = 0
def assertSplineRecoResult(frame):
    global framecount
    epsilon = 1e-9
    fit = frame["spline-reco-test_SplineMPE"]
    #print "zen:", fit.dir.zenith, " azi:", fit.dir.azimuth
    print("Frame:", framecount)
    print("Result of spline-reco fit: zen = %12.10f    azi = %12.10f    status = %s" % (fit.dir.zenith, fit.dir.azimuth, fit.fit_status))
    print("Reference:                 zen = %12.10f    azi = %12.10f    status = %s" % (test_data_zens[framecount], test_data_azis[framecount], dataclasses.I3Particle.FitStatus.OK))
    assert(abs( fit.dir.zenith - test_data_zens[framecount] ) < epsilon)
    assert(abs( fit.dir.azimuth - test_data_azis[framecount] ) < epsilon)
    assert(fit.fit_status == dataclasses.I3Particle.FitStatus.OK)
    framecount += 1
tray.AddModule(assertSplineRecoResult, "assertSplineRecoResults")



tray.Execute()

