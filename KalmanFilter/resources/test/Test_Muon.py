#!/usr/bin/env python
# -*- coding: utf-8 -*-

from I3Tray import *
from icecube import icetray, dataio
from icecube.KalmanFilter import MyKalman, MyKalmanSeed

import math

def ensure_distance(a, b, eps, message="Test FAILED"):
    if math.fabs(a-b) > eps:
        raise Exception(message)


tray = I3Tray()
    
tray.AddModule("I3Reader", "reader",
    FileName = os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
    )

tray.AddModule(MyKalmanSeed, "Seed",
    InputMapName = "Pulses",
    OutputTrack = "Pulses_LineFit",
    )

tray.AddModule(MyKalman, "MyKalman",
    InputTrack = "Pulses_LineFit",
    OutputTrack = "MyKalman",
    InputMapName = "Pulses",
    CutRadius = 100,
    Iterations = 3,
    NoiseQ = 1e-5,
    NoiseR = 30**2,
    IterationMethod = 2,
    )

def TestKalman(frame):
    track = frame['MyKalman_LineFit']
    ensure_distance(track.dir.zenith, 160.301*I3Units.deg, 0.001, "Zenith failed")
    ensure_distance(track.dir.azimuth, 143.197*I3Units.deg, 0.001, "Azimuth failed")
    ensure_distance(track.pos.x, -164.983, 0.001, "X failed")
    ensure_distance(track.pos.y, 84.001, 0.001, "Y failed")
    ensure_distance(track.pos.z, -391.086, 0.001, "Z failed")
    ensure_distance(track.speed, 0.25015, 0.0001, "Speed failed")
    ensure_distance(frame['MyKalman_NHits'].value, 53., 0.001, "NHits failed")
    ensure_distance(frame['MyKalman_Chi2Pred'].value, 73.564, 0.001, "Chi^2 failed")


tray.AddModule(TestKalman, "Test")



tray.Execute()

