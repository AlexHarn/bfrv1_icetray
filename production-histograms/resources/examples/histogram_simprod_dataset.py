#!/usr/bin/env python

import os
from optparse import OptionParser

parser = OptionParser()

parser.add_option("-p","--path",
                  dest = "path",
                  help = "Path to I3Files to histogram")
parser.add_option("-g","--gcd_file",
                  dest = "gcd_file",
                  help = "The GCD file.")
parser.add_option("-x","--password_path",
                  dest = "password_path",
                  default = os.path.expandvars('$HOME/.mongo'),
                  help = "DB password file.")
parser.add_option("-c","--collection-name",
                  dest = "collection_name",
                  help = "Name of the DB collection.")
(options, args) = parser.parse_args()

import sys

from pymongo import MongoClient        

from I3Tray import I3Tray
from icecube import icetray
from icecube import dataio
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.configuration_tool import generate_histogram_configuration_list
from icecube.production_histograms.configuration_tool import generate_i3filelist
from icecube.production_histograms.generate_collection_name import generate_collection_name
from icecube.production_histograms.db import create_simprod_db_client

if not os.path.exists(options.path):
    icetray.logging.log_fatal('%s is an invalid path.' % options.path)
    
if not options.gcd_file:
    icetray.logging.log_fatal('You must specify a GCD file.')

client = create_simprod_db_client(password_path=options.password_path)

collection_name = options.collection_name \
                  if options.collection_name \
                     else generate_collection_name(options.path)

filelist = generate_i3filelist(options.path)

# runs over the files to get a sense of what could be histogrammed.
# this will also identify corrupt files, returning only the list
# of good I3Files.
histograms, good_filelist, corrupt_filelist = generate_histogram_configuration_list(filelist)

icetray.logging.log_info("Collection Name = %s" % collection_name)
icetray.logging.log_info("len(good_filelist) = %d" % len(good_filelist))
icetray.logging.log_info("len(corrupt_filelist) = %d" % len(corrupt_filelist))
icetray.logging.log_info("len(histograms) = %d" % len(histograms))

if len(histograms) == 0:
    icetray.logging.log_fatal("Found nothing to histogram.")

tray = I3Tray()
    
tray.Add("I3Reader", FilenameList = good_filelist)

tray.Add(ProductionHistogramModule, 
         Histograms = histograms,
         CollectionName = collection_name,
         FilenameList = good_filelist
        )

tray.Execute()


