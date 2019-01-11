#!/usr/bin/env python

from I3Tray import *
from I3Units import *
from glob import glob
from os.path import expandvars


import os

outputfile  = "EHE-Level2Events.i3.gz"

if len(sys.argv) > 1:
	outputfile = sys.argv[1]

load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libpfclasses")
load("libjuliet-interface")
load("libromeo-interface")
load("libportia")
load("libophelia")


tray = I3Tray()

file_list = glob("/disk1/data/IceCube/RealData/2006/level2-ehe/EHEevents*.i3.gz")


tray.AddModule("I3Reader", "reader", FileNameList=file_list)

tray.AddModule("I3Writer","writer")(
#	("SkipKeys", ["CleanInIceRawData","CleanIceTopRawData"]),
	("compressionlevel",9),
        ("filename", outputfile)
)

tray.AddModule("Dump","dump")


tray.Execute();


