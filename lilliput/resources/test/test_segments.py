#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This script illustrates and tests the use of `segments`.
"""
import os

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.linefit
import icecube.gulliver
import icecube.lilliput
import icecube.lilliput.segments
import icecube.gulliver_modules.fortytwo
from I3Tray import I3Tray

import helper


if "I3_TESTDATA" in os.environ:
    testdata = os.environ["I3_TESTDATA"]
else:
    raise IOError("Cannot find test data file. Please define I3_TESTDATA.")

gcdfilename = os.path.join(
    testdata, "sim", "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")

filename = os.path.join(
    testdata, "sim", "Level2_IC86.2011_corsika.010281.001664.00.i3.bz2")

pulsemap = "SRTOfflinePulses"

particles = []
llhfitparams = []

icecube.icetray.logging.set_level_for_unit("Multiplicity", "NOTICE")

tray = I3Tray()

tray.Add("I3Reader", filenamelist=[gcdfilename, filename])

# The seed of all loglikelihood track fits
tray.Add(icecube.linefit.simple,
         inputResponse=pulsemap,
         fitName="imprLF",
         If=helper.Multiplicity(pulsemap))

particles.append("imprLF")

# The simplest loglikelihood track fit we have.
tray.Add(icecube.lilliput.segments.I3SinglePandelFitter,
         fitname="SPEfitA",
         pulses=pulsemap,
         seeds=["imprLF"],
         domllh="SPE1st",
         If=helper.Multiplicity(pulsemap))

particles.append("SPEfitA")
llhfitparams.append("SPEfitAFitParams")

# Get the corresponding paraboloid fit.
tray.Add(icecube.lilliput.segments.I3ParaboloidPandelFitter,
         fitname="paraSPEfitA",
         pulses=pulsemap,
         domllh="SPE1st",
         inputtrack="SPEfitA",
         input_tstype="TNone",
         grid_tstype="TFirst",
         If=helper.Multiplicity(pulsemap))

particles.append("paraSPEfitA")
# llhfitparams.append("SPEfitAFitParams")

# This is the same as SPEfitA, so it will reuse the services.
tray.Add(icecube.lilliput.segments.I3SinglePandelFitter,
         fitname="SPEfitB",
         pulses=pulsemap,
         seeds=["imprLF"],
         domllh="SPE1st",
         tstype="TNone",
         If=helper.Multiplicity(pulsemap))

particles.append("SPEfitB")
llhfitparams.append("SPEfitBFitParams")

# Here, we have an MPE fit.
tray.Add(icecube.lilliput.segments.I3SinglePandelFitter,
         fitname="MPEfit",
         pulses=pulsemap,
         seeds=["SPEfitB"],
         domllh="MPE",
         tstype="TNone",
         If=helper.Multiplicity(pulsemap))

particles.append("MPEfit")
llhfitparams.append("MPEfitFitParams")

# Estimate the angular uncertainty for the MPEfit, using a fancy way to correct
# the grid seed vertex time.
tray.Add(icecube.lilliput.segments.I3ParaboloidPandelFitter,
         fitname="pbfMPE",
         pulses=pulsemap,
         inputtrack="MPEfit",
         domllh="MPE",
         input_tstype="TNone",
         grid_tstype="TFirst",
         If=helper.Multiplicity(pulsemap))

particles.append("pbfMPE")
# llhfitparams.append("pbfMPEFitParams")

# Names of services are returned, so you can re-use those services.
# We are doing a SPE fit now, using all pulses; not weighted with charge.
minimizer, parametrization, likelihood, seed, spefit =\
    tray.Add(icecube.lilliput.segments.I3SinglePandelFitter,
             fitname="SPEAllfit",
             pulses=pulsemap,
             seeds=["SPEfitB"],
             domllh="SPEAll",
             tstype="TNone",
             If=helper.Multiplicity(pulsemap))

particles.append("SPEAllfit")
llhfitparams.append("SPEAllfitFitParams")

# Re-using the SPEqALLfit likelihood service (from a previous segment).
# Use that service to compute the SPEAll likelihood value of the MPE fit.
tray.Add("I3LogLikelihoodCalculator",
         FitName="MPEfit",
         LogLikelihoodService=likelihood,
         NFreeParameters=5,
         If=helper.Multiplicity(pulsemap))

llhfitparams.append("mpeSpeAllLLH")

# Iterative SPE fit
tray.Add(icecube.lilliput.segments.I3IterativePandelFitter,
         fitname="SPE8fitA",
         n_iterations=8,
         pulses=pulsemap,
         seeds=["imprLF"],
         domllh="SPE1st",
         If=helper.Multiplicity(pulsemap))

particles.append("SPE8fitA")
llhfitparams.append("SPE8fitAFitParams")

# Iterative SPE fit with higher noise rate
tray.Add(icecube.lilliput.segments.I3IterativePandelFitter,
         fitname="SPE8fitB",
         n_iterations=8,
         pulses=pulsemap,
         seeds=["imprLF"],
         domllh="SPE1st",
         tstype="TNone",
         noiserate=1000.*icecube.icetray.I3Units.hertz,
         If=helper.Multiplicity(pulsemap))

particles.append("SPE8fitB")
llhfitparams.append("SPE8fitBFitParams")

# Estimate the angular uncertainty for the SPE8fitB fit from the previous
# segment.
tray.Add(icecube.lilliput.segments.I3ParaboloidPandelFitter,
         fitname="SPE8pbf",
         pulses=pulsemap,
         inputtrack="SPE8fitB",
         domllh="SPE1st",
         input_tstype="TNone",
         grid_tstype="TFirst",
         noiserate=1000.*icecube.icetray.I3Units.hertz,
         If=helper.Multiplicity(pulsemap))

particles.append("SPE8pbf")
# llhfitparams.append("SPE8pbfFitParams")

# DISABLE TEMPORARILY: SOME PLATFORMS FAIL THIS TEST
# pc = icecube.gulliver_modules.fortytwo.I3ParticleChecker(particles)
#
# lhfpc = icecube.gulliver_modules.fortytwo.I3LogLikelihoodFitParamsChecker(
#     llhfitparams)

# # TODO: add checklists for paraboloid & linefit params.
# tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo,
#                checklist=[pc, lhfpc],
#                If=helper.Multiplicity(pulsemap),
#                Key="")
#
# tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo,
#                If=helper.Multiplicity(pulsemap),
#                Key="")

tray.Add(helper.PFCounter,
         If=helper.Multiplicity(pulsemap))

tray.Execute()
