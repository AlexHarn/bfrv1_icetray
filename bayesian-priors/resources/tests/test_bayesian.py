#!/usr/bin/env python

"""
Unit test for Bayesian Priors. This runs bayes on a Level3 nugen
file with identical parameters as the original result in the file, 
and compares the two. Future Improvements to bayes may cause 
this test to fail.
"""
from I3Tray import *
from icecube import dataio, lilliput,bayesian_priors

testdir = os.environ["I3_TESTDATA"]
files = ["GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz",
         "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"]
filelist = [ os.path.join(testdir,"sim",f) for f in files]

Pulses="TWSRTHVInIcePulses"
UpgoingEvents = lambda frame: frame.Has("SPEFit2BayesianFitParams")
Suffix="Test"

tray = I3Tray()

tray.AddModule("I3Reader","reader",
               FileNameList  = filelist
               )

tray.AddService("I3GulliverMinuitFactory", "Minuit",
                Tolerance=0.01)

tray.AddService( "I3PowExpZenithWeightServiceFactory", "ZenithWeight")

tray.AddService("I3GulliverIPDFPandelFactory", "BayesSPEPandel"+Suffix,
                InputReadout=Pulses,
                Likelihood="SPE1st",
                PEProb="GaussConvoluted",
                NoiseProbability=10* I3Units.hertz,
                JitterTime=15.0 * I3Units.ns)

tray.AddService("I3EventLogLikelihoodCombinerFactory", "BayesZenithWeightPandel"+Suffix,
                InputLogLikelihoods=["BayesSPEPandel"+Suffix,"ZenithWeight"])

tray.AddService("I3SimpleParametrizationFactory", "SimpleTrack",
        StepX = 20*I3Units.m,
        StepY = 20*I3Units.m,
        StepZ = 20*I3Units.m,
        StepZenith = 0.1*I3Units.radian,
        StepAzimuth= 0.2*I3Units.radian,
        BoundsX = [-2000*I3Units.m, 2000*I3Units.m],
        BoundsY = [-2000*I3Units.m, 2000*I3Units.m],
        BoundsZ = [-2000*I3Units.m, 2000*I3Units.m]
)

tray.AddService("I3BasicSeedServiceFactory", "BayesSeed"+Suffix,
                InputReadout=Pulses,
                FirstGuesses=["BestTrack"],
                TimeShiftType="TFirst")

tray.AddModule("I3IterativeFitter","SPEFit"+Suffix+"Bayesian",
               RandomService="SOBOL",
               NIterations=2,
               SeedService="BayesSeed"+Suffix,
               Parametrization="SimpleTrack",
               LogLikelihood="BayesZenithWeightPandel"+Suffix,
               CosZenithRange=[0, 1], # ! This is a downgoing hypothesis
               Minimizer="Minuit",
               If=UpgoingEvents)

def test(frame):
    if UpgoingEvents(frame):
        l1= frame["SPEFit2BayesianFitParams"].logl
        l2= frame["SPEFitTestBayesianFitParams"].logl
        deltal = abs(l1-l2)/l1
        print("%20.12f %20.12f %20.12f"%(l1,l2,deltal))
        assert(deltal<0.01)

tray.AddModule(test)

tray.Execute()

