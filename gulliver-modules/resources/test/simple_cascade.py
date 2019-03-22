#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Test script for Gulliver

This script does some cascade reconstructions: `clast` first guess, and
then Pandel (SPE, MPE, and PSA)

It uses the `fortytwo` functionality to check that the current results
are equal to (or compatible with) the reference results.

"""
import os

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.clast
import icecube.gulliver
import icecube.gulliver_modules
import icecube.gulliver_modules.fortytwo
import icecube.lilliput
from I3Tray import I3Tray

icecube.icetray.logging.set_level_for_unit("I3FortyTwo", "INFO")

if "I3_TESTDATA" in os.environ:
    testdata = os.environ["I3_TESTDATA"]
else:
    raise IOError("cannot find test data file, please define I3_TESTDATA")
                  

filename = os.path.join(
        testdata, "event-viewer", "Level3aGCD_IC79_EEData_Run00115990.i3")

nevents = 10
pulses = "SRTOfflinePulses"

# ----IceTray------------------------------------------------------------------
tray = I3Tray()

# ----Services for Gulliver reconstruction-------------------------------------
tray.AddService("I3SimpleParametrizationFactory", "cTXYZ",
                StepX=20.*icecube.icetray.I3Units.m,
                StepY=20.*icecube.icetray.I3Units.m,
                StepZ=20.*icecube.icetray.I3Units.m,
                StepT=20.*icecube.icetray.I3Units.ns,
                BoundsX=[-1000.*icecube.icetray.I3Units.m,
                         1000.*icecube.icetray.I3Units.m],
                BoundsY=[-1000.*icecube.icetray.I3Units.m,
                         1000.*icecube.icetray.I3Units.m],
                BoundsZ=[-1000.*icecube.icetray.I3Units.m,
                         1000.*icecube.icetray.I3Units.m])

# Bounds in energy 100 GeV - 100 PeV
tray.AddService("I3SimpleParametrizationFactory", "cEdir",
                StepLogE=0.1,
                BoundsLogE=[2, 8])

tray.AddService("I3GSLSimplexFactory", "gslsimplex",
                MaxIterations=15000,
                Tolerance=0.001,
                SimplexTolerance=0.001,
                FlatPatience=200)

tray.AddService("I3GulliverMinuitFactory", "minuit",
                Tolerance=0.001,
                Algorithm="SIMPLEX")

tray.AddService("I3GulliverIPDFPandelFactory", "spe",
                InputReadout=pulses,
                Likelihood="SPE1st",
                EventType="PointCascade",
                PEProb="GaussConvoluted",
                NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                JitterTime=15.*icecube.icetray.I3Units.ns)

tray.AddService("I3GulliverIPDFPandelFactory", "mpe",
                InputReadout=pulses,
                Likelihood="MPE",
                EventType="PointCascade",
                PEProb="GaussConvoluted",
                NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                JitterTime=15.*icecube.icetray.I3Units.ns)

tray.AddService("I3GulliverIPDFPandelFactory", "psa",
                InputReadout=pulses,
                Likelihood="PSA",
                EventType="PointCascade",
                PEProb="GaussConvoluted",
                NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                JitterTime=15.*icecube.icetray.I3Units.ns)

# "Fixed energy" means that all seeds get the same initial energy value it does
# *not* mean that in the energy in the fit is fixed; that is not the seed
# service's business, that's up to the parametrization service.
tray.AddService("I3BasicSeedServiceFactory", "cSPEseed",
                InputReadout=pulses,
                TimeShiftType="TChargeFraction",
                FixedEnergy=1.*icecube.icetray.I3Units.PeV,
                FirstGuesses=["CLastFit"])

tray.AddService("I3BasicSeedServiceFactory", "cMPEseed",
                InputReadout=pulses,
                FixedEnergy=1.*icecube.icetray.I3Units.PeV,
                TimeShiftType="TNone",
                FirstGuesses=["spefit"])

tray.AddService("I3BasicSeedServiceFactory", "cPSAseed",
                InputReadout=pulses,
                FixedEnergy=1.*icecube.icetray.I3Units.PeV,
                TimeShiftType="TNone",
                FirstGuesses=["mpefit"])

# ----Modules------------------------------------------------------------------
tray.AddModule("I3Reader", "reader",
               FilenameList=[filename])

tray.AddModule(icecube.gulliver_modules.fortytwo.MultiplicityCutModule,
               "enough_doms_and_strings",
               pulses=pulses,
               nchmin=12,
               nstrmin=3)

# CFirst was more common in AMANDA days
tray.AddModule("I3CLastModule", "clast",
               Name="CLastFit",
               InputReadout=pulses,
               MinHits=5)

tray.AddModule("I3SimpleFitter",
               OutputName="spefit",
               SeedService="cSPEseed",
               Parametrization="cTXYZ",
               LogLikelihood="spe",
               Minimizer="gslsimplex")

tray.AddModule("I3SimpleFitter",
               OutputName="mpefit",
               SeedService="cMPEseed",
               Parametrization="cTXYZ",
               LogLikelihood="mpe",
               Minimizer="gslsimplex")

tray.AddModule("I3SimpleFitter",
               OutputName="psafit",
               SeedService="cPSAseed",
               Parametrization="cEdir",
               LogLikelihood="psa",
               Minimizer="gslsimplex")

# particles= ["CLastFit", "spefit", "mpefit", "psafit"]
# loglparams = [p+"FitParams" for p in particles]

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

