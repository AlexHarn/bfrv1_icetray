#!/usr/bin/env python

from I3Tray import I3Tray

from os.path import expandvars
from icecube import dataio
from icecube import dataclasses
from icecube import dipolefit

import os
import sys

workspace = expandvars("$I3_BUILD")
gcdfile = expandvars("$I3_TESTDATA") + "/GCD/GeoCalibDetectorStatus_2012.56063_V0.i3.gz"
dataf = expandvars("$I3_TESTDATA") + "/sim/Level2_IC86.2011_corsika.010281.001664.00.i3.bz2"

tray = I3Tray()

tray.AddModule("I3Reader", FilenameList = [gcdfile, dataf])    

tray.AddModule("I3DipoleFit",
    DipoleStep = 0,
    InputRecoPulses = "InIceRecoPulseSeries")

tray.AddModule("Dump")

outf = workspace + "/dipolefit/dipolefit_rawdata.i3"
tray.AddModule("I3Writer",
    filename = outf)

tray.Execute(10)

