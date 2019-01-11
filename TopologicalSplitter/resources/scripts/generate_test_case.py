#!/usr/bin/env python

# Note: This is not an example showing how to use TopologicalSplitter!
#       If that is what you want, see: 
#       TopologicalSplitter/resources/examples/TopologicalSplitting.py
# 
# This script runs the original ttrigger splitter on some data in order to 
# generate a testcase for the new splitter. Note that it relies on both the 
# SeededRTCleaning and ttrigger projects to do this. 

import sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
load("SeededRTCleaning")
load("ttrigger")

tray = I3Tray()

tray.AddModule("I3Reader","reader")(
	("FilenameList",sys.argv[1:-1])
	)

tray.AddModule("Keep","HouseCleaning")(
	("Keys",["I3Geometry","I3Calibration","I3DetectorStatus","DrivingTime",
	"I3EventHeader","OfflinePulses"])
	)

rawPulses = "OfflinePulses"
pulses = "SRTOfflinePulses"
tray.AddModule("I3SeededRTHitMaskingModule","SRTClean")(
	("InputResponse",rawPulses),
	("OutputResponse",pulses),
	("Stream",icetray.I3Frame.DAQ)
	)

XYDist=300
ZDomDist=20
TimeCone=800
tray.AddModule("TTriggerSplitter","OriginalTTrigger")(
	("Topo", 1),
	("Multiplicity", 4),
	("TimeWindow", 4000*I3Units.ns),
	("XYDist", XYDist*I3Units.m),
	("ZDomDist", ZDomDist),
	("TimeCone", TimeCone*I3Units.ns),
	("CBWindow", 0*I3Units.ns),
	("LCSpan", 2),
	("LCWindow", 0*I3Units.ns),
	("OutputName", "TTPulses"),
	("InputName", pulses)
	)

tray.AddModule("I3Writer","i3writer")(
	("Filename",sys.argv[-1]),
	("Streams",[icetray.I3Frame.TrayInfo,icetray.I3Frame.Geometry,
	            icetray.I3Frame.DAQ,icetray.I3Frame.Physics])
	)


tray.Execute(3+200) # guess this many frames to get GCD + 100 QP pairs
