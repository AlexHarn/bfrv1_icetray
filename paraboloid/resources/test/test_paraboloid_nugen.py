#!/usr/bin/env python

"""Unit test for Paraboloid. This runs paraboloid on a Level3 nugen file
with identical parameters as the original result in the file, and
compares the two. Future Improvements to paraboloid may cause this test
to fail.

"""
import math
import os

from icecube import icetray, dataclasses, dataio, lilliput, paraboloid
from I3Tray import I3Tray, I3Units

testdir = os.environ["I3_TESTDATA"]

files = [
    os.path.join(testdir, "GCD", "GeoCalibDetectorStatus_2012.56063_V0.i3.gz"),
    os.path.join(testdir, "sim", "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"),
    ]

filelist = [os.path.join(testdir, "sim", f) for f in files]

tray = I3Tray()

tray.Add("I3Reader", FileNameList=filelist)

tray.Add("I3GulliverMinuitFactory", "Minuit", Tolerance=0.01)

Pulses = "TWSRTHVInIcePulses"
tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed",
         InputReadout=Pulses,
         FirstGuesses=["BestTrack"],
         TimeShiftType="TFirst")

tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel",
         InputReadout=Pulses,
         Likelihood="MPE",
         PEProb="GaussConvolutedFastApproximation",
         JitterTime=4.*I3Units.ns,
         NoiseProbability=10.*I3Units.hertz)

tray.Add("I3ParaboloidFitter",
         OutputName="MPEFitParaboloid2",
         Minimizer="Minuit",
         LogLikelihood="MPEParaboloidPandel",
         SeedService="ParaboloidSeed",
         GridpointVertexCorrection="ParaboloidSeed",
         VertexStepSize=5.*I3Units.m,
         ZenithReach=2.*I3Units.degree,
         AzimuthReach=2.*I3Units.degree,
         MCTruthName="I3MCTree")


def test(frame):
    assert(frame["MPEFitParaboloid2FitParams"].pbfStatus ==
           paraboloid.I3ParaboloidFitParams.PBF_SUCCESS)

    olderr1 = frame["MPEFitParaboloidFitParams"].pbfErr1 * 180. / math.pi
    newerr1 = frame["MPEFitParaboloid2FitParams"].pbfErr1 * 180. / math.pi
    diff1 = abs(olderr1 - newerr1)
    print("err1 %20.17f, %20.17f, diff=%20.17f" % (olderr1, newerr1, diff1))
    assert (diff1 < 1e-6)

    olderr2 = frame["MPEFitParaboloidFitParams"].pbfErr2 * 180. / math.pi
    newerr2 = frame["MPEFitParaboloid2FitParams"].pbfErr2 * 180. / math.pi
    diff2 = abs(olderr2 - newerr2)
    print("err2 %20.17f, %20.17f, diff=%20.17f" % (olderr2, newerr2, diff2))
    assert (diff2 < 1e-6)


tray.Add(test)

tray.Execute()

