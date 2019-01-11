#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Test script for Gulliver

This script does four reconstructions:

    * `dipolefit`: seed for iterative fitters
    * ``I3IterativeFitter`` from `gulliver_modules`

      - Random directions for iterations
      - Directions for iterations from Sobol grid w.r.t. original seed
      - Directions for iterations from Niederreiter2 grid w.r.t.
        original seed

It uses the `fortytwo` functionality to check that the current results
are equal to (or compatible with) the reference results.

"""
import os

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.dipolefit
import icecube.gulliver
import icecube.gulliver_modules
import icecube.gulliver_modules.fortytwo
import icecube.lilliput
from I3Tray import I3Tray

icecube.icetray.logging.set_level_for_unit("I3FortyTwo", "INFO")

if "I3_TESTDATA" in os.environ:
    testdata = os.environ["I3_TESTDATA"]
else:
    raise IOError("Cannot find test data file, please define either "
                  "I3_TESTDATA or I3_PORTS")

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
                Algorithm="SIMPLEX")

tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                InputReadout=pulses,
                Likelihood="SPE1st",
                PEProb="GaussConvoluted",
                NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                JitterTime=15.*icecube.icetray.I3Units.ns)

tray.AddService("I3BasicSeedServiceFactory", "seedprep",
                InputReadout=pulses,
                TimeShiftType="TFirst",
                FirstGuesses=["df"])

tray.AddService("I3GSLRandomServiceFactory", Seed=74088)

# ----Modules------------------------------------------------------------------
tray.AddModule("I3Reader", "reader",
               FilenameList=[filename])

tray.AddModule(icecube.gulliver_modules.fortytwo.MultiplicityCutModule,
               "enough_doms_and_strings",
               pulses=pulses,
               nchmin=12,
               nstrmin=3)

# This is an ancient AMANDA-time first-guess method used here just for fun and
# illustration.
tray.AddModule("I3DipoleFit", "dipolefit",
               Name="df",
               InputRecoPulses=pulses,
               AmpWeightPower=1.,
               DipoleStep=0)

particles = ["df"]
loglparams = []

for prefix, rndservice in zip(["rnd", "sobol", "nr2"],
                              ["I3RandomService", "SOBOL", "NIEDERREITER2"]):
    fitname = prefix + "_iterative"
    tray.AddModule("I3IterativeFitter",
                   OutputName=fitname,
                   SeedService="seedprep",
                   RandomService=rndservice,
                   Parametrization="simpletrack",
                   LogLikelihood="spe1st",
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
tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo, "test_suite")



# ----Execute------------------------------------------------------------------
tray.Execute(3 + 2*nevents)

