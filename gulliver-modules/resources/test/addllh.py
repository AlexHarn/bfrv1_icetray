#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Test script for Gulliver

This script doesn't do any LLH fit at all. It reads some test data, does
a LineFit and then computes the corresponding likelihood and stores that
in the frame.

It uses the `fortytwo` functionality to check that the current results
are equal to (or compatible with) the reference results.

"""
import os

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.linefit
import icecube.gulliver
import icecube.gulliver_modules
import icecube.gulliver_modules.fortytwo
import icecube.lilliput
from I3Tray import I3Tray

icecube.icetray.logging.set_level_for_unit("I3FortyTwo", "INFO")

if "I3_TESTDATA" in os.environ:
    testdata = os.environ["I3_TESTDATA"]
else:
    raise IOError("Cannot find test data file, please define I3_TESTDATA")
                  

filename = os.path.join(
        testdata, "event-viewer", "Level3aGCD_IC79_EEData_Run00115990.i3")

nevents = 10
pulses = "SRTOfflinePulses"

# ----IceTray------------------------------------------------------------------
tray = I3Tray()

# ----Services-----------------------------------------------------------------
tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                InputReadout=pulses,
                Likelihood="SPE1st",
                PEProb="GaussConvoluted",
                IceModel=2,
                AbsorptionLength=98.*icecube.icetray.I3Units.m,
                NoiseProbability=100.*icecube.icetray.I3Units.hertz,
                JitterTime=15.*icecube.icetray.I3Units.ns)

# ----Modules------------------------------------------------------------------
tray.AddModule("I3Reader", "reader",
               FilenameList=[filename])

tray.AddModule(icecube.gulliver_modules.fortytwo.MultiplicityCutModule,
               "enough_doms_and_strings",
               pulses=pulses,
               nchmin=12,
               nstrmin=3)

# Silly old line fit
tray.AddModule("I3LineFit", "lf",
               Name="lf",
               InputRecoPulses=pulses,
               AmpWeightPower=1.)

# Take result from old line fit and compute its spe1st likelihood.
# Store result in lfLOGL.
tray.AddModule("I3LogLikelihoodCalculator", "lfLOGL",
               FitName="lf",
               LogLikelihoodService="spe1st",
               NFreeParameters=5)

# Create/overwrite reference.
# pc = icecube.gulliver_modules.fortytwo.I3ParticleChecker(["lf"])
# lhfpc = icecube.gulliver_modules.fortytwo.I3LogLikelihoodFitParamsChecker(
#     ["lfLOGL"])
#
# tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo, "test_suite",
#                checklist=[pc, lhfpc])

# Run test module.
tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo, "test_suite")



# ----Execute------------------------------------------------------------------
tray.Execute(3 + 2*nevents)

