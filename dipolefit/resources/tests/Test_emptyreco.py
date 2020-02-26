#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube import dipolefit, recclasses, dataclasses
from icecube.dipolefit.test_modules.DFTest import DFTest

## Test 1: general test, read events, reconstruct.

def function(frame):
    del frame['Pulses']

def check(frame):
    assert(frame['DipoleFitParams'] == recclasses.I3DipoleFitParams())


tray = I3Tray()

tray.AddModule("I3Reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.Add(function)

tray.AddModule("I3DipoleFit", InputRecoPulses = "Pulses")

tray.AddModule(DFTest)

tray.Add(check)

tray.Execute()

