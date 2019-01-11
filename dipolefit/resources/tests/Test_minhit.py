#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube.dipolefit.test_modules.DFTestMinHit import DFTestMinHit
from icecube import dataio
from icecube import dataclasses
from icecube import dipolefit

## Test 2: fail on too few hits.

tray = I3Tray()

tray.AddModule("I3Reader", 
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.AddModule("I3DipoleFit",
                InputRecoPulses = "Pulses", 
                MinHits = 999999)

tray.AddModule(DFTestMinHit)

tray.Execute()

