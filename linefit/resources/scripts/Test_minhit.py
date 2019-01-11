#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube.linefit.test_modules.LFTestMinHit import LFTestMinHit

from icecube import dataio

load("libdataclasses")
load("liblinefit")
load("libdataio")

## Test 2: fail on too few hits.

tray = I3Tray()

tray.AddModule("I3Reader", "reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.AddModule("I3LineFit","linefit")(
    ("InputRecoPulses", "Pulses"),
    ("MinHits",       999999))

tray.AddModule(LFTestMinHit,"test")


    
tray.Execute()

