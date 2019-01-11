#!/usr/bin/env python

# This script demonstrates running HiveSplitter to split a
# RecoPulseSeries (located in the Q frame) into separate pulse series
# for each subevent, each in its own P frame.
# USAGE: python [this_script.py] [infile] [outfile]

import os,sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import HiveSplitter

icetray.set_log_level(icetray.I3LogLevel.LOG_DEBUG)
tray = I3Tray()

print ("Output will be written to "+os.path.join(os.path.expandvars("$I3_BUILD")+"/HiveSplitter/resources","example.i3.bz2"))

i3_testdata = os.path.expandvars("$I3_TESTDATA")

tray.AddModule("I3Reader","Reader", 
	FilenameList= [os.path.join(i3_testdata+"/IceHive","example_scenario.i3.bz2")])

#tray.AddModule("Dump", "dump")

# use 300m spheres in IceCube and 150m spheres in the denser DeepCore fidutial region: settings reproduce MaxDist in std-settings
tray.AddModule("I3HiveSplitter","HiveSplit",
	InputName="OfflinePulses",
	OutputName="MaskedOfflinePulses",
	Multiplicity=4,
	TimeWindow=2000*I3Units.ns,
	TimeConeMinus=1000*I3Units.ns,
	TimeConePlus=1000*I3Units.ns,
	SingleDenseRingLimits=[300., 300., 272.7, 272.7, 165.8, 165.8], #I3Units.m
	DoubleDenseRingLimits=[150., 150., 131.5, 131.5, 40.8, 40.8], #I3Units.m
	TrippleDenseRingLimits=[150., 150., 144.1, 144.1, 124.7, 124.7, 82.8, 82.8], #I3Units.m
	Mode = 1,
	NoSplitMode =False,
	SaveSplitCount=True)

tray.AddModule("I3Writer","Writer",
	Filename=os.path.join(os.path.expandvars("$I3_BUILD")+"/HiveSplitter/resources","example.i3.bz2"))
	#Streams=[icetray.I3Frame.Physics, icetray.I3Frame.DAQ])
	

tray.Execute()

