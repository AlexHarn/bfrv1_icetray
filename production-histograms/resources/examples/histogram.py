#!/usr/bin/env python

from optparse import OptionParser

parser = OptionParser()

parser.add_option("-g","--gcd",
                  dest = "GCD",
                  help = "GCD filename.")

parser.add_option("-i","--infiles",
                  dest = "INFILES",
                  help = "Regex of files to glob.")
(options, args) = parser.parse_args()

import sys, os

from glob import glob
from pymongo import MongoClient        

from I3Tray import I3Tray
from icecube import icetray, dataio
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.configuration_tool import generate_histogram_configuration_list
from icecube.production_histograms.categorize import categorize

print(options)
i3files = glob(options.INFILES)          
# remember this just loops over all the frames in the
# I3Files to see what objects need to be histogrammed.
histograms = generate_histogram_configuration_list(i3files)
if len(histograms) == 0:
    icetray.logging.log_fatal("Found nothing to histogram.")

categories = set()
for fn in i3files:
    categories.add(categorize(''.join(fn.split('/')[:-1])))

if len(categories) != 1:
    for c in categories:
        icetray.logging.log_warn(str(c))
    icetray.logging.log_fatal('There can be only one.')
category = categories.pop()

tray = I3Tray()

filelist = [options.GCD]
filelist.extend(i3files)

tray.Add("I3Reader", FilenameList = filelist)
tray.Add(ProductionHistogramModule, 
         Histograms = histograms,
         Category = category,
         FilenameList = filelist
        )
tray.Execute()


