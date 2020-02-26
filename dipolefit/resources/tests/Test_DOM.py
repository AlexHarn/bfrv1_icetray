#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube import dipolefit,dataclasses,icetray
from icecube.dipolefit.test_modules.DFTest import DFTest
import numpy as np

class updateGeometry(icetray.I3Module):

    def __init__(self, context):
        icetray.I3Module.__init__(self, context)

    def Physics(self,frame):
        count = 0
        for i in frame['Pulses'].keys():
            if count == 0:
                frame['Pulses'][icetray.OMKey(1,1,0)] = frame['Pulses'][i]
            else:
                continue
            count+=1
        self.PushFrame(frame)

def check(frame):
    NotPresent = False
    for i in frame['Pulses'].keys():
        for j in frame['I3Geometry'].omgeo.keys():
            if i == j:
                continue
            else:
                NotPresent = True
    assert(NotPresent == True)


tray = I3Tray()

tray.AddModule("I3Reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.Add(updateGeometry)

tray.AddModule("I3DipoleFit", InputRecoPulses = "Pulses")

tray.Add(check)

tray.Execute()

