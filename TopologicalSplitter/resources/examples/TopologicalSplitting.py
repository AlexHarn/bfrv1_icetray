#!/usr/bin/env python

# This script demonstrates running I3TopologicalSplitter to split a 
# pulse series (located in the Q frame) into separate pulse series 
# for each subevent, each in its own P frame. 

import sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import TopologicalSplitter

tray = I3Tray()

tray.AddModule("I3Reader","Reader", 
	FilenameList=sys.argv[1:-1])

# The settings shown here are the same as those chosen by 
# S. Odrowski for the IC79 Point Source Analysis
tray.AddModule("I3TopologicalSplitter","TopologicalSplit",
	InputName="OfflinePulses",
	OutputName="SplitPulses",
	Multiplicity=4,
	TimeWindow=4000*I3Units.ns,
	XYDist=300*I3Units.m,
	ZDomDist=15,
	TimeCone=1000*I3Units.ns,
	SaveSplitCount=True)

tray.AddModule("I3Writer","Writer",
	Filename=sys.argv[-1])
	

tray.Execute()

