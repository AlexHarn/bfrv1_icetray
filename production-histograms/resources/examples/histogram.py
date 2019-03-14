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
from icecube.production_histograms.generate_collection_name import generate_collection_name

print(options)
i3files = glob(options.INFILES)          
# remember this just loops over all the frames in the
# I3Files to see what objects need to be histogrammed.
histograms = generate_histogram_configuration_list(i3files)
if len(histograms) == 0:
    icetray.logging.log_fatal("Found nothing to histogram.")

collection_names = set()
for fn in i3files:
    collection_names.add(generate_collection_name(''.join(fn.split('/')[:-1])))

if len(collection_names) != 1:
    for n in collection_names:
        icetray.logging.log_warn(str(n))
    icetray.logging.log_fatal('There can be only one.')
collection_name = collection_names.pop()

tray = I3Tray()

filelist = [options.GCD]
filelist.extend(i3files)

tray.Add("I3Reader", FilenameList = filelist)
tray.Add(ProductionHistogramModule, 
         Histograms = histograms,
         CollectionName = collection_name,
         FilenameList = filelist
        )
tray.Execute()


