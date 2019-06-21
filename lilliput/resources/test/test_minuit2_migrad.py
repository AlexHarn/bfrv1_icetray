#!/usr/bin/env python
# -*- coding: utf-8 -*-

r"""This test is intended to trigger when the internal tolerance
scaling in Minuit2/MIGRAD should change again.
"""
import os
import unittest

import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.linefit
import icecube.lilliput
import icecube.lilliput.segments

from I3Tray import I3Tray


if "I3_TESTDATA" not in os.environ:
    raise IOError("Cannot find test data. Please define I3_TESTDATA.")

gcdfile = os.path.join(
    os.environ["I3_TESTDATA"], "GCD",
    "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")

inputfile = os.path.join(
    os.environ["I3_TESTDATA"], "sim",
    "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2")

tray = I3Tray()

tray.Add("I3Reader",
         filenamelist=[gcdfile, inputfile])

pulsemap = "TWSRTHVInIcePulses"
linefit = "TestMinuit2MigradLineFit"

tray.Add(icecube.linefit.simple,
         inputResponse=pulsemap,
         fitName=linefit)

tray.Add("I3GulliverMinuit2Factory", "minuit2",
         Algorithm="MIGRAD",
         ignoreEDM=True,
         MaxIterations=1000,
         Tolerance=10.)

spefit = "TestMinuit2MigradSPEFitSingle"

tray.Add(icecube.lilliput.segments.I3SinglePandelFitter,
         fitname=spefit,
         minimizer="minuit2",
         pulses=pulsemap,
         seeds=[linefit])

fitparams = spefit + "FitParams"
reference = spefit + "ReferenceFitParams"


class SPEFitSingleFitParamsTest(unittest.TestCase):
    def setUp(self):
        self.fitparams = self.frame[fitparams]
        self.reference = self.frame[reference]

    def test_ndof(self):
        self.assertEqual(
            self.fitparams.ndof,
            self.reference.ndof)

    def test_nmini(self):
        self.assertAlmostEqual(
            self.fitparams.nmini,
            self.reference.nmini,
            delta=100)

    def test_logl(self):
        self.assertAlmostEqual(
            self.fitparams.logl,
            self.reference.logl,
            delta=0.05 + 0.01 * self.reference.logl)

    def test_rlogl(self):
        self.assertAlmostEqual(
            self.fitparams.rlogl,
            self.reference.rlogl,
            delta=0.05 + 0.01 * self.reference.rlogl)


def test_requirements(frame):
    r"""Check the test requirements.

    Event 189 is excluded because the test cases fail for ROOT 6.09.02;
    see ticket 2114. The strike team concluded that the minimizer
    probably runs into a saddle point and takes a different path than
    before due to minimal changes in ROOT. It was decided that it is
    safe to exclude this event because it is in general badly
    reconstructed (RLogL = 8.1). This should be verified with newer ROOT
    versions in the future. Note that ROOT 6.12.06 and ROOT 6.14.00 seem
    to work again.

    """
    run_test = (
        fitparams in frame and
        reference in frame and
        frame["I3EventHeader"].event_id != 189
        )

    return run_test


tray.Add(icecube.icetray.I3TestModuleFactory(SPEFitSingleFitParamsTest),
         If=test_requirements)

tray.Execute()
