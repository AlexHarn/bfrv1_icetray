#!/usr/bin/env python

"""Example script for Paraboloid

"""
import os

from icecube import icetray, dataclasses, dataio, lilliput, paraboloid
from I3Tray import I3Tray, I3Units

# Generate list of input files for reading from test data.
testdir = os.environ["I3_TESTDATA"]

files = [
    "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz",
    "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"
    ]

filelist = [os.path.join(testdir, "sim", f) for f in files]

tray = I3Tray()

tray.Add("I3Reader", FileNameList=filelist)

# Paraboloid need 4 services to work correclty this is the first one is the
# minimizer service which is used to minimize the liklihoods for each grid
# point.
tray.Add("I3GulliverMinuitFactory", "Minuit", Tolerance=0.01)

# The second and third services are simultaneously provided by the seed service
# this tells paraboloid which I3Particle to use as the center of the grid as
# well as the vertex #location for each point on the grid. More advanced usage
# may use different seeds for these functions.
Pulses = "TWSRTHVInIcePulses"
tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed",
         InputReadout=Pulses,
         FirstGuesses=["BestTrack"],
         TimeShiftType="TFirst")

# The fourth service is provided by  a description of the liklihood function.
tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel",
         InputReadout=Pulses,
         Likelihood="MPE",
         PEProb="GaussConvolutedFastApproximation",
         JitterTime=4.*I3Units.ns,
         NoiseProbability=10.*I3Units.hertz)

# Finally load the Paraboloid module, its self.
tray.Add("I3ParaboloidFitter",
         OutputName="MPEFitParaboloid2",
         Minimizer="Minuit",
         LogLikelihood="MPEParaboloidPandel",
         SeedService="ParaboloidSeed",
         GridpointVertexCorrection="ParaboloidSeed",
         VertexStepSize=5.*I3Units.m,
         ZenithReach=2.*I3Units.degree,
         AzimuthReach=2.*I3Units.degree)

tray.Execute()

