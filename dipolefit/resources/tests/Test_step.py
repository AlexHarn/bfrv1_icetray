#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube.dipolefit.test_modules.DFTest import DFTest
from icecube import dataio
from icecube import dataclasses
from icecube import dipolefit

tray = I3Tray()

def function(frame):
    if frame['DipoleFit'].fit_status == dataclasses.I3Particle.InsufficientHits:
        print('Caught Failure as expected')
    else:
        sys.exit(1)

tray.AddModule("I3Reader", 
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.AddModule("I3DipoleFit",
                InputRecoPulses = "Pulses", 
                DipoleStep=9999999)

#By setting a large step size hit cannot be compared so fit_status is set to InsufficientHits

tray.Add(function)

tray.Execute()

