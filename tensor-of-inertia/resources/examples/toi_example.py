#!/usr/bin/env python

'''
This script runs I3TensorOfInertia on the input I3File and produces an output
I3File with the result added to the frame.  This example assumes the input file
has no G, C, or D frames, so a GCD must be supplied separately.
'''

import os
I3_TESTDATA=os.environ['I3_TESTDATA']

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-i","--input_file",
                  dest="INPUTFILE",
                  default=os.path.join(I3_TESTDATA,"sim",
                                       "Level2_IC86.2011_corsika.010281.001664.00.i3.bz2"),
                  help="Name of input I3File.")

parser.add_option("-g","--gcd_file",
                  dest="GCDFILE",
                  default=os.path.join(I3_TESTDATA,"GCD",
                                       "GeoCalibDetectorStatus_2012.56063_V0.i3.gz"),
                  help="Name of input GCD file.")

parser.add_option("-o","--output_file",
                  dest="OUTPUTFILE",
                  default="toi_example.i3",
                  help="Name of output I3File.")

parser.add_option("-n","--nframes",
                  dest="NFRAMES",
                  type="int", 
                  help="Number of frames to process")

(options, args) = parser.parse_args()

from I3Tray import I3Tray
from os.path import expandvars

from icecube import icetray, dataclasses, dataio, tensor_of_inertia, phys_services

tray = I3Tray()

tray.AddModule("I3Reader", FileNameList = [options.GCDFILE, options.INPUTFILE] )

tray.AddModule("I3NullSplitter")

tray.AddModule("I3TensorOfInertia",
    AmplitudeOption = 0,
    InputReadout = "OfflinePulses")

tray.AddModule("I3Writer", FileName = options.OUTPUTFILE )

if options.NFRAMES :    
    tray.Execute(options.NFRAMES)
else:
    tray.Execute()
    

