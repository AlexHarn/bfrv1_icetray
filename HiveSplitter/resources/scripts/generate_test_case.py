#!/usr/bin/env python
 
# This script runs the Hive-Splitter on some data in order to generate a testcase:
# Some HouseCleaning is done to reduce the the frame-content to the minimal required content in order to run
# USAGE: python [this_script.py] [infile] [outfile]

import sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import HiveSplitter

tray = I3Tray()

tray.AddModule("I3Reader","reader",
	FilenameList = sys.argv[1:-1])

tray.AddModule("Keep","HouseCleaning",
	Keys = ["I3Geometry",
		"I3Calibration",
		"I3DetectorStatus",
		"I3EventHeader",
		"OfflinePulses",
		"I3MCTree"])

tray.AddModule("I3HiveSplitter","OriginalHiveSplitter",
	InputName="OfflinePulses",
	OutputName="MaskedOfflinePulses",
	Multiplicity=4,
	TimeWindow=2000*I3Units.ns,
	TimeConeMinus=1000*I3Units.ns,
	TimeConePlus=1000*I3Units.ns,
	SingleDenseRingLimits=[300., 300., 272.7, 272.7, 165.8, 165.8], #I3Units.m
	DoubleDenseRingLimits=[150., 150., 131.5, 131.5, 40.8, 40.8], #I3Units.m
	TrippleDenseRingLimits=[150., 150.0, 144.1, 144.1, 124.7, 124.7, 82.8, 82.8], #I3Units.m
	SaveSplitCount=True)

tray.AddModule("I3Writer","writer",
	Filename = sys.argv[-1],
	Streams = [icetray.I3Frame.TrayInfo,
		   icetray.I3Frame.Geometry,
		   icetray.I3Frame.DAQ,
		   icetray.I3Frame.Physics])


tray.Execute()
