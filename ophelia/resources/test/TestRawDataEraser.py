#!/usr/bin/env python

from I3Tray import *
from math import *
from os.path import expandvars

import os

workspace  = expandvars("$I3_BUILD")
i3testdata = expandvars("$I3_TESTDATA")

infile = i3testdata + "/2007data/2007_I3Only_Run109732_Nch20.i3.gz"
outphysfile  = workspace + "/TestRawDataEraser.i3.gz"

from icecube import icetray
from icecube import dataclasses
from icecube import phys_services
from icecube import dataio
from icecube import ophelia
from icecube import DomTools

tray = I3Tray()

tray.AddModule("I3Reader","reader")(
	("Filename", infile),
)

tray.AddModule("I3DOMLaunchCleaning","cleaner")(
	("FirstLaunchCleaning", True),
	)

tray.AddModule("I3OpheliaRawDataEraser","eraser")(
	("DataReadoutName", "CleanInIceRawData"),
	)

#tray.AddModule("I3Writer","writer")(
#	("filename", outphysfile),
#        ("SkipKeys", ["InIceRawData", "IceTopRawData"]),
#	("streams",["Physics"])
#	)



tray.Execute(10)

