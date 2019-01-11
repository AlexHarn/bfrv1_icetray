#!/usr/bin/env python

from optparse import OptionParser

parser = OptionParser()

parser.add_option("-p","--path",
                  dest = "PATH",
                  help = "Path to I3Files to histogram")
(options, args) = parser.parse_args()

import sys, os

from pymongo import MongoClient        

from I3Tray import I3Tray
from icecube import icetray, dataio
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.configuration_tool import generate_histogram_configuration_list
from icecube.production_histograms.configuration_tool import generate_filelist
from icecube.production_histograms.categorize import categorize

f = open('/home/olivas/.mongo')
client = MongoClient("mongodb://DBadmin:%s@mongodb-simprod.icecube.wisc.edu" %
                     f.readline().strip())

category = categorize(options.PATH)
collection = client.simprod_filecatalog.filelist
document = collection.find_one({'category': category})
if 'histogram_ids' in document:
    icetray.logging.log_fatal('This collection has been histogrammed already...skipping.')
    
tray = I3Tray()

filelist = generate_filelist(options.PATH)

histograms = generate_histogram_configuration_list(filelist)

if len(histograms) == 0:
    icetray.logging.log_fatal("Found nothing to histogram.")

tray.Add("I3Reader", FilenameList = filelist[:2])

tray.Add(ProductionHistogramModule, 
         Histograms = histograms,
         Category = category,
         FilenameList = filelist
        )

tray.Execute()


