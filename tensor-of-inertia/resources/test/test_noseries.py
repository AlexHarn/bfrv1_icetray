#!/usr/bin/env python

import os.path
from I3Tray import I3Tray
from icecube import dataclasses,tensor_of_inertia

def function(frame):
    del frame['OfflinePulses']
    frame['OfflinePulses'] = None

def check(frame):
    assert(frame['ti'].fit_status == dataclasses.I3Particle.GeneralFailure)

tray = I3Tray()

I3_TESTDATA=os.environ['I3_TESTDATA']
gcd_file = os.path.join(I3_TESTDATA,"GCD","GeoCalibDetectorStatus_2012.56063_V0.i3.gz")
input_file = os.path.join(I3_TESTDATA,"sim","Level2_IC86.2011_corsika.010281.001664.00.i3.bz2")
        
tray.AddModule("I3Reader", FileNameList = [gcd_file,input_file] )

tray.Add(function)

tray.AddModule("I3TensorOfInertia",
               InputReadout = "OfflinePulses")

tray.Add(check)

tray.Execute(5)




