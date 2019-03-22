#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Test script for Gulliver

This script does many reconstructions:

    * ``I3LineFit`` from `linefit`
    * ``I3SimpleFitter`` from `gulliver_modules`: SPE fit with jitter
      set to 2ns, 4ns, 10 ns
    * ``I3SimpleFitter`` from `gulliver_modules`: MPE fit (12x) with PDF
      jitter set to 2ns, 4ns, 10ns; DOM-wise jitter (`MPEtimingerror`)
      set to 0, 2, 4, 10 ns

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
                Algorithm="SIMPLEX")

tray.AddService("I3BasicSeedServiceFactory", "lineseed",
                InputReadout=pulses,
                TimeShiftType="TFirst",
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

particles = ["linefit"]
loglparams = []

for jitter in [2, 4, 10]:
    llh = "spe%d" % jitter
    tray.AddService("I3GulliverIPDFPandelFactory", llh,
                    InputReadout=pulses,
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    AbsorptionLength=98.*icecube.icetray.I3Units.m,
                    NoiseProbability=100.*icecube.icetray.I3Units.hertz,
                    JitterTime=jitter*icecube.icetray.I3Units.ns)

    spefit = llh + "fit"
    tray.AddModule("I3SimpleFitter",
                   OutputName=spefit,
                   SeedService="lineseed",
                   Parametrization="simpletrack",
                   LogLikelihood=llh,
                   Minimizer="minuit")

    particles.append(spefit)
    loglparams.append(spefit+"FitParams")

    for mpe_error in [0, 2, 4, 10]:
        llh = "mpe%d%d" % (jitter, mpe_error)
        mpefit = llh + "fit"
        tray.AddService("I3GulliverIPDFPandelFactory", llh,
                        InputReadout=pulses,
                        Likelihood="MPE",
                        PEProb="GaussConvolutedFastApproximation",
                        IceModel=2,
                        AbsorptionLength=98.*icecube.icetray.I3Units.m,
                        NoiseProbability=100.*icecube.icetray.I3Units.hertz,
                        JitterTime=jitter*icecube.icetray.I3Units.ns,
                        MPETimingError=mpe_error*icecube.icetray.I3Units.ns)

        tray.AddModule("I3SimpleFitter",
                       OutputName=mpefit,
                       SeedService="lineseed",
                       Parametrization="simpletrack",
                       LogLikelihood=llh,
                       Minimizer="minuit")

        particles.append(mpefit)
        loglparams.append(mpefit+"FitParams")

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

