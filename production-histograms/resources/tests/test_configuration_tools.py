#!/usr/bin/env python

import os
import unittest

from icecube import icetray
from icecube import dataclasses
from icecube.production_histograms.configuration_tool import _known_type
from icecube.production_histograms.configuration_tool import _get_modules
from icecube.production_histograms.configuration_tool import _configure_from_frame
from icecube.production_histograms.configuration_tool import _configure
        
class TestHistogram(unittest.TestCase):

    def test_known_type(self):    
        self.assertTrue(_known_type(dataclasses.I3MCTree()))
        self.assertFalse(_known_type(dataclasses.I3DOMLaunch()))

    def test_get_modules(self):    
        self.assertEqual(len(_get_modules(dataclasses.I3MCTree())), 3)

    def test_configure_from_frame(self):
        frame = icetray.I3Frame()
        frame_key = 'I3MCTree'
        frame[frame_key] = dataclasses.I3MCTree()
        histograms = _configure_from_frame(frame, frame_key)
        self.assertEqual(len(histograms), 3)

#    def test_configure(self):
#        I3_TESTDATA = os.environ['I3_TESTDATA']
#        fn = os.path.join(I3_TESTDATA, 'production-histograms', 'Level2_IC86.2016_corsika.020699.000000.i3.zst')
#        histograms = dict()
#        self.assertTrue(_configure(fn, histograms))          
        
unittest.main()
