#!/usr/bin/env python

from optparse import OptionParser

parser = OptionParser()

parser.add_option("-i","--infiles",
                  dest = "INFILES",
                  help = "Regex to pass to glob that specify the input files.")
parser.add_option("-g","--gcdfile",
                  dest = "GCDFN",
                  help = "Input GCD file.")
parser.add_option("-o","--output-filename",
                  dest = "OUTPUT_FN",
                  default = "output.pkl",
                  help = "Filename of the output pickle file.")
(options, args) = parser.parse_args()

import glob, os

filelist = [options.GCDFN]
filelist.extend(glob.glob(options.INFILES))

from I3Tray import I3Tray
from icecube import icetray, dataio
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.histogram_modules.simulation.dom_mainboard_response import InIceResponseModule, IceTopResponseModule
from icecube.production_histograms.histogram_modules.simulation.trigger import TriggerModule
from icecube.production_histograms.histograms.filtering.particles import l1_particle_histograms
from icecube.production_histograms.histograms.L2.particles import l2_particle_histograms

tray = I3Tray()

tray.Add("I3Reader", FilenameList = filelist)
tray.Add(ProductionHistogramModule, 
         Histograms = [InIceResponseModule,
                       IceTopResponseModule,
                       TriggerModule,
                       l1_particle_histograms,
                       l2_particle_histograms],         
         OutputFilename = options.OUTPUT_FN
        )
tray.Execute()

