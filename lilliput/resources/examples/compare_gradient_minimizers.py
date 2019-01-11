#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This example script demonstrates the Gulliver suite's ability to
reconstruct events with gradient based minimizers.
"""
from __future__ import print_function

import os

import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.gulliver
import icecube.lilliput
import icecube.phys_services
from I3Tray import I3Tray


class GaussianLiklihood(icecube.gulliver.I3EventLogLikelihood):
    """Example of a simple Likelihood calculation in Python

    The likelihood is is just a quadratic so that its gradient is simple
    to calculate

    """
    def __init__(self, pulses, error):
        super(GaussianLiklihood, self).__init__()
        self.pulse_name = pulses
        self.error = error

    def GetName(self):
        return "Likelihood"

    def SetGeometry(self, geo):
        self.geometry = geo.omgeo

    def SetEvent(self, frame):
        self.pulses = icecube.dataclasses.I3RecoPulseSeriesMap.from_frame(
            frame, self.pulse_name)

        self.calls_to_gradient = 0
        self.calls_to_likelihood = 0

    def GetLogLikelihoodWithGradient(self, hypo, grad, weight):
        L = self.GetLogLikelihood(hypo)
        part = hypo.particle
        zenith = -2. * (part.dir.zenith - 1.) * 100.
        azimuth = -2. * (part.dir.azimuth - 3.) * 100.
        x = -2. * (part.pos.x - 4.)
        y = -2. * (part.pos.y - 5.)
        z = -2. * (part.pos.z - 6.)
        grad.particle.dir = icecube.dataclasses.I3Direction(zenith, azimuth)
        grad.particle.pos = icecube.dataclasses.I3Position(x, y, z)
        self.calls_to_gradient += 1
        return L

    def GetLogLikelihood(self, hypo):
        part = hypo.particle

        L = -(
            100.*(part.dir.zenith - 1.)**2 + 100.*(part.dir.azimuth - 3.)**2 +
            (part.pos.x - 4.)**2 + (part.pos.y - 5.)**2 + (part.pos.z - 6.)**2
            )

        self.calls_to_likelihood += 1

        return L

    def GetMultiplicity(self):
        return len(self.pulses)

    def GetDiagnostics(self, x):
        m = icecube.dataclasses.I3MapStringDouble()
        m["calls_to_gradient"] = self.calls_to_gradient
        m["calls_to_likelihood"] = self.calls_to_likelihood
        return m

    def HasGradient(self):
        return True


filelist = [
    "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz",
    "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"
    ]

filelist = [
    os.path.join(os.environ["I3_TESTDATA"], "sim", f) for f in filelist
    ]

pulses = "TWSRTHVInIcePulses"

tray = I3Tray()

tray.Add("I3Reader", FileNameList=filelist)

tray.context["GaussianLiklihood"] = GaussianLiklihood(
    pulses, 15.*icecube.icetray.I3Units.ns)

# Stand along version of L-BFGS-B
tray.AddService("I3GulliverLBFGSBFactory", "LBFGSB")

# Minuit2's version of MIGRAD can use gradients but it ignores them most of the
# time.
tray.AddService(
    "I3GulliverMinuit2Factory", "Minuit2Migrad",
    Algorithm="MIGRAD",
    WithGradients=True)

# This is what processing usually used, it does not use gradients, it is here
# for comparison.
tray.AddService(
    "I3GulliverMinuit2Factory", "MinuitSimplex",
    Algorithm="SIMPLEX")

# This is a wrapper for GSL's gradient based minimization, none of them appear
# to work except for steepest descent.
tray.AddService(
    "I3GSLMultiMinFactory", "Steepest",
    Algorithm="steepest_descent")

# Now add a bunch of different algorithms from NLopt. There are more algorithms
# but this is just an example. The last algorithm does not use gradients but
# still works fine with a gradient based likelihood.
nlopt_algs = [
    "LD_LBFGS", "LD_VAR1", "LD_TNEWTON", "LD_MMA", "LD_AUGLAG", "LD_SLSQP",
    "LD_CCSAQ", "LN_BOBYQA"
    ]

for alg in nlopt_algs:
    tray.AddService("I3GulliverNLoptFactory", "NLopt_" + alg, Algorithm=alg)

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

minimizers = ["LBFGSB", "Minuit2Migrad", "Steepest", "MinuitSimplex"]
minimizers.extend("NLopt_" + alg for alg in nlopt_algs)

for m in minimizers:
    tray.Add(
        "I3SimpleFitter",
        OutputName="SPEFitSingle_TWHV_" + m,
        SeedService="seed",
        Parametrization="param",
        LogLikelihood="GaussianLiklihood",
        Minimizer=m)


def print_result(frame):
    print("\nMinimizer    Event ID   Likelihood   Calls to Likelihood   "
          "Calls to Gradient")

    for m in minimizers:
        reco = "SPEFitSingle_TWHV_" + m

        print("{:16}  {:3}   {:10.7f} {:20d} {:20d}".format(
            m, frame["I3EventHeader"].event_id, frame[reco + "FitParams"].logl,
            int(frame[reco + "_Likelihood"]["calls_to_likelihood"]),
            int(frame[reco + "_Likelihood"]["calls_to_gradient"]))
            )

    print()


tray.Add(print_result)
tray.Execute()
