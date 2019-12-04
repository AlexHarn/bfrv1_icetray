#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This example script illustrates how to make a custom likelihood
service in Python.
"""
from __future__ import print_function

import os

import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.gulliver
import icecube.lilliput.segments
import icecube.phys_services
from I3Tray import I3Tray


class GaussianLiklihood(icecube.gulliver.I3EventLogLikelihood):
    """Example of a simple Likelihood calculation in Python

    The likelihood is a Gaussian with a fixed error.

    """
    def __init__(self, pulses, error):
        super(GaussianLiklihood, self).__init__()
        self.pulse_name = pulses
        self.error = error

    def GetName(self):
        return self.pulse_name

    def SetGeometry(self, geo):
        self.geometry = geo.omgeo

    def SetEvent(self, frame):
        self.pulses = icecube.dataclasses.I3RecoPulseSeriesMap.from_frame(
            frame, self.pulse_name)

    def GetLogLikelihood(self, hypo):
        part = hypo.particle
        L = 0.

        for omkey, series in self.pulses.items():
            pos = self.geometry[omkey].position
            time = series[0].time

            tres = icecube.phys_services.I3Calculator.time_residual(
                part, pos, time)

            L -= (tres / self.error)**2

        return L

    def GetMultiplicity(self):
        return len(self.pulses)

    def GetDiagnostics(self, x):
        return None


filelist = [
    os.path.join(os.environ["I3_TESTDATA"], "GCD", "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz"),
    os.path.join(os.environ["I3_TESTDATA"], "sim", "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2")
    ]

pulses = "TWSRTHVInIcePulses"

tray = I3Tray()

tray.Add("I3Reader", FileNameList=filelist)

tray.context["GaussianLiklihood"] = GaussianLiklihood(
    pulses, 15.*icecube.icetray.I3Units.ns)

icecube.lilliput.segments.add_simple_track_parametrization_service(tray)

tray.AddService("I3GSLSimplexFactory", "GSLSimplex")

tray.AddService(
    "I3BasicSeedServiceFactory", "seed",
    InputReadout=pulses,
    FirstGuesses=["LineFit_TWHV"],
    TimeShiftType="TFirst")

tray.Add(
    "I3SimpleFitter",
    OutputName="SPEFitSingle_TWHV_NL",
    SeedService="seed",
    Parametrization="default_simpletrack",
    LogLikelihood="GaussianLiklihood",
    Minimizer="GSLSimplex")


def print_result(frame):
    mctruth = icecube.dataclasses.get_most_energetic_track(frame["I3MCTree"])
    reco = frame["SPEFitSingle_TWHV_NL"]
    print()
    for x in "zenith", "azimuth":
        a = getattr(mctruth.dir, x) / icecube.icetray.I3Units.degree
        b = getattr(reco.dir, x) / icecube.icetray.I3Units.degree
        print("%15s %8.3f %8.3f %8.3f" % (x, a, b, abs(a - b)))


tray.AddModule(print_result)
tray.Execute()
