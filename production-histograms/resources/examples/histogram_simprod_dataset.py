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
from icecube.production_histograms.generate_collection_name import generate_collection_name
from icecube.production_histograms.db import create_simprod_db_client

# if the credentials path is not $HOME/.mongo then you need
# to pass the path to create_simprod_db_client
# e.g. create_simprod_db_client(password_path='/home/fizzycist/please-hacker-dont-read-me.txt')
client = create_simprod_db_client()
collection_name = generate_collection_name(options.PATH)
filelist = generate_filelist(options.PATH)
histograms = generate_histogram_configuration_list(filelist)

icetray.logging.log_info("Collection Name = %s" % collection_name)
icetray.logging.log_info("len(filelist) = %d" % len(filelist))
icetray.logging.log_info("len(histograms) = %d" % len(histograms))

if len(histograms) == 0:
    icetray.logging.log_fatal("Found nothing to histogram.")

tray = I3Tray()
    
tray.Add("I3Reader", FilenameList = filelist[:2])

tray.Add(ProductionHistogramModule, 
         Histograms = histograms,
         CollectionName = collection_name,
         FilenameList = filelist
        )

tray.Execute()


