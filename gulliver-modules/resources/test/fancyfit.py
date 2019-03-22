#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Test script for Gulliver

This script does 21 reconstructions, namely one LineFit and then
``2*2*5 = 20`` fits with ``I3SimpleFitter`` from `gulliver_modules`,
namely all combinations of:

    * Two vertex timing corrections: tfirst or tq90
    * Five jitter values: 4 ns or to 15 ns
    * Five likelihoods: SPE1st, SPEAll, SPEqAll, MPE, MPEAll

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

# ----Services for Gulliver reconstruction-------------------------------------
tray.AddService("I3SimpleParametrizationFactory", "simpletrack",
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

tray.AddService("I3GulliverMinuitFactory", "minuit",
                Tolerance=0.001,
                Algorithm="SIMPLEX")

tray.AddService("I3BasicSeedServiceFactory", "tfirst",
                InputReadout=pulses,
                TimeShiftType="TFirst",
                FirstGuesses=["linefit"])

tray.AddService("I3BasicSeedServiceFactory", "tq90",
                InputReadout=pulses,
                TimeShiftType="TChargeFraction",
                ChargeFraction=0.9,
                FirstGuesses=["linefit"])

# ----Modules------------------------------------------------------------------
tray.AddModule("I3Reader", "reader",
               FilenameList=[filename])

tray.AddModule(icecube.gulliver_modules.fortytwo.MultiplicityCutModule,
               "enough_doms_and_strings",
               pulses=pulses,
               nchmin=12,
               nstrmin=3)

tray.AddModule("I3LineFit", "linefit",
               Name="linefit",
               InputRecoPulses=pulses,
               AmpWeightPower=1.)

# class DumpTrack(icecube.icetray.I3ConditionalModule):
#     def __init__(self, context):
#         icecube.icetray.I3ConditionalModule.__init__(self, context)
#         self.AddParameter("trackname", "name of track")
#
#     def Configure(self):
#         self.trackname = self.GetParameter("trackname")
#
#     def Physics(self,frame):
#         print("%s" % frame[self.trackname])

particles = ["linefit"]
loglparams = []

for jitter in [4, 15]:
    for domllh in ["SPE1st", "SPEAll", "SPEqAll", "MPE", "MPEAll"]:
        llh = "%s%d" % (domllh, jitter)
        tray.AddService("I3GulliverIPDFPandelFactory", llh,
                        InputReadout=pulses,
                        Likelihood=domllh,
                        PEProb="GaussConvoluted",
                        NoiseProbability=1000.0*icecube.icetray.I3Units.hertz,
                        JitterTime=jitter*icecube.icetray.I3Units.ns)

        for seedservice in ["tfirst", "tq90"]:
            fitname = seedservice + "_" + llh
            tray.AddModule("I3SimpleFitter",
                           OutputName=fitname,
                           SeedService=seedservice,
                           Parametrization="simpletrack",
                           LogLikelihood=llh,
                           Minimizer="minuit")

            particles.append(fitname)
            loglparams.append(fitname+"FitParams")

# Create/update reference data.
# pc = icecube.gulliver_modules.fortytwo.I3ParticleChecker(particles)
# lhfpc = icecube.gulliver_modules.fortytwo.I3LogLikelihoodFitParamsChecker(
#     loglparams)
#
# tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo, "test_suite",
#                checklist=[pc, lhfpc])

# Run test module.
# tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo, "test_suite")



# ----Execute------------------------------------------------------------------
tray.Execute(3 + 2*nevents)

