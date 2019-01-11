#!/usr/bin/env python

import unittest
from icecube import tpx

class TestI3TopPulseInfo(unittest.TestCase):

    def test_equality(self):
        info1 = tpx.I3TopPulseInfo()
        info1.amplitude = 1.0e3
        info1.risetime = 10.0
        info1.trailingEdge = 50.0
        info1.status = tpx.OK
        info1.channel = 0
        info1.source_id = 1

        info2 = tpx.I3TopPulseInfo()
        info2.amplitude = 1.0e3
        info2.risetime = 10.0
        info2.trailingEdge = 50.0
        info2.status = tpx.OK
        info2.channel = 0
        info2.source_id = 1

        self.assertEqual(info1, info2, "these should be the same.")

unittest.main()
