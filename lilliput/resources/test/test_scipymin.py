#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This script illustrates and tests the use of `SciPyMinimizer`.
"""
import os
import sys

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.gulliver
import icecube.lilliput
import icecube.dataio
import icecube.common_variables
import icecube.tableio
import icecube.linefit
import icecube.lilliput.segments

try:
    import icecube.lilliput.scipymin
except ImportError:
    print("Omitting SciPyMinimizer test because of missing SciPy.")
    sys.exit(0)

from I3Tray import I3Tray

import helper


icecube.icetray.logging.set_level_for_unit("COGPrior", "INFO")

if "I3_TESTDATA" in os.environ:
    testdata = os.environ["I3_TESTDATA"]
else:
    raise IOError("Cannot find test data file. Please define I3_TESTDATA.")

gcdfilename = os.path.join(
    testdata, "GCD", "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")

filename = os.path.join(
    testdata, "sim", "Level2_IC86.2011_corsika.010281.001664.00.i3.bz2")

pulsemap = "SRTOfflinePulses"
linefit = "ImprovedLineFit"

tray = I3Tray()

tray.Add("I3Reader", filenamelist=[gcdfilename, filename])

tray.Add(icecube.linefit.simple,
         inputResponse=pulsemap,
         fitName=linefit,
         If=helper.Multiplicity(pulsemap))

cog_weight = helper.COGPrior(
    name="cog_weight", pulsemap=pulsemap,
    distance=30.*icecube.icetray.I3Units.m)

likelihood = icecube.lilliput.segments.add_pandel_likelihood_service(
    tray, pulsemap)

tray.Add("I3EventLogLikelihoodCombinerFactory", "cog_weighted_pandel",
         InputLogLikelihoods=[cog_weight, likelihood],
         Multiplicity=likelihood,
         RelativeWeights=[10., 1.])

minimizer = icecube.lilliput.scipymin.SciPyMinimizer(
    name="scipy_simplex", method="Powell",
    tolerance=0.001,
    max_iterations=1000)

parametrization = icecube.lilliput.I3SimpleParametrization("pypar")

parametrization.SetStep(
    icecube.lilliput.I3SimpleParametrization.PAR_X,
    20.*icecube.icetray.I3Units.m, False)
parametrization.SetStep(
    icecube.lilliput.I3SimpleParametrization.PAR_Y,
    20.*icecube.icetray.I3Units.m, False)
parametrization.SetStep(
    icecube.lilliput.I3SimpleParametrization.PAR_Z,
    20.*icecube.icetray.I3Units.m, False)
parametrization.SetStep(
    icecube.lilliput.I3SimpleParametrization.PAR_Zen,
    0.1*icecube.icetray.I3Units.radian, False)
parametrization.SetStep(
    icecube.lilliput.I3SimpleParametrization.PAR_Azi,
    0.2*icecube.icetray.I3Units.radian, True)

seed = icecube.lilliput.segments.add_seed_service(
    tray, pulsemap, [linefit])

tray.Add("I3SimpleFitter",
         LogLikelihood="cog_weighted_pandel",
         Minimizer=minimizer,
         Parametrization=parametrization,
         SeedService=seed,
         OutputName="SciPyFit",
         If=helper.Multiplicity(pulsemap))

minuit = icecube.lilliput.segments.add_minuit_simplex_minimizer_service(tray)

tray.Add("I3SimpleFitter",
         LogLikelihood="cog_weighted_pandel",
         Minimizer=minuit,
         Parametrization=parametrization,
         SeedService=seed,
         OutputName="SemiSciPyFit",
         If=helper.Multiplicity(pulsemap))

tray.Add(helper.PFCounter,
         If=helper.Multiplicity(pulsemap))

tray.Execute()
