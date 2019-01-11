#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube import dipolefit
from icecube.dipolefit.test_modules.DFTest import DFTest

## Test 1: general test, read events, reconstruct.

tray = I3Tray()

tray.AddModule("I3Reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.AddModule("I3DipoleFit", InputRecoPulses = "Pulses")

tray.AddModule(DFTest)

tray.Execute()

