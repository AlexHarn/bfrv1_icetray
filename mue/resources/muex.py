#!/usr/bin/env python

import os
import sys

if(os.getenv("I3_BUILD") == None):
    print("I3_BUILD not set.")
    sys.exit()

from I3Tray import *
from os.path import expandvars

# This GCD file isn't quite a match for the data file, but is close enough for demonstration purposes
gcdfile = expandvars("$I3_TESTDATA/exp/IceCube/2016/Level2_IC86.2015_data_Run00127797_21_242_GCD.i3.gz")
infile = expandvars("$I3_TESTDATA/dataio/olddata/IC86-2015/Level2_IC86.2015_data_Run00126448_Subrun00000000.i3.bz2")

load("libdataio")

tray = I3Tray()

tray.AddModule("I3Reader", "reader")(
    ("FileNameList", [gcdfile, infile]),
    )

load("libmue")
tray.AddModule("muex", "muex")(
    ("pulses", "SRTInIcePulses"),
    ("rectrk", ""), # have muex do its own directional reconstruction
    ("result", "MuEx"),
    ("lcspan", 0),
    ("repeat", 16), # iterate reconstruction 16 times with bootstrapping
    ("rectyp", True), # use a muon track hypothesis
    ("usempe", True), # use the MPE likelihood (leading edge time + total charge)
    ("detail", True), # unfold energy losses
    ("energy", True), # compute an energy estimate
    ("icedir", expandvars("$I3_BUILD/mue/resources/ice/mie"))
    )

tray.AddModule("I3Writer", "writer")(
    ("filename", "muex_outfile.i3"),
    ("Streams",[icetray.I3Frame.TrayInfo,icetray.I3Frame.DAQ,icetray.I3Frame.Physics])
    )

tray.Execute(20)
