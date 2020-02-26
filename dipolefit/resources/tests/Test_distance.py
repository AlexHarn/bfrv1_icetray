#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube import dipolefit,dataclasses,icetray
from icecube.dipolefit.test_modules.DFTest import DFTest
import numpy as np

class updateGeometry(icetray.I3Module):

    def __init__(self, context):
        icetray.I3Module.__init__(self, context)

    def Geometry(self,frame):
        Omgeo = frame['I3Geometry'].omgeo
        for i in Omgeo.keys():
            Omgeo[i].position = dataclasses.I3Position(1.,1.,1.)
            
        frame['I3Geometry'].omgeo.update(Omgeo)
        self.PushFrame(frame)


def check(frame):
    assert(frame['DipoleFit'].fit_status == dataclasses.I3Particle.FailedToConverge)
        
tray = I3Tray()

tray.AddModule("I3Reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.Add(updateGeometry)

tray.AddModule("I3DipoleFit", InputRecoPulses = "Pulses")

tray.Add(check)

tray.Execute()

