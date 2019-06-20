#!/usr/bin/env python
#
#
from I3Tray import *

from os.path import expandvars

import os
import sys

from icecube import dataclasses 
from icecube import phys_services 
from icecube import dataio 
from icecube import icepick 
from icecube import icetray 

workspace = expandvars("$I3_BUILD")
testdata = expandvars("$I3_TESTDATA")

infile = testdata + "/2006data/Run00087451.i3.gz"
outfile = workspace + "/icepick/prescale10.i3"

#######################################################################

## creating an instance of icetray
tray = I3Tray()

# The muxer is a sort of black box which makes frames.
# You just hope that it does what you think it should do.
tray.AddModule("I3Reader", Filename=infile, SkipKeys=["I3PfFilterMask"])

# This module, in this example, will keep only every 10th event
tray.AddModule("I3IcePickModule<I3PrescaleFilter>",
    DiscardEvents=True,
    PrescaleFactor=10
)

tray.AddModule("I3Writer", filename=outfile)    
tray.Execute()

import os
os.unlink(outfile)

