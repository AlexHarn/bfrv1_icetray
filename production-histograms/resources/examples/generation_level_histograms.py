#!/usr/bin/env python

from os.path import expandvars

DEFAULT_GCD = expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_2013.56429_V1.i3.gz")

from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument("infile",
                    help = "Regex to pass to glob that specify the input files.")                                    
parser.add_argument("outputfn",
                  default = "output.pkl",
                  help = "Filename of the output pickle file.")
parser.add_argument("-g","--gcdfile",
                  dest = "GCDFN",
                  default = DEFAULT_GCD,
                  help = "Input GCD file.")
args = parser.parse_args()

filelist = [args.GCDFN, args.infile]

from I3Tray import I3Tray
from icecube import icetray, dataio
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.histogram_modules.simulation.mctree_primary import I3MCTreePrimaryModule
from icecube.production_histograms.histogram_modules.simulation.mctree import I3MCTreeModule
from icecube.production_histograms.histograms.simulation.secondary_multiplicity import SecondaryMultiplicity

tray = I3Tray()

tray.Add("I3Reader", FilenameList = filelist)
tray.Add(ProductionHistogramModule, 
         Histograms = [I3MCTreePrimaryModule,
                       I3MCTreeModule,
                       SecondaryMultiplicity],
         OutputFilename = args.outputfn
        )
tray.Execute()

