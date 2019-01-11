#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Test script for Gulliver

This script illustrates the use of alternative seeds, as defined by the
``I3BasicSeedService`` in `lilliput`. For a given 'first guess' it can
produce extra seeds which go the opposite direction (`Reverse`), or in
three 'backward' directions aligned tetrahedron-wise w.r.t. the original
first guess (`Tetrahedron`), and similarly for `Cube`, with five extra
seeds. For the alternative seeds we restrict the direction phase space
to a half-sphere centered around the seed.

The idea is that when you use a likelihood fit as first guess, you can
make likelihood ratios with the alternatives and use those as a quality
parameter. Dima pioneered this in Rime and it seems to be powerful.

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
    raise IOError("Cannot find test data file, please define either "
                  "I3_TESTDATA or I3_PORTS")

filename = os.path.join(
        testdata, "event-viewer", "Level3aGCD_IC79_EEData_Run00115990.i3")

nevents = 10
pulses = "SRTOfflinePulses"

# ----IceTray------------------------------------------------------------------
tray = I3Tray()

# ----Services-----------------------------------------------------------------
tray.AddService("I3GulliverMinuitFactory", "minuit",
                Tolerance=0.001,
                Algorithm="SIMPLEX")

tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                InputReadout=pulses,
                Likelihood="SPE1st",
                PEProb="GaussConvoluted",
                IceModel=2,
                AbsorptionLength=98.*icecube.icetray.I3Units.m,
                JitterTime=15.*icecube.icetray.I3Units.ns,
                NoiseProbability=100.*icecube.icetray.I3Units.hertz)

# Parametrization which limits the direction phase space to a hemisphere around
# the seed track.
tray.AddService("I3HalfSphereParametrizationFactory", "umbrella",
                DirectionStepSize=0.2*icecube.icetray.I3Units.rad,
                VertexStepSize=30.*icecube.icetray.I3Units.m)

# Parametrization without limits on direction
tray.AddService("I3SimpleParametrizationFactory", "xyzza",
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

# Single seed using linefit first guess
tray.AddService("I3BasicSeedServiceFactory", "oneseed",
                InputReadout=pulses,
                TimeShiftType="TFirst",
                FirstGuesses=["linefit"])

# The reverse seed (based on LLH fit)
tray.AddService("I3BasicSeedServiceFactory", "anotherseed",
                InputReadout=pulses,
                TimeShiftType="TNone",
                AltTimeShiftType="TFirst",
                AddAlternatives="Reverse",
                OnlyAlternatives=False,
                FirstGuesses=["onefit"])

# Double seed using onefit and its reverse as first guess
tray.AddService("I3BasicSeedServiceFactory", "twoseeds",
                InputReadout=pulses,
                TimeShiftType="TNone",
                AltTimeShiftType="TFirst",
                AddAlternatives="Reverse",
                OnlyAlternatives=False,
                FirstGuesses=["onefit"])

# Triple seed: find three other direction vectors that together with onefit's
# direction vector would form a tetrahedron; only use those three directions as
# first guesses (a bit like "reverse" fit, but then with a bit more coverage).
tray.AddService("I3BasicSeedServiceFactory", "threeseeds",
                InputReadout=pulses,
                TimeShiftType="TNone",
                AltTimeShiftType="TFirst",
                AddAlternatives="Tetrahedron",
                OnlyAlternatives=True,
                FirstGuesses=["onefit"])

# Quadruple seed: find three other direction vectors that together with
# onefit's direction vector would form a tetrahedron; use all four as first
# guesses. This would be an interesting alternative to a three-fold iterative
# fit.
tray.AddService("I3BasicSeedServiceFactory", "fourseeds",
                InputReadout=pulses,
                TimeShiftType="TNone",
                AltTimeShiftType="TFirst",
                AddAlternatives="Tetrahedron",
                OnlyAlternatives=False,
                FirstGuesses=["onefit"])

# Five-fold seed: alternatives are arranged like the faces of a cube. Or the
# points of an octahedron. Leaving out the input track. This is actually not
# particularly useful.
tray.AddService("I3BasicSeedServiceFactory", "fiveseeds",
                InputReadout=pulses,
                TimeShiftType="TNone",
                AltTimeShiftType="TFirst",
                AddAlternatives="Cube",
                OnlyAlternatives=True,
                FirstGuesses=["onefit"])

# Six-fold seed: alternatives are arranged like the faces of a cube. Or the
# points of an octahedron.
tray.AddService("I3BasicSeedServiceFactory", "sixseeds",
                InputReadout=pulses,
                TimeShiftType="TNone",
                AltTimeShiftType="TFirst",
                AddAlternatives="Cube",
                OnlyAlternatives=False,
                FirstGuesses=["onefit"])

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

tray.AddModule("I3SimpleFitter",
               OutputName="onefit",
               SeedService="oneseed",
               Parametrization="xyzza",
               LogLikelihood="spe1st",
               Minimizer="minuit")

tray.AddModule("I3SimpleFitter",
               OutputName="anotherfit",
               SeedService="anotherseed",
               Parametrization="umbrella",
               LogLikelihood="spe1st",
               Minimizer="minuit")

tray.AddModule("I3SimpleFitter",
               OutputName="twofits",
               SeedService="twoseeds",
               Parametrization="umbrella",
               LogLikelihood="spe1st",
               Minimizer="minuit",
               StoragePolicy="AllFitsAndFitParams")

tray.AddModule("I3SimpleFitter",
               OutputName="threefits",
               SeedService="threeseeds",
               Parametrization="umbrella",
               LogLikelihood="spe1st",
               Minimizer="minuit",
               StoragePolicy="AllFitsAndFitParams")

tray.AddModule("I3SimpleFitter",
               OutputName="fourfits",
               SeedService="fourseeds",
               Parametrization="umbrella",
               LogLikelihood="spe1st",
               Minimizer="minuit",
               StoragePolicy="AllFitsAndFitParams")

tray.AddModule("I3SimpleFitter",
               OutputName="fivefits",
               SeedService="fiveseeds",
               Parametrization="umbrella",
               LogLikelihood="spe1st",
               Minimizer="minuit",
               StoragePolicy="AllFitsAndFitParams")

tray.AddModule("I3SimpleFitter",
               OutputName="sixfits",
               SeedService="sixseeds",
               Parametrization="umbrella",
               LogLikelihood="spe1st",
               Minimizer="minuit",
               StoragePolicy="AllFitsAndFitParams")

# particles = ["linefit", "onefit", "anotherfit", "twofits", "threefits",
#              "fourfits", "fivefits", "sixfits"
#              ]
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

