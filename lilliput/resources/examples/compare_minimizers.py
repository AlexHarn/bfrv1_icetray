#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This script reconstructs the same events using different minimizers
and prints the angle between MC and reconstructed track for each
minimizer.
"""
import os

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.gulliver
import icecube.gulliver_modules
import icecube.lilliput
import icecube.lilliput.scipymin
import icecube.lilliput.i3minuit
from I3Tray import I3Tray


files = [
    "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz",
    "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"
    ]

filelist = [os.path.join(os.environ["I3_TESTDATA"], "sim", f) for f in files]
pulses = "TWSRTHVInIcePulses"

tray = I3Tray()

tray.AddService("I3GSLRandomServiceFactory")

# GSLMultiMin and LBFGSB are omitted because they use gradiants but
# I3GulliverIPDFPandelFactory does not provide gradiants.
minimizers = [
    "GSLSimplex",
    "NLopt",
    "Annealing",
    "MN",
    "MinuitSimplex",
    "Minuit2Simplex",
    "iminuit",
    "scipy"
    ]

tray.AddService(
    "I3GSLSimplexFactory", "GSLSimplex")

tray.AddService(
    "I3GulliverNLoptFactory", "NLopt",
    Algorithm="LN_BOBYQA",
    Tolerance=0.001)

tray.AddService(
    "I3GulliverAnnealingFactory",
    "Annealing")

tray.AddService(
    "I3GulliverMNFactory", "MN")

tray.AddService(
    "I3GulliverMinuitFactory", "MinuitSimplex",
    Algorithm="SIMPLEX",
    MaxIterations=1000,
    Tolerance=0.001)

tray.AddService(
    "I3GulliverMinuit2Factory", "Minuit2Simplex",
    Algorithm="SIMPLEX",
    MaxIterations=1000,
    Tolerance=0.001,
    IgnoreEDM=True)

tray.context["iminuit"] = icecube.lilliput.i3minuit.IMinuitMinimizer(
    Tolerance=0.001)

tray.context["scipy"] = icecube.lilliput.scipymin.SciPyMinimizer(
    "scipy", method="Nelder-Mead")

tray.AddService(
    "I3SimpleParametrizationFactory", "param",
    StepX=20.*icecube.icetray.I3Units.m,
    StepY=20.*icecube.icetray.I3Units.m,
    StepZ=20.*icecube.icetray.I3Units.m,
    StepZenith=0.1*icecube.icetray.I3Units.radian,
    StepAzimuth=0.2*icecube.icetray.I3Units.radian,
    BoundsX=[-2000.*icecube.icetray.I3Units.m,
             2000.*icecube.icetray.I3Units.m],
    BoundsY=[-2000.*icecube.icetray.I3Units.m,
             2000.*icecube.icetray.I3Units.m],
    BoundsZ=[-2000.*icecube.icetray.I3Units.m,
             2000.*icecube.icetray.I3Units.m])

tray.AddService(
    "I3GulliverIPDFPandelFactory", "pandel",
    InputReadout=pulses,
    EventType="InfiniteMuon",
    Likelihood="SPE1st",
    PEProb="GaussConvolutedFastApproximation",
    JitterTime=15.*icecube.icetray.I3Units.ns,
    NoiseProbability=10.*icecube.icetray.I3Units.hertz)

tray.AddService(
    "I3BasicSeedServiceFactory", "seed",
    InputReadout=pulses,
    FirstGuesses=["LineFit_TWHV"],
    TimeShiftType="TFirst")

tray.AddModule(
    "I3Reader", "reader",
    FileNameList=filelist)

for minimizer in minimizers:
    tray.Add(
        "I3SimpleFitter",
        OutputName="SPEFitSingle_TWHV_"+minimizer,
        SeedService="seed",
        Parametrization="param",
        LogLikelihood="pandel",
        Minimizer=minimizer)


def print_result(frame):
    mctruth = icecube.dataclasses.get_most_energetic_track(frame["I3MCTree"])
    original = frame["SPEFitSingle_TWHV"]

    print "\nEvent: ", frame["I3EventHeader"].event_id
    print "%15s %8s %8s" % ("minimizer", "mc/deg", "orig/deg")

    for minimizer in minimizers:
        reco = frame["SPEFitSingle_TWHV_"+minimizer]
        delta1 = mctruth.dir.angle(reco.dir)
        delta2 = original.dir.angle(reco.dir)

        print "%15s %8.3f %8.3f" % (
            minimizer,
            delta1 / icecube.icetray.I3Units.degree,
            delta2 / icecube.icetray.I3Units.degree)


tray.AddModule(print_result)

tray.Execute()


tray.PrintUsage()

for a in tray.Usage():
    if a.key().startswith("SPEFitSingle_TWHV_"):
        print a.key(), a.data().usertime
