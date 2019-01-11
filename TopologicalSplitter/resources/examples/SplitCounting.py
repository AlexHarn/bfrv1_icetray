#!/usr/bin/env python

# This script demonstrates running I3TopologicalSplitter for the sole purpose of
# counting the number of subevents generated. Persons considering using this as
# a cut for physics analysis should be aware that it behaves poorly on long 
# triggers, such as Slow Particle and Fixed Rate triggers, which are likely to 
# contain multiple physics subevents simply due to their long readout times. 

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
	
# Destroy the P frames which were produced
tray.AddModule(lambda frame: frame["I3EventHeader"].sub_event_stream!="TopologicalSplit")

# The split count will remain in the Q frame (from which it is also visible in 
# any remaining P frames) under the name "TopologicalSplitSplitCount"

tray.AddModule("I3Writer","Writer",
	Filename=sys.argv[-1])
	

tray.Execute()

