#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube.linefit.test_modules.LFTest import LFTest

from icecube import dataio

load("libdataclasses")
load("liblinefit")
load("libdataio")

## Test 4: Change AmpWeightPower.

tray = I3Tray()

tray.AddModule("I3Reader", "reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.AddModule("I3LineFit","linefit")(
    ("InputRecoPulses", "Pulses"),
    ("AmpWeightPower",0.7))

tray.AddModule(LFTest,"test")


    
tray.Execute()

