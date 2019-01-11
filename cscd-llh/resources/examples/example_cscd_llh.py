#!/usr/bin/env python

from I3Tray import *

from os.path import expandvars

from icecube import dataio

import os
import sys

load("libdataclasses")
load("libdataio")
load("libphys-services")
load("libcscd-llh")
load("libclast")

workspace = expandvars("$I3_BUILD")
i3testdata = expandvars("$I3_TESTDATA")
dataf = i3testdata + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"

tray = I3Tray()

tray.AddModule("I3Reader","reader")(
    ("Filename", dataf)
    )

tray.AddModule("I3CLastModule","cfirst")(
  ("InputReadout","MaskedOfflinePulses"),
  ("Name","CFirst")
  )


tray.AddModule("I3CscdLlhModule","cscd-llh")(
  ("InputType","RecoPulse"),
  ("RecoSeries","MaskedOfflinePulses"),
  ("SeedKey","CFirst"),
  ("ResultName","Vertex_Reco"),
  ("Minimizer","Powell"),
  ("PDF","UPandel"),
  ("EnergySeed",2),
  ("ParamT","0.0, 0.0, 0.0, false"),
  ("ParamX","0.0, 0.0, 0.0, false"),
  ("ParamY","0.0, 0.0, 0.0, false"),
  ("ParamZ","0.0, 0.0, 0.0, false")
  )

outf = workspace + "/cscd-llh-test.i3"
tray.AddModule("I3Writer","writer")(
    ("filename", outf )
    )


tray.Execute(5+3)

	
