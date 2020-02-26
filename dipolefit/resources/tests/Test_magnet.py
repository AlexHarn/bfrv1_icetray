#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube import dipolefit,dataclasses,icetray
from icecube.dipolefit.test_modules.DFTest import DFTest
import numpy as np

class updatePulses(icetray.I3Module):

    def __init__(self, context):
        icetray.I3Module.__init__(self, context)

    def Physics(self,frame):
        for i in frame['Pulses'].keys():
            frame['Pulses'][i][0].charge = 0
        self.PushFrame(frame)


def check(frame):
    assert(frame['DipoleFit'].fit_status == dataclasses.I3Particle.FailedToConverge)
        
tray = I3Tray()

tray.AddModule("I3Reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.Add(updatePulses)

tray.AddModule("I3DipoleFit", InputRecoPulses = "Pulses",MinHits=2,AmpWeightPower=1)

tray.Add(check)

tray.Execute()

