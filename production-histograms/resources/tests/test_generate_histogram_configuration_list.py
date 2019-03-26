#!/usr/bin/env python

import os
import unittest

from icecube.production_histograms.configuration_tool import generate_histogram_configuration_list
        
class TestHistogram(unittest.TestCase):

    def test_generate_histogram_configuration_list(self):
        
        
        PATH = os.path.expandvars("$I3_TESTDATA/production-histograms/")
        fn = os.path.join(PATH, "Level2_IC86.2016_corsika.020699.000000.i3.zst")
        histogram_list = generate_histogram_configuration_list(fn)

        # currently there are 456 histograms.
        # we might add more later.
        self.assertTrue(len(histogram_list) >= 456)
        
unittest.main()
