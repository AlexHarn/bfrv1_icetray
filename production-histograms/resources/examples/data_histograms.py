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
from icecube import icetray
from icecube import dataio
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.configuration_tool import generate_histogram_configuration_list

histograms, good_filelist, corrupt_filelist  = generate_histogram_configuration_list(filelist)

tray = I3Tray()

def frame_select(frame):
    '''
    I don't want any frames that pass the moon or sun filter.
    '''
    if 'QFilterMask' in frame and\
       (frame['QFilterMask']['MoonFilter_13'].condition_passed or\
        frame['QFilterMask']['SunFilter_13'].condition_passed):
        return False
    return True

tray.Add("I3Reader", FilenameList = filelist)
tray.Add(ProductionHistogramModule, 
         Histograms = histograms,
         If = frame_select,
         OutputFilename = options.OUTPUT_FN
        )
tray.Execute()

