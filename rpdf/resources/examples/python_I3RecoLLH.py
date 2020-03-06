#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Example script demonstrating the usage of `icecube.rpdf.I3RecoLLH`

"""
import os

from icecube import icetray, dataio, rpdf, gulliver

inputfiles = [
    "GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz",
    "sim/Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"
    ]

inputfiles = [
    os.path.join(os.getenv("I3_TESTDATA"), f) for f in inputfiles
    ]

with dataio.I3File(inputfiles[0]) as i3file:
    geometry = i3file.pop_frame()["I3Geometry"]

with dataio.I3File(inputfiles[1]) as i3file:
    event = i3file.pop_physics()

likelihood = rpdf.I3RecoLLH(
    "SRTHVInIcePulses",
    "SPE1st",
    "GaussConvoluted",
    15.*icetray.I3Units.ns,
    10.*icetray.I3Units.hertz,
    rpdf.H2)

hypothesis = gulliver.I3EventHypothesis(event["SPEFitSingle_TWHV"])

likelihood.set_geometry(geometry)
likelihood.set_event(event)

print(
    "The likelihood value is {}.".format(
        likelihood.get_log_likelihood(hypothesis)))
