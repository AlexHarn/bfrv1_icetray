#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube.dipolefit.test_modules.DFTestIC23 import DFTestIC23
from icecube import dataio
from icecube import dataclasses
from icecube import dipolefit

## Test6: Compare all the variables from the new fit to what was expected 
## for this one event.
  
tray = I3Tray()

tray.AddModule("I3Reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_2.i3.gz'
	)

tray.AddModule("I3DipoleFit",
    InputRecoPulses = "Pulses")

tray.AddModule(DFTestIC23)

tray.Execute()

