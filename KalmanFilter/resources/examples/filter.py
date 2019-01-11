#!/usr/bin/env python
# -*- coding: utf-8 -*-

from I3Tray import *
from icecube import icetray, dataio, dataclasses
from optparse import OptionParser

from icecube.KalmanFilter import MyKalman, MyKalmanSeed


usage = "usage: %prog [options]"
parser = OptionParser(usage)

parser.add_option("-i", "--infile", dest="INFILE",
    help="Read input to INFILE (.i3{.gz} format)")
    
parser.add_option("-o", "--outfile", dest="OUTFILE",
    help="Write output to OUTFILE (.i3{.gz} format)")
    
parser.add_option("-g", "--gcdfile", dest="GCD",
		   help="Read in GCD file")


(options,args) = parser.parse_args()

mandatories = ['INFILE', 'OUTFILE', 'GCD']
for m in mandatories:
    if not options.__dict__[m]:
        print("mandatory option is missing\n")
        parser.print_help()
        exit(-1)

tray = I3Tray()

tray.AddModule("I3Reader","reader")(
    ("FilenameList",[options.GCD, options.INFILE]),
    )

tray.AddModule(MyKalmanSeed, "Seed",
    InputMapName="SLOPPulseMaskTuples",
    OutputTrack="SLOPTuples_LineFit",
    )

tray.AddModule(MyKalman, "MyKalman",
    InputTrack = "SLOPTuples_LineFit",
    OutputTrack = "MyKalman",
    InputMapName = "SLOPPulseMaskMPClean",
    CutRadius = 200,
    Iterations = 3,
    NoiseQ = 1e-11,
    NoiseR = 60**2,
    )

tray.AddModule("I3Writer","writer",
    Filename = options.OUTFILE,
    )

tray.AddModule("Dump", "dump")



tray.Execute()


