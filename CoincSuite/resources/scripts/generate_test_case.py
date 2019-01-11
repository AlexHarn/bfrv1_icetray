#!/usr/bin/env python
 
# This script runs the CoincSuite on some data in order to generate a testcase:
# Some HouseCleaning is done to reduce the frame-content to the minimal required objects in order to execute
# USAGE: python [this_script.py] [infile] [outfile]

import sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import CoincSuite

tray = I3Tray()

tray.AddModule("I3Reader","reader",
        FilenameList = sys.argv[1:-1])

tray.AddModule("Keep","HouseCleaning",
        Keys = ["I3Geometry",
                "I3Calibration",
                "I3DetectorStatus",
                "I3EventHeader",
                "OfflinePulses",
                "MaskedOfflinePulses",
                "I3MCTree"])

tray.AddSegment(CoincSuite.Complete, "",
  SplitName = "split",
  SplitPulses = "MaskedOfflinePulses")

tray.AddModule("I3Writer","writer",
        Filename = sys.argv[-1],
        Streams = [icetray.I3Frame.TrayInfo,
                   icetray.I3Frame.Geometry,
                   icetray.I3Frame.DAQ,
                   icetray.I3Frame.Physics])


tray.Execute()

