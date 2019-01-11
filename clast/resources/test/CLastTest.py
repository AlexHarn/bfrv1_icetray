#!/usr/bin/env python
from icecube import clast
from I3Tray import I3Tray
from os.path import expandvars

workspace = expandvars("$I3_BUILD")
i3testdata = expandvars("$I3_TESTDATA")
dataf = i3testdata + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"

tray = I3Tray()

tray.AddModule("I3Reader","reader")(
    ("Filename", dataf)
    )

tray.AddModule("I3CLastModule","clast")(
    ("InputReadout","MaskedOfflinePulses")
    )

outf = workspace + "/clast/ClastTest.i3"
tray.AddModule("I3Writer","writer")(
    ("filename", outf )
    )


tray.Execute(10+3)


import os
os.unlink(outf)
