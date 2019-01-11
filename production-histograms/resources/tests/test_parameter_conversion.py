#!/usr/bin/env python

import os
import unittest

from icecube import icetray
from icecube.production_histograms.histograms.histogram import Histogram
from icecube.production_histograms.histogram_modules.histogram_module import HistogramModule

# These two are the functions we're testing.
from icecube.production_histograms.icetray_modules.production_histograms_module import parameter_conversion
from icecube.production_histograms.icetray_modules.production_histograms_module import instance_of

class DerivedHistogram(Histogram):
    '''Derived class to test the Histogram base class'''
    def DAQ(self, frame):
        self.fill(frame["integer"].value)    
        
class TestParameterConversion(unittest.TestCase):

    def test_instance_of(self):
        h = Histogram(0,10,10,"Histogram")
        self.assertTrue(isinstance(instance_of(h), Histogram))
        self.assertTrue(isinstance(instance_of(HistogramModule()), HistogramModule))
        self.assertTrue(isinstance(instance_of(HistogramModule), HistogramModule))
                
    def test_conversion(self):
        h = Histogram(0,10,10,"Histogram")
        dh = DerivedHistogram(0,10,10,"DerivedHistogram")

        histograms = [h, dh]
        converted = parameter_conversion(histograms)
        self.assertEqual(len(histograms), len(converted))

    def test_list_conversion(self):
        h = Histogram(0,10,10,"Histogram")
        dh = DerivedHistogram(0,10,10,"DerivedHistogram")

        histograms = [h, dh]
        converted = parameter_conversion([histograms, histograms])
        self.assertEqual(2*len(histograms), len(converted))

    def test_set_conversion(self):
        h = Histogram(0,10,10,"Histogram")
        dh = DerivedHistogram(0,10,10,"DerivedHistogram")

        histograms = set([h, dh])
        converted = parameter_conversion([histograms, histograms])
        self.assertEqual(2*len(histograms), len(converted))
        
    def test_dict_conversion(self):
        h = Histogram(0,10,10,"Histogram")
        dh = DerivedHistogram(0,10,10,"DerivedHistogram")

        hmap = {"base": h, "derived": dh}
        converted = parameter_conversion([h, dh, hmap])
        self.assertEqual(4, len(converted))
                
unittest.main()
