#!/usr/bin/env python

import unittest
from icecube import dataclasses, tpx

class TestI3IceTopBaseline(unittest.TestCase):

    def test_equality(self):
        baseline1 = tpx.I3IceTopBaseline()
        baseline1.source = dataclasses.I3Waveform.ATWD
        baseline1.channel = 0
        baseline1.source_id = 1
        baseline1.baseline = 1.0
        baseline1.slope = 2.0
        baseline1.rms = 0.1

        baseline2 = tpx.I3IceTopBaseline()
        baseline2.source = dataclasses.I3Waveform.ATWD
        baseline2.channel = 0
        baseline2.source_id = 1
        baseline2.baseline = 1.0
        baseline2.slope = 2.0
        baseline2.rms = 0.1

        self.assertEqual(baseline1, baseline2, "these should be the same.")

unittest.main()
