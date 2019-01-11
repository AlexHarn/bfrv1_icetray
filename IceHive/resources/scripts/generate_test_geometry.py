#!/usr/bin/env python
 
#Generate a test geometry
# USAGE: python [this_script.py] [outfile]

import sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import IceHive

tray = I3Tray()

tray.AddModule("I3GeoDeliver", "I3GeoDeliver")
tray.AddModule("I3Writer","writer",
  Filename = sys.argv[1],
  Streams = [icetray.I3Frame.Geometry,])


tray.Execute()
