#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This example script illustrates how to make a custom likelihood
service in Python as well as the usage of the ipdf python interface
"""
import os
import math
from icecube import icetray, dataio, dataclasses, gulliver, ipdf
import icecube.lilliput.segments as lilliput_segments
from I3Tray import I3Tray, I3Units


class IpdfLikelihood(gulliver.I3EventLogLikelihood):
    """Example of how to write a gulliver likelihood service and how to
    use the python interface to ipdf. Should give exactly the same
    output as I3GulliverIPDFPandelFactory with SPE1st.

    """
    def __init__(self, pulses, jitter, noise):
        super(IpdfLikelihood, self).__init__()
        self.pulse_name = pulses
        self.jitter = jitter
        self.noise = noise
        self.events = 0
        self.calls = 0

    def GetName(self):
        return self.pulse_name

    def SetGeometry(self, geo):
        self.pdf = ipdf.muon_pandel_spe1st(geo, self.jitter)

    def SetEvent(self, frame):
        self.events += 1

        self.pulses = dataclasses.I3RecoPulseSeriesMap.from_frame(
            frame, self.pulse_name)

        self.pdf.set_pulses(self.pulses)

    def GetLogLikelihood(self, hypo):
        self.calls += 1
        part = hypo.particle

        llhtot = 0
        for omkey, pulseseries in self.pulses:
            llh = self.pdf.get_likelihood(part, omkey)
            llhtot += math.log(self.noise + llh)

        return llhtot

    def GetMultiplicity(self):
        return len(self.pulses)

    def GetDiagnostics(self, x):
        m = dataclasses.I3MapStringDouble()
        m["events"] = self.events
        m["calls"] = self.calls
        return m

filelist = [
    os.path.join(os.environ["I3_TESTDATA"], "GCD", "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz"),
    os.path.join(os.environ["I3_TESTDATA"], "sim", "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2")
    ]

pulses = "TWSRTHVInIcePulses"
jitter = 15.*I3Units.ns
noise = 10.*I3Units.hertz

tray = I3Tray()

tray.Add("I3Reader", FileNameList=filelist)

lilliput_segments.add_simple_track_parametrization_service(tray)

tray.AddService("I3GSLSimplexFactory", "GSLSimplex")

tray.AddService(
    "I3BasicSeedServiceFactory", "seed",
    InputReadout=pulses,
    FirstGuesses=["LineFit_TWHV"],
    TimeShiftType="TFirst")

tray.context["IpdfLiklihood"] = IpdfLikelihood(pulses, jitter, noise)

tray.AddService(
    "I3GulliverIPDFPandelFactory", "pandel",
    InputReadout=pulses,
    EventType="InfiniteMuon",
    Likelihood="SPE1st",
    PEProb="GaussConvolutedFastApproximation",
    JitterTime=jitter,
    NoiseProbability=noise)

tray.Add(
    "I3SimpleFitter",
    OutputName="SPEFitSingle_TWHV_cpp",
    SeedService="seed",
    Parametrization="default_simpletrack",
    LogLikelihood="pandel",
    Minimizer="GSLSimplex")

tray.Add(
    "I3SimpleFitter",
    OutputName="SPEFitSingle_TWHV_py",
    SeedService="seed",
    Parametrization="default_simpletrack",
    LogLikelihood="IpdfLiklihood",
    Minimizer="GSLSimplex")


def print_result(frame):
    mctruth = dataclasses.get_most_energetic_track(frame["I3MCTree"])
    reco1 = frame["SPEFitSingle_TWHV_cpp"]
    reco2 = frame["SPEFitSingle_TWHV_py"]

    for x in "zenith", "azimuth":
        a = getattr(mctruth.dir, x) / I3Units.degree
        b = getattr(reco1.dir, x) / I3Units.degree
        c = getattr(reco2.dir, x) / I3Units.degree

        print("%15s %8.3f : %8.3f %8.3f : %8.3f %8.3f : %8.3f" % (
            x, a, b, abs(a - b), c, abs(a - c), abs(b-c)))

    print("")


tray.Add(print_result)

tray.Execute()
