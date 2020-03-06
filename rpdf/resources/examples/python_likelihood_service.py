#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This example script illustrates how to make a custom likelihood
service in Python as well as the usage of the rpdf Python interface.

"""
import os
import math

import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.gulliver
import icecube.rpdf
import icecube.lilliput.segments as ls
from I3Tray import I3Tray, I3Units


class RpdfLikelihood(icecube.gulliver.I3EventLogLikelihood):
    """Example likelihood service

    Example of how to write a Gulliver likelihood service and how to use
    the Python interface to rpdf. Should give exactly the same output as
    I3RecoLLH with SPE1st.

    """
    def __init__(self, pulses, jitter, noise):
        super(RpdfLikelihood, self).__init__()

        self.pulse_name = pulses

        self.jitter = jitter
        self.noise = noise
        self.icemodel = icecube.rpdf.H2

        self.peprob = icecube.rpdf.FastConvolutedPandel(
            self.jitter, self.icemodel)

        self.events = 0
        self.calls = 0

    def GetName(self):
        return self.pulse_name

    def SetGeometry(self, geometry):
        self.geometry = geometry

    def SetEvent(self, frame):
        self.pulses = icecube.dataclasses.I3RecoPulseSeriesMap.from_frame(
            frame, self.pulse_name)

        self.events += 1

    def GetLogLikelihood(self, hypothesis):
        self.calls += 1
        particle = hypothesis.particle

        llhtot = 0.
        for omkey, pulseseries in self.pulses.items():
            position = self.geometry.omgeo[omkey].position

            geometry = icecube.rpdf.muon_geometry(
                position, particle, self.icemodel)

            tres = pulseseries[0].time - geometry.first
            llh = self.peprob.pdf(tres, geometry.second)
            llhtot += math.log(self.noise + llh)

        return llhtot

    def GetMultiplicity(self):
        return len(self.pulses)

    def GetDiagnostics(self, x):
        diagnostics = icecube.dataclasses.I3MapStringDouble()
        diagnostics["events"] = self.events
        diagnostics["calls"] = self.calls
        return diagnostics


inputfiles = [
    "GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz",
    "sim/Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"
    ]

inputfiles = [
    os.path.join(os.getenv("I3_TESTDATA"), f) for f in inputfiles
    ]

tray = I3Tray()

tray.Add(
    "I3Reader",
    filenamelist=inputfiles)

likelihood = icecube.rpdf.I3RecoLLH(
    input_readout="TWSRTHVInIcePulses",
    likelihood="SPE1st",
    pe_prob="GaussConvoluted",
    jitter_time=15.*I3Units.ns,
    noise_probability=10.*I3Units.hertz,
    ice_model=icecube.rpdf.H2)

parametrization = ls.add_simple_track_parametrization_service(tray)
minimizer = ls.add_minuit_simplex_minimizer_service(tray)

seed = ls.add_seed_service(
    tray, "TWSRTHVInIcePulses", ["LineFit_TWHV"])

tray.Add(
    "I3SimpleFitter",
    outputname="SPEFitSingle_TWHV_cpp",
    seedservice=seed,
    parametrization=parametrization,
    loglikelihood=likelihood,
    minimizer=minimizer)

tray.Add(
    "I3SimpleFitter",
    outputname="SPEFitSingle_TWHV_py",
    seedservice=seed,
    parametrization=parametrization,
    loglikelihood=RpdfLikelihood(
        "TWSRTHVInIcePulses",
        15.*I3Units.ns,
        10.*I3Units.hertz),
    minimizer=minimizer)


def print_result(frame):
    truth = icecube.dataclasses.get_most_energetic_track(frame["I3MCTree"])
    reco1 = frame["SPEFitSingle_TWHV_cpp"]
    reco2 = frame["SPEFitSingle_TWHV_py"]

    for angle in "zenith", "azimuth":
        true_angle = getattr(truth.dir, angle) / I3Units.degree
        reco_angle1 = getattr(reco1.dir, angle) / I3Units.degree
        reco_angle2 = getattr(reco2.dir, angle) / I3Units.degree

        print(("{:7s}:" + ",".join("{:8.3f}" for _ in range(6))).format(
            angle,
            true_angle,
            reco_angle1, abs(true_angle - reco_angle1),
            reco_angle2, abs(true_angle - reco_angle2),
            abs(reco_angle1 - reco_angle2)))

    print("")


tray.Add(print_result)

tray.Execute()
